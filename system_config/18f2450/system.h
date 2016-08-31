/* 
 * File:   system.h
 * Author: manuel
 *
 * Created on 13. Juli 2016, 21:41
 */

#ifndef SYSTEM_H
#define	SYSTEM_H

#include "system_config.h"
#include "io_mapping.h"
#include "fixed_address_memory.h"
#include "usb_config.h"

#ifdef	__cplusplus
extern "C" {
#endif


#define OUTPUT_PIN 0
#define INPUT_PIN 1

/*
 * 
When Timer1 is enabled, the RC1/T1OSI/UOE and
RC0/T1OSO/T13CKI pins become inputs. This means
the values of TRISC<1:0> are ignored and the pins are
read as ?0?.
 * 
 */    

//#define IR_RCV_TRIS TRISCbits.TRISC2 see above timer1
#define IR_RCV      PORTCbits.CCP1
        
#define RF_OUT_TRIS TRISAbits.TRISA0
#define RF_OUT LATAbits.LATA0

char dummy;    
#define LED1_TRIS dummy
#define LED1 dummy
    
#define LED2_TRIS TRISAbits.TRISA1
#define LED2 LATAbits.LATA1

void SYSTEM_Initialize(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SYSTEM_H */

