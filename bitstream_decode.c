#include <xc.h>
#include <plib.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bitstream_decode.h"
#include "remote_keys.h"
#include "system.h"
#include "usb.h"

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

void rx_raw(uint16_t);
uint16_t swap(uint16_t val);


//file globals
static xcode current_code = 0;
static xcode last_code = 0;

enum fsm_state {
    idle,
    header_a,
    header_b,
    first_edge,
    second_edge,
    done
};

typedef struct {
    enum fsm_state state;
    //    ir_code code;
    //    uint8_t bit_cnt;
} fsm;



fsm ir_rx_fsm;
fsm rf_tx_fsm;


void reset_fsm(fsm* fsm);
void rf_tx(xcode*);
void ir_tx();

void reset_fsm(fsm* fsm) {
    fsm->state = idle;

}

void ir_rx_start() {

    CCPR1 = 0; //Timer data register zero (word)
    IPR1bits.CCP1IP = 1; //Timer 1 as source
    RCONbits.IPEN = 1; //Interrupt priority


    if (PORTCbits.RC2 == 1) {
        OpenCapture1(C1_EVERY_FALL_EDGE &
                CAPTURE_INT_ON);
    } else {
        OpenCapture1(C1_EVERY_RISE_EDGE &
                CAPTURE_INT_ON);
    }

    OpenTimer1(T1_SOURCE_INT &
            TIMER_INT_ON &
            T1_PS_1_8);

}

void ir_rx_stop() {
    CloseCapture1();
}

uint16_t swap(uint16_t val) {
    return (val >> 8) | (val << 8);
}

void ir_rx(uint16_t bit_time) {
    static uint16_t l_first_edge;
    static remote ir_rc;
    uint16_t *dp;
    xcode current_code;
    static uint8_t bit_cnt = 0;

    switch (ir_rx_fsm.state) {
        case idle: //first edgetime not needed
            ir_rx_fsm.state = header_a;
            break;
        case header_a:
            ir_rc.hdr_time_a = bit_time;
            ir_rx_fsm.state = header_b;
            break;
        case header_b:
            ir_rc.hdr_time_b = bit_time;
            ir_rx_fsm.state = first_edge;
            break;
        case first_edge:
            l_first_edge = bit_time;
            ir_rx_fsm.state = second_edge;
            break;
        case second_edge:
            if (bit_cnt < 15) {
                dp = &(ir_rc.pre_code);
            } else if (bit_cnt > 15 &&
                    bit_cnt < 31) {
                dp = &(current_code.code);
            }

            *dp <<= 1;
            if (l_first_edge > bit_time) {
                *dp |= 1;
            }

            if (bit_cnt < 31) {
                bit_cnt++;
                ir_rx_fsm.state = first_edge;
            } else {
                //try to find code in rc
                ir_rc.pre_code = swap(ir_rc.pre_code);
                current_code.code = swap(current_code.code);

                if (ir_rc.pre_code == terratec_ir_rc.pre_code) {
                    for (uint8_t i = 0; i < NELEMS(terratec_ir_rc_codes); i++) {
                        //current_code.code 
                        const struct code2func *c2f = terratec_ir_rc.c2f;
                        uint16_t code = (c2f + i)->key;
                        if (current_code.code == code) {
                            //send a command
                            c2f->txfunc();
                        }
                    }
                }
                bit_cnt = 0;
                reset_fsm(&ir_rx_fsm);
            }
            break;
        default:
            break;

    }
}
xcode code_to_send;

//todo: shoudl be fro hid .. send_code(remote, code)

void send_code(const remote r, uint16_t c) {

    code_to_send.code = c;
    code_to_send.rc = &pollin_rf_rc;

    rf_tx_fsm.state = header_a;
    rf_tx(&code_to_send);
}

void rf_tx(xcode* rf_code) {
    static int8_t tmp;
    static uint8_t bit_cnt = 19;

    switch (rf_tx_fsm.state) {
        case idle:
            break;
        case header_a:
            if (pollin_rf_rc.hdr_time_a != 0) {
                RF_OUT = ~RF_OUT;
                rf_tx_fsm.state = header_b;
                WriteTimer0(0xFFFF - pollin_rf_rc.hdr_time_a);
                break;
            }
        case header_b:
            RF_OUT = ~RF_OUT;
            rf_tx_fsm.state = first_edge;
            WriteTimer0(0xFFFF - pollin_rf_rc.hdr_time_b);
            break;
        case first_edge:
            RF_OUT = ~RF_OUT;
            if (bit_cnt > 15) {
                tmp = pollin_rf_rc.pre_code & (1 << bit_cnt - 16);
            } else {
                tmp = rf_code->code & (1 << bit_cnt);
            }
            rf_tx_fsm.state = second_edge;
            if (tmp) WriteTimer0(0xFFFF - pollin_rf_rc.high_1);
            else {
                WriteTimer0(0xFFFF - pollin_rf_rc.low_0);
            }
            break;
        case second_edge:
            RF_OUT = ~RF_OUT;
            if (tmp) WriteTimer0(0xFFFF - pollin_rf_rc.low_1);
            else {
                WriteTimer0(0xFFFF - pollin_rf_rc.high_0);
            }
            if (bit_cnt > 0) {
                bit_cnt--;
                rf_tx_fsm.state = first_edge;
            } else {
                rf_tx_fsm.state = done;
            }
            break;
        case done:
            RF_OUT = 0; //last edge must be  zero
            bit_cnt = 19;
            rf_tx_fsm.state = idle;
            code_to_send.code = 0;
            code_to_send.rc = NULL;
            break;
        default:
            break;
    }
}

/*
void ReceiveRF(uint16_t bit_time) {
    char byte_nr;
    static uint16_t l_first_edge;
    static uint8_t bit_cnt = 0;

    switch (receiver_fsm.state) {
        case idle:
            receiver_fsm.state = header;
            break;
        case header:
            rcv_code.hdr_time = bit_time;
            receiver_fsm.state = first_edge;
            break;
        case first_edge:
            l_first_edge = bit_time;
            receiver_fsm.state = second_edge;
            break;
        case second_edge:

            if (bit_cnt < 4) byte_nr = 0;
            if (bit_cnt >= 4 && bit_cnt <= 11) byte_nr = 1;
            if (bit_cnt > 11) byte_nr = 2;

            rcv_code.dbyte[byte_nr] <<= 1;
            if (l_first_edge > bit_time) {
                rcv_code.dbyte[byte_nr] |= 1;
            }

            if (bit_cnt > 18) {
                last_code = rcv_code; //structure copy
                bit_cnt = 0;
                receiver_fsm.state = done;
            } else {
                bit_cnt++;
                receiver_fsm.state = first_edge;
            }
            break;
        case done:
            ResetReceiverFsm();

            break;
        default:
            break;

    }
}
 */
void rf_tx_start() {
    OpenTimer0(T0_16BIT &
            T0_PS_1_8 &
            T0_SOURCE_INT);
    RF_OUT = 0;
}

void rf_tx_stop() {
    CloseTimer0();
}

/*
void SendSW1() {
    struct ircode snd_code;

    StartSender();

    snd_code.hdr_time_a = 0x03F0;
    snd_code.dbyte[0] = 0x0B;
    snd_code.dbyte[1] = 0x00;
    snd_code.dbyte[2] = 0x82;

    //    sender_fsm.state = header;
    //SndData();   
    //force start
    WriteTimer3(0xFFFF);
}
 */


void rx_raw(uint16_t bit_time) {
#define MAX_EDGE 35
    static uint16_t edge_times[MAX_EDGE];
    static uint8_t edge_cnt = 0;

    if (edge_cnt < MAX_EDGE) {
        edge_times[edge_cnt] = bit_time;
        edge_cnt++;
    } else {
        //büffer füll
        edge_cnt = 0;
    }
}

void high_priority interrupt high_isr(void) {

    USBDeviceTasks();

    if (PIR1bits.CCP1IF) {
        //invert edge detection 
        CCP1CONbits.CCP1M0 = ~CCP1CONbits.CCP1M0;
        //ir_rx(ReadCapture1());
        rx_raw(ReadCapture1());
        WriteTimer1(0);
        PIR1bits.CCP1IF = 0;
    }
    
    if (PIR1bits.TMR1IF) {
        reset_fsm(&ir_rx_fsm);
        PIR1bits.TMR1IF = 0;
    }

    if (INTCONbits.TMR0IF) {
        rf_tx(&code_to_send);
        INTCONbits.TMR0IF = 0;
    }
}

xcode get_code() {
    return last_code;
}

xcode get_last_code() {
    last_code = current_code;
    current_code.code = 0;
    current_code.rc = 0;
    return last_code;

}
