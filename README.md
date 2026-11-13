# Simple DIY Automatic Fish Feeder
 
A low-cost automatic fish feeder that uses one motor and one Hall-effect sensor to dispense a single portion of food per rotation. Built so the fish survive when everyone is away.
 
The controller comes in two versions that share the same mechanism, motor driver and sensor:
 
| | **Version A: ESP32 + Blue Pill** (original) | **Version B: ESP32 only** (standalone alternative) |
|---|---|---|
| ESP32 does | Gets the time from the internet, sends it over UART | Everything |
| Schedule, motor, sensor | STM32 Blue Pill | ESP32 |
| Extra board | Blue Pill | None |
| Code | `fetchtime.ino` (ESP32), `main.c` (Blue Pill) | `fish_feeder.ino` |
| Good for | Learning UART between two microcontrollers | Fewer parts, simpler build |
 
Version B removes the Blue Pill and the UART link. The trade-off is that Wi-Fi and motor control now live on the same chip and share one power supply.
 
## How it works
 
The feeder is a rotating dosing mechanism. A blade turns under the food hopper, and one full rotation drops one portion through the dropping mouth. A small magnet sits on the blade edge. When it passes the A3144 Hall sensor, the controller stops the motor.
 
### Version A
1. The ESP32 connects to Wi-Fi, fetches the time from an NTP server and sends it to the Blue Pill over UART every 5 minutes.
2. The STM32 Blue Pill compares the time with the feeding schedule.
3. When it is time to feed, the motor turns until the magnet passes the A3144 (one full rotation), then stops.
### Version B
1. The ESP32 gets the time over Wi-Fi (NTP) and checks the schedule every 5 minutes.
2. It talks to the MKE-M17 motor driver over I2C (address `0x40`) to run and stop the motor.
3. The A3144 is wired directly to an ESP32 GPIO. The feed cycle starts the motor, waits for the magnet to leave the sensor, then stops the motor when the magnet comes back (one full turn).
## Parts and cost
 
About 27 USD for Version A (varies by region). Version B is slightly cheaper because it has no Blue Pill.
 
- Breadboard and jumper wires
- ESP32 (ESP32-S dev board)
- STM32 Blue Pill, any version or a Nucleo board (**Version A only**)
- MKE-M17 L9110 I2C motor driver module
- TT gearbox DC motor (yellow)
- Small magnet cube
- A3144 Hall sensor
- 5V 2A wall adapter (Version B)
## Wiring
 
**Version A:** wire everything as shown in `circuit_image.png`.
 
**Version B:**
 
| ESP32-S | MKE-M17 |
|---|---|
| 5V | 5V |
| GND | GND |
| GPIO21 (SDA) | SDA |
| GPIO22 (SCL) | SCL |
 
| Part | Connects to |
|---|---|
| Motor `+` / `-` | MB1 / MB2 |
| A3144 `+` / `-` | MA1 / MA2 (the driver powers the sensor only while feeding) |
| A3144 signal | ESP32 GPIO27 (internal pull-up, no external resistor needed) |
 
Notes:
- **Jumper J1:** follow the MKE-M17 documentation. With J1 **OFF**, the module's 5V pin is an input, which is the case when the ESP32 supplies the 5V. With J1 **ON**, that pin becomes an output (max 700 mA) powered from the motor supply. Do not let two 5V sources drive the same line.
- **Sensor power:** the A3144 is rated for 4.5 V and up. It can also run from the ESP32's 3.3 V pin, which worked in testing but is outside the datasheet rating. To do that, wire it to 3.3 V and GND and set `SENSOR_POWERED_BY_DRIVER` to `0`.
- **Magnet polarity:** the A3144 only responds to one pole facing its marked side. Test it before gluing the magnet.
## Setup
 
### Version A
1. Wire everything as shown in `wiringscheme.png`.
2. In `fetchtime.ino`, set your Wi-Fi SSID and password.
3. Set `gmtOffset_sec` to your timezone (for example `7 * 3600` for UTC+7).
4. Edit the feeding times in `main.c`, and remove the test entries.
5. Flash the ESP32 with the Arduino IDE and the Blue Pill with STM32CubeProgrammer.
Required tools: STM32CubeMX, STM32CubeProgrammer, STM32CubeIDE, Arduino IDE.
 
### Version B
1. Install the Arduino IDE and the ESP32 board package, then select the **ESP32 Dev Module** board.
2. In Library Manager, install **MKE_ONE** by MakerEdu.vn. It provides `MKE_I2C_MotorDriver.h`.
3. Open `fish_feeder.ino` and set `WIFI_SSID` and `WIFI_PASS`.
4. Set `UTC_OFFSET_SEC` for your timezone (default is UTC+7).
5. Edit `SLOTS` with your feeding times.
6. Upload, then open the Serial Monitor at 115200 baud.
7. First test: set `DEMO_FEED_ON_BOOT` to `1`, upload, and watch one feed cycle in the log. Set it back to `0` and upload again when done.
Required tools: Arduino IDE.
 
## Version B settings
 
All settings are at the top of `fish_feeder.ino`.
 
| Setting | Default | Meaning |
|---|---|---|
| `UTC_OFFSET_SEC` | `7 * 3600` | Timezone offset (fixed, no daylight saving) |
| `SLOTS` | `{8, 0}` | Feeding times, up to 15 |
| `WINDOW_SEC` | 10 min | Feeding is allowed from the slot time to slot time + 10 min |
| `CHECK_INTERVAL_MS` | 5 min | How often the schedule is checked |
| `MIN_FEED_GAP_S` | 30 min | Minimum time between any two feeds (`0` disables it) |
| `FALLBACK_INTERVAL_MS` | 10 h | Feeding interval when no time source is available |
| `MOTOR_PWM` | 180 | Motor speed, clamped to 150–200 in code |
| `FEED_TIMEOUT_MS` | 2500 | Hard time limit for one feed cycle |
| `DRIVER_FAILSAFE_MS` | 2900 | The driver's own self-stop time, a backup |
| `HALL_PIN` | 27 | Sensor input pin |
| `MOTOR_B_FORWARD_IS_CW` | 1 | Set to `0` if the blade turns the wrong way |
| `SENSOR_POWER_USES_CW` | 1 | Set to `0` if the sensor never responds (supply polarity) |
| `SENSOR_POWERED_BY_DRIVER` | 1 | `0` if the sensor is powered from the ESP32 instead |
| `DEMO_FEED_ON_BOOT` | 0 | `1` runs one feed right after boot |
| `CLEAR_FED_RECORD_ON_BOOT` | 0 | `1` forgets the saved "already fed" record |
 
## Feeding logic (Version B)
 
- **Window:** a slot is due from its start time until 10 minutes later. It is never due early. With checks every 5 minutes, every window gets at least two checks.
- **No double feeding:** the slot is saved to flash before the motor starts, so a reset or a failed feed cannot repeat it. There is no cooldown timer.
- **Outages:** a missed window is skipped, with no catch-up. If the ESP32 comes back while still inside the window, it feeds.
- **No time source:** if no valid time arrives within 30 seconds of boot, a 10-hour countdown starts and the fish are fed every 10 hours. If NTP arrives later, the schedule takes over. The ESP32 never feeds on an unsynced clock.
- **Feed cycle:**
  1. Power the sensor and let it settle.
  2. Start the motor at the capped speed.
  3. Wait until the magnet has left the sensor. This avoids stopping instantly when the blade rests on the sensor.
  4. Stop when the magnet returns (falling edge on a GPIO interrupt, re-checked after 1 ms).
  5. Stop with `PWM = 0` (coast). The motor never reverses.
- **Timeout:** if the magnet does not return within 2.5 seconds, the motor stops and a fault is printed. There is no automatic retry. The next slot works normally.
## Safety notes
 
- Never drive the motor directly from microcontroller pins. Use the motor driver and connect all grounds together.
- Test with a small amount of food first. Overfeeding pollutes the water.
- The motor runs at a capped speed, stops by coasting rather than reversing, and has a hard timeout of under 3 seconds against mechanical stalls or sensor failure.
- In Version B the driver also receives a timed command, so it stops the motor by itself even if the ESP32 hangs.
### Known limitations
 
- **Power:** the motor and the ESP32 share one 5V adapter. Motor start-up current can dip the supply and reset the ESP32. A bulk capacitor (470–1000 µF) near the driver and a 100 nF capacitor across the motor terminals help. The prototype was built without them.
- **Motor supply:** the MKE-M17 rates its motor input at 6–9 V. Running it from 5V is below spec, so the motor may be weaker and the speed needs tuning.
- **No hardware cutoff:** there is no external hardware to cut motor power. Protection is in software and in the driver's timed command.
- **No jam recovery:** the motor never reverses, so a jam stops the cycle at the timeout and needs a human.
- **No sensing of food level:** an empty hopper looks like a normal feed.
- **Portion size:** portions vary a little. The goal is low cost, so there is only one motor and some variation is accepted.
## Troubleshooting (Version B)
 
| Symptom | Likely cause and fix |
|---|---|
| Every feed ends with `FEED FAULT` | Wrong magnet pole, no sensor power, or a jam. Try `SENSOR_POWER_USES_CW = 0`, flip the magnet, check the log line "Hall idle level". |
| Blade turns the wrong way | Set `MOTOR_B_FORWARD_IS_CW = 0`. |
| `MKE-M17 not answering at 0x40` | Check SDA/SCL wiring and the module's address. |
| Never feeds at the set time | Check Wi-Fi credentials, `UTC_OFFSET_SEC`, and that the log shows "schedule mode". |
| A slot refuses to feed again while testing | The slot was already saved today. Set `CLEAR_FED_RECORD_ON_BOOT = 1` for one boot, or `MIN_FEED_GAP_S = 0`. |
| Random resets when the motor starts | Supply voltage dip. Add capacitors or use a stronger supply. |
 
## Design notes
 
- **Why an ESP32 instead of a Wi-Fi module:** I wanted to experiment with UART.
- **Why a motor driver:** it is a safe way to control current into the motor.
- **Why such a small margin of error:** the goal is low cost, so I use only one motor and accept some variation in portion size.
- **Mechanism:** I first tried an Archimedes screw (impossible without a 3D printer), then a gate (not reliable enough), and finally a rotating design similar to dosing mechanisms used in industry.
## Process
 
I started with an Archimedes screw as the transport medium, but had to abandon it because it is impossible to make without a 3D printer. I then switched to a gate, which I also abandoned because it could not run reliably enough. After a few more hours I settled on this design, which is quite similar to a concept used in industry. I worked it out purely by thinking it through, without looking up an existing solution. After that I wrote the code and assembled everything.
 
## Note
 
Expect trial and error, and buy plenty of spare parts. Do not break the crucial rules, such as never driving the motor through the Blue Pill.
