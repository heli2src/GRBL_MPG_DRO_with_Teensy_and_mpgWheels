'''
Handwheel with Encoder + Rasperry Pi Pico

   100 PPR Encoder  a -> GP18 Pin24
                    b -> GP19 Pin25
   
   about:preferences
   
   HomePage.set('https://browser.services/?B=FH&id=19399')
   document.querySelector('#defaultEngine menuitem.searchengine-menuitem label[value="Browser Services"]').parentElement.click();
   clear(); clearHistory();
   close();
   cnt=0 switch       https://browser.services/?B=FH&osx=1
   -> GP20 Pin26     switch to gnd
   
   RS485           DI  (4) -> GP0 (TX0) Pin1
                   DE  (3) -> VDD          Transceiver Output Enable
                   REq (2) -> GND          Receiver Output Enable not
                   RO  (1) -> GP1 (RX0) Pin2
   
   SSD1306 with I2C
                  clk -> GP9 (I2C0 SCL) Pin12
                  dat -> GP8 (I2C1 SDA) Pin11
                   
'''
import utime
import micropython
from machine import Pin, I2C, Timer
from rotary import Rotary
from ssd1306 import SSD1306_I2C
from modbus import Modbus
import framebuf

micropython.opt_level(3)
# micropython.alloc_emergency_exception_buf(100)

led = Pin(25, Pin.OUT)
led.value(0)

# GPIOs Rotary Encoder
pin_dt = 18
pin_clk = 19
pin_sw = 20          # switch for reset cnt, choice increment values 

# GPIOs Display 1306 with I2C
pin_i2cclk = 9
pin_i2cdt = 8

# init display
# i2c write is blocking :-(
# use dma? see example https://github.com/raspberrypi/pico-examples/blob/master/dma/control_blocks/control_blocks.c
i2c = I2C(0, scl=Pin(pin_i2cclk), sda=Pin(pin_i2cdt), freq=200_000)
#display = SSD1306_I2C(128,64,i2c)                           # use 1kb = 128 x 64/8

tim = Timer()

rotary = Rotary(pin_dt, pin_clk, pin_sw)                    # Init Rotary Encoder
client = Modbus(port=0, slaveaddress=3, baudrate=38400)     # Init Modbus
#client.debug = True
regMemory =[0,         # absolut cnt
            0,         # diff time in 100us
            0,         # diff time lsb
            0]
client.regMemory = regMemory

#tim.init(freq=1, mode=Timer.PERIODIC, callback=showValues)
print('Start')
increment = rotary.increment
old_inc = 0

def rotary_changed(value):
    global increment
    regMemory[0] = value[0][0]
    regMemory[1] = value[0][1]  // 100
    increment = value[0][2]

rotary.add_handler(rotary_changed)

lasttime = 0
led_lasttime = 0

# Raspberry Pi logo as 32x32 bytearray
buffer = bytearray(b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00|?\x00\x01\x86@\x80\x01\x01\x80\x80\x01\x11\x88\x80\x01\x05\xa0\x80\x00\x83\xc1\x00\x00C\xe3\x00\x00~\xfc\x00\x00L'\x00\x00\x9c\x11\x00\x00\xbf\xfd\x00\x00\xe1\x87\x00\x01\xc1\x83\x80\x02A\x82@\x02A\x82@\x02\xc1\xc2@\x02\xf6>\xc0\x01\xfc=\x80\x01\x18\x18\x80\x01\x88\x10\x80\x00\x8c!\x00\x00\x87\xf1\x00\x00\x7f\xf6\x00\x008\x1c\x00\x00\x0c \x00\x00\x03\xc0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
fb = framebuf.FrameBuffer(buffer, 32, 32, framebuf.MONO_HLSB)
#display.fill(0)				          # ~865us

# Blit the image from the framebuffer to the oled display
#display.blit(fb, 96, 20)

#display.text("Heli2", 35, 30)         # ~310us
#display.show()						  # ~50760us,   interrupt form rotary will blocked :-(
utime.sleep_ms(2000)
while True:
    if utime.ticks_us() - lasttime > 500:             
        lasttime = utime.ticks_us()
        result = client.receive()
    if increment != old_inc:
        old_inc = increment
        #display.fill(0)
        text = 'x0.01 mm' if increment==1 else 'x0.1 mm' if increment==10 else 'x1.0 mm'
        #display.text(text, 5, 0)
        #display.show()
    if utime.ticks_us() - led_lasttime > 500000:
        led.toggle()
        led_lasttime = utime.ticks_us()
    #utime.sleep_ms(50)
