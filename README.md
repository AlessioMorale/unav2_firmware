# uNav2 firmware

## uNav2

uNav2 is motor control platform for rovers using DC motors. More information on [the Pizza Robotics Project page](https://blog.alessiomorale.com/unav2/).

## USB interface

The firmware acts as a `gs_usb` CAN interface.
This is because I'm lazy and don't want to rewrite the driver again for future versions that will use an actual CAN interface :)
The `setup` folder contains the files required to automatically setup the interface on Ubuntu. 

## Credits

The CMake build system is based on [Andrés Hessling cmake template](https://github.com/ahessling/STM32F4Template).
