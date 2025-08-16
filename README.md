Factory
=======

Factory is a simple factory-builder game that runs on a [Teensy 4.0](https://www.pjrc.com/store/teensy40.html)
connected to an 84x48 pixel Nokia 5110 LCD screen and 6 buttons.

This was built in 3 weekends. Don't expect anything spectacular.

Setup
-----

The 6 buttons have 1 pin connected to ground and the other connected to the following Teensy pins:
- 17: Left
- 18: Up
- 19: Down
- 20: Right
- 21: Secondary Button
- 22: Primary Button

The LCD's power pins are connected to 3.3V and ground, and logic pins are connected to the Teensy as follows:
- 7: RESET
- 6: SCE
- 5: DC
- 4: SDIN
- 3: SCLK

Handheld
--------

The "case" folder contains STL files to print a case to make a handheld version. In addition to the above components, I used a 9V battery with connector, and a 5V voltage regulator. I used a battery with USB-C charging to make the device rechargeable, but this is optional.
