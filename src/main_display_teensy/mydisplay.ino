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
#include "ILI9341_t3_Menu.h"          // custom utilities definition
#include "ILI9341_t3.h"           // fast display driver lib            https://github.com/PaulStoffregen/ILI9341_t3
#include <font_Arial.h>           // custom fonts that ships with ILI9341_t3.h
#include <font_ArialBold.h>       // custom fonts for the ILI9341_t3.h
#include <XPT2046_Calibrated.h>   // increase Z_THRESHOLD in XPT2046_Calibrated.cpp if touchscreen is to sensitiv, add line 129: if (z > 4095) z = 0;
#include "myTouchCalibration.h"
#include <FlickerFreePrint.h>     // library to draw w/o flicker        https://github.com/KrisKasprzak/FlickerFreePrint
#include "ILI9341_t3_Controls.h"  // https://github.com/KrisKasprzak/ILI9341_t3_controls

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


#define ROW_HEIGHT          64          // 35
#define ROWS                3           // 5
#define DATA_COLUMN         230         // 190

#define LATHE           true
#define COLUMN1         60
#define COLUM_DISTANCE  50

// easy way to include fonts but change globally
#define FONT_SMALL  Arial_16             // font for menus
#define FONT_EDITTITLE  Arial_18_Bold    // font for menus
#define FONT_ITEM   Arial_16             // font for menus
#define FONT_TITLE  Arial_24_Bold        // font for all headings

typedef struct {
    short numstate;
    void (*function)();
    byte para;
  }struStates;

struStates  dstate[WEND+1] = {   //assign numstate to the the calling function and define a parameter
// numstate, function, para
  {    WSTART,   Dinit,   0},         // 0 
  {   WDREHEN, Adrehen,   0},         // 1
  {      WNUM,    Dnum,   0},         // 2
  {  WAOFFSET, Doffset,   0},         // 3
  {    WMENUE,  Dmenue,   0},         // 4
  {    WALARM,  Dalarm,   0},         // 5
  {     WHOME,   Dhome,   0},         // 6
  {    WRESET,  Dreset,   0},         // 7   
  {  WDEFAULT, Ddefault,  0}          // 8
 };

typedef struct {
  int cnt = 0;
  char value[INPUTBUFFER] = "\0";
  float fvalue = 0.0;
  int sign = 1;
  float max = 700.0;
  float min = -700.0;
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
    int MPGkey = 0;
    struMyFlags flags;
  }struMyStates;
  
struMyStates mystate;
input_t input;
int BtnX, BtnY;             // global x y for touch stuff
uint16_t BackColor = C_BLACK;
uint16_t TextColor = C_WHITE;
char buffer10[INPUTBUFFER];          // buffer for use in self defined functions



ILI9341_t3 Display = ILI9341_t3(TFT_CS, TFT_DC);
FlickerFreePrint<ILI9341_t3> Flickerlabel[4]= {
     FlickerFreePrint<ILI9341_t3>(&Display, ILI9341_WHITE, ILI9341_BLACK),     // Foreground, background
     FlickerFreePrint<ILI9341_t3>(&Display, ILI9341_WHITE, ILI9341_BLACK),     // Foreground, background     
     FlickerFreePrint<ILI9341_t3>(&Display, ILI9341_WHITE, ILI9341_BLACK),     // Foreground, background     
     FlickerFreePrint<ILI9341_t3>(&Display, ILI9341_WHITE, ILI9341_BLACK),     // Foreground, background     
};

Button Buttons[4] = {Button(&Display), Button(&Display), Button(&Display), Button(&Display),};
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

void show_display(int ssize, struDisplay* ddisplay){
  // struDisplay = {int x; int y; int TextColor; char Typ[4]; char Text [26]; int Next;} 
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
      if (ddisplay[index].Typ[0] == 'T') {
        if (first) Display.fillRect(0, 0, 480, 38, C_DKBLUE);
        Display.setTextColor(ddisplay[index].TextColor);
        Display.setCursor(ddisplay[index].x , ddisplay[index].y );
        Display.print(F(ddisplay[index].Text));
      }else if (ddisplay[index].Typ[0] == 'B') {
        buttonheight = BUTTONHEIGHT;
        if (ddisplay[index].Typ[1] == 'k'){
            buttonwidth = 20;  // 14 * strlen(ddisplay[index].Text) + 2;
            buttonheight = 25;
        }else{
            buttonwidth = 14 * strlen(ddisplay[index].Text) + 2;
            if (buttonwidth < MINBUTTONWIDTH) 
                buttonwidth = MINBUTTONWIDTH;
        }
        Buttons[buttonindex].resize(ddisplay[index].x, ddisplay[index].y, buttonwidth, buttonheight);
        Buttons[buttonindex].setColors(ddisplay[index].TextColor, BackColor, ddisplay[index].TextColor, BackColor, BackColor, BackColor);
        Buttons[buttonindex].setText(ddisplay[index].Text);
        Buttons[buttonindex].show();
        Buttons[buttonindex].draw();
        Buttons[buttonindex].value = ddisplay[index].Next;
        if (ddisplay[index].Next == 0) Buttons[buttonindex].value = -ddisplay[index].Text[0];
        buttonindex += 1;
      }
//      }else if (ddisplay[index].Typ[0] == 'C') {      
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
           sprintf(buffer10, mystate.grblStateText);
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
        mystate.change_grblState = false;  
        if (mystate.grblState == Idle) {
            DisplayMessageButtons("Start","Z=0");
        }else if (mystate.grblState == Run){
            mystate.alarm = 0;
            DisplayMessageButtons("Stop","");
        }
    }else if (!mystate.svisible)
        mystate.change_grblState = true;
}

void DisplayMessageButtons(const char* buttonL, const char* buttonR){
// Display the messages for the bottom control buttons  
    Display.fillRect(20, 220, 50, 30, BackColor);
    Display.fillRect(250, 220, 70, 30, BackColor);
    Display.setFont(F_A14); 
    Display.setCursor(20, 220);  
    Display.print(buttonL);            
    Display.setCursor(250, 220); 
    Display.print(buttonR);  
}
//=======================================================================================

void Dinit(void) {
   switch (mystate.execute) {
    case Cinit: {    
        struDisplay mytext[]= {
           {  10,   10, C_WHITE, "T", "      --  (c) Heli2  --", 0}, // 0
           { 110,   80, C_BLUE,  "T", "Elektronische",           0}, // 1
           { 120,  120, C_BLUE,  "T", "Leitspindel",             0}, // 2            
           { 130,  160, C_BLUE,  "T", "V0.13",                   0}, // 3
           //{ 130,  160, C_BLUE,  "T", eeprom.Version,            0}, // 3
        };
        DEBUG("   Dinit: Cinit");  
        show_display(sizeof(mytext)/sizeof(struDisplay), mytext);
        delay(1000);
        mystate.stime = millis();
        mystate.prevstate = WSTART;    
        break;}
   case Crun:{
        if (millis()-mystate.stime > 2000) {
            if (mystate.MPGkey != 0 ){
                mystate.alarm = 20;
                mystate.state = WALARM;
                mystate.color_grblState = C_RED;
            }else 
                mystate.state = WDREHEN;
        }
        break;
        }
   case Cend: {
        DEBUG("   Dinit: Cend");      
        break;
        }
   }
}

void Dmenue(void){
   switch (mystate.execute) {
    case Cinit: {
        struDisplay mytext []= {
           { 100,   10, TextColor, "T", "Menue",                  0}, // 0
           { 140, COLUMN1+11+0*COLUM_DISTANCE, TextColor, "B", "Aussendrehen  ",      WDREHEN},  // 1
           { 140, COLUMN1+11+1*COLUM_DISTANCE, TextColor, "B", "Default values",      WDEFAULT}, // 2           
           { 140, COLUMN1+11+2*COLUM_DISTANCE, TextColor, "B", "Home          ",      WHOME},    // 3
           { 140, COLUMN1+11+3*COLUM_DISTANCE, TextColor, "B", "Reset         ",      WRESET}    // 4           
        };
        show_display(sizeof(mytext)/sizeof(struDisplay), mytext);
        break;}
   case Crun:{
        break;
        }
   case Cend: {
//        DEBUG("   Dmenue: Cend");      
        break;
        }        
  }
}

void Dhome(void){
//        DEBUG("   Dhome: Cinit");
        serial_putC(24); 
        sprintf(buffer10, "$H");            
        serial0_writeLn(buffer10);
        serial_writeLn(buffer10); 
        mystate.execute = Cend; 
        mystate.state = mystate.prevstate;             
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

void Adrehen(void) {              // Aussendrehen
   switch (mystate.execute) {
    case Cinit: {
        struDisplay mytext[]= {
           { 50,                           7, TextColor, "T",  "Aussendrehen",      0}, // 0   
           {130, COLUMN1+12+0*COLUM_DISTANCE, TextColor, "B",  "  00.000",        WNUM}, // 1   Button Fz:
           {130, COLUMN1+12+1*COLUM_DISTANCE, TextColor, "B",  "  00.000",        WNUM}, // 2   Button X:
           {130, COLUMN1+12+2*COLUM_DISTANCE, TextColor, "B",  " 000.000",        WNUM}, // 3   Button Z:
           { 15, COLUMN1+12+1*COLUM_DISTANCE, TextColor, "Bk", "  0",         WAOFFSET}, // 4
           { 15, COLUMN1+12+2*COLUM_DISTANCE, TextColor, "Bk", "  0",         WAOFFSET}, // 5
           { 290,                         20, TextColor, "Bk", " M",           WMENUE}, // 6           
        };
        show_display(sizeof(mytext)/sizeof(struDisplay), mytext);
        Display.setTextColor(TextColor);
        // Fz:
        Display.setFont(F_A24);
        Display.setCursor(40, COLUMN1); Display.print("F");
        Display.setFont(F_A10);
        Display.setCursor(58, COLUMN1+15); Display.print("z");
        Display.setFont(F_A24);
        Display.setCursor(66, COLUMN1);Display.print(":");
        // mm/U
        Display.setFont(F_A10); 
        Display.setCursor(120,COLUMN1+12+0*COLUM_DISTANCE-32);Display.print("mm/U");
        // U:
        Display.setFont(F_A24);
        Display.setCursor(195, COLUMN1); Display.print("U:");
        Display.setCursor(40, COLUMN1+1*COLUM_DISTANCE);  Display.print("X:");
        Display.setCursor(40, COLUMN1+2*COLUM_DISTANCE);  Display.print("Z:");
        target.changed = true;
        break;}
   case Crun:{              
    // die Tasten selber werden dann in dro.c/processKeypress bearbeitet.
    // die schnellen Anzeigen, werden mit drawString() in dro.c geschrieben
        if (target.changed) {
            // DEBUG("   Adrehen Crun", target.x, target.z);
            sprintf(buffer10, "%.3f", target.fz);
            Buttons[0].setText(buffer10);
            sprintf(buffer10, "%.3f", target.x);
            Buttons[1].setText(buffer10);
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
//        DEBUG("   Adrehen Cend");  
        break;}
   }
}

void Dalarm(void) {              // Widget for alarm + error messages
   switch (mystate.execute) {
    case Crun: {
      if (mystate.grblState == Idle && mystate.alarm==0 && mystate.error==0 && mystate.prevstate != WSTART){// everything is ok, so switch to previous state
          mystate.state =  mystate.prevstate;
          DEBUG("Dalarm: alarm/error cleared", mystate.grblState, "switch to", mystate.state);
          mystate.execute = Cend;
      }else if (mystate.prevstate == WSTART && mystate.MPGkey == 0) {
          mystate.state =  mystate.prevstate;
          DEBUG("Dalarm: Start ok, switch to ", mystate.state);
      }else{        
          if (mystate.lastAlarm == mystate.alarm && mystate.lastError == mystate.error)
            {}
          else {              
            DEBUG("Dalarm: active grblstate= ", mystate.grblState,"Alarm/Error=", mystate.alarm, mystate.error, mystate.lastAlarm, mystate.lastError);            
            mystate.lastAlarm = mystate.alarm;
            mystate.lastError = mystate.error;
          }
      break;
      }
    }    
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
                DEBUG("Dalarm: Error to high, set to 0", msgindex);
                msgindex = 0;
            }
            msg = errorcodes[msgindex].msg;
            buttonL = errorcodes[msgindex].buttonL;
            buttonR = errorcodes[msgindex].buttonR;
        }else if (mystate.alarm > 0){ 
            msgindex = mystate.alarm;
            if (mystate.alarm > int(sizeof(alarmcodes)/sizeof(structAlarmcodes))){
                DEBUG("Dalarm: Alarm to high, set to 0", msgindex);
                msgindex = 0;
            }
            msg = alarmcodes[msgindex].msg;
            buttonL = alarmcodes[msgindex].buttonL;
            buttonR = alarmcodes[msgindex].buttonR;            
        }
        DEBUG("Dalarm: state= ", mystate.grblState, mystate.prevstate, msgindex, msg);
        for (unsigned int i=0;i<strlen(msg)+1; i++){
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
        DisplayMessageButtons(buttonL, buttonR);
        mystate.lastAlarm = mystate.alarm;
        mystate.lastError = mystate.error;
        break;
    }
    case Cend: {
        DEBUG("   Dalarm: Cend", mystate.bindex, input.fvalue, mystate.state, mystate.oldstate, mystate.prevstate,);
        mystate.state = mystate.prevstate;
        break;
       }
   }
}

void Dnum(void) {        //virtual numeric keyboard
   switch (mystate.execute) {
    case Cinit: {
        struDisplay mytext [15]= { 
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
        show_display(sizeof(mytext)/sizeof(struDisplay), mytext);
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
        DEBUG("   Dnum: Cend", mystate.bindex, input.fvalue);
        if (mystate.bindex == 0) target.fz = abs(input.fvalue);
        if (mystate.bindex == 1) target.x = input.fvalue;
        else if (mystate.bindex == 2) target.z = input.fvalue;
        target.changed = true;
        mystate.state = mystate.prevstate;
        break;}
   }
}

void Ddefault(void) {              // set default values 
   switch (mystate.execute) {
    case Cinit: {
        struDisplay mytext[]= {
           { 30,                           7, TextColor, "T",  "Set default values",      0}, // 0   
           {140, COLUMN1+12+0*COLUM_DISTANCE, TextColor, "B",  " 500.000",        WNUM}       // 1   Button Fz:
        };
        show_display(sizeof(mytext)/sizeof(struDisplay), mytext);
        Display.setTextColor(TextColor);
        // Fz:
        Display.setFont(F_A24);
        Display.setCursor(40, COLUMN1); Display.print("F");
        Display.setFont(F_A10);
        Display.setCursor(58, COLUMN1+15); Display.print("z");
        Display.setFont(F_A24);
        Display.setCursor(66, COLUMN1);Display.print(":");
        // mm/min
        Display.setFont(F_A10); 
        Display.setCursor(210,COLUMN1+12+0*COLUM_DISTANCE);Display.print("mm/min");
        target.changed = true;
        DisplayMessageButtons("","ok");
        break;}
     case Crun: {
        if (target.changed) {
            DEBUG("   Ddefault Crun", target.fzmin);
            sprintf(buffer10, "%.3f", target.fzmin);
            Buttons[0].setText(buffer10);
            target.changed = false;
            Buttons[0].show();            
        }
        break;}
  case Cend: {
        DEBUG("   Ddefault: Cend", mystate.bindex, input.fvalue);
        mystate.state = WMENUE;       // mystate.prevstate;
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

void DisplayFlags(void) {
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
    Display.begin();
    Display.setRotation(1);   
    Display.fillScreen(BackColor);
    Touch.begin();
    Touch.setRotation(1);
    Touch.calibrate(cal);        // cal is from myTouchCalibration.h

    for (int index = 0; index <MAXBUTTONS; index++) {
      Buttons[index].init(10, 80, 40, 40, TextColor, BackColor, TextColor, BackColor, "", -10, -5, FONT_LBUTTON );
      Buttons[index].hide();
    }
    mystate.execute = Cinit;

    target.fz = 1.0;
    target.x = 0.0;
    target.y = 0.0;    
    target.z = 0.0;    
    target.changed = false;    
    mystate.stime = millis();
    mystate.buttonDtime = 0; 
    mystate.grblState = NOTCONNECT;
}

void MyDisplay_loop(void){
  // this is the main loop for the Display 
  // runs through the states:    (dstate is the acutal page)
  //     
  // begin:  dstate.Cinit        # initialize the Page with values and define the polling keys and button to execute the connected-function 
  //         loop dstate.run until news state:
  //            - show new values
  //            - polling touchscreen and keys
  //            - execute button function and change the state (defined in the struDisplay from the page)
  //                   if page Dnum.Cend calling, then the values assigned to its variable    --> this should be make more flexibel
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

void drawString (uint_fast8_t i, const ILI9341_t3_font_t *font, uint16_t x, uint16_t y, const char *string, bool opaque){
    if (mystate.state == WDREHEN) {
        //DEBUG("drawString", x, y, string);
        Display.setFont(*font); 
        Display.setFont(Arial_24);
        Display.setCursor(x, y);
        Flickerlabel[i].print(string);
    }
}

void set_grblstate(int value, const char* string, uint16_t color, int alarm, int error)
{
    //if (value != mystate.grblState) {
    {
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

void processJoystick (int MPGkey) {
  // translate the values from Joystick to drive commands
    int myoffset = 0;
    bool twice = false;
    if ((mystate.grblState == Idle || mystate.grblState == Jog) && (mystate.state == WDREHEN) && (MPGkey!= 0) && ((millis()-mystate.buttontime)>mystate.buttonDtime))  {     // https://github.com/gnea/grbl/wiki/Grbl-v1.1-Jogging
        static char command[50];
        if ((millis()-mystate.buttontime)> 2*mystate.buttonDtime){
            target.fzOld = target.fz;
            Display.fillRect(120, COLUMN1+12+0*COLUM_DISTANCE-32, 50, 30, BackColor);
            Display.setFont(F_A10); 
            Display.setCursor(120,COLUMN1+12+0*COLUM_DISTANCE-32);Display.print("mm/min");
        }
        if (mystate.MPGkey != MPGkey) {
            target.changed = true; 
            mystate.MPGkey = MPGkey;
        }            
        //DEBUG("processMPGpress ok", MPGkey, mystate.buttontime);
        if (abs(MPGkey) == 1) {         //90ms
            mystate.jogS = 0.003;
            mystate.jogF=0.2;
            myoffset = 0;        
        }else if (abs(MPGkey) == 2) {   //120ms
            mystate.jogS = 0.002;
            mystate.jogF=1;
        }else if (abs(MPGkey) == 3) {   //120ms
            mystate.jogS = 0.02;        
            mystate.jogF=10.0;
        }else if (abs(MPGkey) == 4) {   //120ms
            mystate.jogS = 0.2;
            mystate.jogF=100.0;         
            if (target.changed)
                twice = true;                         
        }else if (abs(MPGkey) == 5) {   //120ms
            mystate.jogS = 1.0;
            mystate.jogF=500.0;
            myoffset = -5;            
            if (target.changed)
                twice = true;                     
        }else if (abs(MPGkey) == 6) {   //120ms
            mystate.jogS = 4.0;
            mystate.jogF=2000.0;
            myoffset = -5;
            if (target.changed)
                twice = true; 
        }                                           
        mystate.buttonDtime = (unsigned long)(60.0/mystate.jogF * mystate.jogS *1000.0) + myoffset; 
        target.fz = mystate.jogF;    
        char* vz = (char*)"";
        if (MPGkey < 0)
            vz = (char*)"-";
        sprintf(command, "$J=G91 Z%s%.3f F%.3f", vz, mystate.jogS, mystate.jogF);
        serial_writeLn(command);
        if (twice)
            serial_writeLn(command);
        mystate.buttontime = millis(); 
    }else if ((mystate.grblState == Idle || mystate.grblState == Jog) && (mystate.state == WDREHEN) && (MPGkey== 0)){
        serial_putC(CMD_STOP); 
        serial0_writeLn("processMPGpress Stop");
        Display.fillRect(120, COLUMN1+12+0*COLUM_DISTANCE-32, 50, 30, BackColor);
        Display.setFont(F_A10); 
        Display.setCursor(120,COLUMN1+12+0*COLUM_DISTANCE-32);Display.print("mm/U");
        target.fz = target.fzOld;
        mystate.MPGkey = MPGkey;
        target.changed = true;               
    }else if ((millis()-mystate.buttontime)>mystate.buttonDtime)
        mystate.MPGkey = MPGkey;
}

void processMpg (char MPGkey, int MPGcnt, int MPGswitch) {
  // translate the values from MPG-handwheel to drive commands
  // is called when you operate the hand wheel
  // MPGkey = X,Y,Z
    int index = int(MPGkey-'X'); 
    // DEBUG("processMpg", MPGkey, MPGcnt, MPGswitch);
    char buffer50[50];
    sprintf(buffer50, "processMpg %d %d", MPGcnt, MPGswitch);
    debugDisplay(buffer50);
    if ((mystate.grblState == Idle || mystate.grblState == Jog) && (mystate.state == WDREHEN) && (MPGkey!= 0))  {     // https://github.com/gnea/grbl/wiki/Grbl-v1.1-Jogging
        // DEBUG(mystate.grblState, Idle, Jog, mystate.state, MPGkey);
        
        static char command[50];
        float speed;
        unsigned long time = millis();
        unsigned long dtime = time-mystate.mpgtime[index];
        mystate.mpgtime[index] = time;   
        // 
        if (MPGswitch == 0) {
          speed = 600.0 * float(abs(MPGcnt)) / float(dtime);        // 60*1000 MPGcnt *0.01 / (millis() - oldmillis)  				= mm/min
          if (speed < 1.00)
            speed = 1.00;
        }else if (MPGswitch == 1) 
          speed = target.fz*10;
        else
          speed = target.fz*100;
 
 /*       if ((millis()-mystate.buttontime)> 2*mystate.buttonDtime){
            target.fzOld = target.fz;
            Display.fillRect(120, COLUMN1+12+0*COLUM_DISTANCE-32, 50, 30, BackColor);
            Display.setFont(F_A10); 
            Display.setCursor(120,COLUMN1+12+0*COLUM_DISTANCE-32);Display.print("mm/min");
        }*/
        sprintf(command, "$J=G91 %c%.3f F%.1f", MPGkey, float(MPGcnt)*0.01, speed);   //e.q. $J=G91 Z1.000 F100.0   G91 = relative movement 
        serial_writeLn(command);
        DEBUG("cnt= ", MPGcnt, "time delta=", dtime, speed, command);      
    }          
/*    }else if ((mystate.grblState == Idle || mystate.grblState == Jog) && (mystate.state == WDREHEN) && (MPGkey== 0)){
        serial_putC(CMD_STOP); 
        serial0_writeLn("processMPGpress Stop");
        Display.fillRect(120, COLUMN1+12+0*COLUM_DISTANCE-32, 50, 30, BackColor);
        Display.setFont(F_A10); 
        Display.setCursor(120,COLUMN1+12+0*COLUM_DISTANCE-32);Display.print("mm/U");
        target.fz = target.fzOld;
        mystate.MPGkey = MPGkey;
        target.changed = true;               
    }else if ((millis()-mystate.buttontime)>mystate.buttonDtime)
        mystate.MPGkey = MPGkey;
        */
}




void processKeypress (int DROkey, int keydown, float rpm){
  // call from dro.c->DROProcessEvents 
    static char command[50];
    char *dbutton = (char*)"-1";
    DEBUG("processKeypress state=", DROkey, mystate.grblState, mystate.state, Idle, Alarm, Run); 
    switch (mystate.grblState){
          case Run:       // = 2
              DEBUG("  Run");   
              if (DROkey == 0) {
                  DEBUG("Stop");       
                  serial_putC(CMD_STOP);
              }
              break;      
          case Idle:   // = 1
              DEBUG("  Idle");
              if (mystate.state == WDREHEN){
                  if (DROkey == 0){                       // left button
                      if (rpm == 0.0) {
                          sprintf(command, "G94 F%.3f G90 G01 Z%.3f", target.fz, target.z);     // *100 ??
                      }else{
                          sprintf(command, "G95 F%.3f G90 G01 Z%.3f", target.fz, target.z);
                      }
                  }else if (DROkey == 1)              // right button
                      sprintf(command, "G00 Z0");     // Verfahren im Eilgang auf 0 
                  serial0_writeLn(command);
                  serial_writeLn(command);
                  break;
              }else if (mystate.state == WALARM){
                  DEBUG("processKeypress: case Idle: Alarm");
              }
          case Alarm:  //=5
              DEBUG("  processKeypress: case Alarm");
              unsigned int msgindex=0;
              char *buttonL = alarmcodes[0].buttonL;
              char *buttonR = alarmcodes[0].buttonR;
              if (mystate.error >0) {
                  msgindex=mystate.error;
                  if (msgindex > sizeof(errorcodes)/sizeof(structAlarmcodes)){
                      DEBUG("processKeypress error value to high -> set to 0", msgindex);
                      msgindex = 0;
                  }
                  buttonL = errorcodes[msgindex].buttonL;
                  buttonR = errorcodes[msgindex].buttonR;
                  DEBUG("processKeypress Error:", msgindex, "Buttons", buttonL, buttonR);                  
              }else if (mystate.alarm >0) {
                  msgindex=mystate.alarm;
                  if (msgindex > sizeof(alarmcodes)/sizeof(structAlarmcodes)){
                      DEBUG("processKeypress alarm value to high -> set to 0", msgindex);
                      msgindex = 0;
                  }
                  buttonL = alarmcodes[msgindex].buttonL;
                  buttonR = alarmcodes[msgindex].buttonR;
                  DEBUG("processKeypress Alarm:", msgindex, "Buttons", buttonL, buttonR);   
              }
              DEBUG("processKeypress index=", msgindex);
              if (DROkey==0)
                  dbutton = buttonL;
              else if (DROkey == 1) 
                  dbutton = buttonR;
              DEBUG("processKeypress Button=", dbutton);
              if (strlen(dbutton)>0){
                  if (strcmp(dbutton, "Home")==0){
                      DEBUG("processKeypress Home");
                      serial_putC(24);
                      sprintf(buffer10, "$X ");                 // unlock
                      serial0_writeLn(buffer10);
                      sprintf(buffer10, "$H");
                  }else if (strcmp(dbutton, "Reset")==0) {
                      DEBUG("processKeypress Reset");
                      serial_putC(24);                        //Reset send #24
                  }else if (strcmp(dbutton, "ok")==0) {
                      DEBUG("processKeypress ok");
                      serial_putC(24);                         //Reset send #24 
                      sprintf(buffer10, "$X");                 // unlock                      
                  }else if (strcmp(dbutton, "Unlock")==0) {
                      DEBUG("processKeypress Unlock");
                      serial_putC(24);                         //Reset send #24
                      sprintf(buffer10, "$X");                 // unlock
                  }
                  serial0_writeLn(buffer10);
                  serial_writeLn(buffer10);
                  delay(200);
              }
              break;
    }
}
