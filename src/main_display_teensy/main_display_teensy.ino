/*
 *   Software for Teensy 3.2 (with MK20DX256VLH7, 3,3V)
 *   
 *   used libs:
 *    - https://github.com/KrisKasprzak/FlickerFreePrint
 *    - https://github.com/KrisKasprzak/ILI9341_t3_controls
 *    - https://github.com/KrisKasprzak/ILI9341_t3_Menu
 *    - https://github.com/ardnew/XPT2046_Calibrated
 *    - Bugtton      Version 1.0.5
 *    - MicroDebug   Version 1.1
 * 
 *  free Pins:
 *  - 4-6, 16-23
 *  used Pins:
 *  - Buttons left  ->  Pin 14          DROkey= 0
 *            left1 ->  Pin 15                = 1
 *            right1->  Pin 16                = 2
 *            right ->  Pin 17                = 3
 *            mpg mode -> Pin 18              = 4
 *            
 *  - LED
 *            mpg mode -> Pin 19            // related function: MyDisplay_LedMPG
 *            
 *  - Uart
 +       for communication with the grblHAL:
 *            RX1 ->   Pin 0   weiss
 *            TX1 ->   Pin 1   rot
 *       rs485-Bus for communication with the mpg wheels and ext. switches:     main_loop call MPGPollSerial every 20ms
 *            RX3 ->   Pin 7   to rs485     (red)
 *            TX3 ->   Pin 8   to rs485     (white)
 *            
 *  - switch   
 *                      ->  Pin 16 (A2)     Analog Input used for joystick ....      -> not used 
 *
 *  - mydiplay   2,8" Display        
 *        ILI9341      -> Teensy:
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
 * Configuration:
 *      mydisplay.c:   mystate.lathe =  0 -> milling with U/min, x,z
 *                                      1 -> lathe   with x,y,z
 *      handwheel.ino   usedAxis[] = {'X', 'Y', 'Z', 'A'};
 *             
 *  Bugs:
 *      not known 
 *      
 *  to do:
 *      - evaluate grblGetInfo() and get version from GrblHAL
 *      - add calibratrion Menue for touch screen see C:\Users\model\Eigene Dokumente\Projecte\Elektronik\Arduino\libraries\XPT2046_Calibrated
 *      - add in mydisplax.Buttons variable which are manipula item e.q. target.x, target.y, target.z
 *      - use lathe variable from grblhalt and set mystate.lathe        #partly implemented, use additional main_display_teensy.h: #define LATHEMODE 
 *      - tastendruck hält alles an :-(   -> umbauen auf millis
        - mpg stop abfragen und auswerten !!
        - mpg slave id must be changed for each achse
        - start.taste am Display geht nicht mehr??
        - speed von mpg bei x10, x100 über Fz, Fx einstellungen
        - check axis A : handwheel unkomment Debug 'call DROmpgEvent'
*       - use 3,5" ILI9488  https://github.com/jaretburkett/ILI9488
*                           https://forum.pjrc.com/index.php?threads/tft-from-buydisplay-with-ili9488-and-ft6236-controllers-wiring.45316/
*                           https://forum.pjrc.com/index.php?threads/teensy-4-0-4-1-ili9488-lcd.66712/
 *
 * 
 */
#include "main_display_teensy.h"
#include <SerialDebug.h>
#include <avr/wdt.h>
#include "para_eeprom.h"
#include "dro.h"
#include "mydisplay.h"

#ifdef ILI9341
    #include "ILI9341_t3_Controls.h"
#endif
#ifdef ILI9488
    #include "ILI9488_t3_controls.h"
#endif

void watchdogSetup (void){} // muss definiert werden, denn die default Funktion macht ein: WDT_Disable (WDT);
// und das Disable verhindert den nächste enable see https://forum.arduino.cc/index.php?topic=450355.0

void setup() 
{  
   wdt_enable(WDTO_1S);       // Watchdog auf 1 s stellen 
   //watchdogEnable(1000);
   Serial.begin(115200);      //usb bus
   Serial1.begin(115200);     //grblhal bus                                                                                                                                                                                                                                                                                                                                                                                                                                  
   //Serial1.serial_putC(0x8B);   // ‹ 0x8B = 139 = '‹'   Enable MPG full control1    all $commands and Gcode commands have to be terminated with a CR
   Serial1.setTimeout(100);
   
   DEBUG("Start E-Gear");
//   read_mac();
//   DEBUG("MAC:");
//   print_mac();    
//   DEBUG(" ");  
// eeprom_write_default(); 
   eeprom_read();
   if (strcmp(eeprom.Copyright, "Heli2")!=0) {
      DEBUG("EEPROM init first");
      eeprom_write_default();
      eeprom_read();
   }
   DEBUG("EEPROM read ok");
   MyDisplay_init();
   DEBUG("Display init done");   
   //Switch_init();
   DROInitCanvas();
   DROShowCanvas();
   MPG_init();
   DEBUG("Setup Init done");
   grblDebug(2);
} 

bool tim50=false; 
bool tim25=false;
void loop() 
{   
    char buffer [60];
    unsigned long mylooptime = millis();

    if (mylooptime % 25 == 0 && tim25) {    //call every 25ms   
        MPGPollSerial();                    //defined in handwheel.ino
        tim25 = false;        
    }else if (mylooptime % 25 != 0)
        tim25 = true;

    if (mylooptime % 50 == 0 && tim50) {    //call every 50ms
        MyDisplay_loop();
        Switch_loop();                      // Query the switches
        tim50 = false;      
    }else if (mylooptime % 50 != 0)
        tim50 = true;
    DROProcessEvents();                     // processing from switches(key), MPG events and DRO events(than send CMD_STATUS_REPORT)
    DROSet_EventDRO();                      // every 200ms set EVENT_DRO (=5Hz, see https://github.com/gnea/grbl/wiki/Grbl-v1.1-Interface), ask for Status Report if mpg mode
    grblPollSerial();                       // new messages from Uart-Port?

    if (Serial.available() > 0) {
      Serial.readBytesUntil('\n', buffer, 50);       //see Possible commands https://www.sainsmart.com/blogs/news/grbl-v1-1-quick-reference 
      if (!strncmp(buffer, "0x", 2)) {
           uint8_t cmd = (uint8_t) strtol(buffer+2, NULL, 16);
           Serial1.write(cmd);
           grblDebug(5);
      }else if (!strncmp(buffer, "$I", 2)) {
          grblDebug(10);                            // display 10 status messages 
          DROGetInfo();
          DROprintOut();                            // get is_loaded, lathe, mpg, grblHAL MPG & DRO 
      }else if (!strncmp(buffer, "report", 6)) {
          Serial.println(String(buffer));
          serial_putC(CMD_STATUS_REPORT_ALL);
      }else if (!strncmp(buffer, "reset", 6)) {
          Serial.println(String(buffer));
          serial_putC(24); 
      }else if (!strncmp(buffer, "unlock", 6)) {
          serial0_writeLn("$X");
      }else {
          Serial.println(String(buffer));
          DROprintOut();
          grblDebug(-1);
          grblAwaitACK(buffer, 50);         //timeout after 50ms
          grblDebug(0);
      }
    }
    wdt_reset();
} 
