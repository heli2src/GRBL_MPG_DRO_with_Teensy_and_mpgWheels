/*
  The MIT License (MIT)
  Copyright (c) 2023 Christian Jung
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <SerialDebug.h>
#ifdef ILI9341
    #include "ILI9341_t3_Menu.h"      // custom utilities definition
    #include "ILI9341_t3.h"           // fast display driver lib            https://github.com/PaulStoffregen/ILI9341_t3
    #include "ILI9341_t3_Controls.h"  // https://github.com/KrisKasprzak/ILI9341_t3_controls
    #include <font_Arial.h>           // custom fonts that ships with ILI9341_t3.h
    #include <font_ArialBold.h>       // custom fonts for the ILI9341_t3.h    
#endif
#ifdef ILI9488
    #include "ILI9488_t3_Menu.h"      // custom utilities definition
    #include "ILI9488_t3.h"           // fast display driver lib            https://github.com/PaulStoffregen/ILI9488_t3
    #include "ILI9488_t3_controls.h"  // https://github.com/KrisKasprzak/ILI9488_t3_controls
    #include "ili9488_t3_font_Arial.h"
    #include "ili9488_t3_font_ArialBold.h"
#endif

#include <XPT2046_Calibrated.h>   // increase Z_THRESHOLD in XPT2046_Calibrated.cpp if touchscreen is to sensitiv, add line 129: if (z > 4095) z = 0;
#include "myTouchCalibration.h"
#include <FlickerFreePrint.h>     // library to draw w/o flicker        https://github.com/KrisKasprzak/FlickerFreePrint

#include "dro.h"
#include "error_alarmcodes.h"

XPT2046_Calibrated Touch(TS_CS_PIN);    // , TS_IRQ_PIN);


const uint16_t  C_VALUES[] = {  0XFFFF, 0X0000, 0XC618, 0X001F, 0XF800, 0X07E0, 0X07FF, 0XF81F, //7
                                0XFFE0, 0X0438, 0XFD20, 0XF81F, 0X801F, 0XE71C, 0X73DF, 0XFBAE, //15
                                0X7FEE, 0X77BF, 0XFBB7, 0XF7EE, 0X77FE, 0XFDEE, 0XFBBA, 0XD3BF, //23
                                0X7BCF, 0X1016, 0XB000, 0X0584, 0X04B6, 0XB010, 0XAD80, 0X0594, //31
                                0XB340, 0XB00E, 0X8816, 0X4A49, 0X0812, 0X9000, 0X04A3, 0X0372, //39
                                0X900B, 0X94A0, 0X0452, 0X92E0, 0X9009, 0X8012 //45
                             };
// set default colors
uint16_t MENU_TEXT = C_VALUES[1];
uint16_t MENU_BACKGROUND = C_VALUES[0];
uint16_t MENU_HIGHLIGHTTEXT = C_VALUES[1];
uint16_t MENU_HIGHLIGHT = C_VALUES[21];
uint16_t MENU_HIGHBORDER = C_VALUES[10];
uint16_t MENU_SELECTTEXT = C_VALUES[0];
uint16_t MENU_SELECT = C_VALUES[4];
uint16_t MENU_SELECTBORDER = C_VALUES[37];
uint16_t MENU_DISABLE = C_VALUES[2];
uint16_t TITLE_TEXT = C_VALUES[13];
uint16_t TITLE_BACK = C_VALUES[36];

#define LED_MPG              19

#define ROW_HEIGHT          64          // 35
#define ROWS                3           // 5
#define DATA_COLUMN         230         // 190

//#define LATHE           true
#define COLUMN1         60
#define COLUM_DISTANCE  50

// easy way to include fonts but change globally
#define FONT_SMALL  Arial_16             // font for menus
//#define FONT_EDITTITLE  Arial_18_Bold    // font for menus
//#define FONT_ITEM   Arial_16             // font for menus
//#define FONT_TITLE  Arial_24_Bold        // font for all headings

char command[50];

typedef struct {
    short numstate;
    void (*function)();
    byte para;
  }struStates;

struStates  dstate[WEND+1] = {   //assign numstate to the the calling function and define a parameter
// numstate, function, para
  {    WSTART,   Dinit,   0},         // 0         startwindow
  {     Wmain,   Dmain,   0},         // 1         manuell Aussendrehen/Fräsen
  {      WNUM,    Dnum,   0},         // 2         Tastatur
  {  WAOFFSET, Doffset,   0},         // 3         set z- or x-achse to 0
  {    WMENUE,  Dmenue,   0},         // 4
  {    WALARM,  Dalarm,   0},         // 5
  {     WHOME,   Dhome,   0},         // 6
  {    WRESET,  Dreset,   0},         // 7   
  {  WDEFAULT, Ddefault,  0},         // 8
  {   WMACRO1,  Dmacro1,  0}          // 9
 };

typedef struct {
  int cnt = 0;
  char value[INPUTBUFFER] = "\0";
  float fvalue = 0.0;
  int sign = 1;
  float max = 900.0;
  float min = -900.0;
  int decplace = 3;
  int signcnt = 0;
}input_t;


typedef struct { 
    uint32_t change :1,
             mx :1,
             my :1,
             mz :1,
             unassigned: 28;
            } struMyFlags;

typedef struct {
    int lathe = 0;
    bool mpgMode = false;
    int state = WSTART;
    call_enum_t execute;    
    int oldstate = -1;    
    int prevstate = -1;
    int bindex = -1;
    int para = 0;
    int alarm = 0;
    int error = 0; 
    int lastAlarm = 0;
    int lastError = 0;    
                         //                          0         1      2      3       4       5        6       7       8      9        10
    int grblState = 0;   // = dro.c/grbl.state : "Unknown", "Idle", "Run", "Jog", "Hold", "Alarm", "Check", "Door", "Tool", "Home", "Sleep"
    int substate;
    int DROkey = -1;
    int DROkeyvalue = -1;
    unsigned long DROtime = 0;
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float rpm = 0.0;
    bool change_grblState = true;
    uint16_t  color_grblState;
    char grblStateText[8];
    char message[40];
    bool svisible=false;
    unsigned long stime;
    unsigned long buttontime= 0;
    unsigned long buttonDtime =0;
    unsigned long mpgtime[3]= { 0, 0, 0};
    float jogS =0;
    float jogF =0;
    struMyFlags flags;
  }struMyStates;
  
struMyStates mystate;
struTarget target;
input_t input;
int BtnX, BtnY;             // global x y for touch stuff
uint16_t BackColor = C_BLACK;
uint16_t TextColor = C_WHITE;
char buffer10[INPUTBUFFER];          // buffer for use in self defined functions


#ifdef ILI9341
    ILI9341_t3 Display = ILI9341_t3(TFT_CS, TFT_DC);
    FlickerFreePrint<ILI9341_t3> Flickerlabel[4]= {
         FlickerFreePrint<ILI9341_t3>(&Display, ILI9341_WHITE, ILI9341_BLACK),     // Foreground, background
         FlickerFreePrint<ILI9341_t3>(&Display, ILI9341_WHITE, ILI9341_BLACK),     // Foreground, background     
         FlickerFreePrint<ILI9341_t3>(&Display, ILI9341_WHITE, ILI9341_BLACK),     // Foreground, background     
         FlickerFreePrint<ILI9341_t3>(&Display, ILI9341_WHITE, ILI9341_BLACK),     // Foreground, background     
    };
#endif

#ifdef ILI9488
    ILI9488_t3 Display = ILI9488_t3(TFT_CS, TFT_DC);
    FlickerFreePrint<ILI9488_t3> Flickerlabel[4]= {
         FlickerFreePrint<ILI9488_t3>(&Display, ILI9488_WHITE, ILI9488_BLACK),     // Foreground, background
         FlickerFreePrint<ILI9488_t3>(&Display, ILI9488_WHITE, ILI9488_BLACK),     // Foreground, background     
         FlickerFreePrint<ILI9488_t3>(&Display, ILI9488_WHITE, ILI9488_BLACK),     // Foreground, background     
         FlickerFreePrint<ILI9488_t3>(&Display, ILI9488_WHITE, ILI9488_BLACK),     // Foreground, background     
    };
#endif

Button Buttons[MAXBUTTONS] = {Button(&Display), Button(&Display), Button(&Display), Button(&Display), Button(&Display),
                              Button(&Display), Button(&Display), Button(&Display), Button(&Display), Button(&Display),
                              Button(&Display), Button(&Display), Button(&Display), Button(&Display), Button(&Display),};
//Button Buttons[MAXBUTTONS](&Display);

//ItemMenu MainMenu(&Display, true);
//EditMenu Drehen(&Display, true);

void ProcessTouch() {
//    uint16_t x, y, z;
//    Touch.readData(&x, &y, &z);
//    DEBUG("Touch ", x, y, z); 

    TS_Point p = Touch.getPoint();
    BtnX = p.x;
    BtnY = p.y;
  //DEBUG("Touch ", BtnX, BtnY);     
  //Display.drawPixel(BtnX, BtnY, C_GREEN);
}


void MyDisplay_LedMPG(bool on){
    if (on == 1) {
        digitalWrite(LED_MPG, HIGH);
        mystate.mpgMode = true;
    } else {
       digitalWrite(LED_MPG, LOW);
       mystate.mpgMode = false;
    }
}

void MyDisplay_LedMPG_toggle(void){
    DEBUG("  Toggle MPG_Mode");
    serial_putC(CMD_MPG_MODE_TOGGLE);
    if (digitalRead(LED_MPG) == 1)
        digitalWrite(LED_MPG, LOW);
    else
       digitalWrite(LED_MPG, HIGH);
}

bool ProcessButtonPress(Button TheButton) {
    if (TheButton.press(BtnX, BtnY)) {
        TheButton.draw(B_PRESSED);
        while (Touch.touched()) {
            if (TheButton.press(BtnX, BtnY)) {
                TheButton.draw(B_PRESSED);
            }
            else {
                TheButton.draw(B_RELEASED);
                return false;
            }
            ProcessTouch();
        }
        TheButton.draw(B_RELEASED);
        return true;
    }
    return false;
} 

void showPage(int ssize, struPage* ddisplay){
  // struPage = {int x; int y; int TextColor; char Typ[4]; char Text [26]; int Next;} 
  Display.fillScreen(BackColor);
  for (byte index = 0; index <MAXBUTTONS; index++) {
      Buttons[index].hide();  
  }
  bool first = true;  
  byte buttonindex = 0;
  byte buttonwidth,buttonheight;
  //byte checkboxindex = 0; 
  Display.setFont(F_A24);
  for (byte index = 0; index < ssize; index++) {
      if (ddisplay[index].Typ[0] == 'T') {            // Text type
        switch (ddisplay[index].Typ[1]){
          case '0':  Display.setFont(F_A24);
                     break;
          case '1':  Display.setFont(F_A14);
                     break;
          case '2':  Display.setFont(F_A10);
                     break;                     
          default:   Display.setFont(F_A10);
        }   
        if (first) Display.fillRect(0, 0, 480, 38, C_DKBLUE);
        Display.setTextColor(ddisplay[index].TextColor);
        Display.setCursor(ddisplay[index].x , ddisplay[index].y );
        Display.print(F(ddisplay[index].Text));
      }else if (ddisplay[index].Typ[0] == 'B') {      // Button type
        buttonheight = BUTTONHEIGHT;
        if (ddisplay[index].Typ[1] == 'k'){           // Button type short
            buttonwidth = 20;  // 14 * strlen(ddisplay[index].Text) + 2;
            buttonheight = 25;
        }else{
            buttonwidth = 14 * strlen(ddisplay[index].Text) + 2;
            if (buttonwidth < MINBUTTONWIDTH) 
                buttonwidth = MINBUTTONWIDTH;
        }
//        DEBUG("   Create Button: ", ddisplay[index].Text, buttonindex);  
        Buttons[buttonindex].resize(ddisplay[index].x, ddisplay[index].y, buttonwidth, buttonheight);
        Buttons[buttonindex].setColors(ddisplay[index].TextColor, BackColor, ddisplay[index].TextColor, BackColor, BackColor, BackColor);
        Buttons[buttonindex].setText(ddisplay[index].Text);
        Buttons[buttonindex].show();
        Buttons[buttonindex].draw();
        Buttons[buttonindex].value = ddisplay[index].Next;
        if (ddisplay[index].Next == 0) Buttons[buttonindex].value = -ddisplay[index].Text[0];
        buttonindex += 1;
      }
//      }else if (ddisplay[index].Typ[0] == 'C') {      // Checkbox type
//        CheckBoxs[checkboxindex].resize(ddisplay[index].x, ddisplay[index].y, CHECKBOX_SIZE);
//        // OutlineColor, UPColor, ,DownColor BackgroundColor, DisableOutlineColor, DisableTextColor, DisableUPColor, DisableDownColor
//        CheckBoxs[checkboxindex].setColors(ddisplay[index].TextColor, ddisplay[index].TextColor, ddisplay[index].TextColor, ddisplay[index].TextColor, C_DISABLE_LIGHT, C_DISABLE_MED, C_DISABLE_DARK, C_DISABLE_MED);
//        CheckBoxs[checkboxindex].setText(20,5, ddisplay[index].Text, F_A20);      
//        CheckBoxs[checkboxindex].show();
//        CheckBoxs[checkboxindex].draw(true);
//        checkboxindex += 1; 
//        CheckBoxs[checkboxindex].value = index;
//      }
      if (first) {
        first = false;
        Display.setFont(FONT_LBUTTON);
      }
  }
}

void showMessage(void){         // display a status and Button Line in the lower part of the display
//  mystate.grblState = "Unknown", "Idle", "Run", "Jog", "Hold", "Alarm", "Check", "Door", "Tool", "Home", "Sleep"
    if (mystate.stime+BLINKDELAY < millis()){
       Display.fillRect(100, 220, 150, 30, BackColor);
       if (mystate.svisible){
           Display.setFont(F_A20);             
           Display.setCursor(100, 220);
           // sprintf(buffer10, mystate.grblStateText);
           if (mystate.grblState == Idle) {
               if (mystate.mpgMode)
                   sprintf(buffer10, "%s-mpg", mystate.grblStateText);
                else
                   sprintf(buffer10, "%s-dro", mystate.grblStateText);
           } else {
               sprintf(buffer10, "%s", mystate.grblStateText);
           }

           Display.setTextColor(mystate.color_grblState);
           if (mystate.error > 0){
              sprintf(buffer10, "ERROR:%d", mystate.error);           // ERROR has Prio 0 for display
              mystate.prevstate = mystate.state;
              mystate.state = WALARM;
           }else if (mystate.grblState == Alarm){                         // ALARM
              sprintf(buffer10, "ALARM:%d", mystate.alarm);
              mystate.prevstate = mystate.state;
              mystate.state = WALARM;
           }else if (mystate.grblState == NOTCONNECT) {
              Display.setTextColor(C_RED);
              sprintf(buffer10, "not connect");
           }else if(mystate.message[0] != '\0'){
              sprintf(buffer10, mystate.message); 
           }
           Display.print(buffer10);
           Display.setTextColor(TextColor);
         }
       mystate.svisible = !mystate.svisible;
       mystate.stime = millis();
    }
    if (mystate.svisible && mystate.change_grblState) {    // Display active buttons
        //DEBUG("showMessage: active buttons");
        mystate.change_grblState = false;
        //DEBUG("            not buttons active");
        if (mystate.grblState == Idle) {
            showMessageButtons("Start", "", "", "d0/S0");
        }else if (mystate.grblState == Run){
            mystate.alarm = 0;
            showMessageButtons("Stop", "", "", "");
        }
    }else if (!mystate.svisible)
        mystate.change_grblState = true;
}

void showMessageButtons(const char* buttonL1, const char* buttonL2, const char* buttonR2, const char* buttonR1){
// Display the messages for the bottom control buttons  
    Display.fillRect(20, 220, 50, 30, BackColor);
    Display.fillRect(250, 220, 70, 30, BackColor);
    Display.setFont(F_A14); 
    Display.setCursor( 30, 220); Display.print(buttonL1);
    Display.setCursor(105, 220); Display.print(buttonL2);
    Display.setCursor(190, 220); Display.print(buttonR2);  
    Display.setCursor(265, 220); Display.print(buttonR1);
}
//=======================================================================================

void Dinit(void) {
   switch (mystate.execute) {
    case Cinit: {    
        struPage mytext[]= {
           {  10,   10, C_WHITE, "T0", "      --  (c) Heli2  --", 0}, // 0
           {  90,   80, C_BLUE,  "T1", "MPG/DRO Display",         0}, // 1
           { 120,  110, C_BLUE,  "T1", " ",             0},           // 2            
           { 130,  140, C_BLUE,  "T1", VERSION,                   0}, // 3
        };
        DEBUG("   Dinit: Cinit");  
        showPage(sizeof(mytext)/sizeof(struPage), mytext);
        delay(1000);
        mystate.stime = millis();
        mystate.prevstate = WSTART;    
        break;}
   case Crun:{
        if (millis()-mystate.stime > 2000) {
            mystate.execute = Cend;
        }
        break;}
   case Cend: {
        DEBUG("   Dinit: Cend");
        serial_putC(CMD_STATUS_REPORT_ALL);                     // get the first Report, to validate the display
        delay(0.1);
        serial_putC(24);                  //Reset send #24
        delay(0.1);
        sprintf(buffer10, "$X");          // unlock   
        mystate.state = Wmain;    
        break;}
   case Ckeys:{
        DEBUG("   Dmenue Ckeys", mystate.DROkey);  
        break;}        
   }
}

void Dmenue(void){
   switch (mystate.execute) {
    case Cinit: {
        struPage mytext []= {
           { 100,                          10, TextColor, "T0","Menue",                 0},     // 0
           { 140, COLUMN1+11+0*COLUM_DISTANCE, TextColor, "B", "Manuell Drehen",      Wmain},  // 1
           { 140, COLUMN1+11+1*COLUM_DISTANCE, TextColor, "B", "Default values",      WDEFAULT}, // 2           
           { 140, COLUMN1+11+2*COLUM_DISTANCE, TextColor, "B", "Home          ",      WHOME},    // 3
           { 140, COLUMN1+11+3*COLUM_DISTANCE, TextColor, "B", "Reset         ",      WRESET}    // 4           
        };
        showPage(sizeof(mytext)/sizeof(struPage), mytext);
        break;}
   case Crun:{
        break;
        }
   case Cend: {
//        DEBUG("   Dmenue: Cend");      
        break;
        }
   case Ckeys:{
        DEBUG("   Dmenue Ckeys", mystate.DROkey);
        break;}            
  }
}

void Dhome(void){
    switch (mystate.execute) {
        case Cinit: {
            DEBUG("   Dhome: Cinit");
            serial_putC(24); 
            sprintf(buffer10, "$X ");                 // unlock
            serial0_writeLn(buffer10);
            serial_writeLn(buffer10);
            sprintf(buffer10, "$H");
            serial0_writeLn(buffer10);
            serial_writeLn(buffer10);
            // sprintf(buffer10, "G92 X0 Z0");         // set Display to 0
            //serial0_writeLn(buffer10);
            //serial_writeLn(buffer10); 
            if (mystate.lathe == 1) {
                sprintf(buffer10, "G7");                  // enable Diameter
                serial0_writeLn(buffer10);
                serial_writeLn(buffer10);
            }     
            mystate.execute = Cend; 
            // mystate.state = mystate.prevstate;
            mystate.prevstate = Wmain;
            mystate.state = Wmain;
            }
        default: break;
    }
}

void Dreset(void){
        DEBUG("   Dreset: "); 
        serial_putC(24);                  //Reset send #24
        sprintf(buffer10, "$X");          // unlock   
        serial0_writeLn(buffer10);
        serial_writeLn(buffer10); 
        mystate.execute = Cend; 
        mystate.state = mystate.prevstate;             
}

void Init_lathe(void){
   struPage mytext[]= {
       { 10,                           7, TextColor, "T0", "Manuell Drehen",     0}, // 0
       { 10, COLUMN1+0*COLUM_DISTANCE,    TextColor, "T0", "F :",               0}, // 1
       { 28, COLUMN1+0*COLUM_DISTANCE+15, TextColor, "T2", "z",                 0}, // 1
       { 80, COLUMN1+0*COLUM_DISTANCE-20, TextColor, "T2", "target",            0}, // 1
       {220, COLUMN1+0*COLUM_DISTANCE-20, TextColor, "T2", "actual",            0}, // 1
       {150, COLUMN1+0*COLUM_DISTANCE+10, TextColor, "T2", "mm/U",              0}, // 1
       {205, COLUMN1+0*COLUM_DISTANCE,    TextColor, "T0", "U:",                0}, // 1  
       { 10, COLUMN1+1*COLUM_DISTANCE,    TextColor, "T0", "X :",                0}, // 1
       { 10, COLUMN1+2*COLUM_DISTANCE,    TextColor, "T0", "Z :",                0}, // 1                              
       {100, COLUMN1+0*COLUM_DISTANCE+12, TextColor, "B",  "00.000",          WNUM}, // 1   Button Fz:
       {115, COLUMN1+1*COLUM_DISTANCE+12, TextColor, "B",  "  00.000",        WNUM}, // 2   Button X:
       {115, COLUMN1+2*COLUM_DISTANCE+12, TextColor, "B",  " 000.000",        WNUM}, // 3   Button Z:
//           { 15, COLUMN1+1*COLUM_DISTANCE+12, TextColor, "Bk", "  0",         WAOFFSET}, // 4
//           { 15, COLUMN1+2*COLUM_DISTANCE+12, TextColor, "Bk", "  0",         WAOFFSET}, // 5
       { 280,                         20, TextColor, "Bk", " M",           WMENUE},  // 6
    };
    DEBUG("Init_lathe");
    showPage(sizeof(mytext)/sizeof(struPage), mytext);
    Display.setTextColor(TextColor);
    target.changed = true;
}

void Init_milling(void){
    struPage mytext[]= {
       { 10,                           7, TextColor, "T0", "Manuell Fraesen",    0}, // 0
       { 80, COLUMN1+0*COLUM_DISTANCE-20, TextColor, "T2", "target",            0}, // 1
       {220, COLUMN1+0*COLUM_DISTANCE-20, TextColor, "T2", "actual",            0}, // 1
       { 10, COLUMN1+0*COLUM_DISTANCE,    TextColor, "T0", "X :",               0}, // 1                
       { 10, COLUMN1+1*COLUM_DISTANCE,    TextColor, "T0", "Y :",               0}, // 1
       { 10, COLUMN1+2*COLUM_DISTANCE,    TextColor, "T0", "Z :",               0}, // 1                              
       {115, COLUMN1+0*COLUM_DISTANCE+12, TextColor, "B",  "  00.000",        WNUM}, // 1   Button X:
       {115, COLUMN1+1*COLUM_DISTANCE+12, TextColor, "B",  "  00.000",        WNUM}, // 2   Button Y:
       {115, COLUMN1+2*COLUM_DISTANCE+12, TextColor, "B",  "  00.000",        WNUM}, // 3   Button Z:
       {180, COLUMN1+0*COLUM_DISTANCE+30, TextColor, "T2", "Feed rate:",        0}, 
 //      { 15, COLUMN1+1*COLUM_DISTANCE+12, TextColor, "Bk", "  0",         WAOFFSET}, // 4
 //      { 15, COLUMN1+2*COLUM_DISTANCE+12, TextColor, "Bk", "  0",         WAOFFSET}, // 5
       { 260,                         20, TextColor, "Bk", " M",            WMENUE},  // 6
       { 305,                         20, TextColor, "Bk", "  >",          WMACRO1},  // 7
    }; 
    DEBUG("Init_milling");
    showPage(sizeof(mytext)/sizeof(struPage), mytext);
    Display.setTextColor(TextColor);
    target.changed = true; 

    Display.setFont(F_A10);   //10
    //Display.setTextColor(ddisplay[index].TextColor);
    Display.setCursor(270 , COLUMN1+0*COLUM_DISTANCE+30);
    Display.print(target.fxmin);
}

void Dmain(void) {              // Aussendrehen
   switch (mystate.execute) {
    case Cinit: {
        if (mystate.lathe == 1) 
            Init_lathe();
        else
            Init_milling();
        break;}
   case Crun:{              
    // die Tasten selber werden dann in dro.c/processKeypress bearbeitet.
    // die schnellen Anzeigen, werden mit drawString() in dro.c geschrieben
        if (target.changed) {
            DEBUG("   Dmain Crun target(x/y/z)", target.x, target.y, target.z, "grblstate=", mystate.grblState);
            if (mystate.lathe == 1) {
                sprintf(buffer10, "%.3f", target.fz);
                Buttons[0].setText(buffer10);
                sprintf(buffer10, "%.3f", target.x);
                Buttons[1].setText(buffer10);
            } else {
                sprintf(buffer10, "%.3f", target.x);
                Buttons[0].setText(buffer10);
                sprintf(buffer10, "%.3f", target.y);
                Buttons[1].setText(buffer10);
                printf(buffer10, "%.3f", target.fxmin);
            }
            sprintf(buffer10, "%.3f", target.z);
            Buttons[2].setText(buffer10);
            target.changed = false;
            Buttons[0].show();
            Buttons[1].show();
            Buttons[2].show();
        }
        showMessage();
        break;}
   case Cend: { 
//        DEBUG("   Dmain Cend");  
        break;}
   case Ckeys:{
          char axis;
          float mystateaxis;
          if (mystate.DROkey < 4)                // key from Display, only 0-3 allowed for driving....
              axis = 'a';
          else if (mystate.DROkey < 10) {
              MyDisplay_LedMPG_toggle();
              return;
          } else
              axis = mystate.DROkey / 10 + 'X' -1;
          DEBUG("   Dmain Ckeys", mystate.DROkey, mystate.DROkeyvalue, axis); 

          if (mystate.grblState == Idle) {        // = 1
              float targetfmin, targetf, targetaxis;
              switch (axis) {
                  case 'a': {
                            command[0] = 0;
                            if (mystate.DROkey == 0) {                          // start XYZ
                                sprintf(command, "G94F%.3fG90G01X%.3fY%.3fZ%.3f", target.fxmin, target.x, target.y, target.z);      // mm/min
                            }else if (mystate.DROkey == 3) {                    // drive to 0 / set to 0
                            DEBUG("BUttontime:", millis()-mystate.DROtime);
                                if (mystate.DROkeyvalue == EVENT_KEYDOWN)              // /EVENT_KEYUP//EVENT_KEYDOWN
                                    mystate.DROtime = millis();
                                else if (mystate.DROkeyvalue == EVENT_KEYUP) {
                                    unsigned long DROtime = millis()-mystate.DROtime;
                                    if (DROtime < 500) {
                                        sprintf(command, "G00X0Y0Z0");                 // drive to 0
                                        if (mystate.lathe == 0) {
                                            sprintf(buffer10, "%.3f", mystate.x);
                                            Buttons[0].setText(buffer10);
                                            Buttons[0].show();
                                            sprintf(buffer10, "%.3f", mystate.y);
                                            Buttons[1].setText(buffer10);
                                            Buttons[1].show();
                                            sprintf(buffer10, "%.3f", mystate.z);
                                            Buttons[2].setText(buffer10);
                                            Buttons[2].show();
                                        }else{
                                            sprintf(buffer10, "%.3f", mystate.x);
                                            Buttons[1].setText(buffer10);
                                            Buttons[1].show();
                                            sprintf(buffer10, "%.3f", mystate.z);
                                            Buttons[2].setText(buffer10);
                                            Buttons[2].show();
                                        }
                                    }else if (DROtime < 800)
                                        DEBUG("do nothing, time not defined!!");
                                    else if (DROtime < 2000)                           // set to 0
                                        sprintf(command, "G92X0Y0Z0");
                                }
                            }
                            if (command[0] == 0)
                                return;
                            break;}
                  case 'X': {targetfmin = target.fxmin;
                            targetf = target.fx;
                            targetaxis = target.x; 
                            mystateaxis = mystate.x;
                            break;}
                  case 'Y': {targetfmin = target.fymin;
                             targetf = target.fy;
                             targetaxis = target.y;
                             mystateaxis = mystate.y; 
                             break;}
                  case 'Z': {targetfmin = target.fzmin;
                            targetf = target.fz;
                            targetaxis = target.z;
                            mystateaxis = mystate.z; 
                            break;}
                  default:  {targetfmin = 100;
                            targetf = 0;
                            targetaxis = 'C';
                            mystateaxis = 0.0;
                            return;
                            break;}
              }
              if (mystate.DROkey < 10){
                // all axis...
              }else if (mystate.DROkey % 10  == 1){           // left button
                  if (mystate.rpm == 0.0) {
                      sprintf(command, "G94 F%.3f G90 G01 %c%.3f", targetfmin, axis, targetaxis);         // mm/min
                  }else{
                      sprintf(command, "G95 F%.3f G90 G01 %c%.3f", targetf, axis, targetaxis);            // mm/U
                  }
              }else if (mystate.DROkey % 10  == 4 and mystateaxis != 0.0) {     // right button drive axis to 0
                  sprintf(command, "G00 %c0", axis);
                  sprintf(buffer10, "%.3f", mystateaxis);
                  switch (axis) {
                      case 'X': {target.x = mystateaxis;
                                 byte index = 0;
                                 if (mystate.lathe == 1)
                                     index = 1;
                                 Buttons[index].setText(buffer10);
                                 Buttons[index].show();
                                 break;}
                      case 'Y': {target.y = mystateaxis;
                                 Buttons[1].setText(buffer10);
                                 Buttons[1].show();
                                 break;}
                      case 'Z': {target.z = mystateaxis;
                                 Buttons[2].setText(buffer10);
                                 Buttons[2].show();
                                 break;}
                      default:  {
                                break;}
                  }
              }else if (mystate.DROkey % 10  == 8) {                           // right button set axis to 0
                  DEBUG(" set axis to 0", axis);
                  sprintf(command, "G92 %c0", axis);
              }
              else
                  sprintf(command, "\n");
              mystate.DROkey = -1;
              DEBUG(command);
              serial_writeLn(command);
          }else if (mystate.grblState == Run) { 
              if (axis=='a') {
                  if (mystate.DROkey == 0) {
                      DEBUG("Stop");       
                      serial_putC(CMD_STOP);
                  }
              }else if (mystate.DROkey % 10  == 1){
                  DEBUG("Stop");       
                  serial_putC(CMD_STOP);
              }                   
          }
        break;}
   }
}

struMillobject pocket_milling;
void Dmacro1(void) {
  switch (mystate.execute) {
    case Cinit: {
        DEBUG("Dmacro1: Cinit"); 
           struPage mytext[]= {
            { 10,                           7, TextColor, "T0", "Pocket Milling",    0}, // 0
            {220, COLUMN1+0*COLUM_DISTANCE-20, TextColor, "T2", "actual",            0}, //
            {170, COLUMN1+0*COLUM_DISTANCE,    TextColor, "T0", "X:",                0}, //                
            {170, COLUMN1+1*COLUM_DISTANCE,    TextColor, "T0", "Y:",                0}, // 
            {170, COLUMN1+2*COLUM_DISTANCE,    TextColor, "T0", "Z:",                0}, // 
            { 10, COLUMN1+0*35,                TextColor, "Bk", "X",              WNUM}, //                 
            { 10, COLUMN1+1*35,                TextColor, "Bk", "Y",              WNUM}, // 
            { 10, COLUMN1+2*35,                TextColor, "Bk", "Z",              WNUM}, //
            { 15, COLUMN1+3*35,                TextColor, "B", "Cutter ",         WNUM}, //
            {180, COLUMN1+0*COLUM_DISTANCE+30, TextColor, "T2", "Feed rate:",        0},  
            {305,                         20, TextColor, "Bk", "  >",            Wmain},  // 
        };
        // TODO missing:
        //   input X, Y
        //   Tiefe Z
        //   Fräsdurchmesser
        //   innen oder aussen
        //   wenn Innen: Ecken ausräumen?
        //   seitliche Zustellung
        //   Z Zustellung
        //    Gegenlauf, Gleichlauf
        //   Aufmass Schruppen
        pocket_milling.x = 20.0;
        pocket_milling.y = 20.0;
        pocket_milling.z = 1.0;
        pocket_milling.update = true;

        showPage(sizeof(mytext)/sizeof(struPage), mytext);
        Display.setTextColor(TextColor);
        target.changed = true; 
        Display.setFont(F_A10);   //10
        Display.setCursor(270 , COLUMN1+0*COLUM_DISTANCE+30);
        Display.print(target.fxmin);

        Display.setFont(F_A14);   //14
        Display.setCursor(30 , COLUMN1+0*35-5);
        Display.print(pocket_milling.x);
        Display.setCursor(30 , COLUMN1+1*35-5);
        Display.print(pocket_milling.y);
        Display.setCursor(30 , COLUMN1+2*35-5);
        Display.print(pocket_milling.z);

        break;}
    case Crun: { 
        // DEBUG("   Dmacro1 Crun", "grblstate=", mystate.grblState);
        if (pocket_milling.update){
            pocket_milling.update = false;
        }
        break;}
    case Cend: {
        break;}
    case Ckeys:{
        DEBUG("   Dmain Ckeys", mystate.DROkey, mystate.DROkeyvalue);
        pocket_milling.update = true;
        break;}
  }
}

void Dalarm(void) {              // Widget for alarm + error messages
   switch (mystate.execute) {  
    case Cinit: {
        DEBUG("Dalarm: Cinit");
        //for (int index = 1; index <MAXBUTTONS; index++) {
        //    Buttons[index].hide();  
        //}
        Display.fillRect(27,60, 260, 150, TEXTCOLOR);
        Display.fillRect(30,63, 254, 144, C_RED);
        Display.setFont(F_A14); 
        Display.setFont(FONT_SMALL);
        char line[30];
        int cnt=0;
        int y=0;
        unsigned int msgindex=0;
        char *msg = alarmcodes[0].msg;
        char *buttonL = alarmcodes[0].buttonL;
        char *buttonR = alarmcodes[0].buttonR;
        if (mystate.error > 0){
            msgindex = mystate.error;
            if (mystate.error > int(sizeof(errorcodes)/sizeof(structAlarmcodes))){
                DEBUG("       Error to high, set to 0", msgindex);
                msgindex = 0;
            }
            msg = errorcodes[msgindex].msg;
            buttonL = errorcodes[msgindex].buttonL;
            buttonR = errorcodes[msgindex].buttonR;
        }else if (mystate.alarm > 0){ 
            msgindex = mystate.alarm;
            if (mystate.alarm > int(sizeof(alarmcodes)/sizeof(structAlarmcodes))){
                DEBUG("        Alarm to high, set to 0", msgindex);
                msgindex = 0;
            }
            msg = alarmcodes[msgindex].msg;
            buttonL = alarmcodes[msgindex].buttonL;
            buttonR = alarmcodes[msgindex].buttonR;            
        }
        DEBUG("        state= ", mystate.grblState, mystate.prevstate, msgindex, msg);
        for (unsigned int i=0;i<strlen(msg)+1; i++){            // display the alam/error code
             line[cnt] = msg[i];
             cnt++;
             if (line[cnt-1] == '\n'){
                 line[cnt-1] = '\0';
                 Display.setCursor(40, 90+y*20);
                 Display.print(F(line));
                 y++;
                 cnt = 0;
             }
        }
        if (cnt!=0) {
            Display.setCursor(40, 90+y*20);
            Display.print(F(line));
        }
        showMessageButtons(buttonL, "", "", buttonR);
        mystate.lastAlarm = mystate.alarm;
        mystate.lastError = mystate.error;
        break;
    }
    case Crun: {
      if (mystate.grblState == Idle && mystate.alarm==0 && mystate.error==0 && mystate.prevstate != WSTART){// everything is ok, so switch to previous state
          mystate.state =  mystate.prevstate;
          DEBUG("Dalarm: alarm/error cleared", mystate.grblState, "switch to", mystate.state);
          mystate.execute = Cend;
      }else if (mystate.prevstate == WSTART) {     // && mystate.MPGkey == 0) {
          mystate.state =  mystate.prevstate;
          DEBUG("Dalarm: Start ok, switch to ", mystate.state);
      }else{        
          if (mystate.lastAlarm == mystate.alarm && mystate.lastError == mystate.error)
            {}
          else {              
            DEBUG("Dalarm(Crun): grblstate= ", mystate.grblState,"Alarm/Error/lastAlarm/lastError=", mystate.alarm, mystate.error, mystate.lastAlarm, mystate.lastError);            
            mystate.lastAlarm = mystate.alarm;
            mystate.lastError = mystate.error;
          }
      break;
      }
    }
    case Cend: {
        DEBUG("   Dalarm: Cend bindex|fvalue|state|oldstate|prevstate", mystate.bindex, input.fvalue, mystate.state, mystate.oldstate, mystate.prevstate,);
        DEBUG("         Alarm=", mystate.alarm, "Error=", mystate.error);
        mystate.state = mystate.prevstate;
        break;}
    case Ckeys:{
          DEBUG("   Dalarm Ckeys", mystate.DROkey); 
          unsigned int msgindex=0;
          char *buttonL = alarmcodes[0].buttonL;
          char *buttonR = alarmcodes[0].buttonR;
          char *dbutton = (char*)"-1";          
          if (mystate.error >0) {
              msgindex=mystate.error;
              if (msgindex > sizeof(errorcodes)/sizeof(structAlarmcodes)){
                  DEBUG("         error value to high -> set to 0", msgindex);
                  msgindex = 0;
              }
              buttonL = errorcodes[msgindex].buttonL;
              buttonR = errorcodes[msgindex].buttonR;
              DEBUG("         Error:", msgindex, "Buttons", buttonL, buttonR);                  
          }else if (mystate.alarm >0) {
              msgindex=mystate.alarm;
              if (msgindex > sizeof(alarmcodes)/sizeof(structAlarmcodes)){
                  DEBUG("         alarm value to high -> set to 0", msgindex);
                  msgindex = 0;
              }
              buttonL = alarmcodes[msgindex].buttonL;
              buttonR = alarmcodes[msgindex].buttonR;
              DEBUG("         Alarm:", msgindex, "Buttons", buttonL, buttonR);   
          }
          DEBUG("         index=", msgindex);
          if (mystate.DROkey==0)
              dbutton = buttonL;
          else if (mystate.DROkey == 3) 
              dbutton = buttonR;
          DEBUG("         Button=", dbutton);
          if (strlen(dbutton)>0){
              if (strcmp(dbutton, "Home")==0){
                  DEBUG("        do: Home");
                  serial_putC(24);
                  sprintf(buffer10, "$X");                 // unlock
                  serial0_writeLn(buffer10);
                  serial_writeLn(buffer10);
                  sprintf(buffer10, "$H");
                  serial0_writeLn(buffer10);
                  serial_writeLn(buffer10);
                  delay(200);
              }else if (strcmp(dbutton, "Reset")==0) {
                  DEBUG("        do: Reset");
                  serial_putC(24);                        //Reset send #24
              }else if (strcmp(dbutton, "ok")==0) {
                  DEBUG("        do: ok");
                  serial_putC(24);                         //Reset send #24 
                  sprintf(buffer10, "$X");                 // Kill Alarm Lock state 
                  serial0_writeLn(buffer10);
                  serial_writeLn(buffer10);
                  delay(200);                     
              }else if (strcmp(dbutton, "Unlock")==0) {
                  DEBUG("        do: Unlock");
                  serial_putC(24);                         // Reset send #24
                  delay(200);
                  sprintf(buffer10, "$X");                 // Kill Alarm Lock state
                  serial0_writeLn(buffer10);
                  serial_writeLn(buffer10);
                  delay(200);
                  //serial_putC(CMD_MPG_MODE_TOGGLE);
              }
        break;}
       }
   }
}

void Dnum(void) {        //virtual numeric keyboard
   switch (mystate.execute) {
    case Cinit: {
        struPage mytext [15]= { 
           {  40,   40, TextColor, "B", "7",      0}, //0
           { 120,   40, TextColor, "B", "8",      0}, //1 
           { 200,   40, TextColor, "B", "9",      0}, //2 
           { 285,   40, TextColor, "B","+-",      0}, //3   
           {  40,  100, TextColor, "B", "4",      0}, //4 
           { 120,  100, TextColor, "B", "5",      0}, //5   
           { 200,  100, TextColor, "B", "6",      0}, //6 
           { 280,  100, TextColor, "B", "Cancel", 0}, //7           
           {  40,  160, TextColor, "B", "1",      0}, //8 
           { 120,  160, TextColor, "B", "2",      0}, //9 
           { 200,  160, TextColor, "B", "3",      0}, //10 
           { 280,  160, TextColor, "B", "del",    0}, //11
           {  40,  220, TextColor, "B", "0",      0}, //12
           { 120,  220, TextColor, "B", ".",      0}, //13  
           { 220,  220, TextColor, "B", " 000.00",  0}, //14
        };
        showPage(sizeof(mytext)/sizeof(struPage), mytext);
        input.cnt = 0;
        input.value[0] = 0;
        Buttons[14].setText(" 000.000");
        DEBUG("   Dnum: Cinit");
        break;}
   case Crun: {
        //DEBUG("   Dnum: Crun");
        Display.setFont(FONT_SMALL); 
        Display.setCursor(180, 215);
        if (mystate.para > 0) {
          if (mystate.para == 'C') {                //Cancel and go back
              mystate.state = mystate.prevstate;
          }else if (mystate.para == ' ') {          //Enter
              mystate.execute = Cend;
          }else if (mystate.para == 'd') {         // delete last character
              if(input.cnt>0) {
                  if (input.value[input.cnt-1] == '.') {
                      Buttons[13].show();
//                      Buttons[13].draw();
                      input.signcnt = 0;
                  }
                  input.cnt -= 1;            
                  input.value[input.cnt] = 0;
                  }
          }else if (mystate.para == '+') {
              input.sign *= -1;
          }else{
              input.value[input.cnt] = mystate.para;
              input.value[input.cnt+1] = 0;
              if (mystate.para == '.') {           // decimal sign
                  Buttons[13].hide();
                  input.signcnt = input.cnt;
              }           
              if (input.cnt < INPUTBUFFER) input.cnt += 1;
          }
          if ((input.signcnt>0) && (input.cnt > (input.signcnt + input.decplace + 1))){
              input.cnt -= 1;
              input.value[input.cnt] = 0;
          }
          input.fvalue = atof(input.value) * input.sign;
          if ((input.fvalue < input.min) || (input.fvalue> input.max)){
              input.cnt -= 1;
              input.value[input.cnt] = 0;
              input.fvalue = atof(input.value) * input.sign;
          }
          sprintf(buffer10, "%.3f", input.fvalue);
          Buttons[14].setText(buffer10);
          Buttons[14].draw();
          //DEBUG(input.cnt, input.sign, input.value);          
        }
        break;}
   case Cend: {                                                     // this should be changed: make it independent from the state 
        DEBUG("   Dnum: Cend", mystate.bindex, input.fvalue, "prevstate=", mystate.prevstate, "lathe=", mystate.lathe);
        if (mystate.prevstate == Wmain ) {
          if (mystate.lathe == 0) {
            if (mystate.bindex == 0) target.x = input.fvalue;
            else if (mystate.bindex == 1) target.y = input.fvalue;
            else if (mystate.bindex == 2) target.z = input.fvalue;
          }
          else if (mystate.lathe == 1) {
            if (mystate.bindex == 0) target.fz = abs(input.fvalue);
            else if (mystate.bindex == 1) target.x = input.fvalue;
            else if (mystate.bindex == 2) target.z = input.fvalue;         
          }
        }else if (mystate.prevstate == WDEFAULT ) {
           if (mystate.bindex == 0) target.fzmin = abs(input.fvalue);
        }
        target.changed = true;
        mystate.state = mystate.prevstate;
        break;}
    case Ckeys:{
          DEBUG("   Dnum Ckeys", mystate.DROkey); 
        break;}        
   }
}

void Ddefault(void) {              // set default values 
   switch (mystate.execute) {
    case Cinit: {
        struPage mytext[]= {
           { 30,                           7, TextColor, "T0", "Set default values",    0}, // 0
           { 40, COLUMN1+0*COLUM_DISTANCE,    TextColor, "T0", "F :",                   0}, // 1
           { 60, COLUMN1+0*COLUM_DISTANCE+15, TextColor, "T2", "z",                     0}, // 2           
           {210, COLUMN1+0*COLUM_DISTANCE+5,  TextColor, "T1", "mm/min",                0}, // 3
           {210, COLUMN1+1*COLUM_DISTANCE,    TextColor, "T1", "mm/U",                  0}, // 4
           { 40, COLUMN1+2*COLUM_DISTANCE,    TextColor, "T0", "F :",                   0}, // 5
           { 60, COLUMN1+2*COLUM_DISTANCE+15, TextColor, "T2", "x",                     0}, // 6           
           {210, COLUMN1+2*COLUM_DISTANCE+5,  TextColor, "T1", "mm/min",                0}, // 7
           {210, COLUMN1+3*COLUM_DISTANCE,    TextColor, "T1", "mm/U",                  0}, // 8                   
           {140, COLUMN1+12+0*COLUM_DISTANCE, TextColor, "B",  " 000.000",           WNUM}, // 9   Button Fzmin:
           {140, COLUMN1+12+1*COLUM_DISTANCE, TextColor, "B",  " ???.000",           WNUM}, // 10   Button Fz default:
           {140, COLUMN1+12+2*COLUM_DISTANCE, TextColor, "B",  " 000.000",           WNUM}, // 11   Button Fxmin:
           {140, COLUMN1+12+3*COLUM_DISTANCE, TextColor, "B",  " ???.000",           WNUM}  // 12   Button Fx default:         
        };
        showPage(sizeof(mytext)/sizeof(struPage), mytext);
        target.changed = true;
        showMessageButtons("", "", "", "ok");
        break;}
     case Crun: {
        if (target.changed) {
            sprintf(buffer10, "%.3f", target.fzmin); Buttons[0].setText(buffer10);
            sprintf(buffer10, "%.3f", target.fz);    Buttons[1].setText(buffer10);
            sprintf(buffer10, "%.3f", target.fxmin); Buttons[2].setText(buffer10);
            sprintf(buffer10, "%.3f", target.fx);    Buttons[3].setText(buffer10);
            target.changed = false;
            Buttons[0].show(); Buttons[1].show(); Buttons[2].show(); Buttons[3].show();
        }
        break;}
  case Cend: {
        DEBUG("   Ddefault: Cend", mystate.bindex, input.fvalue);
        switch (mystate.bindex) {
           case 0 : { target.fzmin = input.fvalue;
                    break;}
           case 1 : { target.fz = input.fvalue;
                    break;}
           case 2 : { target.fxmin = input.fvalue;
                    break;} 
           case 3 : { target.fx = input.fvalue;
                    break;}
           default: break;
        }
        DEBUG("      fz:", target.fzmin, target.fz);
        DEBUG("      fx:", target.fxmin, target.fx);
        eeprom.fzmin = target.fzmin; eeprom.fzU = target.fz;
        eeprom.fxmin = target.fxmin; eeprom.fxU = target.fx;
        eeprom_write();
        break;}
  case Ckeys:{
        DEBUG("   Ddefault Ckeys", mystate.DROkey);
        if (mystate.DROkey == 3){           // right button
            mystate.state = WMENUE;       // mystate.prevstate;
        }
        mystate.DROkey = -1;   
        break;}
   }
}

void Doffset(void) {  
    int key = mystate.bindex;
    DEBUG("   Doffset:", key);
//  Display.setCursor(31, COLUMN1);Display.print(":");    
    if (key == 3) {
        sprintf(buffer10, "G92 X0");
    }else if (key == 4) {
        sprintf(buffer10, "G92 Z0");
    } 
    serial0_writeLn(buffer10);
    serial_writeLn(buffer10); 
    mystate.state = mystate.prevstate;    
}

void showFlags(void) {
  if (mystate.flags.change) {;
    Display.setFont(F_A10);
    Display.setCursor(5, 230);
    Display.print("mx");
  }
}

// #########################################################################################################################
// global function calls

void MyDisplay_init(void) { 
  // configure the display and the state machine for the menues  
    DEBUG("MyDisplay_init start");
    Display.begin();
    Display.setRotation(1);   
    Display.fillScreen(BackColor);
    Touch.begin();
    Touch.setRotation(1);
    Touch.calibrate(cal);        // cal is from myTouchCalibration.h

    DEBUG("now init Buttons");

    for (int index = 0; index <MAXBUTTONS; index++) {
      Buttons[index].init(10, 80, 40, 40, TextColor, BackColor, TextColor, BackColor, "", -10, -5, FONT_LBUTTON );
      Buttons[index].hide();
    }
    DEBUG("... ready Buttons");
    mystate.execute = Cinit;
    target.fxmin = eeprom.fxmin;
    target.fymin = eeprom.fymin;
    target.fzmin = eeprom.fzmin;
    target.fx = eeprom.fxU;
    target.fz = eeprom.fzU;
    target.x = 0.0;
    target.y = 0.0;    
    target.z = 0.0;    
    target.changed = false;    
    mystate.stime = millis();
    mystate.buttonDtime = 0; 
    mystate.grblState = NOTCONNECT;

    // init LED's
    pinMode(LED_MPG, OUTPUT);
}

void MyDisplay_loop(void){
  // this is the main loop for the Display 
  // runs through the states:    (dstate is the acutal page)
  //     
  // begin:  dstate.Cinit        # initialize the Page with values and define the polling keys and button to execute the connected-function 
  //         loop dstate.run until news state:
  //            - show new values
  //            - polling touchscreen 
  //                  polling keys is do in the main loop
  //            - execute button function and change the state (defined in the struPage from the page)
  //                   if page Dnum.Cend calling, then the values assigned to its variable    --> this should be done in the page functions
  //         dstate.Cend from the old state   
  //         jump to begin   
  //              
   if (mystate.execute == Cend)  {
      dstate[mystate.oldstate].function();    // actual state ends
   }else{
      dstate[mystate.state].function();      // call the function from the actual state
   }

   if (mystate.execute == Cinit)
      mystate.execute = Crun;
   mystate.para = -1;
   if (mystate.oldstate != mystate.state) {
      DEBUG("old state", mystate.oldstate, "new state", mystate.state);
      mystate.oldstate = mystate.state;
      mystate.execute = Cinit;
   }
   else if (Touch.touched()) {
      ProcessTouch();   
      int bvalue = -1;
      int bindex = -1;
      for (int index = 0; index <MAXBUTTONS; index++) {
          if (Buttons[index].isVisible() and (ProcessButtonPress(Buttons[index]))) {
              bvalue = Buttons[index].value;
              bindex = index;
              //DEBUG("Touch", index, " value:",bvalue);    
              break;
          }
      }
      if ((bvalue > -1) and (bvalue < WEND)) {        // Button clicked for Canvas change, -> save new state
          mystate.execute = Cend;        
          DEBUG("Touch", bvalue);             
          if (mystate.state == WALARM) {
            mystate.state = mystate.prevstate;
          }else {            
            mystate.prevstate = mystate.state;
            mystate.state = bvalue;
            mystate.bindex  = bindex;
          }
      }else if (bvalue < -1){                        // Button clicked 
          mystate.para = -bvalue;
      }
   }
}

void drawString (uint_fast8_t i, const ILI9341_t3_font_t *font, uint16_t x, uint16_t y, float value, const char *string, bool opaque){
    if ((mystate.state == Wmain) | (mystate.state == WMACRO1)) {
        //DEBUG("drawString", i, x, y, value, string);
        Display.setFont(*font); 
        Display.setFont(Arial_24);
        Display.setCursor(x, y);
        if ((mystate.lathe == 1) || ((mystate.lathe == 0) && (i<3)))
            Flickerlabel[i].print(string);
        switch (i){
           case 0 : mystate.x = value;break;
           case 1 : mystate.y = value;break;         
           case 2 : mystate.z = value;break; 
           case 3 : mystate.rpm = value; break; 
        }    
    }
}

void set_myDisplay(int value, const char* string, uint16_t color, int alarm, int error, int lathe)
// will be called from grblcomm.parseData(grbl_data)
{
    //if (value != mystate.grblState) {
    {
        mystate.lathe = lathe;
        mystate.grblState = value;
        mystate.change_grblState = true;
        if (alarm < 20)                    // todo: Alarm wird nach poweroff von grblcom nicht mehr gesetzt ??!!
          mystate.alarm = alarm;                  
        else
          mystate.alarm = 0;
        if (error < 70) 
          mystate.error = error;
        else
          mystate.error = 0;
        mystate.color_grblState = color;
        strcpy(mystate.grblStateText, string);
    }
}

void debugDisplay(const char *string){
  Display.setFont(F_A10);  
  //Display.setTextColor(ddisplay[index].TextColor);
  Display.setCursor(10 , 200 );
  Display.fillRect(10, 200, 200, 10, BackColor);
  Display.print(string);
}

void processMpg (char MPGkey, int MPGcnt, int MPGdtime) {
  // translate the values from MPG-handwheel to drive commands
  // is called when you operate the hand wheel
  // MPGkey = X,Y,Z
    int index = int(MPGkey-'X'); 
    //DEBUG("processMpg", MPGkey, MPGcnt, MPGdtime);
    // char buffer50[50];
    // sprintf(buffer50, "processMpg %d %d", MPGcnt, MPGdtime); debugDisplay(buffer50);
    if ((mystate.grblState == Idle || mystate.grblState == Jog) && (mystate.state == Wmain) && (MPGkey!= 0))  {     // https://github.com/gnea/grbl/wiki/Grbl-v1.1-Jogging
        // DEBUG(mystate.grblState, Idle, Jog, mystate.state, MPGkey);
        float speed;
        unsigned long time = millis();
        // unsigned long dtime = time-mystate.mpgtime[index];
        mystate.mpgtime[index] = time;   
        if (abs(MPGcnt) < 5) speed = eeprom.fzjog001;
        else if (abs(MPGcnt) < 20) speed = eeprom.fzjog01;
        else speed = eeprom.fzjog1;
        //  speed = 600.0 * float(abs(MPGcnt)) / float(dtime);        // 60*1000 MPGcnt *0.01 / (millis() - oldmillis)          = mm/min
        // todo:  for better and smoother driving evaluate MPGdtime (it is the time beetween 2 handwheel pulses) 
        if (mystate.rpm > 0.0) { 
             speed = speed / 10;
        }
        // in mpg mode G95 is not allowed :-(
        sprintf(command, "$J=G91 %c%.3f F%.1f", MPGkey, float(MPGcnt)*0.01, speed);   //e.q. $J=G91 Z1.000 F100.0   # G91 = relative movement
        serial_writeLn(command);
        // DEBUG("cnt= ", MPGcnt, "time delta=", dtime, speed, mystate.rpm, command);      
    } 
}


void processKeypress (int DROkey, int keydown, float rpm){
    // kdeydown = EVENT_KEYDOWN/EVENT_KEYUP

    // call from dro.c->DROProcessEvents 
    //char *dbutton = (char*)"-1";

    DEBUG("processKeypress DROkey=", DROkey, "Value=", keydown, "State=", mystate.grblState, mystate.grblStateText, mystate.message);
    mystate.rpm = rpm;
    
    // evaluate key in the deticated page:
    mystate.DROkey = DROkey;
    mystate.DROkeyvalue = keydown;

    call_enum_t execute_state = mystate.execute; 
    mystate.execute = Ckeys;
    dstate[mystate.state].function();
    mystate.execute = execute_state;

    DROkey = mystate.DROkey;
    // evaluate key for global function: 
    if (keydown == EVENT_KEYUP) {                    // 
        switch (mystate.grblState){
              case Run:       // == 2
                  DEBUG("  Run");   
                  if (DROkey == 0) {
                      DEBUG("Stop");       
                      serial_putC(CMD_STOP);
                 }
                 break;
              case Idle:
                  if (DROkey == 4) {
                      MyDisplay_LedMPG_toggle();
                  } 
                  break;
              case Alarm:       // == 5
                  if (DROkey == 4) {
                      MyDisplay_LedMPG_toggle();
                  } 
                  break;
    }
    }
}
