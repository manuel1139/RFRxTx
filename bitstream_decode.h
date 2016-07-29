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


#define MAX_BYTES 4

    struct ircode {
        uint16_t hdr_time_a;
        uint16_t hdr_time_b;
        uint8_t dbyte[MAX_BYTES];
    };
    
    
bool compare_ircode(struct ircode *irc1, struct ircode *irc2);
void StartReceiver();
       
#ifdef	__cplusplus
}
#endif

#endif	/* BITSTREAM_DECODE_H */

