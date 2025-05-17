#include "handwheel.h"

// modbus https://forum.pjrc.com/threads/37018-Modbus-RTU/page2?s=0b70591297bc806d553c15a620b15758
// https://www.pjrc.com/teensy/td_uart.html

char usedAxis[] = {'X', 'Y', 'Z', 'A'};
byte paxis = 0x01;
bool busy = false;
byte readMpg[] = { 0x03, 0x03, 0x00, 0x00, 0x00, 0x03,0x85, 0xE8};       // Modbus protocoll 2,2ms sendtime
//                  |     |      |          |           |- CRC msb/lsb
//                  |     |      |          |- Quantity of registers msb/lsb
//                  |     |      |- Start Adress msb/lsb
//                  |     |- 0x03 = function code:  read Multiple Holding Registers (16-bit)
//                  |slave ID, must be changed for each achse





void MPG_init(void) {
//RS485SERIAL.setTX(8);
//RS485SERIAL.setRX(7);
RS485SERIAL.begin(38400);     //modubus rs485

// addMemoryForRead( buffer, size);
//addMemoryForWrite( buffer, size);
}

uint _calculate_crc_string(byte* buffer, uint length){
  uint result = 0xFFFF;
  for(unsigned int i = 0; i<length; i++) {
    result = (result >> 8) ^ _CRC16TABLE[(result ^ int(buffer[i])) & 0xFF];
  }
  return(result);
}

void MPGPollSerial(void){
  int *mpgData;
  int *mpgSwitch;
    if (RS485SERIAL.available()){
      mpg_data.counter = 0;
      while (RS485SERIAL.available()){
        char c = RS485SERIAL.read();             //receive b'\x03\x03\x02\x00\x00\xc1\x84'
        if(mpg_data.counter < MPG_BLOCK_LENGTH - 1)
          mpg_data.block[mpg_data.counter++] = c;  
      }
      mpg_data.latest_read_time = millis();
      if (mpg_data.counter ==11 && mpg_data.block[1] == readMpg[1] && mpg_data.block[2] == 6) {     
          uint result = _calculate_crc_string(mpg_data.block, mpg_data.counter);
          if (result == 0 &&  (mpg_data.block[0] >= 0) &&  (mpg_data.block[0] <=4)){        //  Axis 0 = X, 1 = Y, 2 = Z
             int mpgValue = int(mpg_data.block[3] << 8) + int(mpg_data.block[4]);
             int dtime = int(mpg_data.block[5] << 8) + int(mpg_data.block[6]);
             int vswitch  = int(mpg_data.block[7] << 8) + int(mpg_data.block[8]);
             if (mpgValue > 32768) mpgValue= mpgValue-65536;
             //if (mpg_data.block[0] == 1) { // and  vswitch !=0) {DEBUG("read", int(mpg_data.block[0]), mpgValue, dtime, vswitch, millis()); }
             switch (mpg_data.block[0]){
                case 1: mpgData = &mpg_data.x;
                        mpgSwitch = &mpg_data.x_switch;
                        break;
                case 2: mpgData = &mpg_data.y;
                        mpgSwitch = &mpg_data.y_switch;
                        break;
                case 3: mpgData = &mpg_data.z;
                        mpgSwitch = &mpg_data.z_switch;
                        break;
                //case 4: mpgData = &mpg_data.a;break;                 
                default: mpgData = 0;
             }
             if (mpgValue != *mpgData) {
               if (mpgValue == 0 and (abs(mpgValue-*mpgData) > 2))       // mpgwheel get a reset or power off/on, but also 0 cnt :-(
                   *mpgData = 0;
               else{
                   // DEBUG("call DROmpgEvent: axis = ", int(mpg_data.block[0]), "cnt =", mpgValue, *mpgData); 
                   DROmpgEvent(true, mpg_data.block[0]+'X'-1, mpgValue-(*mpgData), dtime);          
                   *mpgData = mpgValue;
               }
             }
             if (vswitch != *mpgSwitch){
                  *mpgSwitch = vswitch;
                  if (vswitch > 0)
                      DROkeyEvent(true, vswitch);
             }             
          }else{
             mpg_data.counter = 0;
             DEBUG("MPGPollSerial crc error, result", result, "mpg_data.block[0]", int(mpg_data.block[0]), "axis", int(readMpg[0]));
          }
          busy = false;
      }
    }else if (mpg_data.latest_read_time-millis() > timeout) {
 //     DEBUG("Missing", readMpg[0], millis());
      mpg_data.counter = 0;
      busy = false;
    }
    if (!busy && RS485SERIAL.availableForWrite()) {
      readMpg[0] = usedAxis[paxis] - 'X' +1;
      uint crc = _calculate_crc_string(readMpg, 6);             
      readMpg[6] = crc & 0x00FF;             // crc-lsb
      readMpg[7] = (crc & 0xFF00) >>8;       // crc-lsb
      //DEBUG("write", readMpg[0], millis());
      for(unsigned int i = 0; i<sizeof(readMpg); i++)
          RS485SERIAL.write(readMpg[i]);
      paxis++;
      if (paxis > sizeof(usedAxis)-1) paxis = 0;
      busy = true;
      mpg_data.latest_read_time = millis();
    }
}
