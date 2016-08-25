/* 
 * File:   bitstream_decode.h
 * Author: manuel
 *
 * Created on 13. Juli 2016, 16:01
 */

#ifndef BITSTREAM_DECODE_H
#define	BITSTREAM_DECODE_H

#include <stdint.h>
#include <stdbool.h>

#include "remote_keys.h"

#ifdef	__cplusplus
extern "C" {
#endif
    
    
const struct code2func {
    uint16_t key;
    void (*txfunc)();
};

typedef struct {
    //const unsigned char* name;
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
    KEY_1, NULL,
    KEY_2, NULL
};

const remote terratec_ir_rc = {
   //"Terratec",
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
    //"Pollin",
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


void ir_rx_start();
void rf_tx_start();

void send_code(const remote, uint16_t);
xcode get_code();
xcode get_last_code();

#ifdef	__cplusplus
}
#endif

#endif	/* BITSTREAM_DECODE_H */

