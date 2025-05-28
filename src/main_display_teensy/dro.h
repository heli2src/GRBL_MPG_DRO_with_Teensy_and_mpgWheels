//#include "../LCD/lcd.h"
#include <math.h>
#include "grblcomm.h"
#include "interface.h"

// Event flags
#define EVENT_DRO            (1<<0)
#define EVENT_MPG            (1<<1)
#define EVENT_SIGNALS        (1<<2)
#define EVENT_KEYDOWN        (1<<3)
#define EVENT_KEYUP          (1<<4)
#define EVENT_JOGMODECHANGED (1<<5)


#ifdef __cplusplus
 extern "C" {
#endif

#ifndef _MAIN_H_
#define _MAIN_H_

void DROInitCanvas (void);
void DROShowCanvas (void);         // (lcd_display_t *screen);
void DROProcessEvents(void);
void DROSet_EventDRO (void);
void DROkeyEvent (bool keyDown, char key);
void DROJoystickEvent (bool change, int key);
void DROmpgEvent (bool change, int key, int cnt, int modeswitch);
void DROprintOut (void);
void DROGetInfo(void);

#endif


#ifdef __cplusplus
}
#endif
