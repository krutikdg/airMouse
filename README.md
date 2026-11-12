# ESP32-S3 Air Mouse

This is a thumb operated air mouse that lets me use my Mac without the trackpad. A Seeed XIAO ESP32S3 sends everything over Bluetooth LE, and a small gyro follows how I rotate my wrist to move the cursor. A toggle button turns that movement on and off, so the cursor stays put while I carry the mouse. A thumb joystick scrolls up, down, left and right, and pressing it gives a left click. A second button handles right click. One LED shows whether the cursor is active and how much battery is left. Everything runs from a small LiPo cell that charges over USB-C.

Status: the firmware and the case have not been tested on real hardware yet, and the case dimensions use placeholder sizes until the parts are measured with calipers.

## Built with

- Seeed XIAO ESP32S3 (Bluetooth LE HID mouse)
- MPU6050 gyro on a GY-521 breakout
- Analog thumb joystick
- WS2812B LED
- Single cell LiPo battery
- Arduino IDE with the ESP32 board package and the ESP32-BLE-Mouse library
- Fusion 360 for the case

# Bill Of Materials


| Component | Qty | Price/Unit (USD) | Purpose | Source |
|---|---:|---:|---|---|
| Seeed XIAO ESP32S3 (non-Sense version) | 1 | 27.20 | Main controller with Bluetooth LE and LiPo charging; code and pin map assume this board | [Amazon](https://www.amazon.in/ESP32S3-2-4GHz-Wi-Fi-Dual-core-Supported-Efficiency-Interface/dp/B0BYSB66S5) |
| GY-521 MPU6050 gyro and accelerometer breakout | 1 | 8.55 | Reads wrist rotation to move the cursor | [Amazon](https://www.amazon.in/Gyroscope-Accelerometer-Interface-Tracking-Robotics/dp/B0H1JYZHGC) |
| Mini analog thumb joystick module with push switch | 1 | 1.50 | Scroll on both axes and left click on press | [Amazon](https://www.amazon.in/Robocraze-Joystick-Breakout-Multi-Directional-High-Precision/dp/B073B647CW) |
| Momentary tactile push button (about 6 x 6 mm) | 2 | 2.00 | Right click and clutch toggle | [Amazon](https://www.amazon.in/ElectroBot-Momentary-Tactile-Push-Button/dp/B07PRRRBRY) |
| WS2812B addressable RGB LED | 1 | 2.00 | Clutch status and battery level indicator | [Hubtronics](https://hubtronics.in/neopixel-ws2811-led-strip-india) |
| 3.7V LiPo battery with protection circuit (502535 class, about 500 mAh) | 1 | 2.40 | Onboard power; case size depends on this battery | [Electropi](https://www.electropi.in/3.7v-500mah-lithium-polymer-lipo-rechargeable-battery-model-kp-502535) |
| Mini slide switch (SPDT) | 1 | 1.10 | Main power on and off | [Amazon](https://www.amazon.in/Electronicspices-10pcs-Slide-Switch-3-Pin/dp/B08L56VG5B) |
| 100k ohm resistor pack | 1 | 2.15 | Battery voltage divider for the ADC | [Amazon](https://www.amazon.in/100k-100K-0-25W-metal-Resistor/dp/B0G4DLDMDC) |
| M2 countersunk self-tapping screw pack (about 6 to 8 mm long) | 1 | 1.50 | Fix the bottom cover to the case posts | [Robu](https://robu.in/product/easymech-ss-304-csk-countersunk-philips-head-m2-x-8-mm-bolt-25-pcs/) |
| Double-sided foam tape | 1 | 0.95 | Hold the XIAO and battery in place | [BigBasket](https://www.bigbasket.com/pd/40355872/nuovi-double-sided-foam-tape-24-mm-10-m-1-pc/) |
| Thin flexible hookup wire pack | 1 | 1.45 | Internal wiring inside the small case | [Flipkart](https://www.flipkart.com/symfonia-combo-5-color-multi-strand-flexible-hookup-wire-7-36-2-meter-each-total-10-meter-full-copper-pvc-insulated-electric-model-building-tools-all-kinds-diy-electrical-hobby-projects-working-models-science-experiment-kit-electronic-components/p/itma0205243e375e) |
| Total | 11 | 52.80 | Estimated project BOM cost | — |

# Circuit Diagram

<img width="806" height="731" alt="airMouse" src="https://github.com/user-attachments/assets/6b8940be-32d0-424b-ac6e-150612922509" />


## Pin Map (XIAO ESP32S3)

| Component | Component Pin | MCU Pin |
| :---- | :---- | :---- |
| Joystick | VCC / +5V | 3V3 |
| Joystick | GND | GND |
| Joystick | VRx | D0 (GPIO1) |
| Joystick | VRy | D1 (GPIO2) |
| Joystick | SW | D8 (GPIO7) |
| Right click button | Leg 1 | D9 (GPIO8) |
| Right click button | Leg 2 | GND |
| Clutch button | Leg 1 | D10 (GPIO9) |
| Clutch button | Leg 2 | GND |
| MPU6050 | VCC | 3V3 |
| MPU6050 | GND | GND |
| MPU6050 | SDA | D4 (GPIO5) |
| MPU6050 | SCL | D5 (GPIO6) |
| MPU6050 | AD0 | GND |
| MPU6050 | INT, XDA, XCL | Not connected |
| WS2812B LED | VDD | 3V3 |
| WS2812B LED | GND | GND |
| WS2812B LED | DIN | D6 (GPIO43) |
| Slide switch | Common pin | LiPo \+ |
| Slide switch | Throw pin | BAT+ pad (XIAO) |
| LiPo | Negative | BAT- pad (XIAO) |
| Divider R1 (100k) | Pin 1 | Switched BAT+ (switch output) |
| Divider R1 (100k) | Pin 2 | D3 (GPIO4) |
| Divider R2 (100k) | Pin 1 | D3 (GPIO4) |
| Divider R2 (100k) | Pin 2 | GND |

## I2C addresses

| Device | Address with AD0 low | Address with AD0 high |
| :---- | :---- | :---- |
| GY-521 (MPU6050) | 0x68 | 0x69 |

## Before you start

You need a soldering iron and solder, a multimeter, calipers, a small screwdriver for M2 screws, hot glue or instant glue, and a USB-C cable that carries data. Part sizes vary by seller, so measure every part and check the case model before printing the final parts.

## Assembly

1. Flash and test on a breadboard first. Install the ESP32 board package and the ESP32-BLE-Mouse library in the Arduino IDE, select the XIAO ESP32S3 board, open the sketch and upload it over USB-C. Wire every part on a breadboard following the pin map, with no battery yet. The LED should turn white during the two second gyro calibration and then blink blue. The Mac should list AirMouse in its Bluetooth settings. After pairing, the clutch button should start the cursor, the stick should scroll, and the stick press and the second button should click. Fix problems now, because soldered joints are harder to fix.

2. Find the right button legs. On a four legged tactile switch, two legs on the same side are often joined permanently. Set the multimeter to continuity and find the pair that beeps only while the button is pressed. Wire across that pair.

3. Test fit the case before soldering. Put the joystick through its hole and check that the stick tilts without touching the edge. Check that the button plungers move freely, that the USB-C cable plug fits the slot, that the switch lever moves in its slot, and that an M2 screw threads into each post. Adjust the model or file the plastic.

4. Prepare the wiring. The case is small, so cut thin flexible wire to short lengths with a little slack. Tin the pads on the XIAO. Build the battery divider from two 100k resistors: R1 from the switched battery positive to D3 (GPIO4), and R2 from D3 to GND. If you use the optional 100 nF capacitor, solder it between D3 and GND.

5. Solder the signal wires.
   - Joystick: VCC to 3V3, GND to GND, VRx to D0, VRy to D1, SW to D8. Some modules print +5V on the VCC pin. Connect it to 3V3 anyway.
   - Right click button: one leg to D9, the other to GND.
   - Clutch button: one leg to D10, the other to GND.
   - Gyro: VCC to 3V3, GND to GND, SDA to D4, SCL to D5, AD0 to GND. Leave INT, XDA and XCL open.
   - LED: VDD to 3V3, GND to GND, DIN to D6.

6. Wire the power last. Connect the battery positive to the slide switch common pin, the switch output to the XIAO BAT+ pad, and the battery negative to the BAT- pad. Check the pad positions and polarity against Seeed's pinout, since a reversed battery can destroy the board. Keep the switch off. With the multimeter, check that there is no short between BAT+ and BAT-. Cover bare battery leads with heat shrink or tape, and keep the iron away from the cell.

7. Mount the parts in the case body.
   - Joystick: fix the board under the top face with the stick centered in its hole. If the stick barely clears the lid, add a thumb cap.
   - Buttons: fix each one under its hole so the plunger sits in the hole.
   - LED: glue it under its window with the lens facing the hole.
   - Slide switch: put the lever through the side slot and glue the body to the wall.
   - Gyro: fix it rigidly to the case, for example with hot glue to a small printed standoff under the top plate. The board has to move exactly with the case. Keep its orientation the same as in the breadboard test, and if you mount it rotated, change the axis settings in the sketch.

8. Close the case. Lay the cover face up and set the battery against the stop, with the XIAO taped on top and its USB-C end toward the back. Lower the body over it so the XIAO slips between the guide ribs and the USB-C port lines up with the slot. Check that no wire is trapped, then drive the four M2 screws in gently. Overtightening strips printed plastic.

9. First power up. Charging goes through the slide switch, so keep it on while you charge. Switch on, keep the mouse still for about two seconds while the LED is white, and do not touch the stick. Pair it with the Mac, then press the clutch button to start moving the cursor.

## LED guide

| LED | Meaning |
|---|---|
| White | Calibrating the gyro, hold the mouse still |
| Blinking blue | Waiting to pair |
| Steady green, orange or red | Paired and the cursor is on. The color shows the battery: green above 60 percent, orange from 20 to 60, red below 20 |
| Short flash every 3 seconds | Paired and the cursor is off |
| Fast red blinking | Battery below 10 percent, or at power on the gyro was not found |

## Troubleshooting

All the settings mentioned below are in the settings block at the top of the sketch.

**Red blinking right after power on.** The gyro did not answer. Check SDA (D4), SCL (D5), 3V3, ground, and that AD0 is tied to GND so the address is 0x68. Look at the solder joints. If the wiring is right and it still blinks, I'm not certain, but some clone chips report a different ID, so check what yours returns.

**The LED stays white or flashes red at startup.** The mouse moved during calibration, so it retried up to five times. Set it down and power cycle it. After five failed tries it carries on with the last measurement, which may drift.

**Blinking blue and the Mac does not connect.** Remove AirMouse in the Mac's Bluetooth settings, switch the mouse off and on, and pair again. macOS remembers old device names.

**The cursor creeps while the mouse is still.** Calibration happened while it was moving, so power cycle it at rest. If it persists, raise `GYRO_DEADZONE_DPS` and check that the gyro is fixed firmly. The chip drifts a little as it warms.

**The cursor moves the wrong way or on the wrong axis.** `GYRO_AXIS_FOR_X` and `GYRO_AXIS_FOR_Y` pick the gyro axis (0, 1 or 2), and `GYRO_X_SIGN` and `GYRO_Y_SIGN` flip the directions. The right values depend on how the gyro is mounted.

**The cursor is too fast, too slow or jittery.** `CURSOR_SENSITIVITY` sets the speed. `SMOOTHING` runs from 0 to 1, and lower values are steadier but slower to react.

**The clutch button toggles several times per press, or seems stuck.** Check the continuity test from the assembly steps, since a button wired across the wrong pair looks permanently pressed. Otherwise raise `DEBOUNCE_MS`.

**The cursor turns itself off after a minute.** That is the auto off. It triggers after 60 seconds without motion, scrolling or clicks. Change `AUTO_OFF_MS`, or set it to 0 to disable it.

**Scroll goes the wrong way.** Flip `SCROLL_V_SIGN` or `SCROLL_H_SIGN`. The Mac's natural scrolling setting also reverses it.

**The page scrolls by itself, or pressing the stick scrolls too.** The stick's resting position is measured at power on, so keep your thumb off it during startup. Raise `JOY_DEADZONE`, and check the VRx and VRy joints.

**Scrolling is too fast or too slow.** Change `SCROLL_MAX_NOTCHES_PER_SEC`.

**Horizontal scroll does nothing.** Vertical may work while horizontal does not. It depends on the library version and the app, and I'm not certain your version supports it. Try an app that scrolls sideways, and check the library.

**The battery reading jumps or looks wrong.** Two 100k resistors are a high impedance for the ADC, so add the 100 nF capacitor at D3. The reading is also higher while the USB charger is connected. The percentage curve in `VBAT_TABLE_MV` and `VBAT_TABLE_PCT` is a rough guess for a typical cell, so replace it with readings from your own battery. Check `DIVIDER_RATIO` against your resistor values.

**The LED flickers at power on.** D6 (GPIO43) is also the serial transmit pin. Keep serial prints out of the sketch.

**Nothing happens on battery power.** Check the slide switch, the battery polarity and the BAT pads. A battery protection circuit that tripped may need a charge over USB first.

**The upload fails.** Use a data capable USB-C cable. Hold the BOOT button while plugging the board in and try again. Verify the exact procedure in Seeed's guide for your board.

**The sketch will not compile and the errors point at the BLE library.** The library and the ESP32 board package versions have to match. Try an older board package or a maintained fork of the library, and check which versions your setup needs.

**The cover rocks or the screws strip the plastic.** Overtightening is the usual cause. If a pilot hole came out too large, reprint with a smaller hole or add a drop of glue.

**The stick rubs its hole, a button sticks, or the USB-C plug will not fit.** Enlarge the matching hole in the CAD model and reprint. The sizes are parameters in the model.

# CAD Models
## Body
<img width="1600" height="1039" alt="image" src="https://github.com/user-attachments/assets/1d151112-460b-42d7-ac22-0d2dcc55b14e" />

## Cover 
<img width="1600" height="1039" alt="image" src="https://github.com/user-attachments/assets/d863c2b4-7bac-4c05-928f-6cdb18a999cb" />

## Complete
<img width="1600" height="1039" alt="image" src="https://github.com/user-attachments/assets/ac04f525-9ea1-4381-844b-97df6c700523" />
