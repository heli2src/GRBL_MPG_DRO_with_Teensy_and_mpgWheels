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

### - Teensy for the touch display:

you have to use the arduino version 1.8.19. With the latest compiler 2.x, you get compiler errors.

Following libs are used:
- https://github.com/KrisKasprzak/FlickerFreePrint
- https://github.com/KrisKasprzak/ILI9341_t3_controls
- https://github.com/KrisKasprzak/ILI9341_t3_Menu
- https://github.com/ardnew/XPT2046_Calibrated
- [Bugtton](https://github.com/sakabug/Bugtton)      Version 1.0.5
- [MicroDebug](https://github.com/rlogiacco/MicroDebug)   Version 1.1

### - Raspi pico for the mpg wheels:
- download the latest micropython version from  https://micropython.org/download/RPI_PICO/.
- Hold down the BOOTSEL button while plugging the board into USB.
  The uf2 file should then be copied to the USB mass storage device that appears.
- Copy the files src\mpg_wheel\RP2040 with the Thonny IDE (https://thonny.org/) to your raspi pico.
- or you can use https://docs.micropython.org/en/latest/reference/mpremote.html 
- rename 'display_cnc_achse.py' to 'main.py' on the raspi pico

  
