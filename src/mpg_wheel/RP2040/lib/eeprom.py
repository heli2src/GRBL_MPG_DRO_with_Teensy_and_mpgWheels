
import os
import json

class EEPROM():
    
    filename = '../eeprom.dat'
    
    def __init__(self):
        if self.exist():
            with open(self.filename, 'r') as f:
                self.values = json.load(f)
        else:
            self.values = {}
            
    def write(self):
        with open(self.filename, 'w') as f:
            json.dump(self.values, f)
            
    def exist(self):
        return self.filename.split('/')[1] in os.listdir('..')
            
    def __repr__(self):
        return self.values


if __name__ == "__main__":
    def eeprom_default(eeprom):
        eeprom.values = {'board': {'t3l': {'low': 0x0000,
                                             'mid': 0x4000,
                                             'high': 0xa000}
                                     }
                          }
        eeprom.write()
        print('eeprom set to default values')

    eeprom = EEPROM()
    eeprom_default(eeprom)
    
    print(eeprom)
    print(eeprom.values['board']['t3l']['low'])
    