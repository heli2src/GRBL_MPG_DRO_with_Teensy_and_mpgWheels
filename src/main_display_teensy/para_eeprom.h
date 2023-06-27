

struct EEPROMs                      // see https://www.arduino.cc/en/Reference/EEPROM
{
    char Copyright[6];
    char Version[10];
    uint8_t        BGred;            // Background red
    uint8_t        BGgreen;          // Background green
    uint8_t        BGblue;           // Background blue 
    float          fzmin;            // mm/min 
} eeprom;
