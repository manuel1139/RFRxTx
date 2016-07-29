#include <xc.h>
#include <plib.h>
#include <stdio.h>
#include <stdint.h>

#include "bitstream_decode.h"
#include "rf_bit_stream.h"
#include "system_config.h"

#define LOW_1 0x43F
#define HIGH_1 0x7B2
#define LOW_0 0x3C8
#define HIGH_0 0x829

#define ACTIVE_LOW 0
#define MAX_BYTES 4

void ResetFsm(struct fsm* fsm);

enum fsm_state {
    idle,
    header,
    first_edge,
    second_edge,
    done
};

struct ircode {
    uint16_t hdr_time;
    uint8_t dbyte[MAX_BYTES];
};

struct fsm {
    enum fsm_state state;
    struct ircode code;
    uint8_t byte_cnt;
    uint8_t bit_cnt;
};

void ResetFsm(struct fsm* fsm) {
    fsm->state = idle;
    fsm->bit_cnt = 0;
    fsm->byte_cnt = 0;

    fsm->code.hdr_time = 0;
    for (int i = 0; i < MAX_BYTES; i++) {
        fsm->code.dbyte[i] = 0;
    }
}

void StartReceiver() {

    CCPR1 = 0; //Timer data register zero (word)
    IPR1bits.CCP1IP = 1; //Timer 1 as source
    RCONbits.IPEN = 1; //Interrupt priority

    if (ACTIVE_LOW) {
        OpenCapture1(C1_EVERY_FALL_EDGE &
                CAPTURE_INT_ON);
    } else {
        OpenCapture1(C1_EVERY_RISE_EDGE &
                CAPTURE_INT_ON);
    }

    OpenTimer1(T1_SOURCE_INT &
            TIMER_INT_ON &
            T1_PS_1_8);

    WriteTimer1(0xFFFF); //to reset FSM in oder to not create duplicate versions
    // of StartReceiver
}

void StopReceiver() {
    CloseCapture1();
}

static struct fsm receiver_fsm;

void Receive(unsigned int bit_time) {
    static uint16_t l_first_edge;

    switch (receiver_fsm.state) {
        case idle: //first edge time not needed
            receiver_fsm.state = first_edge;
            break;
        case first_edge:
            l_first_edge = bit_time;
            receiver_fsm.state = second_edge;
            break;
        case second_edge:
            if (receiver_fsm.byte_cnt < MAX_BYTES) {
                if (receiver_fsm.bit_cnt > 7) {
                    receiver_fsm.byte_cnt++;
                    receiver_fsm.bit_cnt = 0;
                }
                if (l_first_edge > bit_time) {
                    receiver_fsm.code.dbyte[receiver_fsm.byte_cnt] <<= 1;
                    receiver_fsm.code.dbyte[receiver_fsm.byte_cnt] |= 1;
                } else {
                    receiver_fsm.code.dbyte[receiver_fsm.byte_cnt] <<= 1;
                }
                receiver_fsm.bit_cnt++;
                receiver_fsm.state = first_edge;
            } else {
                receiver_fsm.state = done;
            }
            break;
        case done:
            ResetFsm(&receiver_fsm);

            break;
        default:
            break;

    }
}

/*
void ReceiveRF(unsigned int bit_time) {
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
void StartSender() {
    OpenTimer3(T3_16BIT_RW &
            T3_PS_1_8 &
            T3_SOURCE_INT);

    RF_OUT = ACTIVE_LOW;
}

void StopSender() {
    CloseTimer3();
}

void SendSW1() {
    struct ircode snd_code;

    StartSender();

    snd_code.hdr_time = 0x03F0;
    snd_code.dbyte[0] = 0x0B;
    snd_code.dbyte[1] = 0x00;
    snd_code.dbyte[2] = 0x82;

    //    sender_fsm.state = header;
    //SndData();   
    //force start
    WriteTimer3(0xFFFF);
}

/*
 * void SndData() {
    uint8_t byte_nr, bit_idx;
    static int8_t tmp;
    static uint8_t bit_cnt = 19;

    switch (sender_fsm.state) {
        case idle:
            break;
        case header:
            RF_OUT = ~RF_OUT;
            WriteTimer3(0xFFFF - snd_code.hdr_time);
            sender_fsm.state = first_edge;
            break;
        case first_edge:
            RF_OUT = ~RF_OUT;

            if (bit_cnt > 15) byte_nr = 0;
            if (bit_cnt <= 15 && bit_cnt >= 8) byte_nr = 1;
            if (bit_cnt <= 7) byte_nr = 2;

            bit_idx = bit_cnt >> 3;
            tmp = snd_code.dbyte[byte_nr] & (1 << (bit_cnt - bit_idx * 8));

            if (tmp) WriteTimer3(0xFFFF - HIGH_1);
            else {
                WriteTimer3(0xFFFF - LOW_0);
            }


            sender_fsm.state = second_edge;
            break;
        case second_edge:
            RF_OUT = ~RF_OUT;
            if (tmp) WriteTimer3(0xFFFF - LOW_1);
            else {
                WriteTimer3(0xFFFF - HIGH_0);
            }
            //printf("bit_cnt %d\n", bit_cnt);
            if (bit_cnt > 0) {
                bit_cnt--;
                sender_fsm.state = first_edge;
            } else {
                sender_fsm.state = done;
            }
            break;
        case done:
            //StopSender();
            RF_OUT = ~RF_OUT;
            bit_cnt = 19;
            sender_fsm.state = header;
           snd_code.dbyte[2] == 0x82 ? snd_code.dbyte[2] = 0x93 : snd_code.dbyte[2] = 0x82;
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

 * */
void high_priority interrupt high_isr(void) {
    if (PIR1bits.CCP1IF) {
        PIR1bits.CCP1IF = 0;
        WriteTimer1(0);
        // get the hi->lo lo->hi transition
        CCP1CONbits.CCP1M0 = ~CCP1CONbits.CCP1M0; //invert  edge detection
        Receive(ReadCapture1());
        //RecvRaw(ReadCapture1());
    }

    if (PIR1bits.TMR1IF) {
        PIR1bits.TMR1IF = 0;
        ResetFsm(&receiver_fsm);

    }

    if (PIR2bits.TMR3IF) {
        PIR2bits.TMR3IF = 0;
        //        SndData();
    }
}


uint16_t edge_time[150];

void RecvRaw(unsigned int bit_time) {

    static uint8_t edge_cnt = 0;

    if (edge_cnt < 150) {
        edge_time[edge_cnt] = bit_time;
        edge_cnt++;
    } else {
        //büffer füll
        edge_cnt = 0;
    }
}
