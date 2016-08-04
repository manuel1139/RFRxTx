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

#ifdef	__cplusplus
extern "C" {
#endif

void ir_rx_start();
void rf_tx_start();

void test_send();

#ifdef	__cplusplus
}
#endif

#endif	/* BITSTREAM_DECODE_H */

