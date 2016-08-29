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
 */    
//#define IR_RCV_TRIS TRISCbits.TRISC2 see above timer1    
#define IR_RCV      PORTCbits.CCP1
        
#define RF_OUT_TRIS TRISAbits.TRISA5
#define RF_OUT LATAbits.LATA5

#define LED1 LATBbits.LB4
#define LED1_TRIS TRISBbits.RB5
    
#define LED2 LATBbits.LB4
#define LED2_TRIS TRISBbits.RB4
    
    
    void SYSTEM_Initialize(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SYSTEM_H */

