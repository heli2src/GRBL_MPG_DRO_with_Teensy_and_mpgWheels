'''
Created on 11.01.2023

@author: Christian Jung

Austesten des RS485-Bus für das Handrad
Schnittstelle über COM5,
'''


# import tkinter as tk
from time import sleep
import minimalmodbus


class PCmodbusServer(object):

    register = {
                'axis':  {'adr': 2, 'dat': ord('X')},
                'puls':  {'adr': 1, 'dat': 0},
                'key':  {'adr': 2, 'dat': 1},       # actual configured amps value, RW
                }

    terror = {'errorcnt': 'error ',
              'writebus': 'write: ',
              'ok': 'ok: '}

    MAXREADMODBUS = 1
    MAXERROR = 10

    def __init__(self, device, port):
        self.port = port
        self.errorcnt = 0
        self.interface = self.deviceOpen(device)

    def _check4error(self, reg, dat, typ, ex=None):
        # self.logger.debug(" check4error reg={}, dat= {}".format(reg, dat))
        if dat is not None:
            self.errormsg = self.terror['ok']
            self.errorcnt = 0
        else:
            self.errormsg = self.terror[typ]
            msg = None
            if ex is not None:
                msg = "An exception of type {0} occurred.".format(ex)
            # self.logger.error(self.errormsg + msg)
            print(self.errormsg + msg)
            self.errorcnt += 1
            if self.errorcnt < self.MAXERROR:
                dat = self.register[reg]['dat']         # use old value....
            else:
                self.errormsg = self.terror['errorcnt']
                self.error = True
                dat = -1
        return dat

    def read(self, reg=None):
        if reg is None:
            registers = self.register
        elif isinstance(reg, list):
            registers = reg
        else:
            registers = [reg]
        for reg in registers:                # read all register and save the value to dat
            dat = None
            for cnt in range(0, self.MAXREADMODBUS):
                try:
                    dat = self.interface.read_register(self.register[reg]['adr'])
                except Exception as ex:
                    error = ex
                # self.logger.debug("Read register {} = {}".format(reg, dat))
                print("Read register {} = {}".format(reg, dat))
                if dat is not None:
                    error = None
                    break
                sleep(0.5)
            dat = self._check4error(reg, dat, 'readbus', error)
            self.register[reg].update({'dat': dat})
        return dat              # return with last read value

    def write(self, reg, value):
        register = self.register[reg]
        print("Write register {} {} = {}".format(reg, register['adr'], value))
        # self.logger.debug("Write register {} {} = {}".format(reg, register['adr'], value))
        for cnt in range(0, self.MAXREADMODBUS):
            try:
                self.interfacewrite_register(register['adr'], value)
                register['date'] = value
                error = None
            except Exception as ex:
                error = ex
        if error is not None:
            self._check4error(reg, None, 'writebus', error)

    def deviceOpen(self, device):
        try:                                                # intialise modbus
            minimalmodbus.TIMEOUT = 0.5
            interface = minimalmodbus.Instrument(device, self.port, mode='rtu', debug=False)
            interface.serial.baudrate = 38400     # 19200
        except Exception as ex:
            self._init_complete = False
            print("can't configure modbus : {}".format(ex))
            return
        self._init_complete = True
        return interface

    def twos_comp(self, val, bits):
        """compute the 2's complement of int value val"""
        if (val & (1 << (bits - 1))) != 0:     # if sign bit is set e.g., 8bit: 128-255
            val = val - (1 << bits)            # compute negative value
        return val


if __name__ == '__main__':
    device = 'COM3'
    xAxis = PCmodbusServer(device, 1)
    zAxis = PCmodbusServer(device, 3)
    xAxis.interface.debug = False
    zAxis.interface.debug = False
 #   server.write('axis', ord('X'))
    Xoldcnt = -1
    Zoldcnt = -1
    resultX = resultZ = None
    while True:
        for i in range(0, 1):
            try:
                resultX = xAxis.twos_comp(xAxis.interface.read_register(i), 16)
                print(f'Xcnt={resultX}')  # , Xdiff)
            except Exception:
                print('X read error')
                pass
            sleep(0.005)
            try:
                resultZ = zAxis.twos_comp(zAxis.interface.read_register(i), 16)
                #print(f'Zcnt={resultZ}')  # , Zdiff)
            except Exception:
                #print('Z read error')
                pass
            if i == 0:
                Xcnt = resultX
                Zcnt = resultZ
            else:
                Xdiff = resultX
                Zdiff = resultZ
            # sleep(0.05)
            sleep(1.0)
        if Xoldcnt != Xcnt:
            print(f'Xcnt={Xcnt}')  # , Xdiff)
            Xoldcnt = Xcnt
        if Zoldcnt != Zcnt:
            print(f'Zcnt={Zcnt}')  # , Zdiff)
            Zoldcnt = Zcnt

# 'read_bit', 'read_bits', 'read_float', 'read_long', 
# 'read_register', 'read_registers', 'read_string',
# 'write_bit', 'write_bits', 'write_float', 'write_long', 
# 'write_register', 'write_registers', 'write_string']