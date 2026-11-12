#include <Arduino.h>
#include <Wire.h>
#include <BleMouse.h>

const char* BLE_NAME = "AirMouse";
const char* BLE_MAKER = "Raaj";

const int PIN_JOY_X = 1;
const int PIN_JOY_Y = 2;
const int PIN_JOY_SW = 7;
const int PIN_BTN_RIGHT = 8;
const int PIN_BTN_CLUTCH = 9;
const int PIN_SDA = 5;
const int PIN_SCL = 6;
const int PIN_LED = 43;
const int PIN_VBAT = 4;

const uint8_t MPU_ADDR = 0x68;
const int GYRO_AXIS_FOR_X = 2;
const int GYRO_AXIS_FOR_Y = 0;
const float GYRO_X_SIGN = 1.0f;
const float GYRO_Y_SIGN = 1.0f;
const float GYRO_LSB_PER_DPS = 65.5f;
const float CURSOR_SENSITIVITY = 25.0f;
const float GYRO_DEADZONE_DPS = 1.5f;
const float SMOOTHING = 0.35f;
const int CALIBRATION_SAMPLES = 500;
const int CALIBRATION_MAX_RANGE = 250;
const unsigned long REPORT_INTERVAL_MS = 10;

const int JOY_DEADZONE = 250;
const int JOY_HALF_RANGE = 1800;
const int JOY_CENTER_SAMPLES = 64;
const float SCROLL_MAX_NOTCHES_PER_SEC = 14.0f;
const float SCROLL_V_SIGN = 1.0f;
const float SCROLL_H_SIGN = 1.0f;

const unsigned long DEBOUNCE_MS = 20;
const unsigned long AUTO_OFF_MS = 60000;
const float ACTIVITY_DPS = 8.0f;

const float DIVIDER_RATIO = 2.0f;
const unsigned long BATTERY_INTERVAL_MS = 10000;
const int LED_BRIGHTNESS = 24;

const int VBAT_TABLE_MV[] = {3300, 3500, 3600, 3700, 3800, 3900, 4000, 4100, 4200};
const int VBAT_TABLE_PCT[] = {0, 10, 20, 35, 50, 65, 80, 90, 100};
const int VBAT_TABLE_SIZE = 9;

BleMouse bleMouse(BLE_NAME, BLE_MAKER, 100);

struct Button {
  int pin;
  bool stable;
  bool lastRaw;
  unsigned long changedAt;
  bool pressedEdge;
  bool releasedEdge;
};

Button btnJoy = {PIN_JOY_SW, false, false, 0, false, false};
Button btnRight = {PIN_BTN_RIGHT, false, false, 0, false, false};
Button btnClutch = {PIN_BTN_CLUTCH, false, false, 0, false, false};

float gyroBias[3] = {0, 0, 0};
float filtX = 0;
float filtY = 0;
float cursorAccX = 0;
float cursorAccY = 0;
float scrollAccV = 0;
float scrollAccH = 0;
int joyCenterX = 2048;
int joyCenterY = 2048;
bool cursorActive = false;
unsigned long lastReport = 0;
unsigned long lastBattery = 0;
unsigned long lastActivity = 0;
int batteryPercent = 100;
bool batteryKnown = false;
int ledLastR = -1;
int ledLastG = -1;
int ledLastB = -1;

void ledWrite(int r, int g, int b) {
  if (r == ledLastR && g == ledLastG && b == ledLastB) return;
  ledLastR = r;
  ledLastG = g;
  ledLastB = b;
  neopixelWrite(PIN_LED, r * LED_BRIGHTNESS / 255, g * LED_BRIGHTNESS / 255, b * LED_BRIGHTNESS / 255);
}

void updateButton(Button &btn, unsigned long now) {
  btn.pressedEdge = false;
  btn.releasedEdge = false;
  bool raw = digitalRead(btn.pin) == LOW;
  if (raw != btn.lastRaw) {
    btn.lastRaw = raw;
    btn.changedAt = now;
  }
  if (now - btn.changedAt >= DEBOUNCE_MS && raw != btn.stable) {
    btn.stable = raw;
    if (raw) {
      btn.pressedEdge = true;
    } else {
      btn.releasedEdge = true;
    }
  }
}

void mpuWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void mpuInit() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);
  mpuWrite(0x6B, 0x00);
  delay(100);
  mpuWrite(0x1A, 0x03);
  mpuWrite(0x1B, 0x08);
}

bool mpuPresent() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x75);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)MPU_ADDR, 1) != 1) return false;
  uint8_t id = Wire.read();
  return id == 0x68;
}

bool readGyroRaw(int16_t *out) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)MPU_ADDR, 6) != 6) return false;
  for (int i = 0; i < 3; i++) {
    uint8_t hi = Wire.read();
    uint8_t lo = Wire.read();
    out[i] = (int16_t)((hi << 8) | lo);
  }
  return true;
}

void haltWithError() {
  while (true) {
    ledWrite(255, 0, 0);
    delay(200);
    ledWrite(0, 0, 0);
    delay(200);
  }
}

bool calibrateGyroOnce() {
  long sum[3] = {0, 0, 0};
  int mn[3] = {32767, 32767, 32767};
  int mx[3] = {-32768, -32768, -32768};
  int good = 0;
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    int16_t raw[3];
    if (readGyroRaw(raw)) {
      for (int a = 0; a < 3; a++) {
        sum[a] += raw[a];
        if (raw[a] < mn[a]) mn[a] = raw[a];
        if (raw[a] > mx[a]) mx[a] = raw[a];
      }
      good++;
    }
    delay(4);
  }
  if (good == 0) return false;
  bool still = true;
  for (int a = 0; a < 3; a++) {
    gyroBias[a] = (float)sum[a] / good;
    if (mx[a] - mn[a] > CALIBRATION_MAX_RANGE) still = false;
  }
  return still;
}

void calibrateGyro() {
  for (int attempt = 0; attempt < 5; attempt++) {
    ledWrite(255, 255, 255);
    if (calibrateGyroOnce()) return;
    ledWrite(255, 0, 0);
    delay(300);
  }
}

int readAxis(int pin) {
  long sum = 0;
  for (int i = 0; i < 4; i++) sum += analogRead(pin);
  return (int)(sum / 4);
}

void calibrateJoystick() {
  long sx = 0;
  long sy = 0;
  for (int i = 0; i < JOY_CENTER_SAMPLES; i++) {
    sx += analogRead(PIN_JOY_X);
    sy += analogRead(PIN_JOY_Y);
    delay(5);
  }
  joyCenterX = (int)(sx / JOY_CENTER_SAMPLES);
  joyCenterY = (int)(sy / JOY_CENTER_SAMPLES);
}

float axisFraction(int raw, int center) {
  int d = raw - center;
  int a = abs(d);
  if (a < JOY_DEADZONE) return 0;
  float mag = (float)(a - JOY_DEADZONE) / (float)(JOY_HALF_RANGE - JOY_DEADZONE);
  if (mag > 1.0f) mag = 1.0f;
  mag = mag * mag;
  return d < 0 ? -mag : mag;
}

int percentFromMv(int mv) {
  if (mv <= VBAT_TABLE_MV[0]) return 0;
  if (mv >= VBAT_TABLE_MV[VBAT_TABLE_SIZE - 1]) return 100;
  for (int i = 1; i < VBAT_TABLE_SIZE; i++) {
    if (mv <= VBAT_TABLE_MV[i]) {
      float t = (float)(mv - VBAT_TABLE_MV[i - 1]) / (float)(VBAT_TABLE_MV[i] - VBAT_TABLE_MV[i - 1]);
      return VBAT_TABLE_PCT[i - 1] + (int)(t * (VBAT_TABLE_PCT[i] - VBAT_TABLE_PCT[i - 1]));
    }
  }
  return 100;
}

void updateBattery(bool connected) {
  long sum = 0;
  for (int i = 0; i < 16; i++) sum += analogReadMilliVolts(PIN_VBAT);
  int mv = (int)((sum / 16.0f) * DIVIDER_RATIO);
  int pct = percentFromMv(mv);
  batteryPercent = batteryKnown ? (batteryPercent * 3 + pct) / 4 : pct;
  batteryKnown = true;
  if (connected) bleMouse.setBatteryLevel(batteryPercent);
}

void handleClutch(unsigned long now) {
  if (btnClutch.pressedEdge) {
    cursorActive = !cursorActive;
    filtX = 0;
    filtY = 0;
    cursorAccX = 0;
    cursorAccY = 0;
    lastActivity = now;
  }
  if (cursorActive && AUTO_OFF_MS > 0 && now - lastActivity > AUTO_OFF_MS) {
    cursorActive = false;
  }
}

void handleClicks(bool connected, unsigned long now) {
  if (!connected) return;
  if (btnJoy.pressedEdge) {
    bleMouse.press(MOUSE_LEFT);
    lastActivity = now;
  }
  if (btnJoy.releasedEdge) bleMouse.release(MOUSE_LEFT);
  if (btnRight.pressedEdge) {
    bleMouse.press(MOUSE_RIGHT);
    lastActivity = now;
  }
  if (btnRight.releasedEdge) bleMouse.release(MOUSE_RIGHT);
}

void handleMotion(bool connected, float dt, unsigned long now) {
  int16_t raw[3];
  if (readGyroRaw(raw)) {
    float rx = GYRO_X_SIGN * (raw[GYRO_AXIS_FOR_X] - gyroBias[GYRO_AXIS_FOR_X]) / GYRO_LSB_PER_DPS;
    float ry = GYRO_Y_SIGN * (raw[GYRO_AXIS_FOR_Y] - gyroBias[GYRO_AXIS_FOR_Y]) / GYRO_LSB_PER_DPS;
    filtX += SMOOTHING * (rx - filtX);
    filtY += SMOOTHING * (ry - filtY);
  }
  float ox = fabsf(filtX) < GYRO_DEADZONE_DPS ? 0.0f : filtX;
  float oy = fabsf(filtY) < GYRO_DEADZONE_DPS ? 0.0f : filtY;
  if (sqrtf(ox * ox + oy * oy) > ACTIVITY_DPS) lastActivity = now;

  int mx = 0;
  int my = 0;
  if (cursorActive) {
    cursorAccX += ox * dt * CURSOR_SENSITIVITY;
    cursorAccY += oy * dt * CURSOR_SENSITIVITY;
    mx = (int)cursorAccX;
    my = (int)cursorAccY;
    cursorAccX -= mx;
    cursorAccY -= my;
    mx = constrain(mx, -127, 127);
    my = constrain(my, -127, 127);
  }

  float fh = SCROLL_H_SIGN * axisFraction(readAxis(PIN_JOY_X), joyCenterX);
  float fv = SCROLL_V_SIGN * axisFraction(readAxis(PIN_JOY_Y), joyCenterY);
  int wh = 0;
  int wv = 0;
  if (fh == 0.0f) {
    scrollAccH = 0;
  } else {
    scrollAccH += fh * SCROLL_MAX_NOTCHES_PER_SEC * dt;
    wh = (int)scrollAccH;
    scrollAccH -= wh;
    lastActivity = now;
  }
  if (fv == 0.0f) {
    scrollAccV = 0;
  } else {
    scrollAccV += fv * SCROLL_MAX_NOTCHES_PER_SEC * dt;
    wv = (int)scrollAccV;
    scrollAccV -= wv;
    lastActivity = now;
  }

  if (connected && (mx != 0 || my != 0 || wv != 0 || wh != 0)) {
    bleMouse.move(mx, my, wv, wh);
  }
}

void updateLed(unsigned long now, bool connected) {
  if (!connected) {
    if (now % 1500 < 200) {
      ledWrite(0, 0, 255);
    } else {
      ledWrite(0, 0, 0);
    }
    return;
  }
  int r;
  int g;
  int b = 0;
  if (batteryPercent >= 60) {
    r = 0;
    g = 255;
  } else if (batteryPercent >= 20) {
    r = 255;
    g = 140;
  } else {
    r = 255;
    g = 0;
  }
  bool on;
  if (batteryPercent < 10) {
    on = (now / 250) % 2 == 0;
  } else if (cursorActive) {
    on = true;
  } else {
    on = (now % 3000) < 120;
  }
  if (on) {
    ledWrite(r, g, b);
  } else {
    ledWrite(0, 0, 0);
  }
}

void setup() {
  pinMode(PIN_JOY_SW, INPUT_PULLUP);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_BTN_CLUTCH, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  ledWrite(255, 255, 255);
  mpuInit();
  if (!mpuPresent()) haltWithError();
  delay(500);
  calibrateGyro();
  calibrateJoystick();
  updateBattery(false);
  bleMouse.begin();
  lastReport = millis();
  lastBattery = lastReport;
  lastActivity = lastReport;
}

void loop() {
  unsigned long now = millis();
  updateButton(btnJoy, now);
  updateButton(btnRight, now);
  updateButton(btnClutch, now);
  bool connected = bleMouse.isConnected();
  handleClutch(now);
  handleClicks(connected, now);
  if (now - lastReport >= REPORT_INTERVAL_MS) {
    float dt = (now - lastReport) / 1000.0f;
    lastReport = now;
    handleMotion(connected, dt, now);
  }
  if (now - lastBattery >= BATTERY_INTERVAL_MS) {
    lastBattery = now;
    updateBattery(connected);
  }
  updateLed(now, connected);
  delay(1);
}
