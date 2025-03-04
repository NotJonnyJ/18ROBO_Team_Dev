//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  Texas Instruments, Inc
//  July 2013
//***************************************************************************************

#include <msp430.h>

#define LED1    BIT4  // P2.4
#define LED2    BIT5  // P2.5
#define LED3    BIT7  // P4.7
#define DELAY   250   // 250ms delay for 50% duty cycle

void delay_ms(unsigned int ms) {
    while (ms--) {
        __delay_cycles(10000); // Assuming 1MHz clock, 1ms delay
    }
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    // Configure P2.4 and P2.5 as output
    P2DIR |= LED1 | LED2;
    P2OUT &= ~(LED1 | LED2); // Ensure LEDs are OFF initially

    // Configure P4.7 as output
    P4DIR |= LED3;
    P4OUT &= ~LED3; // Ensure LED3 is OFF initially

    PM5CTL0 &= ~LOCKLPM5;

    while (1) {
        P2OUT |= LED1;  // Turn ON LED1
        delay_ms(DELAY);
        P2OUT &= ~LED1;  // Turn OFF LED1
        delay_ms(DELAY); // Keep it OFF for the same duration

        P2OUT |= LED2;   // Turn ON LED2
        delay_ms(DELAY);
        P2OUT &= ~LED2;  // Turn OFF LED2
        delay_ms(DELAY);

        P4OUT |= LED3;   // Turn ON LED3
        delay_ms(DELAY);
        P4OUT &= ~LED3;  // Turn OFF LED3
        delay_ms(DELAY);
    }
}
