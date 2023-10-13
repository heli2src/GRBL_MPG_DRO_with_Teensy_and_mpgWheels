
"""
https://buildmedia.readthedocs.org/media/pdf/ssd1306/latest/ssd1306.pdf
https://www.waveshare.com/w/upload/7/71/0.96inch-OLED-UserManual.pdf
https://docs.micropython.org/en/latest/library/framebuf.html

ssd1306 = width x height = 128 x 64    = 8192 Pixel 
    framebuffer =  1024 Bytes


https://github.com/rm-hull/luma.oled
https://github.com/rm-hull/luma.core

https://luma-core.readthedocs.io/en/latest/device.html#

    
    image write:
        self.command(0x10, 0x00, 0xb0 | page)   # from luma.core.device import device
                                                # from luma.core.interface.serial import i2c
        self.data(list(buf))
        
        
        pages = high // 8
        
        def display(self,image):
           ....
            self.command(
            # Column start/end address
            self._const.COLUMNADDR, self._colstart, self._colend - 1,   # COLUMNADDR = 0x21
            # Page start/end address
            self._const.PAGEADDR, 0x00, self._pages - 1)
    
"""


import utime
import micropython
from machine import Pin, I2C, Timer, WDT, freq as CPUfreq
from rotary import Rotary
#from lib.ssd1306 import SSD1306_I2C
from ssd1306 import SSD1306_I2C
from modbus import Modbus
import framebuf
from ws2812 import ws2812

__version__ = "V0.0.1"
#Configuration
ROTDIR = 1                   # 1 = normal rotary, -1 = invers

#_____________________________


# GPIOs Display 1306 with I2C
pin_i2cclk = 9
pin_i2cdt = 8

statusline = 48

CPUfreq(125_000_000)
i2c = I2C(0, scl=Pin(pin_i2cclk), sda=Pin(pin_i2cdt), freq=200_000)
display = SSD1306_I2C(128, 64, i2c)
display.rotate(False)

buffer = bytearray(b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00|?\x00\x01\x86@\x80\x01\x01\x80\x80\x01\x11\x88\x80\x01\x05\xa0\x80\x00\x83\xc1\x00\x00C\xe3\x00\x00~\xfc\x00\x00L'\x00\x00\x9c\x11\x00\x00\xbf\xfd\x00\x00\xe1\x87\x00\x01\xc1\x83\x80\x02A\x82@\x02A\x82@\x02\xc1\xc2@\x02\xf6>\xc0\x01\xfc=\x80\x01\x18\x18\x80\x01\x88\x10\x80\x00\x8c!\x00\x00\x87\xf1\x00\x00\x7f\xf6\x00\x008\x1c\x00\x00\x0c \x00\x00\x03\xc0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
fb = framebuf.FrameBuffer(buffer, 32, 32, framebuf.MONO_HLSB)
display.fill(0)                       # ~865us
display.blit(fb, 96, 18)
display.text(f"Heli2   {__version__}", 10, 10)

display.show()
display.fill(0)

def show(buffer):
    """
    50760us
    """
    x0 = 0
    x1 = self.width - 1
    if self.width == 64:
        # displays with width of 64 pixels are shifted by 32
        x0 += 32
        x1 += 32
    display.write_cmd(SET_COL_ADDR)   # 0x21, 0x0, 127
    display.write_cmd(x0)
    display.write_cmd(x1)
    display.write_cmd(SET_PAGE_ADDR)  # 0x22 , 0, 7 
    display.write_cmd(0)
    display.write_cmd(self.pages - 1)
    display.write_data(self.buffer)   # 1024 bytes
    

for i in range(0,10):
    display.text('*', 1, statusline-10)
    start = utime.ticks_us()
    display.show()
    delta = utime.ticks_us() - start
    print(f'show time= {delta}')
    utime.sleep_ms(200)
    display.text('*', 1, statusline-10,0)    # remove text
    display.show()
    utime.sleep_ms(200)

print(f'display width x height = {display.width} x {display.pages*8}')
display.vline(10, 0, 100, 1)        # x,y,h,c
display.vline(30, 0, 100, 1)
display.hline(0, 0, 127, 1)
display.show()
display.fill(0)
display.show()
display.buffer[0] = 255
display.show()
