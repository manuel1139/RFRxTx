/* 
 * File:   remote_keys.h
 * Author: manuel
 *
 * Created on 26. Juli 2016, 12:42
 */

#ifndef REMOTE_KEYS_H
#define	REMOTE_KEYS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define KEY_1 0x40BF
#define KEY_2 0xC03F
#define KEY_3 0x20DF
#define KEY_4 0xA05F
#define KEY_5 0x609F
#define KEY_6 0xE01F
#define KEY_7 0x10EF
#define KEY_8 0x906F
#define KEY_9 0x50AF
#define KEY_0 0x30CF

/* Pollin RF Remote wall plugs*/
/*
S1 ON 0x11  off 0x00
S2 ON 0x93  off 0x82
S3 ON 0x50  0ff 0x41
S4 ON 0xD2  OFF 0xC3
ALLON 0xF0  OFF 0xE1
DRK   0x1B  BRI 0x0A
*/

#ifdef	__cplusplus
}
#endif

#endif	/* REMOTE_KEYS_H */

