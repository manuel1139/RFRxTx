/* 
 * File:   bitstream_decode.h
 * Author: manuel
 *
 * Created on 13. Juli 2016, 16:01
 */

#ifndef BITSTREAM_DECODE_H
#define	BITSTREAM_DECODE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
void StartReceiver();
void IRRcvISR(void);
void SendSW1();

#ifdef	__cplusplus
}
#endif

#endif	/* BITSTREAM_DECODE_H */

