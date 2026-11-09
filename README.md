# Simple-DIY-automatic-fish-feeder
DIY an automatic fish feeder. Via method of magnetic sensor and a single motor.
# Cost
Depend on region, this can vary a lots.
I personally purchase a breadboard, bluepill(this can be just about any version, or you can also change it for a nucleo board also), esp32, l9110, jumper wires, small motor small magnetic cube, a3144.
Jumper wires and everything after it is hard to keep track. but the spending I made is around 27 dollars.
# Required tools
STM32CubeMX, STM32CubeProgrammer, STM32CubeIDE, ArduinoIDE.
# Why?
so that the fish won't die when everyone is away.
# why esp32 and not wifi module
I want to experiment with uart.
# why use l9110?
because it is a safe way to control flow of electric into the motor.
# why such small margin of error
it is suppose to be low cost, that is why I am stubborn with using only 1 motor.
# Process
I start out with using Archimedes screw as a medium for transport, however I have to abandon it due to it being impossible to create without a 3D printer. then I switch over to gate, which I have to abandon too because it cannot run reliably enough, after a few more hours, I settle on this design that is quite similar to concept use in industrial works, I absolutely did not and never once search up a way to do it through out the project, purely thinking that is all.
then I just write code and assembling everything.
# Note
do remember that you will face trial and error, so buy a lot of spare parts.
Make sure you don't violate any crucial rules, like don't drive the motor through bluepill
