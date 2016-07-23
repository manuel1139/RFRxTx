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

#define INIT_EDGE 0
#define MAX_BYTES 4

enum fsm_state {
    idle,
    header,
    first_edge,
    second_edge,
    done
};

struct {
    enum fsm_state state;
    int bit_cnt;
} sender_fsm, receiver_fsm;

struct ircode_t {
    uint16_t hdr_time;
    uint8_t dbyte[MAX_BYTES];
} rcv_code, snd_code, last_code;


void SndData();
void XferData();
void TestXX();
void StopSender();
void ResetReceiverFsm();

void ResetReceiverFsm() {
    receiver_fsm.state = idle;
    receiver_fsm.bit_cnt = 0;

    rcv_code.hdr_time = 0;
    rcv_code.dbyte[0] = 0;
    rcv_code.dbyte[1] = 0;
    rcv_code.dbyte[2] = 0;
    rcv_code.dbyte[3] = 0;
}

void StartReceiver() {

    CCPR1 = 0; //Timer data register zero (word)
    IPR1bits.CCP1IP = 1; //Timer 1 as source
    RCONbits.IPEN = 1; //Interrupt priority

    if (INIT_EDGE == 1) {
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

void Receive(unsigned int bit_time) {
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

void StartSender() {
    OpenTimer3(T3_16BIT_RW &
            T3_PS_1_8 &
            T3_SOURCE_INT);

    RF_OUT = INIT_EDGE;
}

void StopSender() {
    CloseTimer3();
}
struct ircode_t myPrtCode;

void SendSW1() {
    StartSender();

    snd_code.hdr_time = 0x03F0;
    snd_code.dbyte[0] = 0x0B;
    snd_code.dbyte[1] = 0x00;
    snd_code.dbyte[2] = 0x82;

    sender_fsm.state = header;
    //SndData();   
    //force start
    WriteTimer3(0xFFFF);
}

void SndData() {
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
            RF_OUT = 0;
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

void high_priority interrupt high_isr(void) {
    if (PIR1bits.CCP1IF) {
        PIR1bits.CCP1IF = 0;
        WriteTimer1(0);
        // get the hi->lo lo->hi transition
        CCP1CONbits.CCP1M0 = ~CCP1CONbits.CCP1M0; //invert  edge detection
        Receive(ReadCapture1());
    }

    if (PIR1bits.TMR1IF) {
        PIR1bits.TMR1IF = 0;
        ResetReceiverFsm();

    }

    if (PIR2bits.TMR3IF) {
        PIR2bits.TMR3IF = 0;
        SndData();
    }
}



/*
    void RFSend() {
        RF_OUT = ~RF_OUT;

        if (cntr < 40) {
            WriteTimer3(0xFFFF - code_stream[cntr]);
            cntr++;
        } else {
            RF_OUT = INIT_EDGE;
            cntr = 0;

            for (int i = 0; i < 1000; i++) {
                cntr = 0;
            }
        }
    }

 */



/*
void DecodeSpace(unsigned int bit_time) {
    if (fsm.have_header == 0 && fsm.initiating_edge == 0) { //check for the first edge
        fsm.initiating_edge = 1;
    } else if (fsm.have_header == 0 && fsm.initiating_edge == 1) {
        //next value; time between init_edge and now
        //ist the header time
        if (bit_time < (MIN_HEADER_TIME)) {
            //too short - not a header
            ResetDsm();
        } else {
            fsm.have_header = 1;
        }
    } else {
        //header receiver start decding bits
        if (bit_time > BIT_AVG_TIME) { //greater than avg bit time assue positive bit
            //            ir_code.code <<= 1;
            //            ir_code.code |= 1;
        } else {
            //            ir_code.code <<= 1;
        }
        cnt[fsm.bit_cnt]=bit_time;
        fsm.bit_cnt++;
        //is this the last bit?
        if (fsm.bit_cnt == 8) {
             fsm.is_code_valid = 1;
            //IRComplete(ir_code.code);
            ResetDsm();
        }
    }
}
 
void DecodeSpace(unsigned int bit_time) {
    if (fsm.initiating_edge == 0) { //check for the first edge
        fsm.initiating_edge = 1;
    } else {
        //only with simulator 
        //printf("bit: %d  val:  %x\n", fsm.bit_cnt, bit_time);
        cnt[fsm.bit_cnt] = bit_time;
        fsm.bit_cnt++;
        //is this the last bit?
        if (fsm.bit_cnt == 41) {
            fsm.is_code_valid = 1;
            ResetDsm();
        }
    }
}
 */
