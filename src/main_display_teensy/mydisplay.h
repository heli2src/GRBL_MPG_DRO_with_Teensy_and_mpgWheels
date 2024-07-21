/*
**************************************************************************
**
*/
/**!   \file diplay_ILI9341.h		
**     \brief Header file for the display
**      Display 240 x 320     
**
**
**	-----------------------------------------------------------------------
** \par           Copyright (c) 2023 Heli2
**
** \author        C.Jung
** \version       V0.01
**
**************************************************************************
*/
#include "font_Arial.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* -------
   DEFINES
  -------- */
#define BLINKDELAY 500

//defining fonts
#define F_A72 Arial_72
#define F_A24 Arial_24
#define F_A20 Arial_20
#define F_A14 Arial_14
#define F_A10 Arial_10
#define FONT_LBUTTON Arial_16
#define FONT_SBUTTON Arial_12_Bold

#define STATE_MAX   2
#define TFT_DC  9
#define TFT_CS 10

// some color defs
#define BACKCOLOR BackColor
#define TEXTCOLOR TextColor
  
#define TASTEDELAY 200
#define MAXBUTTONS 15
#define MINBUTTONWIDTH 70
#define BUTTONHEIGHT 40
#define MAXCHECKBOX  1
#define INPUTBUFFER 40

#define BROW1 70
#define BROW2 120
#define BROW3 170

#define Font   ILI9341_t3_font_t

typedef enum {
   Align_Left,
   Align_Right,
   Align_Center,
} align_t;

typedef struct {
  float fx;         // value for Fx mm/U
  float fy; 
  float fz; 
  float fzOld; 
  float fxmin;      // value for Fx mm/min
  float fymin;      // value for Fy mm/min
  float fzmin;      // value for Fz mm/min
  float x;
  float y;
  float z;
  bool changed;
} struTarget;

typedef enum {
    WSTART   =  0,          // Wellcome Page
    Wmain  =  1,            // Drehen Page 
    WNUM = 2,               // Virtual numeric keypad
    WAOFFSET = 3,           // Set achs offset to 0
    WMENUE = 4,             // Main Menue
    WALARM = 5,             // display alarm messages
    WHOME = 6,              // go to position 
    WRESET = 7,             // send reset & unlock
    WDEFAULT = 8,           // set default values
    WEND = 9
} EnumWidgetState;

typedef enum {
//    WidgetCanvas      = 0x00,
//    WidgetButton      = 0x01,
//    WidgetList        = 0x02,
//    WidgetListElement = 0x04,
//    WidgetFrame       = 0x08,
//    WidgetImage       = 0x10,
    WidgetLabel       = 0x20,
//    WidgetTextBox     = 0x40,
//    WidgetCheckBox    = 0x80,
    WidgetAll         = 0xFF
} WidgetType;

  typedef enum {
      EventNullEvent = 0,
      EventPointerDown = 100,
      EventPointerUp,
      EventPrePointerChange,
      EventPointerChanged,
      EventPointerEnter,
      EventPointerLeave,
      EventListOffStart,
      EventListOffEnd,
      EventWidgetPainted,
      EventWidgetClose,
      EventWidgetClosed,
      EventKeyDown,
      EventKeyUp,
      EventValidate,
      EventRemote,
      EvenUserDefined
  } EventReason;

typedef union {
    uint32_t value;
    struct {
        uint32_t selected    :1,
                 highlighted :1,
                 noBox       :1,
                 hidden      :1,
                 visible     :1,
                 disabled    :1,
                 opaque      :1,
                 allocated   :1,
                 alignment   :2;
    };
} WidgetFlags;

typedef struct Event {
    EventReason reason;
    uint16_t x;
    uint16_t y;
    void *data;
    bool claimed;
} Event;

typedef struct Widget {
    WidgetType type;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t xMax;
    uint16_t yMax;
    uint16_t fgColor;
    uint16_t bgColor;
    uint16_t disabledColor;
    WidgetFlags flags;
    void *privateData;
    void (*eventHandler)(struct Widget *self, Event *event);
    struct Widget *parent;
    struct Widget *firstChild;
    struct Widget *lastChild;
    struct Widget *prevSibling;
    struct Widget *nextSibling;
} Widget;

typedef struct Canvas {
    Widget widget;
    int8_t tabNav;
  uint16_t bgColor;
} Canvas;

typedef struct {
    Widget widget;
    Font font;
    char *string;
} Label;

typedef enum {
   Cinit,
   Crun,
   Cend,
   Ckeys
} call_enum_t;

typedef struct call {
  call_enum_t excecute;
  int ipara;
} call_t;

typedef struct {
    int x;
    int y;
    int TextColor;
    char Typ[4];    
    char Text [26];     // maximum length from a string +1
    int Next;
} struPage;

/* ------------------
   Public VARIABLES
   ------------------ 
*/


/* ------------------
   Public FUNCTIONS
   ------------------ */
   
void MyDisplay_init(void);
void MyDisplay_loop(void);
void drawString(uint_fast8_t i, const ILI9341_t3_font_t *font, uint16_t x, uint16_t y, float value, const char *string, bool opaque);
void set_grblstate(int value, const char* string, uint16_t color, int alarm, int error, int lathe);
void processKeypress(int DROkey, int keydown, float rpm);
void processJoystick(int MPGkey);
void processMpg (char MPGkey, int MPGcnt, int MPGswitch);


// Label *UILibLabelCreate(Widget *parent, ILI9341_t3_font_t *font, uint16_t fgColor, uint16_t x, uint16_t y, uint16_t width, void (*eventHandler)(Widget *self, Event *event));


#ifdef __cplusplus
}
#endif
