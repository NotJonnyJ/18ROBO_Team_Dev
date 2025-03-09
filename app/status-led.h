/*

18 ROBO, Logan Graham

This header file contains all the needed functionality for the offboard status LED.

*/

#include <msp430.h>

#ifndef STATUS_LED_H
#define STATUS_LED_H


// Constants
#define GREEN_LED    BIT4  // P2.4 green
#define WHITE_LED    BIT5  // P2.5 white
#define RED_LED      BIT7  // P4.7 red
#define DRY_CCR0     32768 // dry state ccr0 point
#define DRY_CCR1     16384 // dry state ccr1 point
#define HAZARD_CCR0  32768 // hazard state ccr0 point
#define HAZARD_CCR1  16384  // hazard state ccr1 point

// variables
int count = 0;


inline void init_status_LED(void)
{
    // pin configurations
    P2DIR |= GREEN_LED | WHITE_LED;     // Configure P2.4 and P2.5 as output
    P2OUT &= ~(GREEN_LED | WHITE_LED);  // Ensure LEDs are OFF initially
    P4DIR |= RED_LED;                   // Configure P4.7 as output
    P4OUT &= ~RED_LED;                  // Ensure LED3 is OFF initially
    
    // timer config
    TB2CTL |= TBCLR;
    TB2CTL |= TBSSEL__ACLK;
    TB2CTL |= MC__UP;
    
    TB2CCR0 = DRY_CCR0;
    TB2CCTL0 &= ~CCIFG;
    TB2CCTL0 |= CCIE;

    TB2CCR1 = DRY_CCR1;
    TB2CCTL0 &= ~CCIFG;
    TB2CCTL0 |= CCIE;

    return;
}

inline void update_status_led(int critical_state, int hazard_state, int dry_state)
{
    if (critical_state == 1)
    {
        P4OUT |= RED_LED;
        P2OUT &= ~(GREEN_LED | WHITE_LED);
    }
    else if (hazard_state == 1)
    {
        TB2CCR0 = HAZARD_CCR0;
        TB2CCR1 = HAZARD_CCR1;
    }
    else if (dry_state == 1)
    {
        TB2CCR0 = DRY_CCR0;
        TB2CCR1 = DRY_CCR1;
    }

    return;
}

inline void status_led_CCR0(int hazard_state, int dry_state)
{
    if (count > 9)
    {
    if (hazard_state == 1)
    {
        P2OUT |= WHITE_LED;
        P2OUT &= ~GREEN_LED;
    }
    else if (dry_state == 1)
    {
        P2OUT |= GREEN_LED;
        P2OUT &= ~WHITE_LED;
    }
    count = 0;
    }
    TB2CCTL0 &= ~CCIFG;
    return;
}

inline void status_led_CCR1(void)
{

    P2OUT &= ~GREEN_LED;
    P2OUT &= ~WHITE_LED;
    count++;

    TB2CCTL1 &= ~CCIFG;
    return;
}


#endif