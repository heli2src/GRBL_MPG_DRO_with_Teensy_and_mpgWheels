
typedef struct {
   char *const buttonL;
   char *const buttonR;
   char *const msg;
}structAlarmcodes;

structAlarmcodes alarmcodes []= {
    {(char *const)"Unlock", (char *const)"Home", (char *const)"unknown Alarm :-( "},
    {(char *const)"Unlock", (char *const)"Home", (char *const)"Hard limit has been\ntriggered.\n\nDo Unlock/Re-homing"},                   //1
    {(char *const)"",       (char *const)"Unlock", (char *const)"Soft limit alarm.\nG-code motion target\nexceeds machine travel.\nAlarm may be safely\nunlocked"},         //2
    {(char *const)"",       (char *const)"Home", (char *const)"Reset while in\nmotion. Re-homing"},                                //3
    {(char *const)"",       (char *const)"", (char *const)"Probe fail. "},                                                         //4
    {(char *const)"",       (char *const)"", (char *const)"Probe fail. "},                                                         //5
    {(char *const)"",       (char *const)"Home", (char *const)"Homing fail.\n The active homing cycle was reset"},                 //6
    {(char *const)"",       (char *const)"Home", (char *const)"Homing fail.\n Safety door was opening during homing cycle"},       //7
    {(char *const)"",       (char *const)"Home", (char *const)"Homing fail.\n Pull of travel faild to clear limit switch"},        //8
    {(char *const)"",       (char *const)"Home", (char *const)"Homing fail.\n Could not find limit switch\n within search distances"}, //9
    {(char *const)"",       (char *const)"Reset", (char *const)"EStop asserted.\nClear and reset"},                                 //10
    {(char *const)"",       (char *const)"Home", (char *const)"Homing required."},                                                 //11
    {(char *const)"",       (char *const)"Reset", (char *const)"Limit switch engaged.\nClear before continuning"},                  //12
    {(char *const)"",       (char *const)"Reset", (char *const)"Probe protection triggered,\nClear before continuing"},             //13
    {(char *const)"",       (char *const)"Reset", (char *const)"Spindle at speed timeout.\nClear before continuing"},               //14
    {(char *const)"",       (char *const)"Home", (char *const)"Homing fail.\nCould not find second limit\nswitch for auto squared axis\nwithin search distances"}, //15
    {(char *const)"",       (char *const)"", (char *const)"Power on selftest (POS) failed"},   //16
    {(char *const)"",       (char *const)"", (char *const)"Motor fault."},                     //17
    {(char *const)"",       (char *const)"", (char *const)"Homing fail. Bad configuration."},  //18
    {(char *const)"",       (char *const)"", (char *const)" "},                                //19    
    {(char *const)"",       (char *const)"Reset", (char *const)"MPG Joystick active\n -> switch off!"}//20    
};  


structAlarmcodes errorcodes []= {
  {(char *const)"", (char *const)"ok",    (char *const)"unknown Error :-( "},                                                                            //0
  {(char *const)"", (char *const)"ok",    (char *const)"Expected command letter,\nG-code words consist of\na letter and a value.\nLetter was not found."},    //1
  {(char *const)"", (char *const)"ok",    (char *const)"Bad number format,\nMissing the expected\nG-code word value\nor numeric value format\nis not valid."}, //2
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid statement,\nGrbl '$' system command\nwas not recognized or\nsupported."},  //3
  {(char *const)"", (char *const)"ok",    (char *const)"Value < 0,Negative value\nreceived for an expected\npositive value."}, //4
  {(char *const)"", (char *const)"ok",    (char *const)"Setting disabled,Homing cycle failure. Homing is not enabled via settings."}, //5
  {(char *const)"", (char *const)"ok",    (char *const)"Value < 3 usec,Minimum step pulse time must be greater than 3usec."},  //6
  {(char *const)"", (char *const)"ok",    (char *const)"EEPROM read fail.\nUsing defaults,An EEPROM read failed. Auto-restoring affected EEPROM to default values."}, //7
  {(char *const)"", (char *const)"Reset", (char *const)"Not idle,Grbl '$'\ncommand cannot be\nused unless Grbl is\nIDLE. Ensures smooth\n operation during a job."},      //8
  {(char *const)"", (char *const)"Reset", (char *const)"G-code lock,\nG-code commands are\nlocked out during alarm\n or jog state."},  //8
  {(char *const)"", (char *const)"ok",    (char *const)"Homing not enabled,Soft limits cannot be enabled without homing also enabled."},  //9
  {(char *const)"", (char *const)"ok",    (char *const)"Line overflow,Max characters per line exceeded. Received command line was not executed."},  //10
  {(char *const)"", (char *const)"ok",    (char *const)"Step rate > 30kHz,Grbl '$' setting value cause the step rate to exceed the maximum supported."},  //11
  {(char *const)"", (char *const)"ok",    (char *const)"Check Door,Safety door detected as opened and door state initiated."},  //12
  {(char *const)"", (char *const)"ok",    (char *const)"Line length exceeded,Build info or startup line exceeded EEPROM line length limit. Line not stored."}, //13
  {(char *const)"", (char *const)"Reset", (char *const)"Travel exceeded, Jog\ntarget exceeds machine\ntravel. Jog command \nhas been ignored."},   //14
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid jog command,\nJog command has no\n'=' or contains prohibited\ng-code."},  //15
  {(char *const)"", (char *const)"ok",    (char *const)"Setting disabled,\nLaser mode requires PWM output."},  //16
  {(char *const)"", (char *const)"ok",    (char *const)"Reset asserted,"}, //17
  {(char *const)"", (char *const)"ok",    (char *const)"Non positive value"}, //18
  {(char *const)"", (char *const)"ok",    (char *const)"Unsupported command,\nUnsupported or invalid\ng-code command found\nin block."},   //19
  {(char *const)"", (char *const)"ok",    (char *const)"Modal group violation,\nMore than one g-code command\nfrom same modal group found in block."}, //20
  {(char *const)"", (char *const)"ok",    (char *const)"Undefined feed rate,\nFeed rate has not yet\nbeen set nor is\nundefined."},   //21
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:23,\nG-code command in block\nrequires an integer value."},  //22
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:24,\nMore than one g-code command\nthat requires axis words found in block."},  //23
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:25,\nRepeated g-code word found\nin block."}, //24
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:26,\nNo axis words found in\nblock for g-code command\nor current modal state which\nrequires them."},   //25
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:27,\nLine number value is\ninvalid."},   //26
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:28,\nG-code command is\nmissing a required value\nword."},   //17
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:29,\nG59.x work coordinate\nsystems are not supported."},   //28
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:30,\nG53 only allowed with G0 and\nG1 motion modes."},      //29
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:31,\nAxis words found in block\nwhen no command or current\nmodal state uses them."},  //30
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:32,\nG2 and G3 arcs require at\nleast one in-plane axis word."},   //31
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:33,\nMotion command target is\ninvalid."},    //32
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:34,\nArc radius value is invalid."},         //33
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:35,\nG2 and G3 arcs require at least\none in-plane offset word."},    //34}
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:36,\nUnused value words found in block."},    //35
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:37,\nG43.1 dynamic tool length\noffset is not assigned\nto configured tool length axis."},   //36
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:38,\nTool number greater than\nmax supported value or\nundefined tool selected."},           //37
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:39,\nValue out of range."},    //38
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:40,\nG-code command not allowed\nwhen tool change is pending."},   //39
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:41,\nSpindle not running when\nmotion commanded in CSS\nor spindle sync mode."},    //40
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:42,\nPlane must be ZX for threading."},   //41
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:43,\nMax. feed rate exceeded."},          //42
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:44,\nRPM out of range."},                 //43
  {(char *const)"", (char *const)"ok",    (char *const)"Limit switch engaged,\nOnly homing is allowed when\na limit switch is engaged."},    //44
  {(char *const)"", (char *const)"ok",    (char *const)"Homing required,\nHome machine to continue."},             //45
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:47,\nATC: current tool is not set.\n Set current tool with M61."},   //46
  {(char *const)"", (char *const)"ok",    (char *const)"Invalid gcode ID:48,\nValue word conflict."},             //47
  {(char *const)"", (char *const)"ok",    (char *const)"E-stop,Emergency stop active."},                         //48
  {(char *const)"", (char *const)"ok",    (char *const)"Error 49"},   //49
  {(char *const)"", (char *const)"ok",    (char *const)"SD Card,SD Card mount failed."},                         //50
  {(char *const)"", (char *const)"ok",    (char *const)"Error 51"},   //51
  {(char *const)"", (char *const)"ok",    (char *const)"Error 52"},   //52
  {(char *const)"", (char *const)"ok",    (char *const)"Error 53"},   //53
  {(char *const)"", (char *const)"ok",    (char *const)"Error 54"},   //54
  {(char *const)"", (char *const)"ok",    (char *const)"Error 55"},   //55
  {(char *const)"", (char *const)"ok",    (char *const)"Error 56"},   //56
  {(char *const)"", (char *const)"ok",    (char *const)"Error 57"},   //57
  {(char *const)"", (char *const)"ok",    (char *const)"Error 58"},   //58
  {(char *const)"", (char *const)"ok",    (char *const)"Error 59"},   //59  
  {(char *const)"", (char *const)"ok",    (char *const)"Error 60"},   //60
  {(char *const)"", (char *const)"ok",    (char *const)"SD Card,\nSD Card file open/read failed."},                //61
  {(char *const)"", (char *const)"ok",    (char *const)"SD Card,\nSD Card directory listing failed."},             //63
  {(char *const)"", (char *const)"ok",    (char *const)"SD Card,\nSD Card directory not found."},                  //63
  {(char *const)"", (char *const)"ok",    (char *const)"SD Card,\nSD Card file empty."},                           //64
  {(char *const)"", (char *const)"ok",    (char *const)"Error 65"},   //65
  {(char *const)"", (char *const)"ok",    (char *const)"Error 66"},   //66
  {(char *const)"", (char *const)"ok",    (char *const)"Error 67"},   //67
  {(char *const)"", (char *const)"ok",    (char *const)"Error 68"},   //68
  {(char *const)"", (char *const)"ok",    (char *const)"Error 69"},   //69
  {(char *const)"", (char *const)"ok",    (char *const)"Bluetooth,\nBluetooth initalisation failed."}             //70
};
