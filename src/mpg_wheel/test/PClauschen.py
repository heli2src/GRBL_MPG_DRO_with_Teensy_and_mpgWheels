# -*- coding: utf-8 -*-
"""
Spyder Editor


Date:  28.1.2021
Author  C. Jung

einstellen auf die gewünschte COM,
und dann mit test.run(100) werden einkommende Daten gelesen

"""
import serial
import time


class lauschen(object):
    buffer = [0x03, 0x01, 0x00, 0x00, 0x00, 0x01]
    # [0x03, 0x01, 0x01, 0x01, 0x91, 0xf0]   Read Coils (0x01) 0

    def __init__(self, com):
        try:
            self.ser = serial.Serial(port=com, baudrate=9600, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
                                     stopbits=serial.STOPBITS_ONE, timeout=1, xonxoff=False, rtscts=False, dsrdtr=False, writeTimeout=2)
        except Exception:
            print("can't open")
            # quit()

    def ser_open(self, ser):
        ser.close()
        try:
            ser.open()
        except Exception:
            print('can not open')
            # quit()
        ser.flushInput()
        ser.flushOutput()
        return

    def run(self, cnt):
        if self.ser.isOpen():
            end = False
            for i in range(0, cnt):
                ch = self.ser.read()   # .decode()      #'utf-8'
                if ch != b'':
                    print('{} '.format(ch.hex()), end='')
                    end = False
                elif not end:
                    print()
                    end = True
                # print (ch.encode("hex"), 16)

    def write(self):
        for ch in self.buffer:
            self.ser.write(ch)
            print('{:2x}'.format(ch))


if __name__ == '__main__':
    test = lauschen('COM5')
    test.write()
    test.run(100)
    # for i in range(0,3):
    #    test.write()
    #    time.sleep(1)
    test.ser.close()
