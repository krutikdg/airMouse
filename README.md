# airMouse

# Pin Map (XIAO ESP32S3)

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

