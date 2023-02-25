#include "dro.h"

typedef struct {
    bool Button1=false;
    bool Button2=false;
  }struButtons;

struButtons Sbutton;


/* ------------------
   Public FUNCTIONS
   ------------------ */
void Switch_init(void);
void Switch_loop(void);
void Switch_mode(int mode);
