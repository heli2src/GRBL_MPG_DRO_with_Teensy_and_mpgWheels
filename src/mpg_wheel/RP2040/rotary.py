import machine
import utime
import array
from machine import Pin
import micropython

class Rotary:
    
#    ROT_CW = 1
#    ROT_CCW = 2
#    SW_PRESS = 4
#    SW_RELEASE = 8
    
    def __init__(self,dt,clk,sw):
        self.dt_pin = Pin(dt, Pin.IN)
        self.clk_pin = Pin(clk, Pin.IN)
        self.sw_pin = Pin(sw, Pin.IN, Pin.PULL_UP)
        self.last_status = (self.dt_pin.value() << 1) | self.clk_pin.value()
        self.dt_pin.irq(handler=self.rotary_change, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )
        self.clk_pin.irq(handler=self.rotary_change, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )
        self.sw_pin.irq(handler=self.switch_detect, trigger=Pin.IRQ_FALLING | Pin.IRQ_RISING )
        self.handlers = []
        self.last_button_status = self.sw_pin.value()
        self.lasttime = utime.ticks_us()
        self.buttontime = 0
        self.arg = array.array('i',[0, 0, 1])
        self.increment = 1
        
    def rotary_change(self, pin):
        new_status = (self.dt_pin.value() << 1) | self.clk_pin.value()
        if new_status == self.last_status:
            return
        transition = (self.last_status << 2) | new_status
        if transition == 0b1110:
            self.arg[0] += self.increment
        elif transition == 0b1101:
            self.arg[0] -= self.increment    
        if transition == 0b1110 or transition == 0b1101:
            self.newtime = utime.ticks_us()
            diff = utime.ticks_diff(self.newtime, self.lasttime)
            self.arg[1] = diff if diff<3000000 else 3000000
            self.lasttime = self.newtime
            micropython.schedule(self.call_handlers, [self.arg])            
        self.last_status = new_status
        
    def switch_detect(self,pin):
        if self.last_button_status == self.sw_pin.value():
            return
        self.last_button_status = self.sw_pin.value()
        dtime = utime.ticks_ms()-self.buttontime
        if self.buttontime == 0 and not self.sw_pin.value():
            self.buttontime = utime.ticks_ms()
            return
        elif self.sw_pin.value() and (dtime > 100 and dtime < 1000):
            if self.increment == 1:
                self.increment = 10
            elif self.increment == 10:
                self.increment = 100
            elif self.increment == 100:
                self.increment = 1
            self.buttontime = 0
            self.arg[2] = self.increment
        elif self.sw_pin.value() and dtime > 1000:
            self.buttontime = 0
            self.arg[0] = 0
            self.arg[1] = 0
        micropython.schedule(self.call_handlers, [self.arg])
                        
    def add_handler(self, handler):
        self.handlers.append(handler)
    
    def call_handlers(self, value):
        for handler in self.handlers:
            handler(value)