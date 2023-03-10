/*
 *   Software for Teensy 3.2 (mit MK20DX256VLH7, 3,3V)
 *   
 *   used libs:
 *    - https://github.com/KrisKasprzak/ILI9341_t3_controls
 *    - https://github.com/KrisKasprzak/ILI9341_t3_Menu
 * 
 *  free Pins:
 *  - 3-6, 16-23
 *  used Pins:
 *  - Buttons left  ->  Pin 14
 *            right ->  Pin 15
 *                  ->  Pin 16 
 *            
 *  - Uart
 *            RX1 ->   Pin 0   weiss
 *            TX1 ->   Pin 1   rot
 *            RX3 ->   Pin 7   to rs485     (red)
 *            TX3 ->   Pin 8   to rs485     (white)
 *            
 *  - switch
 *                      ->  Pin 16 (A2)     Analog Input used for joystick ....
 *
 *  - mydiplay   2,8" Display        
 *         ILI9341      -> Teensy:
 *            VCC       -> Pin VIN 1     5V/3.3V power input
 *            2         -> Pin GND       Ground
 *            3         -> Pin 10        CS  LCD chip select signal, low level enable
 *            4         -> Pin +3.3V     RESET   LCD reset signal, low level reset
 *            5         -> Pin 9         DC/RS   LCD register / data selection signal, high level: register, low level: data
 *            6         -> Pin 11        SDI(MOSI)   SPI bus write data signal
 *            7         -> Pin 13        SCK   SPI bus clock signal
 *            8         -> Vin+110Ohm    LED   Backlight control, high level lighting, if not controlled, connect 3.3V always bright
 *            9         -> 12            SDO(MISO)   SPI bus read data signal, if you do not need to the read function, you can not connect it
 *          TFT ILI9341:
 *           10         -> Pin 13        T_CLK   Touch SPI bus clock signal
 *           11         -> Pin 3         T_CS    Touch screen chip select signal, low level enable
 *           12         -> Pin 11        T_DIN   Touch SPI bus input
 *           13         -> Pin 12        T_DO    Touch SPI bus output
 *           14         -> Pin 2         T_IRQ   Touch screen interrupt signal, low level when touch is detected 
 *            
 *           
 *             
 *  Bugs:
 *      not known 
 *      
 *  to do:
 *      - tastendruck h??lt alles an :-(   -> umbauen auf millis

        - mpg stop abfragen und auswerten !!
        - start.taste am Display geht nicht mehr??
        - speed von mpg bei x10, x100 ??ber Fz, Fx einstellungen
 *
 * 
 */
#include <SerialDebug.h>
#include <avr/wdt.h>
#include "para_eeprom.h"
#include "ILI9341_t3_Controls.h"
#include "dro.h"
#include "mydisplay.h"

void watchdogSetup (void){} // muss definiert werden, denn die default Funktion macht ein: WDT_Disable (WDT);
// und das Disable verhindert den n??chste enable see https://forum.arduino.cc/index.php?topic=450355.0

void setup() 
{  
   wdt_enable(WDTO_1S);       // Watchdog auf 1 s stellen 
   //watchdogEnable(1000);
   Serial.begin(115200);      //usb bus
   Serial1.begin(115200);     //grblhal bus                                                                                                                                                                                                                                                                                                                                                                                                                                  
   //Serial1.serial_putC(0x8B);   // ??? 0x8B = '???'   Enable MPG full control1    all $commands and Gcode commands have to be terminated with a CR
   Serial1.setTimeout(100);
   
   DEBUG("Start E-Gear");
//   read_mac();
//   DEBUG("MAC:");
//   print_mac();    
//   DEBUG(" ");   
   eeprom_read();
   if (strcmp(eeprom.Copyright, "Heli2")!=0) {
      DEBUG("EEPROM init first");
      eeprom_write_default();
      eeprom_read();
   }
   MyDisplay_init();
   //Switch_init();
   DROInitCanvas();
   DROShowCanvas();
   MPG_init();
} 

bool tim20=false; 
void loop() 
{   
    char buffer [60];
    String sbuffer;
    unsigned long mylooptime = millis();
    //const char *data

    if (mylooptime % 20 == 0 && tim20) {    //call every 20ms
        MyDisplay_loop();
        Switch_loop();
        tim20 = false;      
        MPGPollSerial();
    }else if (mylooptime % 20 != 0){
        tim20 = true;
    }
    DROProcessEvents();
    grblPollSerial();

    if (Serial.available() > 0) {
      Serial.readBytesUntil('\n', buffer, 50);       //buffer = array of char or byte.
      sbuffer = String(buffer);
      Serial.println(sbuffer);
      Serial1.println(sbuffer);
    }
    wdt_reset();
} 
