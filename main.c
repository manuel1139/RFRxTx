#include <xc.h>
#include <stdbool.h>       /* For true/false definition */
#include <stdint.h>        /* For uint8_t definition */
#include <stdio.h>

#include "system.h"
#include "bitstream_decode.h"

/*
void putch(unsigned char data) {
    while (!PIR1bits.TXIF) // wait until the transmitter is ready
        continue;
    TXREG = data; // send one character
}

void init_uart(void) {
    TXSTAbits.TXEN = 1; // enable transmitter
    RCSTAbits.SPEN = 1; // enable serial port
}
 */

int main(void) {

    //init_uart();
    //rintf("starting\n");
    SYSTEM_Initialize();
    ir_rx_start();
    rf_tx_start();
    while (1) {
        
    }
}



