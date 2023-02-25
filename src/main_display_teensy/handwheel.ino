#include "handwheel.h"

// modbus https://forum.pjrc.com/threads/37018-Modbus-RTU/page2?s=0b70591297bc806d553c15a620b15758
// https://www.pjrc.com/teensy/td_uart.html


byte readMpg[] = { 0x03, 0x03, 0x00, 0x00, 0x00, 0x01, 0x85, 0xE8};
unsigned long timeout = 6;

void MPG_init(void) {
//RS485SERIAL.setTX(8);
//RS485SERIAL.setRX(9);
RS485SERIAL.begin(38400);     //modubus rs485

// addMemoryForRead( buffer, size);
//addMemoryForWrite( buffer, size);
}

uint _calculate_crc_string(char* buffer, uint length){
  uint result = 0xFFFF;
  for(unsigned int i = 0; i<length; i++) {
    result = (result >> 8) ^ _CRC16TABLE[(result ^ int(buffer[i])) & 0xFF];
  }
  return(result);
}

void MPGPollSerial(void){
    if (RS485SERIAL.availableForWrite()) {
      for(unsigned int i = 0; i<sizeof(readMpg); i++)
          RS485SERIAL.write(readMpg[i]);
    }
    if (RS485SERIAL.available()){
      mpg_data.counter = 0;      
      while (RS485SERIAL.available()){
        char c = RS485SERIAL.read();             //receive b'\x03\x03\x02\x00\x00\xc1\x84'
        if(mpg_data.counter < MPG_BLOCK_LENGTH - 1)
          mpg_data.block[mpg_data.counter++] = c;    
      }
      mpg_data.latest_read_time = millis();
      if (mpg_data.counter == 7 &&  mpg_data.block[0] == readMpg[0] && mpg_data.block[1] == readMpg[1] && mpg_data.block[2] == 2) {
          uint result = _calculate_crc_string(mpg_data.block, mpg_data.counter);
          if (result == 0){    
             int mpg = int(mpg_data.block[3] << 8) + int(mpg_data.block[4]);
             if (mpg > 32768) mpg= mpg-65536;
             if (mpg != mpg_data.x) {
               DROmpgEvent(true, 'Z', mpg-mpg_data.x);          
               mpg_data.x = mpg;
             }               
          }
      }
    }else if (mpg_data.latest_read_time-millis() > timeout) {
      mpg_data.counter = 0;
    }
}
