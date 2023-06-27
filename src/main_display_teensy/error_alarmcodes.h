
typedef struct {
   char *const buttonL;
   char *const buttonR;
   char *const msg;
}structAlarmcodes;

structAlarmcodes alarmcodes []= {
    {"Unlock", "Home", "unknown Alarm :-( "},
    {"",       "Home", "Hard limit has been\ntriggered.\n\nDo Re-homing"},                   //1
    {"",       "Unlock", "Soft limit alarm.\nG-code motion target\nexceeds machine travel.\nAlarm may be safely\nunlocked"},         //2
    {"",       "Home", "Reset while in\nmotion. Re-homing"},                                //3
    {"",       "", "Probe fail. "},                                                         //4
    {"",       "", "Probe fail. "},                                                         //5
    {"",       "Home", "Homing fail.\n The active homing cycle was reset"},                 //6
    {"",       "Home", "Homing fail.\n Safety door was opening during homing cycle"},       //7
    {"",       "Home", "Homing fail.\n Pull of travel faild to clear limit switch"},        //8
    {"",       "Home", "Homing fail.\n Could not find limit switch\n within search distances"}, //9
    {"",       "Reset", "EStop asserted.\nClear and reset"},                                 //10
    {"",       "Home", "Homing required."},                                                 //11
    {"",       "Reset", "Limit switch engaged.\nClear before continuning"},                  //12
    {"",       "Reset", "Probe protection triggered,\nClear before continuing"},             //13
    {"",       "Reset", "Spindle at speed timeout.\nClear before continuing"},               //14
    {"",       "Home", "Homing fail.\nCould not find second limit\nswitch for auto squared axis\nwithin search distances"}, //15
    {"",       "", "Power on selftest (POS) failed"},   //16
    {"",       "", "Motor fault."},                     //17
    {"",       "", "Homing fail. Bad configuration."},  //18
    {"",       "", " "},                                //19    
    {"",       "Reset", "MPG Joystick active\n -> switch off!"}//20    
};  


structAlarmcodes errorcodes []= {
  {"","ok", "unknown Error :-( "},                                                                            //0
  {"","ok", "Expected command letter,\nG-code words consist of a\nletter and a value.\nLetter was not found."},    //1
  {"","ok", "Bad number format,\nMissing the expected\nG-code word value\nor numeric value format\nis not valid."}, //2
  {"","ok", "Invalid statement,\nGrbl '$' system command\nwas not recognized or\nsupported."},  //3
  {"","ok", "Value < 0,Negative value\nreceived for an expected\npositive value."}, //4
  {"","ok", "Setting disabled,Homing cycle failure. Homing is not enabled via settings."}, //5
  {"","ok", "Value < 3 usec,Minimum step pulse time must be greater than 3usec."},  //6
  {"","ok", "EEPROM read fail.\nUsing defaults,An EEPROM read failed. Auto-restoring affected EEPROM to default values."}, //7
  {"","ok", "Not idle,Grbl '$' command cannot be used unless Grbl is IDLE. Ensures smooth operation during a job."},      //8
  {"","ok", "G-code lock,G-code commands are locked out during alarm or jog state."},  //8
  {"","ok", "Homing not enabled,Soft limits cannot be enabled without homing also enabled."},  //9
  {"","ok", "Line overflow,Max characters per line exceeded. Received command line was not executed."},  //10
  {"","ok", "Step rate > 30kHz,Grbl '$' setting value cause the step rate to exceed the maximum supported."},  //11
  {"","ok", "Check Door,Safety door detected as opened and door state initiated."},  //12
  {"","ok", "Line length exceeded,Build info or startup line exceeded EEPROM line length limit. Line not stored."}, //13
  {"","Reset", "Travel exceeded, Jog\ntarget exceeds machine\ntravel. Jog command \nhas been ignored."},   //14
  {"","ok", "Invalid jog command,\nJog command has no '=' or\ncontains prohibited g-code."},  //15
  {"","ok", "Setting disabled,\nLaser mode requires PWM output."},  //16
  {"","ok", "Reset asserted,"}, //17
  {"","ok", "Non positive value"}, //18
  {"","ok", "Unsupported command,\nUnsupported or invalid\ng-code command found\nin block."},   //19
  {"","ok", "Modal group violation,\nMore than one g-code command\nfrom same modal group found in block."}, //20
  {"","ok", "Undefined feed rate,\nFeed rate has not yet been set\nor is undefined."},   //21
  {"","ok", "Invalid gcode ID:23,\nG-code command in block\nrequires an integer value."},  //22
  {"","ok", "Invalid gcode ID:24,\nMore than one g-code command\nthat requires axis words found in block."},  //23
  {"","ok", "Invalid gcode ID:25,\nRepeated g-code word found\nin block."}, //24
  {"","ok", "Invalid gcode ID:26,\nNo axis words found in\nblock for g-code command or current modal state which requires them."},   //25
  {"","ok", "Invalid gcode ID:27,\nLine number value is\ninvalid."},   //26
  {"","ok", "Invalid gcode ID:28,\nG-code command is missing\na required value word."},   //17
  {"","ok", "Invalid gcode ID:29,\nG59.x work coordinate\nsystems are not supported."},   //28
  {"","ok", "Invalid gcode ID:30,\nG53 only allowed with G0 and\nG1 motion modes."},      //29
  {"","ok", "Invalid gcode ID:31,\nAxis words found in block\nwhen no command or current modal state uses them."},  //30
  {"","ok", "Invalid gcode ID:32,\nG2 and G3 arcs require at\nleast one in-plane axis word."},   //31
  {"","ok", "Invalid gcode ID:33,\nMotion command target is\ninvalid."},    //32
  {"","ok", "Invalid gcode ID:34,\nArc radius value is invalid."},         //33
  {"","ok", "Invalid gcode ID:35,\nG2 and G3 arcs require at least one in-plane offset word."},    //34}
  {"","ok", "Invalid gcode ID:36,\nUnused value words found in block."},    //35
  {"","ok", "Invalid gcode ID:37,\nG43.1 dynamic tool length offset is not assigned to configured tool length axis."},   //36
  {"","ok", "Invalid gcode ID:38,\nTool number greater than max supported value or undefined tool selected."},           //37
  {"","ok", "Invalid gcode ID:39,\nValue out of range."},    //38
  {"","ok", "Invalid gcode ID:40,\nG-code command not allowed when tool change is pending."},   //39
  {"","ok", "Invalid gcode ID:41,\nSpindle not running when motion commanded in CSS or spindle sync mode."},    //40
  {"","ok", "Invalid gcode ID:42,\nPlane must be ZX for threading."},   //41
  {"","ok", "Invalid gcode ID:43,\nMax. feed rate exceeded."},          //42
  {"","ok", "Invalid gcode ID:44,\nRPM out of range."},                 //43
  {"","ok", "Limit switch engaged,\nOnly homing is allowed when a limit switch is engaged."},    //44
  {"","ok", "Homing required,\nHome machine to continue."},             //45
  {"","ok", "Invalid gcode ID:47,\nATC: current tool is not set. Set current tool with M61."},   //46
  {"","ok", "Invalid gcode ID:48,\nValue word conflict."},              //47
  {"","ok", "E-stop,Emergency stop active."},                         //48
  {"","ok", "Error 49"},   //49
  {"","ok", "SD Card,SD Card mount failed."},                         //50
  {"","ok", "Error 51"},   //51
  {"","ok", "Error 52"},   //52
  {"","ok", "Error 53"},   //53
  {"","ok", "Error 54"},   //54
  {"","ok", "Error 55"},   //55
  {"","ok", "Error 56"},   //56
  {"","ok", "Error 57"},   //57
  {"","ok", "Error 58"},   //58
  {"","ok", "Error 59"},   //59  
  {"","ok", "Error 60"},   //60
  {"","ok", "SD Card,\nSD Card file open/read failed."},                //61
  {"","ok", "SD Card,\nSD Card directory listing failed."},             //63
  {"","ok", "SD Card,\nSD Card directory not found."},                  //63
  {"","ok", "SD Card,\nSD Card file empty."},                           //64
  {"","ok", "Error 65"},   //65
  {"","ok", "Error 66"},   //66
  {"","ok", "Error 67"},   //67
  {"","ok", "Error 68"},   //68
  {"","ok", "Error 69"},   //69
  {"","ok", "Bluetooth,\nBluetooth initalisation failed."}             //70
};