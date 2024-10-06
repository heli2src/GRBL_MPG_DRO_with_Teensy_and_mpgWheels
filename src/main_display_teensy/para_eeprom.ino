
#include <EEPROM.h>


void  eeprom_read(void){
    EEPROM.get( 0, eeprom );
    DEBUG(eeprom.Copyright, eeprom.Version);
    DEBUG("Background",eeprom.BGred,eeprom.BGgreen,eeprom.BGblue);
    DEBUG("Default",eeprom.fzmin);
}

void eeprom_write(void){
    EEPROM.put( 0, eeprom );  
}

void eeprom_write_default(void){
    strcpy(eeprom.Copyright, "Heli2");
    strcpy(eeprom.Version, "V0.0.17");
    eeprom.BGred = 0;
    eeprom.BGgreen = 0;
    eeprom.BGblue = 0;
    eeprom.fxmin = 500.0;
    eeprom.fymin = 500.0;
    eeprom.fzmin = 500.0;
    eeprom.fxU = 0.1;
    eeprom.fzU = 0.1;
    eeprom.fzjog001  = 100;
    eeprom.fzjog01 = 500;   
    eeprom.fzjog1 = 2000;

    eeprom_write();
}
