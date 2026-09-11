# structure

Drivers are seperated by what they are for 
For example a ps2 keyboard driver would be located in drivers/keyboard/ps2.h
Generic drivers are in generic.h
For example a generic keyboard driver would be in drivers/keyboard/generic.h

These generic drivers take a struct. A simple keyboard struct might look like this:
struct keyboard_device_struct =
