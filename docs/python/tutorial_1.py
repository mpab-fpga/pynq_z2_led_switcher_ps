import time
from pynq import Overlay
tutorial = Overlay("./design_1.bit")
bram = tutorial.bram
buttons = tutorial.buttons
switches = tutorial.switches
leds = tutorial.leds
outputMask = 0x0
tristateRegisterOffset = 0x4
leds.write(tristateRegisterOffset, outputMask)
dataRegisterOffset = 0x0
led_pattern = 0xa
print("press a button to light an LED")
print("toggle a switch to stop")
while(not switches.read()):
    leds.write(dataRegisterOffset, buttons.read())
