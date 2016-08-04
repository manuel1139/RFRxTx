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

//Technotrend     
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

//Pollin RF Remote wall plugs
#define S1_ON  0x0011
#define S1_OFF 0x0000    
#define S2_ON  0x0093
#define S2_OFF 0x0082    
#define S3_ON  0x0050
#define S3_OFF 0x0041    
#define S4_ON  0x00D2
#define S4_OFF 0x00C3    
#define ALL_ON 0x00F0
#define ALL_OFF 0x00E1
#define DARK   0x001B
#define LIGHT  0x000A
    

#ifdef	__cplusplus
}
#endif

#endif	/* REMOTE_KEYS_H */

