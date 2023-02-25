
#include <stdint.h>
//#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

//typedef bool (*on_serial_block_ptr)(void);
//
//
//typedef struct interface
//{
////    on_keyclick_ptr on_keyclick;
////    on_keyclick_ptr on_keyclick2;
//    on_serial_block_ptr on_serial_block;
////    on_jogModeChanged_ptr on_jogModeChanged;
////    on_mpgChanged_ptr on_mpgChanged;
////    on_navigator_event_ptr on_navigator_event;
//} interface_t;
//
//extern interface_t interface;
//
//
//
//
//
//
//
//
//extern void serial_init (void);
int16_t serial_getC (void);
bool serial_putC (const char c);
void serial_writeLn (const char *data);
void serial0_writeLn (const char *data);
void serial0_write (const char data);
void serial_RxCancel (void);


unsigned long lcd_systicks(void);

#ifdef __cplusplus
}
#endif
