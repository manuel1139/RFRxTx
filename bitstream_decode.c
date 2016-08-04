#include <xc.h>
#include <plib.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "bitstream_decode.h"
#include "remote_keys.h"
#include "system.h"

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

void ir_tx();
void rx_raw(uint16_t);
uint16_t swap(uint16_t val);

const struct code2func {
    uint16_t key;
    void (*txfunc)();
};

typedef struct {
    const char* name;
    uint16_t hdr_time_a;
    uint16_t hdr_time_b;
    uint16_t low_1;
    uint16_t high_1;
    uint16_t low_0;
    uint16_t high_0;
    uint16_t pre_code;
    uint8_t bit_cnt;
    const struct code2func *c2f;
} remote;


const struct code2func terratec_ir_rc_codes[] = {
    KEY_1, &ir_tx,
    KEY_2, &ir_tx
};

const remote terratec_ir_rc = {
    "Terratec",
    0x3575, //header_a
    0x199C, //header_b
    0, //low_1
    0, //high_1
    0,
    0,
    0x28D7, //pre_code
    32,
    &terratec_ir_rc_codes
}; //codes

const struct code2func pollin_rf_rc_codes[] = {
    S1_ON, 0,
    S1_OFF, 0,
    S2_ON, 0,
    S2_OFF, 0
};

const remote pollin_rf_rc = {
    "Pollin",
    0x0, //header_a
    0x03F0, //header_b
    0x43F, //low_1
    0x7B2, //high_1
    0x3C8, //low_0
    0x829, //high_0
    0x0B, //pre_code
    20, //bit count
    &pollin_rf_rc_codes //codes
};

typedef struct {
    const remote *rc;
    uint16_t code;
} xcode;

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
void rf_tx(xcode* rf_code);
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
                ir_rx_fsm.state = idle;
            }
            break;
        default:
            break;

    }
}
xcode rf_code;

void ir_tx() {
    rf_code.rc = &pollin_rf_rc;
    rf_code.code = S2_ON;

    rf_tx_fsm.state = header_a;
    WriteTimer3(0xFFFF);
}

void rf_tx(xcode* rf_code) {
    uint8_t byte_nr, bit_idx;
    static int8_t tmp;
    static uint8_t bit_cnt = 19;

    switch (rf_tx_fsm.state) {
        case idle:
            break;
        case header_a:
            if (pollin_rf_rc.hdr_time_a != 0) {
                RF_OUT = ~RF_OUT;
                WriteTimer3(0xFFFF - pollin_rf_rc.hdr_time_a);
                rf_tx_fsm.state = header_b;
                break;
            }
        case header_b:
            RF_OUT = ~RF_OUT;
            WriteTimer3(0xFFFF - pollin_rf_rc.hdr_time_b);
            rf_tx_fsm.state = first_edge;
            break;
        case first_edge:
            RF_OUT = ~RF_OUT;
            if (bit_cnt > 15) {
                tmp = pollin_rf_rc.pre_code & (1 << bit_cnt - 16);
            } else {
                tmp = rf_code->code & (1 << bit_cnt);
            }

            if (tmp) WriteTimer3(0xFFFF - pollin_rf_rc.high_1);
            else {
                WriteTimer3(0xFFFF - pollin_rf_rc.low_0);
            }
            rf_tx_fsm.state = second_edge;
            break;
        case second_edge:
            RF_OUT = ~RF_OUT;
            if (tmp) WriteTimer3(0xFFFF - pollin_rf_rc.low_1);
            else {
                WriteTimer3(0xFFFF - pollin_rf_rc.high_0);
            }
            if (bit_cnt > 0) {
                bit_cnt--;
                rf_tx_fsm.state = first_edge;
            } else {
                rf_tx_fsm.state = done;
            }
            break;
        case done:
            //StopSender();
            RF_OUT = ~RF_OUT;
            bit_cnt = 19;
            //           rf_tx_fsm.state = header;
            //         snd_code.dbyte[2] == 0x82 ? snd_code.dbyte[2] = 0x93 : snd_code.dbyte[2] = 0x82;
            for (uint32_t i = 0; i < 320000; i++) {
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
                NOP();
            }
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
    OpenTimer3(T3_16BIT_RW &
            T3_PS_1_8 &
            T3_SOURCE_INT);
    RF_OUT = 0;
}

void rf_tx_stop() {
    CloseTimer3();
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

/*
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
 */

void high_priority interrupt high_isr(void) {
    if (PIR1bits.CCP1IF) {
        //invert edge detection 
        CCP1CONbits.CCP1M0 = ~CCP1CONbits.CCP1M0;
        ir_rx(ReadCapture1());
        //        rx_raw(ReadCapture1());
        WriteTimer1(0);
        PIR1bits.CCP1IF = 0;
    }
    if (PIR1bits.TMR1IF) {
        reset_fsm(&ir_rx_fsm);
        PIR1bits.TMR1IF = 0;
    }
    if (PIR2bits.TMR3IF) {
        PIR2bits.TMR3IF = 0;
        rf_tx(&rf_code);
    }
}
