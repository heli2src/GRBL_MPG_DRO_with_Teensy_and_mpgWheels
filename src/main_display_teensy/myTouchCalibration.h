
#include <XPT2046_Calibrated.h>

// -- CONFIGURATION --

#define TS_CS_PIN 3 
//#define TS_IRQ_PIN  2

#define TFT_CS_PIN 10
#define TFT_DC_PIN 9

static uint16_t const SCREEN_WIDTH    = 320;
static uint16_t const SCREEN_HEIGHT   = 240;
static uint8_t  const SCREEN_ROTATION = 1U;

enum class PointID { NONE = -1, A, B, C, COUNT };

// source points used for calibration
static TS_Point _screenPoint[] = {
  TS_Point( 13,  11), // point A
  TS_Point(312, 113), // point B
  TS_Point(167, 214)  // point C
};

// touchscreen points used for calibration verification
static TS_Point _touchPoint[] = {
  TS_Point(3723, 3575), // point A
  TS_Point( 459, 2130), // point B
  TS_Point(2063,  783), // point C
};

static TS_Calibration cal(
    _screenPoint[(int)PointID::A], _touchPoint[(int)PointID::A],
    _screenPoint[(int)PointID::B], _touchPoint[(int)PointID::B],
    _screenPoint[(int)PointID::C], _touchPoint[(int)PointID::C],
    SCREEN_WIDTH,
    SCREEN_HEIGHT
);


// -- LOCAL VARIABLES --

//#ifdef TIRQ_PIN
//XPT2046_Calibrated touch(TS_CS_PIN, TS_IRQ_PIN);
//#else
//XPT2046_Calibrated touch(TS_CS_PIN);
//#endif // TIRQ_PIN

// -----------------------------------------------------------------------------

// -- UTILITY ROUTINES (DECLARATION) --

//inline bool touched();


// -- UTILITY ROUTINES (DEFINITION) --

////inline bool touched() {
//#ifdef TIRQ_PIN
//  if (touch.tirqTouched()) {
    //return touch.touched();
//  }
//  return false;
//#else
//  return touch.touched();
//#endif
//}
