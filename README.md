# Simple-DIY-automatic-fish-feeder
A low-cost automatic fish feeder that uses a single motor and a Hall-effect
sensor to dispense one portion of food per rotation. Built so the fish
survive when everyone is away.
## How it works

1. The ESP32 connects to WiFi, fetches the time from an NTP server and sends
   it to the Blue Pill over UART every 5 minutes.
2. The STM32 Blue Pill compares the time with the feeding schedule.
3. When it is time to feed, the motor turns until a magnet on the rotating
   mechanism passes the A3144 sensor (one full rotation), then stops.
## Parts and cost (about 27 USD, varies by region)

- Breadboard and jumper wires
- STM32 Blue Pill (any version, or a Nucleo board)
- ESP32
- Motor driver: [FILL IN: L9110 or MKE-M17 over I2C]
- Small DC motor
- Small magnet cube
- A3144 Hall sensor
## Setup

1. Wire everything as shown in `wiringscheme.png`.
2. In `fetchtime.ino`, set your WiFi SSID and password.
3. Set `gmtOffset_sec` to your timezone (for example `7 * 3600` for UTC+7).
4. Edit the feeding times in `main.c`, and remove the test entries.
5. Flash the ESP32 with Arduino IDE and the Blue Pill with STM32CubeProgrammer.
# Required tools
STM32CubeMX, STM32CubeProgrammer, STM32CubeIDE, ArduinoIDE.
## Design notes

- Why ESP32 instead of a WiFi module: I wanted to experiment with UART.
- Why a motor driver: it is a safe way to control current into the motor.
- Why such a small margin of error: the goal is low cost, so I use only one
  motor and accept some variation in portion size.
- Mechanism: I first tried an Archimedes screw (impossible without a 3D
  printer), then a gate (not reliable enough), and finally a rotating design
  similar to dosing mechanisms used in industry.
  
# Process
I start out with using Archimedes screw as a medium for transport, however I have to abandon it due to it being impossible to create without a 3D printer. then I switch over to gate, which I have to abandon too because it cannot run reliably enough, after a few more hours, I settle on this design that is quite similar to concept use in industrial works, I absolutely did not and never once search up a way to do it through out the project, purely thinking that is all.
then I just write code and assembling everything.
## Safety notes

- Never drive the motor directly from Blue Pill pins. Use the motor driver
  with its own power supply and connect all grounds together.
- Test with a small amount of food first. Overfeeding pollutes the water.
# Note
do remember that you will face trial and error, so buy a lot of spare parts.
Make sure you don't violate any crucial rules, like don't drive the motor through bluepill
