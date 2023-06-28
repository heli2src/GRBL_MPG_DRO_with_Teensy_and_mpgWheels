
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
    strcpy(eeprom.Version, "V0.0.13");
    eeprom.BGred = 0;
    eeprom.BGgreen = 0;
    eeprom.BGblue = 0;
    eeprom.fzmin = 500.0;
    eeprom_write();
}
