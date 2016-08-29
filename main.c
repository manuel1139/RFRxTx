#include <xc.h>
#include <stdbool.h>       /* For true/false definition */
#include <stdint.h>        /* For uint8_t definition */
#include <stdio.h>

#include "system.h"
#include "bitstream_decode.h"

#include "usb.h"
#include "usb_device_hid.h"

#include "app_device_custom_hid.h"

/*
void putch(unsigned char data) {
    while (!PIR1bits.TXIF) // wait until the transmitter is ready
        continue;
    TXREG = data; // send one character
}

void init_uart(void) {
    TXSTAbits.TXEN = 1; // enable transmitter
    RCSTAbits.SPEN = 1; // enable serial port
}
 */

int main(void) {

    //init_uart();
    //rintf("starting\n");
    SYSTEM_Initialize();

    USBDeviceInit();
    USBDeviceAttach();

    ir_rx_start();
    rf_tx_start();

#undef __DEBUG    
#ifdef __DEBUG
    send_code(pollin_rf_rc, S2_ON);
    while (1);
#endif
   

    while (1) {
        //Non USB tasks
        LED1 = IR_RCV;
        LED2 = RF_OUT;
        
        if (get_code().code != 0) {
            if (get_code().rc == &terratec_ir_rc) {
              send_code(&pollin_rf_rc, S2_OFF);
            }
        }
                
                
        //USB Only tasks
        /* If the USB device isn't configured yet, we can't really do anything
         * else since we don't have a host to talk to.  So jump back to the
         * top of the while loop. */
        if (USBGetDeviceState() < CONFIGURED_STATE) {
            /* Jump back to the top of the while loop. */
            continue;
        }

        /* If we are currently suspended, then we need to see if we need to
         * issue a remote wakeup.  In either case, we shouldn't process any
         * keyboard commands since we aren't currently communicating to the host
         * thus just continue back to the start of the while loop. */
        if (USBIsDeviceSuspended() == true) {
            /* Jump back to the top of the while loop. */
            continue;
        }
        LED1 = 1;
        //Application specific tasks
        APP_DeviceCustomHIDTasks();

    }
}

bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
    switch((int)event)
    {
        case EVENT_TRANSFER:
            break;

        case EVENT_SOF:
            /* We are using the SOF as a timer to time the LED indicator.  Call
             * the LED update function here. */
//            APP_LEDUpdateUSBStatus();
            break;

        case EVENT_SUSPEND:
            /* Update the LED status for the suspend event. */
//            APP_LEDUpdateUSBStatus();
            break;

        case EVENT_RESUME:
            /* Update the LED status for the resume event. */
//            APP_LEDUpdateUSBStatus();
            break;

        case EVENT_CONFIGURED:
            /* When the device is configured, we can (re)initialize the demo
             * code. */
            APP_DeviceCustomHIDInitialize();
            break;

        case EVENT_SET_DESCRIPTOR:
            break;

        case EVENT_EP0_REQUEST:
            /* We have received a non-standard USB request.  The HID driver
             * needs to check to see if the request was for it. */
            USBCheckHIDRequest();
            break;

        case EVENT_BUS_ERROR:
            break;

        case EVENT_TRANSFER_TERMINATED:
            break;

        default:
            break;
    }
    return true;
}

