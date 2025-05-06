'''
Handwheel with Encoder + Rasperry Pi Pico Zero

use https://micropython.org/download/rp2-pico/
        https://github.com/micropython/micropython
        MicroPython:  RPI_PICO-20240602-v1.23.0.uf2
        
    install necessary libs from https://github.com/micropython/micropython-lib:   (from your python shell and pip install mpremote)
        see https://docs.micropython.org/en/latest/reference/mpremote.html 
        C:\\Users\\model\\AppData\\Roaming\\Python\\Python39\\Scripts\\mpremote mip install ssd1306


   100 PPR Encoder  a -> GP10  (Pin12)
                    b -> GP11  (Pin11)
   
   4x switch 26, 15, 14, 13  to gnd
           SW1 = Drive axis:               regMemory[2] = axis*10 + 1
           SW2 = 0.01/0.1/1.0 mm
           SW3 = disable/config/joggin
           SW4 = drive to 0                regMemory[2] = axis*10 + 4
               = set to 0                  regMemory[2] = axis*10 + 8
   
   RS485           DI  (4) -> GP0 (TX0) Pin1
                   DE  (3) -> VDD          Transceiver Output Enable
                   REq (2) -> GND          Receiver Output Enable not
                   RO  (1) -> GP1 (RX0) Pin2
   
   SSD1306 with I2C   (128 x 64, 0.96" or 1.3")
                  clk -> GP9 I2C0 SCL (Pin9)
                  dat -> GP8 I2C0 SDA (Pin10)
                  
    WS2812 LED :      -> GP16
    
    Function:
            - poll every 500us the RS485 bus
               - if receive nothing: display 'no connection'
               - else send
                       - slave address (e.q 0x03)
                       - Function Code 0x03 = Multiple Holding Registers 
                       - The Data Address of the first register e.q: 0x06,0x00
                       - No. of bytes of response 0x00,0x00
                       - Databyte 0x00, 0x00
                       - Checksum 16bit crc
             - 4 Buttons
                - see SW1-SW4
             - rotary wheel
    
    todo:
       - disable button im setup mode
       - use dma for the data from SSD1306
       - display values from the axis
       - control mode only dummy yet
                   
'''
import os
import utime
import micropython
import framebuf
from machine import Pin, I2C, Timer, WDT, freq as CPUfreq
from lib.eeprom import EEPROM
from lib.rotary import Rotary
from lib.sh1106 import SH1106_I2C
from lib.modbus import Modbus
from lib.ws2812 import ws2812

__version__ = "V0.0.11"
#Configuration
#_____________________________

# GPIOs Rotary Encoder
PIN_DT = 10
PIN_CLK = 11

# GPIOs Display 1306 with I2C
PIN_I2CCLK = 9
PIN_I2CDT = 8

statusline = 52
linedistance = 10
cursorposition = 88
cursorlinemax = 3

EN_WADOG = True


# 'EEPROM'
eeprom_values = {'display': {'rotate': False,
                             'invert': False},
                 'rotary': 1,                        # 1 = normal rotary, -1 = invers
                 'axis': 1,                          # 0 = X, 1 = Y, 2 = Z
                 'keylayout': False                  # 
                }


class Cursor():
    
    def __init__(self, display, blinktime = 500):
        self.display = display
        self.blinktime = blinktime
        self.cursor = False
        self.cursorticks = utime.ticks_ms()
        self.line = 0
        
    def blink(self, page):
        #if self._line != line:
         #   self.display.hline(x+10, line*linedistance+8, 8, False)
         #   self.line = line
        if page != 2:
            return
        if utime.ticks_ms() - self.cursorticks > self.blinktime:
            self.display.hline(cursorposition, self.line*linedistance+8, 8, self.cursor)    # ~67us blocking                    
            self.display.show()                                           # ~25.85 ms blocking !!          
            self.cursor = not self.cursor
            self.cursorticks = utime.ticks_ms()

    def incline(self):
        self.display.hline(cursorposition, self.line*linedistance+8, 8, False)
        self.cursor = True
        self.line += 1
        if self.line == cursorlinemax:
            self.line = 0


class cnc_axis():
    
    regMemory = [0,         # absolut cnt
                 0,         # diff time in 100us
                 0,         # switch values
                 0]
    choice = {'Axis': ['X', 'Y', 'Z', 'A'],     # X=0, Y=1, Z=2, A=3
              }
    
    def __init__(self):
        self.debug = True
        micropython.opt_level(3)
        # micropython.alloc_emergency_exception_buf(100)
        CPUfreq(125_000_000)
        self.eeprom = EEPROM()
        if not self.eeprom.exist():
            self.eeprom_default(self.eeprom)

        if ws2812:
            self.led = ws2812(16)
            self.led.on()
        else:            
            self.led = Pin(25, Pin.OUT)
            self.led.value(0)

        # init display
        # i2c write is blocking :-(
        # use dma? see example https://github.com/raspberrypi/pico-examples/blob/master/dma/control_blocks/control_blocks.c
        i2c = I2C(0, scl=Pin(PIN_I2CCLK), sda=Pin(PIN_I2CDT), freq=200_000)
        self.display = SH1106_I2C(128, 64, i2c)        # use 1kb = 128 x 64/8: 0- 47 blue, 48- 63 yellow         
        if self.display is None:                        # if display not found:
            from lib.ssd1306_dummy import SSD1306_dummy
            self.display = SSD1306_dummy()
            print('SH1106 not found')
        self.cursor = Cursor(self.display)
        self.setdisplay_choice = 0
        self.init()
            
    def init(self):
        if self.eeprom.values['keylayout']:
            pin_sw1 = 26          # Start/Stop
            pin_sw2 = 15          # choice incValue
            pin_sw3 = 14          # disable, jog-mode, control mode or switch to setup page
            pin_sw4 = 13          # set to zero
        else:
            pin_sw1 = 13          # Start/Stop
            pin_sw2 = 14          # choice incValue
            pin_sw3 = 15          # disable, jog-mode, control mode or switch to setup page
            pin_sw4 = 26          # set to zero     
        
        self.axis = self.eeprom.values['axis']                     # 0 = X, 1 = Y, 2 = Z
        self.display.invert(self.eeprom.values['display']['invert'])
        self.display.rotate(self.eeprom.values['display']['rotate'])
        #tim = Timer()
        #tim.init(freq=1, mode=Timer.PERIODIC, callback=showValues)
        
        self.jogmode = 0                # 0-2 : button  disable, jsend b'\x03\x03\x06\x00\x00\x00\x00\x00\x008\x15'ogging, control, > 10: >4s switch to changing axis
        self.displayChange = True
        self.buttontime = 0        

        self.rotary = Rotary(self.eeprom.values['rotary'], PIN_DT, PIN_CLK, pin_sw2)      # Init Rotary Encoder, sw=choice increment 
        self.rotary.add_handler(self.rotary_changed)
        self.valueChange = False
        self.switchtime = 0
        
        sw1 = Pin(pin_sw1, Pin.IN, Pin.PULL_UP)                                            # Start/Stop button
        sw1.irq(handler=self.sw1_irq, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )
        sw3 = Pin(pin_sw3, Pin.IN, Pin.PULL_UP)
        sw3.irq(handler=self.sw3_irq, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )
        sw4 = Pin(pin_sw4, Pin.IN, Pin.PULL_UP)                                            # drive axis to 0/set axis to 0
        sw4.irq(handler=self.sw4_irq, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )

        self.client = Modbus(port=0, slaveaddress=self.axis+1, baudrate=38400, debug=True) # Init Modbus Slaveadress should be X,Y,Z = 88, 89, 90,
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
        self.display.text(f"       {__version__}", 10, 4)
        for y in range(4, 40, 10):
            self.display.text("Heli", y, y)       # ~310us
            self.display.text("2", y+33, y-4)
            self.display.show()                   # ~50760us,   interrupt form rotary will blocked :-(
            utime.sleep_ms(800)
        utime.sleep_ms(500)
        self.page = 1
        if EN_WADOG:
            self.wdt = WDT(timeout=1000)  # enable Watchdog, with a timeout of 1s
        
    def eeprom_default(self, eeprom):
        eeprom.values = eeprom_values
        eeprom.write()
        print('eeprom set to default values')

    def sw1_irq(self, pin):
        ''' Start/Stop button'''
        value = pin.value()
        if value== 0:
            self.buttontime = utime.ticks_ms()
            return
        if self.buttontime == 0:
            return
        dtime = utime.ticks_ms()- self.buttontime
        if self.jogmode < 10 and value == 1 and (dtime > 100 and dtime < 900):
            value = value + (self.axis+1) *10
            self.regMemory[2] = value
            print(f'       sw1 ={value}')  
            self.switchtime = utime.ticks_ms()

    def sw3_irq(self, pin):
        """
        Choose: Disable/jog-mode/control mode or switch to setup page (push button >4s)
        """
        value = pin.value()
        if value== 0:
            self.buttontime = utime.ticks_ms()
            return
        if self.buttontime == 0:
            return
        dtime = utime.ticks_ms()- self.buttontime
        if value == 1 and (dtime > 100 and dtime < 500):
            if self.jogmode >= 10 and self.jogmode-9 < len(self.choice['Axis']):
                self.cursor.incline()
            elif self.jogmode < 2:             # next state: Disable/jog-mode/control mode
                self.jogmode += 1
            else:
                self.jogmode = 0               # or start from begin
            self.displayChange = True
            self.buttontime = 0
        elif value == 1 and (dtime > 2000):                      # call or return from Setup page
            self.buttontime = 0
            self.displayChange = True
            self.jogmode = 0 if self.jogmode >= 10 else 10
            if self.jogmode == 0:
                self.eeprom.write()
                print(f'       write eeprom')
                self.init()
            print(f'       config {dtime}')

    def sw4_irq(self, pin):
        ''' Drive axis to 0/set axis to 0'''
        value = pin.value()
        if value== 0:
            self.buttontime = utime.ticks_ms()
            return
        if self.buttontime == 0:
            return
        dtime = utime.ticks_ms()- self.buttontime
        if self.jogmode < 10:
            if value == 1 and (dtime > 100 and dtime < 900):
                value = value * 4 + (self.axis+1) *10
                self.regMemory[2] = value
                # print(f'       sw4 ={value}')
                self.switchtime = utime.ticks_ms()
            elif value == 1 and (dtime > 900 and dtime < 3000):
                value = value * 8 + (self.axis+1) *10
                self.regMemory[2] = value
                # print(f'       sw4 ={value}')
                self.switchtime = utime.ticks_ms()

    def rotary_changed(self, value):
        if self.jogmode >= 10:
            self.setdisplay_choice = True
            return
        self.regMemory[0] = value[0][0]                           # absolut cnt
        self.regMemory[1] = value[0][1]  // 100                   # diff time in 100us
        self.valueChange = True
        if self.incValue != value[0][2]:
            self.incValue = value[0][2]
            self.displayChange = True
        # self.dprint(f'rotary_changed {self.jogmode}  {self.regMemory}')
            
    def display_page1(self):
        self.page = 1
        self.display.fill(0)
        self.display.text(f'{self.choice["Axis"][self.axis]}-axis', 0, 0)
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
        print(f"display_page2 {self.jogmode}")
        self.page = 2
        self.display.fill(0)
        self.rotary.enable(True)
        text = f'Axis:      {self.choice["Axis"][self.axis]}'
        self.display.text(text, 0, 0)
        text = f'Wheel:     {self.eeprom.values["rotary"]}'
        self.display.text(text, 0, 10)
        text = f'Keypad :   {self.eeprom.values["keylayout"]}'
        self.display.text(text, 0, 20)
        self.display.show()
        self.displayChange = False
        
    def display_choice(self):
        line = self.cursor.line
        if line == 0 :
            oldtext = self.choice["Axis"][self.axis]
            self.axis += 1
            if self.axis == len(self.choice['Axis']):
                self.axis = 0
            self.eeprom.values['axis'] = self.axis
            newtext = self.choice["Axis"][self.axis]
        elif line == 1:
            oldtext = self.eeprom.values['rotary']
            newtext = -oldtext
            self.eeprom.values['rotary'] = newtext
        elif line == 2:
            oldtext = self.eeprom.values['keylayout']
            newtext = not oldtext
            self.eeprom.values['keylayout'] = newtext
        oldtext = str(oldtext)
        newtext = str(newtext)
        self.display.text(oldtext, cursorposition, line*linedistance, 0)        # erase old text
        self.display.text(newtext, cursorposition, line*linedistance)
        self.display.show()
        self.setdisplay_choice = False
        

    def loop(self):
        if utime.ticks_us() - self.lasttime > 500:              # call every 500us
            self.lasttime = utime.ticks_us()
            result = self.client.receive()                      # read modbus
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
 #               self.display.text('*', 1, statusline-10)
 #               self.display.show()
 #               self.display_page1()
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
            if EN_WADOG:
                self.wdt.feed()
        self.cursor.blink(self.page)
        if self.setdisplay_choice:
            self.display_choice()    
        #utime.sleep_ms(50)
            
    def dprint(self, msg):
        if self.debug:
            print(msg)


if __name__ == "__main__":
    run_axis = cnc_axis()
    while True:
        run_axis.loop()
