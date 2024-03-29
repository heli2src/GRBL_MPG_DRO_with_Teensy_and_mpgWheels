# GRBL_MPG_DRO_with_Teensy_and_MPG_Wheels
Control of the grpHall with a small touch display (ILI9341 320x240) and mpg-wheels connected via RS485.

This Project is starting from [GRBL_MPG_DRO_BoosterPack](https://github.com/terjeio/GRBL_MPG_DRO_BoosterPack).
But is use a [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) for controlling.
The MPG wheels are connected via an rs485 busconnection.
<br>
<br>
Features:
- Control board with a SPI LCD and touch interface (ILI9341 320x240), connect via Uart with the [grbHAL driver](https://github.com/grblHAL).
- each MPG wheels has its own [Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/) connected via rs485 with the control board.
  Own control switches and a small display is possible (SSD1306 via I2C).
    
Source code not ready yet, there are some bugs, use it carfully!
<br><br><br>
<img src="docs/images/dev_1.jpg"></img>
<br><br><br>
## Compiling
you have to use the arduino version 1.8.19. With the latest compiler 2.x, you get compiler errors.

Following libs are used:
- https://github.com/KrisKasprzak/FlickerFreePrint
- https://github.com/KrisKasprzak/ILI9341_t3_controls
- https://github.com/KrisKasprzak/ILI9341_t3_Menu
- https://github.com/ardnew/XPT2046_Calibrated
- Bugtton      Version 1.0.5
- MicroDebug   Version 1.1