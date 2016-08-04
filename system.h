/* 
 * File:   system.h
 * Author: manuel
 *
 * Created on 13. Juli 2016, 21:41
 */

#ifndef SYSTEM_H
#define	SYSTEM_H

#include "system_config.h"

#ifdef	__cplusplus
extern "C" {
#endif


#define OUTPUT_PIN 0
#define INPUT_PIN 1

//#define IR_RCV_TRIS TRISCbits.TRISC2
#define IR_RCV      LATACbits.LATC2
    
    
#define RF_OUT LATBbits.LATB5
#define RF_OUT_TRIS TRISBbits.TRISB5 


    
#define SW1 PORTBbits.RB0
#define SW1_TRIS TRISBbits.TRISB0

#define SW2 PORTBbits.RB1
#define SW2_TRIS TRISBbits.TRISB1

#define SW3 PORTBbits.RB2
#define SW3_TRIS TRISBbits.TRISB2

    
    void SYSTEM_Initialize(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SYSTEM_H */

