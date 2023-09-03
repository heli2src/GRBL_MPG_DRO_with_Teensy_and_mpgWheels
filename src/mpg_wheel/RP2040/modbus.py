"""

some points from https://github.com/pyhys/minimalmodbus
"""

__author__ = "Christian Jung"
__license__ = "Apache License, Version 2.0"
__status__ = "Development"
__url__ = ""
__version__ = "0.0.2"

import sys
import utime
from machine import UART, Pin


# ############### #
# Named constants #
# ############### #

MODE_RTU = "rtu"
MODE_ASCII = "ascii"

_errortype = ['crc fail']

_CRC16TABLE = (
    0, 49345, 49537, 320, 49921, 960, 640, 49729, 50689, 1728, 1920, 51009, 1280, 50625, 50305, 1088,
    52225, 3264, 3456, 52545, 3840, 53185, 52865, 3648, 2560, 51905, 52097, 2880, 51457, 2496, 2176,
    51265, 55297, 6336, 6528, 55617, 6912, 56257, 55937, 6720, 7680, 57025, 57217, 8000, 56577, 7616,
    7296, 56385, 5120, 54465, 54657, 5440, 55041, 6080, 5760, 54849, 53761, 4800, 4992, 54081, 4352,
    53697, 53377, 4160, 61441, 12480, 12672, 61761, 13056, 62401, 62081, 12864, 13824, 63169, 63361,
    14144, 62721, 13760, 13440, 62529, 15360, 64705, 64897, 15680, 65281, 16320, 16000, 65089, 64001,
    15040, 15232, 64321, 14592, 63937, 63617, 14400, 10240, 59585, 59777, 10560, 60161, 11200, 10880,
    59969, 60929, 11968, 12160, 61249, 11520, 60865, 60545, 11328, 58369, 9408, 9600, 58689, 9984,
    59329, 59009, 9792, 8704, 58049, 58241, 9024, 57601, 8640, 8320, 57409, 40961, 24768, 24960, 41281,
    25344, 41921, 41601, 25152, 26112, 42689, 42881, 26432, 42241, 26048, 25728, 42049, 27648, 44225,
    44417, 27968, 44801, 28608, 28288, 44609, 43521, 27328, 27520, 43841, 26880, 43457, 43137, 26688,
    30720, 47297, 47489, 31040, 47873, 31680, 31360, 47681, 48641, 32448, 32640, 48961, 32000, 48577,
    48257, 31808, 46081, 29888, 30080, 46401, 30464, 47041, 46721, 30272, 29184, 45761, 45953, 29504,
    45313, 29120, 28800, 45121, 20480, 37057, 37249, 20800, 37633, 21440, 21120, 37441, 38401, 22208,
    22400, 38721, 21760, 38337, 38017, 21568, 39937, 23744, 23936, 40257, 24320, 40897, 40577, 24128,
    23040, 39617, 39809, 23360, 39169, 22976, 22656, 38977, 34817, 18624, 18816, 35137, 19200, 35777,
    35457, 19008, 19968, 36545, 36737, 20288, 36097, 19904, 19584, 35905, 17408, 33985, 34177, 17728,
    34561, 18368, 18048, 34369, 33281, 17088, 17280, 33601, 16640, 33217, 32897, 16448)

# Several instrument instances can share the same serialport
_serialports = {}  # Key: port name (str), value: port instance
_latest_read_times_byte = {}  # Key: port name (str), value: timestamp (float)


class Modbus:
    """Modbus for slaves.

    Uses the Modbus RTU or ASCII protocols (via RS485 or RS232).
    1 Start-bit, 8 Data bits, 1 Stop-bit = 10 Bits

    Args:
        * port (str): The serial (Uart) port 0/1.
        * slaveaddress (int): Slave address in the range 1 to 247 (use decimal numbers,
          not hex). Address 0 is for broadcast, and 248-255 are reserved.
        * mode (str): Mode selection. Can be MODE_RTU or MODE_ASCII.
        * tx (Pin): UART0 can be mapped to GPIO 0/1, 12/13 and 16/17,
          and UART1 to GPIO 4/5 and 8/9.
        * rx (Pin): "
        * debug (bool): Set this to :const:`True` to print the communication details

    """
    
    def __init__(
        self,
        port,
        slaveaddress,
        mode=MODE_RTU,
        baudrate = 19200,
        tx=Pin(0),
        rx=Pin(1),
        debug=False,
    ):
        """Initialize modbus and open corresponding serial port."""
        self.address = slaveaddress
        """Slave address (int). Most often set by the constructor
        (see the class documentation). """

        self.mode = mode
        """Slave mode (str), could be only MODE_RTU, MODE_ASCII not yet implemented
        Most often set by the constructor (see the class documentation).

        Changing this will not affect how other instruments use the same serial port.

        """
        self.debug = debug
        """Set this to :const:`True` to print the communication details.
        Defaults to :const:`False`.

        Most often set by the constructor (see the class documentation).

        Changing this will not affect how other instruments use the same serial port.
        """
        
        self.serial = None
        if port not in _serialports or not _serialports[port]:
            self._print_debug("Create serial port {}".format(port))
            self.serial = _serialports[port] = UART(
                port,
                baudrate=baudrate,
                parity=None,
                stop=1,
                tx=tx,
                rx=rx
            )
        else:
            self._print_debug("Serial port {} already exists".format(port))
            self.serial = _serialports[port]
        self._latest_read_times_byte = 0
        self._latest_read_times_protocol = 0
        self.timeout_time = int(1750 * 3.5 )
        self.count = 0
        self.rw = 1
        self.regMemory = []
        self.rxbuffer = bytes()
            
    def _print_debug(self, text):
        if self.debug:
            sys.stdout.write("Modbus debug mode: " + text)

    def _calculate_crc_string(self, inputstring):
        """Calculate CRC-16 for Modbus.

        Args:
            inputstring (str): An arbitrary-length message (without the CRC).

        Returns:
            A two-byte CRC string, where the least significant byte is first.

        """
        # Preload a 16-bit register with ones
        register = 0xFFFF
        for char in inputstring:
            register = (register >> 8) ^ _CRC16TABLE[(register ^ int(char)) & 0xFF]
        return register

    def _calculate_lrc_string(self, inputstring):
        """Calculate LRC for Modbus.

        Args:
            inputstring (str): An arbitrary-length message (without the beginning
            colon and terminating CRLF). It should already be decoded from hex-string.

        Returns:
            A one-byte LRC bytestring (not encoded to hex-string)

        Algorithm from the document 'MODBUS over serial line specification and
        implementation guide V1.02'.

        The LRC is calculated as 8 bits (one byte).

        For example a LRC 0110 0001 (bin) = 61 (hex) = 97 (dec) = 'a'. This function will
        then return 'a'.

        """
        register = 0
        for character in inputstring:
            register += int(character)
        lrc = ((register ^ 0xFF) + 1) & 0xFF
        return lrc

    def txmsg(self, msg):
        for dat in msg:
            self.serial.write(dat.to_bytes(1,'little'))            
        
    def rxChar(self):
        rxData = None
        if self.serial.any() > 0:
            rxData = self.serial.read(1)
            self._latest_read_times_byte = utime.ticks_us()
        return rxData

    def rxAny(self):
        rxData = bytes()
        while self.serial.any() > 0:
            rxData += self.serial.read(1)
            self._latest_read_times_byte = utime.ticks_us()
        return rxData
    
    def receive(self):
        if utime.ticks_us()-self._latest_read_times_byte>self.timeout_time:           # hit timeout 
            self.rxbuffer = bytes()
            self.count = 0
            # self.dprint(f'{self.__class__.__name__}.receive: timeout > {self.timeout_time}')
        rxData = self.rxChar()
        if rxData is not None and self.count != -20:
            #self.dprint(f'{self.count}: {rxData}')
            self.rxbuffer += rxData
            self.count += 1
            if self.count==1 and self.rxbuffer[0] == self.address:
                if rxData > b'\x04':
                    self.rw = 0
                else:
                    self.rw = 1
            elif self.count == 1:                    #wrong adress, wait till timeout
                self.count = -20
            elif self.rw==1 and self.count ==8 and self.rxbuffer[1]==3:
                if self._calculate_crc_string(self.rxbuffer) == 0:
                    regaddr = int(self.rxbuffer[2]<<8) + int(self.rxbuffer[3])
                    regcnt = int(self.rxbuffer[4]<<8) + int(self.rxbuffer[5])
                    return self.send_dat(regaddr, regcnt)
                else:
                    self.error = _errortype[0]
                    self.dprint(f'{self.error} {self.rxbuffer}') 
                    # print(self.error)
                self.count = 0
                self.rxbuffer = bytes()
            elif self.rw==0 and self.count > 9:
                self.dprint(f'{self.__class__.__name__}.write {self.rxbuffer} not implemented yet')
        return None
                
    def send_dat(self, regaddr, regcnt):
        answer = self.rxbuffer[0:2] + (regcnt << 1).to_bytes(1,'big')
        datum = bytes()
        for i in range(0, regcnt):
            if regaddr+i >= len(self.regMemory):
                return            
            dat = self.regMemory[regaddr+i]
            datum += dat.to_bytes(2,'big')
        answer += datum
        answer += self._calculate_crc_string(answer).to_bytes(2,'little')        
        self.dprint(f'{self._latest_read_times_byte-self._latest_read_times_protocol} read adr={regaddr}, {regcnt}-> send {answer}')
        self._latest_read_times_protocol = self._latest_read_times_byte
        self.txmsg(answer)
        return datum
        
    def dprint(self, msg):
        if self.debug:
            print(msg)

                
if __name__ == '__main__':
    client = Modbus(port=0, slaveaddress=3)
    client.regMemory=[0xAAAA, 0x5555, 0x1234, 0x5678, 0]
    lasttime = 0
    while True:
         if utime.ticks_us() - lasttime > 500:             
             lasttime = utime.ticks_us()
             client.receive()
             delta = utime.ticks_us() - lasttime
             if delta> 160:
                 print(utime.ticks_us() - lasttime)
         