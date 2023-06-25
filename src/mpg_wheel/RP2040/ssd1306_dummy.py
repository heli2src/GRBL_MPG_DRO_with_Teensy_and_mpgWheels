# MicroPython SSD1306 dummy driver...

from micropython import const
import framebuf

# register definitions
SET_CONTRAST = const(0x81)
SET_ENTIRE_ON = const(0xA4)
SET_NORM_INV = const(0xA6)
SET_DISP = const(0xAE)
SET_MEM_ADDR = const(0x20)
SET_COL_ADDR = const(0x21)
SET_PAGE_ADDR = const(0x22)
SET_DISP_START_LINE = const(0x40)
SET_SEG_REMAP = const(0xA0)
SET_MUX_RATIO = const(0xA8)
SET_COM_OUT_DIR = const(0xC0)
SET_DISP_OFFSET = const(0xD3)
SET_COM_PIN_CFG = const(0xDA)
SET_DISP_CLK_DIV = const(0xD5)
SET_PRECHARGE = const(0xD9)
SET_VCOM_DESEL = const(0xDB)
SET_CHARGE_PUMP = const(0x8D)


class SSD1306_dummy():
    def __init__(self):
        print(f'{self.__class__.__name__}init()')

    def init_display(self):
        print(f'{self.__class__.__name__}.init_display()')

    def poweroff(self):
        print(f'{self.__class__.__name__}.poweroff()')

    def poweron(self):
        print(f'{self.__class__.__name__}.poweron()')

    def contrast(self, contrast):
        print(f'{self.__class__.__name__}.contrast()')

    def invert(self, invert):
        print(f'{self.__class__.__name__}.invert()')

    def rotate(self, rotate):
        print(f'{self.__class__.__name__}.rotate()')

    def show(self):
        print(f'{self.__class__.__name__}.show()')
        
    def fill(self, value):
        print(f'{self.__class__.__name__}.fill({value})')
        
    def blit(self, fb, x, y):
        print(f'{self.__class__.__name__}.blit(..)')        
        
    def text(self, msg, x, y):
        print(f'{self.__class__.__name__}: {x}, {y}  {msg}')
