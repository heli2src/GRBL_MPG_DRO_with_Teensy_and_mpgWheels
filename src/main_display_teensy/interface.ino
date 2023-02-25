

#include "interface.h"

//interface_t interface = {0};
//
//// Serial interface
//__attribute__((weak)) void serial_init (void) {}
//__attribute__((weak)) int16_t serial_getC (void) { return -1; }
//__attribute__((weak)) void serial_writeS (void) {}
//__attribute__((weak)) void serial_writeLn (const char *data) {}
//__attribute__((weak)) bool serial_putC (const char c) { return false; }
//__attribute__((weak)) void serial_RxCancel (void);


//
// serialGetC - returns -1 if no data available
//


void serial_RxCancel (void)
{
  while (Serial1.available() > 0)
      Serial1.read(); 
}

bool serial_putC (const char c){
    Serial1.write(c);  
    return true;
}


int16_t serial_getC (void)
{
    int16_t data;

    if (Serial1.available() == 0)
        return -1; // no data available else EOF
    data = Serial1.read();
    return data;
}


void serial_writeLn (const char *data)
{
  Serial1.println(data);  
}

void serial0_writeLn (const char *data)
{
  Serial.println(data);  
}

void serial0_write(const char data)
{
  Serial.write(data);  
}


unsigned long lcd_systicks(void)
{
    unsigned long data;
    data = millis();
    return data;
}
