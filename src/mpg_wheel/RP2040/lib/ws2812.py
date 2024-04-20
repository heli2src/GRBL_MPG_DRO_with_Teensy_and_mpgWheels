from rp2 import PIO, StateMachine, asm_pio
from machine import Pin,

class ws2812():

    @asm_pio(sideset_init=PIO.OUT_LOW, out_shiftdir=PIO.SHIFT_LEFT, autopull=True, pull_thresh=24)
    def smws2812():
        T1 = 2
        T2 = 5
        T3 = 3
        label("bitloop")
        out(x, 1)               .side(0)    [T3 - 1] 
        jmp(not_x, "do_zero")   .side(1)    [T1 - 1] 
        jmp("bitloop")          .side(1)    [T2 - 1] 
        label("do_zero")
        nop()                   .side(0)    [T2 - 1]
        
    def __init__(self, ledpin):
        self.sm = StateMachine(0, self.smws2812, freq=8000000, sideset_base=Pin(ledpin))
        self.sm.active(1)
        self._on = False
        self.color = 255
        
    def toggle(self):
        if self._on:
            self.sm.put(0, 8)
        else:
            self.sm.put(self.color, 8)
        self._on = not self._on
        
    def on(self):
        self._on = True
        self.sm.put(self.color, 8)
         
    def off(self):
        self._on = False
        self.sm.put(0, 8)
        