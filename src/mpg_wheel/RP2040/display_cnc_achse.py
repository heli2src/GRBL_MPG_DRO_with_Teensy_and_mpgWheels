'''
Handwheel with Encoder + Rasperry Pi Pico Zero

use https://micropython.org/download/rp2-pico/
        MicroPython v1.20.0 on 2023-04-26; Raspberry Pi Pico with RP2040    

   100 PPR Encoder  a -> GP10  (Pin12)
                    b -> GP11  (Pin11)
   
   4x switch 26, 15, 14, 13  to gnd 
   
   RS485           DI  (4) -> GP0 (TX0) Pin1
                   DE  (3) -> VDD          Transceiver Output Enable
                   REq (2) -> GND          Receiver Output Enable not
                   RO  (1) -> GP1 (RX0) Pin2
   
   SSD1306 with I2C
                  clk -> GP9 I2C0 SCL (Pin9)
                  dat -> GP8 I2C0 SDA (Pin10)
                  
    WS2812 LED :      -> GP16    
    
    todo:
       - AXIS should get from the eeprom lib (emulated)
       - use dma for the data from SSD1306
       - display values from the axis
       - control mode only dummy yet
                   
'''
import utime
import micropython
from machine import Pin, I2C, Timer, WDT, freq as CPUfreq
from rotary import Rotary
from ssd1306 import SSD1306_I2C
from modbus import Modbus
import framebuf
from ws2812 import ws2812

__version__ = "V0.5"
#Configuration
AXIS = 0                     # 0 = X, 1 = Y, 2 = Z
ROTDIR = -1                  # 1 = normal rotary, -1 = invers
KEYLAYOUT = False            # if False: mirrored switch layout
#_____________________________

# GPIOs Rotary Encoder
PIN_DT = 10
PIN_CLK = 11

if KEYLAYOUT:
    PIN_SW1 = 26          # Start/Stop
    PIN_SW2 = 15          # choice incValue
    PIN_SW3 = 14          # disable, jog-mode, control mode
    PIN_SW4 = 13          # set to zero
else:
    PIN_SW1 = 13          # Start/Stop
    PIN_SW2 = 14          # choice incValue
    PIN_SW3 = 15          # disable, jog-mode, control mode
    PIN_SW4 = 26          # set to zero     

# GPIOs Display 1306 with I2C
PIN_I2CCLK = 9
PIN_I2CDT = 8

statusline = 48


class cnc_axis():
    
    axis = 0           # todo: read from eeprom        
    
    regMemory = [0,         # absolut cnt
                 0,         # diff time in 100us
                 0,         # switch values
                 0]
    choiceAxis = ['X', 'Y', 'Z', 'A']
    
    def __init__(self, pin_dt, pin_clk, pin_sw1, pin_sw2, pin_sw3, pin_sw4, pin_i2cclk, pin_i2cdt):
        micropython.opt_level(3)
        # micropython.alloc_emergency_exception_buf(100)
        CPUfreq(125_000_000)
        
        if ws2812:
            self.led = ws2812(16)
            self.led.on()
        else:            
            self.led = Pin(25, Pin.OUT)
            self.led.value(0)
        
        self.debug = True

        # init display
        # i2c write is blocking :-(
        # use dma? see example https://github.com/raspberrypi/pico-examples/blob/master/dma/control_blocks/control_blocks.c
        i2c = I2C(0, scl=Pin(pin_i2cclk), sda=Pin(pin_i2cdt), freq=200_000)
        self.display = SSD1306_I2C(128, 64, i2c)                         # use 1kb = 128 x 64/8
                                                                         #  0- 47 blue
                                                                         # 48- 63 yellow         
        if self.display.notFound:
            from ssd1306_dummy import SSD1306_dummy
            self.display = SSD1306_dummy()
            print('SSD1306 not found')
        #self.display.invert(True)
        self.display.rotate(False)
        #tim = Timer()
        #tim.init(freq=1, mode=Timer.PERIODIC, callback=showValues)
        
        self.jogmode = 0				# 0-2 : button  disable, jogging, control
                                        # > 10: >4s switch to changing axis
        self.displayChange = True
        self.buttontime = 0        

        self.rotary = Rotary(ROTDIR, pin_dt, pin_clk, pin_sw2)                    # Init Rotary Encoder
        self.rotary.add_handler(self.rotary_changed)
        self.valueChange = False
        self.switchtime = 0
        
        sw1 = Pin(pin_sw1, Pin.IN, Pin.PULL_UP)
        sw1.irq(handler=self.sw1_irq, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )
        sw3 = Pin(pin_sw3, Pin.IN, Pin.PULL_UP)
        sw3.irq(handler=self.sw3_irq, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )
        sw4 = Pin(pin_sw4, Pin.IN, Pin.PULL_UP)
        sw4.irq(handler=self.sw4_irq, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )

        self.client = Modbus(port=0, slaveaddress=AXIS+1, baudrate=38400, debug=False)     # Init Modbus Slaveadress should be X,Y,Z = 88, 89, 90,
                                                                         # should be einstellbar über EEPROM ?!
        self.client.regMemory = self.regMemory
        self.noConnectionTime = -5

        print('Start')
        self.incValue = self.rotary.increment
        self.lasttime = 0
        self.led_lasttime = 0

        # Raspberry Pi logo as 32x32 bytearray
        buffer = bytearray(b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00|?\x00\x01\x86@\x80\x01\x01\x80\x80\x01\x11\x88\x80\x01\x05\xa0\x80\x00\x83\xc1\x00\x00C\xe3\x00\x00~\xfc\x00\x00L'\x00\x00\x9c\x11\x00\x00\xbf\xfd\x00\x00\xe1\x87\x00\x01\xc1\x83\x80\x02A\x82@\x02A\x82@\x02\xc1\xc2@\x02\xf6>\xc0\x01\xfc=\x80\x01\x18\x18\x80\x01\x88\x10\x80\x00\x8c!\x00\x00\x87\xf1\x00\x00\x7f\xf6\x00\x008\x1c\x00\x00\x0c \x00\x00\x03\xc0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
        fb = framebuf.FrameBuffer(buffer, 32, 32, framebuf.MONO_HLSB)
        self.display.fill(0)                       # ~865us
        # Blit the image from the framebuffer to the oled display
        self.display.blit(fb, 96, 18)
        self.display.text(f"Heli2    {__version__}", 10, 10)
        for y in range(20, 60, 10):
            self.display.show()                    # ~50760us,   interrupt form rotary will blocked :-(
            utime.sleep_ms(500)
            self.display.text("Heli2", y, y)       # ~310us
        utime.sleep_ms(500)
        self.wdt = WDT(timeout=1000)  # enable Watchdog, with a timeout of 1s

    def sw1_irq(self, pin):
        value = pin.value()
        if value== 0:
            self.buttontime = utime.ticks_ms()
            return
        if self.buttontime == 0:
            return
        dtime = utime.ticks_ms()- self.buttontime
        if value == 1 and (dtime > 100 and dtime < 900):
            value = value + (AXIS+1) *10
            self.regMemory[2] = value
            print(f'       sw1 ={value}')  
            self.switchtime = utime.ticks_ms()

    def sw3_irq(self, pin):
        value = pin.value()
        if value== 0:
            self.buttontime = utime.ticks_ms()
            return
        if self.buttontime == 0:
            return
        dtime = utime.ticks_ms()- self.buttontime
        if value == 1 and (dtime > 100 and dtime < 500):
            if self.jogmode >= 10 and self.jogmode-9 < len(self.choiceAxis):
                self.jogmode += 1
            elif self.jogmode >= 10:
                self.jogmode = 10
            elif self.jogmode < 2:
                self.jogmode += 1
            else:
                self.jogmode = 0
            self.displayChange = True
            self.buttontime = 0
        elif value == 1 and (dtime > 3000):
            self.buttontime = 0
            self.displayChange = True
            self.jogmode = 0 if self.jogmode >= 10 else 10
            print(f'       config {dtime}')

    def sw4_irq(self, pin):
        value = pin.value()
        if value== 0:
            self.buttontime = utime.ticks_ms()
            return
        if self.buttontime == 0:
            return
        dtime = utime.ticks_ms()- self.buttontime
        if value == 1 and (dtime > 100 and dtime < 900):
            value = value * 4 + (AXIS+1) *10
            self.regMemory[2] = value
            print(f'       sw4 ={value}')
            self.switchtime = utime.ticks_ms()

    def rotary_changed(self, value):
        self.regMemory[0] = value[0][0]                           # absolut cnt
        self.regMemory[1] = value[0][1]  // 100                   # diff time in 100us
        self.valueChange = True
        if self.incValue != value[0][2]:
            self.incValue = value[0][2]
            self.displayChange = True
        #self.dprint(f'changed {self.regMemory}')
            
    def display_page1(self):
        self.display.fill(0)
        self.display.text(f'{self.choiceAxis[AXIS]}-axis', 0, 0)
        if self.noConnectionTime > 4000:
            self.display.text('no connection', 20, 10)
        else:
            self.display.text('ok', 60, 0)
        text = 'x0.01mm' if self.incValue==1 else 'x0.1mm' if self.incValue==10 else 'x1.0mm'
        self.display.text(text, 1, statusline)
        text = 'disable' if self.jogmode==0 else 'joggling' if self.jogmode==1 else 'control'
        self.rotary.enable(True if self.jogmode==1 else False)
        self.display.text(text, 72, statusline)
        self.display.show()
        self.displayChange = False
        
    def display_page2(self):
        print("display_page2")
        self.display.fill(0)
        AXIS = self.jogmode-10
        text = f'axis = {self.choiceAxis[AXIS]}'
        self.display.text(text, 20, statusline-20)
        self.display.show()
        self.displayChange = False        

    def loop(self):
        if utime.ticks_us() - self.lasttime > 500:              # call every 500us
            self.lasttime = utime.ticks_us()
            result = self.client.receive()                      # read bus
            if result is None:
                self.noConnectionTime += 1
                if 5000 > self.noConnectionTime > 1000 or self.noConnectionTime < 0:
                    self.displayChange = True
                    self.noConnectionTime = 5000
            else:
                if self.noConnectionTime > 1000 or self.noConnectionTime < 0:
                    self.displayChange = True
                self.noConnectionTime = 0
            if self.valueChange:
                self.dprint(f'change {self.regMemory}')
                self.valueChange = False
            if self.switchtime>0 and utime.ticks_ms() - self.switchtime > 500:
                print("switch = 0")
                self.switchtime = 0
                self.regMemory[2] = 0
        if self.displayChange and self.jogmode < 10:
            self.display_page1()
        elif self.displayChange and self.jogmode >= 10:
            self.display_page2()
        
        if utime.ticks_us() - self.led_lasttime > 500000:
            self.led.toggle()
            self.led_lasttime = utime.ticks_us()
            self.wdt.feed()
        #utime.sleep_ms(50)
            
    def dprint(self, msg):
        if self.debug:
            print(msg)


if __name__ == "__main__":
    run_axis = cnc_axis(PIN_DT, PIN_CLK, PIN_SW1, PIN_SW2, PIN_SW3, PIN_SW4, PIN_I2CCLK, PIN_I2CDT)
    while True:
        run_axis.loop()
