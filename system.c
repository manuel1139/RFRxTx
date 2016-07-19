#include <xc.h>

#include "system.h"

#include "bitstream_decode.h"

void SYSTEM_Initialize(void)
{
    
//    IR_RCV_TRIS = INPUT_PIN;
//    IR_SEND_TRIS = OUTPUT_PIN;
    
    //todo: check if we need 38khz/40khz PWM for IR-OUT 
    
    ADCON1 = 0x0f;
    TRISA = 0x00;
 
    /*******************************************************************/
    // Enable System Interupts
    /*******************************************************************/
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    RF_OUT_TRIS = OUTPUT_PIN; //define as output 

    SW1_TRIS = INPUT_PIN;
    SW2_TRIS = INPUT_PIN;
    SW3_TRIS = INPUT_PIN;
    
  
}