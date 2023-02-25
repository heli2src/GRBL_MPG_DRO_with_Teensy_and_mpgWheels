
#ifndef _GRBLCOMM_H_
#define _GRBLCOMM_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

#include "grbl.h"
#include "interface.h"

#define NUMSTATES 11

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define MAX_BLOCK_LENGTH 256


#define Black       0x0000
#define White       0xFFFF
#define C_WHITE       0xFFFF
#define C_GREEN       0x07E0
#define C_LTGREEN     0xBFF7
#define C_YELLOW      0xFFE0  
#define C_LTORANGE    0xFE73
#define C_RED         0xF800
#define C_LTBLUE      0xB6DF  

typedef enum {
    NOTCONNECT = -1,
    Unknown = 0,
    Idle,
    Run,
    Jog,
    Hold,
    Alarm,
    Check,
    Door,
    Tool,
    Home,
    Sleep
} grbl_state_t;

typedef union {
    uint16_t value;
    struct {
        uint8_t R;
        uint8_t G;
        uint8_t B;
    };
} RGBColor_t;

typedef struct {
    grbl_state_t state;
    uint8_t substate;
    //RGBColor_t state_color;
    uint16_t state_color;
    char state_text[8];
} grbl_t;

typedef struct {
    float x;
    float y;
    float z;
} coord_values_t;

typedef struct {
    float rpm_programmed;
    float rpm_actual;
    bool on;
    bool ccw;
    int32_t rpm_override;
} spindle_data_t;

typedef struct {
    bool flood;
    bool mist;
} coolant_data_t;

typedef union {
    uint32_t flags;
    struct {
        uint32_t mpg :1,
                 state :1,
                 xpos :1,
                 ypos :1,
                 zpos :1,
                 offset :1,
                 await_ack: 1,
                 await_wco_ok: 1,
                 leds :1,
                 dist : 1,
                 message: 1,
                 feed: 1,
                 rpm: 1,
                 alarm: 1,
                 error: 1,
                 xmode: 1,
                 pins: 1,
                 reset: 1,
                 feed_override: 1,
                 rapid_override: 1,
                 rpm_override: 1,
                 unassigned: 11;
    };
} changes_t;

typedef union {
    uint8_t flags;
    struct {
        uint32_t sd_card: 1,
                 lathe:   1,
                 tool_change: 1,
                 is_loaded: 1,
                 unassigned: 5;
    };
} grbl_options_t;

typedef struct {
    bool is_loaded;
    grbl_options_t options;
    char device[MAX_STORED_LINE_LENGTH];
} grbl_info_t;

typedef struct sd_file {
    uint32_t length;
    char name[MAX_STORED_LINE_LENGTH];
    struct sd_file *next;
} sd_file_t;

typedef struct {
    uint_fast16_t num_files;
    sd_file_t *files;
} sd_files_t;

typedef struct {
    grbl_t grbl;
    float position[3];
    float offset[3];
    spindle_data_t spindle;
    coolant_data_t coolant;
    int32_t feed_override;
    int32_t rapid_override;
    float feed_rate;
    bool useWPos;
    bool awaitWCO;
    bool absDistance;
    bool mpgMode;
    bool xModeDiameter;
    changes_t changed;
    uint8_t alarm;
    uint8_t error;
    char pins[10];
    char block[MAX_BLOCK_LENGTH];
    char message[MAX_BLOCK_LENGTH];
} grbl_data_t;

typedef struct {
    float fast_speed;
    float slow_speed;
    float step_speed;
    float fast_distance;
    float slow_distance;
    float step_distance;
} jog_config_t;

typedef struct {
    float rpm;
    float mpg_rpm;
    float rpm_min;
    float rpm_max;
} spindle_state_t;

typedef struct {
    bool is_loaded;
    bool homing_enabled;
    uint8_t mode;
    spindle_state_t spindle;
    jog_config_t jog_config;
} settings_t;

typedef void (*grbl_callback_ptr)(char *line);
typedef void (*grbl_settings_received_ptr)(settings_t *settings);
typedef void (*grbl_info_received_ptr)(grbl_info_t *info);
typedef void (*grbl_parser_state_received_ptr)(grbl_data_t *info);
typedef void (*grbl_sd_files_received_ptr)(sd_files_t *files);

grbl_data_t *setGrblReceiveCallback (grbl_callback_ptr fn);
void setGrblTransmitCallback (void (*fn)(bool ok, grbl_data_t *grbl_data));
void grblPollSerial (void);
void grblSerialFlush (void);
void grblClearAlarm (void);
void grblClearError (void);
void grblClearMessage (void);
void grblSendSerial (char *line);
bool grblParseState (char *state, grbl_t *grbl);
bool grblAwaitACK (const char *command, uint_fast16_t timeout_ms);
bool grblIsMPGActive (void);

void grblGetInfo (grbl_info_received_ptr on_info_received);
void grblGetSettings (grbl_settings_received_ptr on_settings_received);
void grblGetParserState (grbl_parser_state_received_ptr on_parser_state_received);
void grblGetSDFiles (grbl_sd_files_received_ptr on_sd_files_received);
grbl_options_t grblGetOptions (void);

void setGrblLegacyMode (bool on);
char mapRTC2Legacy (char c);
#endif

#ifdef __cplusplus
}
#endif
