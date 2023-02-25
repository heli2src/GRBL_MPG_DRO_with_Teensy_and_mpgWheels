# GRBL_MPG_DRO_with_Teensy_and_MPG_Wheels
Control of the grpHall with a small touch display and mpg-wheels connected via RS485

This Project is starting from [GRBL_MPG_DRO_BoosterPack](https://github.com/terjeio/GRBL_MPG_DRO_BoosterPack).
But is use a [Teensy 3.2](https://www.pjrc.com/store/teensy32.html) for controlling.
The MPG wheels are connected via an rs485 busconnection.
<br>
<br>
Features:
- Control board with a SPI LCD and touch interface, connect via Uart with the [grbHAL driver](https://github.com/grblHAL).
- each MPG wheels has its own [Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/) connected via rs485 with the control board.
  Own control switches and a small display possible.
    
Source code not ready yet, there are some bugs, use it carfully!
<br><br><br>
<img src="docs/images/dev_1.jpg"></img>
