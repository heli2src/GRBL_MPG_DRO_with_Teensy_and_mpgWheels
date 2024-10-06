

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <font_Arial.h>
//#include <font_ArialBold.h>
#include "main_display_teensy.h"
#include "dro.h"
#include "mydisplay.h"
#include "grblcomm.h"

#define RPMROW 200
#define STATUSROW 218
#define MSGROW 238
#ifdef LATHEMODE
#define XROW 110
#else
#define XROW 62
#endif
#define YROW XROW + 50
#define ZROW YROW + 50
#define POSCOL 190
#define POSFONT Arial_24

//colors
#define C_WHITE         0xFFFF
#define C_DKGREY        0x3186

char DROkey;
char MPGkey;
int MPGcnt;
int MPGdtime;

typedef struct {
    float mpg_base;
    float mpg_factor;
    uint_fast8_t mpg_idx;
    int32_t mpg_position;
    bool dro_lock;
    bool visible;
    uint16_t row;
//    Label *lblAxis; 
    const char *label;
} axis_data_t;

typedef struct {
    uint_fast16_t dro_refresh;
    uint_fast16_t mpg_refresh;
    uint_fast16_t signal_reset;
} event_counters_t;

//

volatile uint_fast8_t event = 0;
static uint_fast8_t mpg_axis = X_AXIS;
static float mpg_rpm = 200.0f;
static bool mpgMove = false, endMove = false;
static bool jogging = false, keyreleased = true, disableMPG = false, mpgReset = false, active = false;
static event_counters_t event_count;
static grbl_data_t *grbl_data = NULL;
static Canvas *canvasMain = 0;

//CJ static Label *lblDevice, *lblResponseL = NULL, *lblResponseR = NULL, *lblGrblState = NULL, *lblPinState = NULL, *lblFeedRate = NULL, *lblRPM = NULL, *lblJogMode = NULL;
static char *lblDevice, *lblResponseL = NULL, *lblResponseR = NULL, *lblGrblState = NULL, *lblPinState = NULL, *lblFeedRate = NULL, *lblRPM = NULL, *lblJogMode = NULL;

static settings_t *settings = NULL;
static grbl_info_t *grbl_info = NULL;
static axis_data_t axis[3] = {
    { .label = "X:"},
    { .label = "Y:"},
    { .label = "Z:"}
};
static event_counters_t event_interval = {
    .dro_refresh  = 20,
    .mpg_refresh  = 10,
    .signal_reset = 20
};

char *ftoa (float value, char *format)
{
    static char buffer[30];

    sprintf(buffer, format, value);
    return buffer;
}

static void MPG_ResetPosition (bool await)
{
//CJ    mpg_reset();
    axis[X_AXIS].mpg_position = 0;
    axis[Y_AXIS].mpg_position = 0;
    axis[Z_AXIS].mpg_position = 0;
    if(!(grbl_data->awaitWCO = await)) {
        axis[X_AXIS].mpg_base = grbl_data->position[X_AXIS];
        axis[Y_AXIS].mpg_base = grbl_data->position[Y_AXIS];
        axis[Z_AXIS].mpg_base = grbl_data->position[Z_AXIS];
    }
}


static void displayPosition (uint_fast8_t i){
    if(axis[i].visible) {
        ///CJ setColor(axis[i].dro_lock ? Yellow : White);
        drawString(i, &POSFONT, POSCOL, axis[i].row, grbl_data->position[i] - grbl_data->offset[i], ftoa(grbl_data->position[i] - grbl_data->offset[i], "% 5.3f"), true);
        //CJ setColor(White);
    }
}


static void on_settings_received (settings_t *setn)
{
    settings = setn;

    if(settings->is_loaded) {

        if((settings->mode == 2)) {
            axis[Y_AXIS].visible = false;
            axis[Z_AXIS].row = YROW;
            axis[Z_AXIS].visible = true;
//CJ            displayXMode("?");
        }
    }

//CJ    displayBanner(grbl_data->mpgMode ? (settings->is_loaded ? Blue : Red) : White);

    if(grbl_data->mpgMode) // Get parser state
        serial_writeLn("$G");
    MyDisplay_LedMPG(grbl_data->mpgMode);
}

static void on_info_received (grbl_info_t *info)
{
    grbl_info = info;

    if(settings == NULL || !settings->is_loaded)
        grblGetSettings(on_settings_received);
}

static void displayGrblData (char *line)
{
    uint32_t c;

    if(endMove || mpgReset)
        grbl_data->changed.offset = true;

    if(grbl_data->changed.flags) {
        if(grbl_data->changed.reset)
            settings->is_loaded = false;

        if(grbl_data->changed.state) {
            set_grblstate(grbl_data->grbl.state, grbl_data->grbl.state_text, grbl_data->grbl.state_color, grbl_data->alarm, grbl_data->error, grbl_info->options.lathe);   // update the state to mydisplay
//CJ            if  (grbl_data->grbl.state != Alarm)      
//CJ            leds.run = grbl_data->grbl.state == Run || grbl_data->grbl.state == Jog;
//CJ            leds.hold = grbl_data->grbl.state == Hold;
//CJ            leds_setState(leds);

            switch(grbl_data->grbl.state) {
                case Idle:
//CJ                    MPG_ResetPosition(false);
                    break;

                case Home:
//CJ                    signal_setLimitsOverride(false);
                    break;

                case Alarm: // Switch to MPG mode if alarm #11 issued (homing required)  
//CJ                    if(grbl_data->grbl.substate == 11 && !settings->is_loaded && !grbl_data->mpgMode)
//CJ                        signal_setMPGMode(true); // default is MPG on
                    break;
                default:
                    break;
            }
        }

        if(grbl_data->changed.alarm) {
            if(grbl_data->alarm) {
            }
        }

        if(grbl_data->changed.error) {
            if(grbl_data->error) {
            }
        }

        //if(grbl_data->changed.message)
        if(grbl_data->changed.offset || grbl_data->changed.await_wco_ok) {
            if((mpgReset || grbl_data->changed.await_wco_ok) && grbl_data->grbl.state == Idle) {
                mpgReset = false;
//CJ                MPG_ResetPosition(false);
            }
            grbl_data->changed.xpos =
            grbl_data->changed.ypos =
            grbl_data->changed.zpos = true;
        }

        if (!mpgMove) {

            if(grbl_data->changed.xpos)
                displayPosition(X_AXIS);

            if(grbl_data->changed.ypos)
                displayPosition(Y_AXIS);

            if(grbl_data->changed.zpos) {
                displayPosition(Z_AXIS);
            }

            endMove = false;
        }

        if(grbl_data->changed.mpg) {
//CJ            if(grbl_data->mpgMode != signal_getMPGMode())
//CJ                   MyDisplay_LedMPG(grbl_data->mpgMode);
//CJ            keypad_forward(!grbl_data->mpgMode);
            if(grbl_data->mpgMode) {
                if(grbl_info->is_loaded)
                    grblAwaitACK("$G", 50);
//CJ                MPG_ResetPosition(true);
//CJ                displayMPGFactor(X_AXIS, axis[X_AXIS].mpg_idx);
//CJ                displayMPGFactor(Y_AXIS, axis[Y_AXIS].mpg_idx);
//CJ                displayMPGFactor(Z_AXIS, axis[Z_AXIS].mpg_idx);
            } else {
                c = 3;
                do {
                    if(axis[--c].dro_lock) {
                        axis[c].dro_lock = false;
                        displayPosition(c);
                    }
//CJ                    setMPGFactorBG(c, Black);
                } while(c);                
//CJ #ifdef UART_MODE
//CJ                if(jogModePending) {
//CJ                    jogModePending = false;
//CJ                    serial_putC('0' + (char)jogMode);
//CJ                }
//CJ #endif
            }
            MyDisplay_LedMPG(grbl_data->mpgMode);
//CJ            leds.mode = grbl_data->mpgMode;
//CJ            leds_setState(leds);
//CJ            displayBanner(grbl_data->mpgMode ? (settings->is_loaded ? Blue : Red) : White);
        }

//CJ        if(grbl_data->changed.feed) {
//CJ            sprintf(line, grbl_data->feed_rate < 1000.0f ? "%6.1f" : "%5.f", grbl_data->feed_rate);
            //serial0_writeLn(line);   
//CJ            UILibLabelDisplay(lblFeedRate, line);
//CJ        }

        if(grbl_data->changed.rpm) {
//CJ            bool display_actual = grbl_data->spindle.on && grbl_data->spindle.rpm_actual > 0.0f;
//CJ            if(grbl_data->spindle.rpm_programmed > 0.0f)
//CJ                mpg_rpm = grbl_data->spindle.rpm_programmed;
//CJ                if (grbl_data->spindle.rpm_actual >0.0f) 
//CJ                    DisplayClear('A',100);                
//CJ            if(display_actual || leds.spindle != grbl_data->spindle.on) {
//CJ                if(display_actual){ 
//CJ                sprintf(line, "%6.1f", display_actual ? grbl_data->spindle.rpm_actual : mpg_rpm);
                    drawString(3, &POSFONT,POSCOL+40, 60, grbl_data->spindle.rpm_actual, ftoa(grbl_data->spindle.rpm_actual, "% 4.0f"), true);
//CJ                lblRPM->widget.fgColor = display_actual ? (grbl_data->spindle.rpm_actual > 2000.0f ? Coral : White) : Coral;
//CJ                UILibLabelDisplay(lblRPM, line);
            //CJ}
        }

//CJ        if(grbl_data->changed.leds) {
//CJ            leds.mist = grbl_data->coolant.mist;
//CJ            leds.flood = grbl_data->coolant.flood;
//CJ            leds.spindle = grbl_data->spindle.on;
//CJ            leds_setState(leds);
//CJ        }
  
//CJ        if(grbl_data->changed.pins)
//CJ            UILibLabelDisplay(lblPinState, grbl_data->pins);

//CJ        if(grbl_data->changed.xmode && isLathe)
//CJ            displayXMode(grbl_data->xModeDiameter ? "D" : "R");

//CJ        if(grbl_data->changed.await_wco_ok)
//CJ            MPG_ResetPosition(false);

        grbl_data->changed.flags = 0;
    }
    
    if(grbl_data->mpgMode) {
        if(grbl_info == NULL || !grbl_info->is_loaded)
           grblGetInfo(on_info_received);
    }
}

static unsigned long eventSignal;
void DROProcessEvents (void)
{
     if(!active)        
        return;

     if(event) {                  
        if(event & EVENT_DRO) {
            event &= ~EVENT_DRO;
//            if(!mpgMove && settings->is_loaded)
//                serial_putC(grbl_data->awaitWCO ? CMD_STATUS_REPORT_ALL : mapRTC2Legacy(CMD_STATUS_REPORT)); // Request realtime status from grbl
              //serial_putC(CMD_STATUS_REPORT_ALL);
              serial_putC(mapRTC2Legacy(CMD_STATUS_REPORT));         
        }
     }
     if(event & EVENT_KEYDOWN) {
          processKeypress(DROkey, EVENT_KEYDOWN, grbl_data->spindle.rpm_actual);    // from mydisplay.ino
          //if(!keypad_has_keycode())
          event &= ~EVENT_KEYDOWN;  
     }
     if(event & EVENT_KEYUP) {
          processKeypress(DROkey, EVENT_KEYUP, grbl_data->spindle.rpm_actual);
          event &= ~EVENT_KEYUP;       
     }

     if(event & EVENT_MPG) {
          //processJoystick(MPGkey);                    //from mydisplay.ino
          processMpg(MPGkey, MPGcnt, MPGdtime);         //from mydisplay.ino          
          event &= ~ EVENT_MPG;       
     }

     if(event & EVENT_SIGNALS){
          eventSignal = lcd_systicks();
          event &= ~ EVENT_SIGNALS;
     }
}


static unsigned long eventTimeDRO;
void DROSet_EventDRO (void)
{
    if (lcd_systicks() - eventTimeDRO > 200){
        eventTimeDRO = lcd_systicks();
        grbl_data = setGrblReceiveCallback(displayGrblData);
        event |= EVENT_DRO;
    }
}


//static void canvasHandler (Widget *self, Event *uievent)
//{
//
//}


void DROInitCanvas (void)
{
//  int_fast8_t i;  
   serial0_writeLn("start DROInitCanvas");

  eventTimeDRO = lcd_systicks();
  
#ifdef LATHEMODE
    axis[X_AXIS].row = XROW;
    axis[X_AXIS].visible = true;
    axis[Y_AXIS].row = YROW;
    axis[Y_AXIS].visible = false;
    axis[Z_AXIS].row = YROW;
    axis[Z_AXIS].visible = true;
#else
    axis[X_AXIS].row = XROW;
    axis[X_AXIS].visible = true;
    axis[Y_AXIS].row = YROW;
    axis[Y_AXIS].visible = true;
    axis[Z_AXIS].row = ZROW;
    axis[Z_AXIS].visible = true;
#endif

    axis[X_AXIS].label = "X:";
    axis[Y_AXIS].label = "Y:";
    axis[Z_AXIS].label = "Z:";

//    i = 3;
//    do {
//        //cj axis[--i].mpg_factor = mpgFactors[axis[i].mpg_idx];
//    } while(i);        
}


void DROGetInfo(void) 
{
  grblGetInfo(on_info_received);
}

void DROprintOut(void)
{
  static char buffer[80];
  sprintf(buffer, "DROprintOut: is_loaded=%d, lathe=%d, mpg=%d, %s ", grbl_info->is_loaded, grbl_info->options.lathe, grbl_data->mpgMode, grbl_info->device);
  serial0_writeLn(buffer);
}

void DROkeyEvent (bool keyDown, char key)
{
    static char buffer[80];
    // NOTE: key is read from input buffer during event processing
    if(keyDown)
        event |= EVENT_KEYDOWN;
    else
        event |= EVENT_KEYUP;
    DROkey = key;
    keyreleased = !keyDown;
    sprintf(buffer, "DROkeyEvent: keyDown=%d, key=%d ", keyDown, key);
    serial0_writeLn(buffer);
}

void DROJoystickEvent (bool change, int key)
{
    if (change)
       event |= EVENT_MPG;
    MPGkey = key;
}

void DROmpgEvent (bool change, int key, int cnt, int dtime)
{
    if (change)
       event |= EVENT_MPG;
    MPGkey = key; 
    MPGcnt = cnt;
    MPGdtime = dtime;
}
  

void DROShowCanvas (void)           // (lcd_display_t *lcd_screen)
{  
   int_fast8_t i;
   
   if(!canvasMain) {

//       for(i = 0; i < 3; i++) {
//            if(axis[i].visible) {
//                axis[i].lblAxis = UILibLabelCreate((Widget *)canvasMain, &POSFONT, C_WHITE, 0, axis[i].row, POSCOL - 5, NULL);     //POSFONT
//                axis[i].lblAxis->widget.flags.alignment = Align_Right;
//            }
//        }    
   }

   if(grbl_info == NULL) {
       grblGetInfo(on_info_received);
   }

   active = true;
}
