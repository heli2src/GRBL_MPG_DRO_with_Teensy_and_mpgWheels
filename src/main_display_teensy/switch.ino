#include <Bugtton.h>
#include <ADC.h>
#include <ADC_util.h>

const int adcin2 = A2; // ADC1

ADC *adc = new ADC(); // adc object;

const uint8_t buttonCount = 5;
const uint8_t buttonPins[buttonCount] = {14,15,16,17,18}; // pin with pull down resistor, active-low (positive number), or active-high (negative number).
Bugtton buttons(buttonCount, buttonPins, 25);

uint8_t switchmode =0;
int result;
int oldvalue = - 100;
int minv = 100;
int maxv = 0;

/* ------------------
   Public FUNCTIONS
   ------------------ */
   
void Switch_init(void) {
    pinMode(adcin2, INPUT);
    buttons.setMode(18, INPUT_PULLUP);  
    buttons.setMode(17, INPUT_PULLUP);  
    buttons.setMode(16, INPUT_PULLUP);  
    buttons.setMode(15, INPUT_PULLUP);
    buttons.setMode(14, INPUT_PULLUP);   

    adc->adc0->setAveraging(16); // set number of averages
    adc->adc0->setResolution(8); // set bits of resolution
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::LOW_SPEED); // change the conversion speed
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_LOW_SPEED); // change the sampling speed
}

void Switch_mode(int mode){
    switchmode = mode; 
}

void Switch_loop(void){
    buttons.update();
    if (switchmode ==0){  
        if (buttons.fell(0))
            DROkeyEvent(true, 0);         // see dro.c
        if (buttons.fell(1))
            DROkeyEvent(true, 1);
         if (buttons.fell(2))
            DROkeyEvent(true, 2);
         if (buttons.fell(3))
            DROkeyEvent(true, 3);
         if (buttons.fell(4))
            DROkeyEvent(true, 4);
    }else {
        if (buttons.held(0))
            DROkeyEvent(true, 0);
        else if (buttons.rose(0))
            DROkeyEvent(false, 0);
        if (buttons.held(1))
            DROkeyEvent(true, 1);
        else if (buttons.rose(1))
            DROkeyEvent(false, 1);            
    }
/*    
    value = adc->adc0->analogRead(adcin2);     
    if (value < 45){
        result = -6;
    }else if (value > 45 && value < 52){
        result = -5;
    }else if (value > 55 && value < 62){
        result = -4; 
    }else if (value > 64 && value < 72){
        result = -3; 
    }else if (value > 74 && value < 82){    
        result = -2;
    }else if (value > 81 && value < 94){
        result = -1;
    }else if (value > 94 && value <103){
        result = 0; 
    }else if (value >104 && value <112){
        result = 0;
    }else if (value >115 && value <130){
        result = 0;
    }else if (value >140 && value <155){
        result = 1;
    }else if (value >160 && value <180){
        result = 2;
    }else if (value >190 && value <210){
        result = 3;
    }else if (value >210 && value <230){
        result = 4;
    }else if (value >230 && value <250){
        result = 5;
    }else if (value >250){
        result = 6;
    }                                    
    if (result != 0  || (result==0 && oldvalue !=0)){
        //DEBUG("Switch_loop DROmpgEvent", value, result);
        DROJoystickEvent(true, result);
    }
  */
    oldvalue = result;
}
