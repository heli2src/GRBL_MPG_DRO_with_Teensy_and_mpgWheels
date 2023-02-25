# -*- coding: utf-8 -*-
"""
Created on Tue Jan 17 20:04:16 2023

@author: 49163
"""

import serial
import sys


class Uart(object):

    def __init__(self, port):
        try: 
            self.inst = serial.Serial(
                port=port,
                baudrate=9600,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=1,
                xonxoff=False,
                rtscts=False,
                dsrdtr=False,
                writeTimeout=.5
            )
        except Exception:
            print(f'Der serielle Port {port} konnte nicht geoeffnet werden')
            sys.exit()
        self.inst.flushInput() 								# Eingangspuffer leeren
        self.inst.flushOutput()								# Ausgangspuffer leeren


if __name__ == '__main__':
    # root = tk.Tk()
    # server=modbusserver(root,tk)
    ser = Uart('COM5')
    ser.inst.write('X'.encode())
    ser.inst.write(b'\x03\10\00')
    #ser.inst.readline()            # .encode("hex")
    
    ser.inst.close()