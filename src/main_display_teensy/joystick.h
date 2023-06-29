void processJoystick (int MPGkey) {
  // translate the values from Joystick to drive commands
    int myoffset = 0;
    bool twice = false;
    if ((mystate.grblState == Idle || mystate.grblState == Jog) && (mystate.state == WDREHEN) && (MPGkey!= 0) && ((millis()-mystate.buttontime)>mystate.buttonDtime))  {     // https://github.com/gnea/grbl/wiki/Grbl-v1.1-Jogging
        static char command[50];
        if ((millis()-mystate.buttontime)> 2*mystate.buttonDtime){
            target.fzOld = target.fz;
            Display.fillRect(120, COLUMN1+12+0*COLUM_DISTANCE-32, 50, 30, BackColor);
            Display.setFont(F_A10); 
            Display.setCursor(120,COLUMN1+12+0*COLUM_DISTANCE-32);Display.print("mm/min");
        }
        if (mystate.MPGkey != MPGkey) {
            target.changed = true; 
            mystate.MPGkey = MPGkey;
        }            
        //DEBUG("processMPGpress ok", MPGkey, mystate.buttontime);
        if (abs(MPGkey) == 1) {         //90ms
            mystate.jogS = 0.003;
            mystate.jogF=0.2;
            myoffset = 0;        
        }else if (abs(MPGkey) == 2) {   //120ms
            mystate.jogS = 0.002;
            mystate.jogF=1;
        }else if (abs(MPGkey) == 3) {   //120ms
            mystate.jogS = 0.02;        
            mystate.jogF=10.0;
        }else if (abs(MPGkey) == 4) {   //120ms
            mystate.jogS = 0.2;
            mystate.jogF=100.0;         
            if (target.changed)
                twice = true;                         
        }else if (abs(MPGkey) == 5) {   //120ms
            mystate.jogS = 1.0;
            mystate.jogF=500.0;
            myoffset = -5;            
            if (target.changed)
                twice = true;                     
        }else if (abs(MPGkey) == 6) {   //120ms
            mystate.jogS = 4.0;
            mystate.jogF=2000.0;
            myoffset = -5;
            if (target.changed)
                twice = true; 
        }                                           
        mystate.buttonDtime = (unsigned long)(60.0/mystate.jogF * mystate.jogS *1000.0) + myoffset; 
        target.fz = mystate.jogF;    
        char* vz = (char*)"";
        if (MPGkey < 0)
            vz = (char*)"-";
        sprintf(command, "$J=G91 Z%s%.3f F%.3f", vz, mystate.jogS, mystate.jogF);
        serial_writeLn(command);
        if (twice)
            serial_writeLn(command);
        mystate.buttontime = millis(); 
    }else if ((mystate.grblState == Idle || mystate.grblState == Jog) && (mystate.state == WDREHEN) && (MPGkey== 0)){
        serial_putC(CMD_STOP); 
        serial0_writeLn("processMPGpress Stop");
        Display.fillRect(120, COLUMN1+12+0*COLUM_DISTANCE-32, 50, 30, BackColor);
        Display.setFont(F_A10); 
        Display.setCursor(120,COLUMN1+12+0*COLUM_DISTANCE-32);Display.print("mm/U");
        target.fz = target.fzOld;
        mystate.MPGkey = MPGkey;
        target.changed = true;               
    }else if ((millis()-mystate.buttontime)>mystate.buttonDtime)
        mystate.MPGkey = MPGkey;
}
