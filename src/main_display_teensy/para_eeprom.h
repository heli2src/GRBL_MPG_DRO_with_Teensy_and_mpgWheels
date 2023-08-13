

struct EEPROMs                      // see https://www.arduino.cc/en/Reference/EEPROM
{
    char Copyright[6];
    char Version[10];
    uint8_t        BGred;            // Background red
    uint8_t        BGgreen;          // Background green
    uint8_t        BGblue;           // Background blue 
    float          fxmin;            // mm/min if spindle isn't running
    float          fymin;            // mm/min if spindle isn't running
    float          fzmin;            // mm/min if spindle isn't running
    float          fxU;              // mm/U default value if spindle is running
    float          fzU;              // mm/U default value if spindle is running
    float          fzjog001;         // mm/min if jogging with 0.01 mm/step
    float          fzjog01;          // mm/min if jogging with 0.1 mm/step   
    float          fzjog1;           // mm/min if jogging with 1mm/step
} eeprom;
