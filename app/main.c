/*
 * RoboSub Leak Detector Firmware
 * 
 * File: main.c
 * Author: 18ROBO LEak Detection
 * Date: 2025-03-06
 * Description: This firmware runs on the MSP430FR2355 microcontroller to detect water leaks in the RoboSub.
 * It reads from multiple water sensors, processes the data, and triggers alerts using LEDs and I2C communication.
 * The system ensures early detection of water intrusion and initiates emergency shutdown procedures when necessary.
 * 
 * Features:
 * - Reads water sensor inputs (continuity and optical prism sensors)
 * - Processes ADC values for Critcial Sensors
 * - Processes GPIO pins for Hazard Sensors
 * - Controls LED indicators to signal hazard and critical states
 * - Communicates detection status to the Jetson AGX Orin over I2C
 * - Implements emergency shutdown when critical water levels are reached
 *
 * Future Improvements:
 * - Optimize power consumption for extended operation
 * - Implement more precise water level differentiation
 * - Improve I2C communication reliability and add error handling
 */
#include "intrinsics.h"
#include "msp430fr2355.h"
#include <driverlib.h>
#include <msp430.h>
#include "status-led.h"


//Pin descriptions for the off board LED

#define DELAY   250   // 250ms delay for 50% duty cycle

// To indicate the state of the system
int DRY_STATE = 1;
int HAZARD_STATE = 0;
int CRITICAL_STATE = 0;

// Used in the verification timer for sensor detection
int CRITICAL_FLAG[8];
int CRITCAL_FLAGS_PREV[8];
int HAZARD_FLAG = 0;

int HAZARD_COUNT = 0;
int CRITICAL_COUNT = 0;
int CRITICAL_TIMER_RUNNING = 0;

unsigned int ADC_Values[4];  // Array to store values for channels A4-A7
unsigned int currentChannel = 4; // Start from channel A4
int try_channel = 0;
unsigned int ADC_Value;

unsigned int i, j;

int timer_count = 0;

char dataIn;

void startConversion();


int main(void) {
//test
    volatile uint32_t i;

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // -- I2C PINS-----------------------------------------------------------------------
    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;
    P1SEL1 &= ~BIT3;                     // P3.1,2 option select
    P1SEL0 |= BIT3;

    // -- I2C Setup 
    UCB0CTLW0 = UCSWRST;                      // Enable SW reset
    UCB0CTLW0 |= UCMODE_3 + UCSYNC;                      // I2C Master, synchronous mode
    UCB0I2COA0 = 0x22 | UCOAEN;                   // 0x48
    UCB0CTLW0 &= ~UCSWRST;                     // Clear SW reset, resume operation
    
    

    // Take out of Low Power Mode ---------------------------------------------
    PM5CTL0 &= ~LOCKLPM5;

    // -- TIMER 2 SETUP (CRITICAL) --------------------------------------------------------
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;
    TB0CTL |= MC__UP;
    TB0CCR0 = 32768;

    TB0CCTL0 &= ~CCIFG;
    TB0CCTL0 |= CCIE;

    // Enable I2C Send and recieve interrupts
    UCB0IE |= UCTXIE;
    UCB0IE |= UCRXIE;
    UCB0IE |= UCRXIE0; //Enable i2C receive interrupt

    // Enable timers, and ADC interrupts
    __enable_interrupt();
     _BIS_SR(GIE); //Enable global interrupts.
     init_status_LED();

    // --MAIN LOOP -------------------------------------------------------------
    while(1){
       //__disable_interrupt();

       update_status_led(CRITICAL_STATE, HAZARD_STATE, DRY_STATE);
       //__enable_interrupt();
    }
}


//--------------------------------------------------------------
// TIMER 1 (LED ALERT) ISR
//--------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void  ISR_TB0_CCR0(void){
    timer_count++;
    if (timer_count <= 300){
        DRY_STATE = 1;
        CRITICAL_STATE = 0;
        HAZARD_STATE = 0;
    } else if (timer_count > 300 && timer_count <=460){
        HAZARD_STATE = 1;
        CRITICAL_STATE = 0;
        DRY_STATE = 0;
    } else if (timer_count > 460){
        CRITICAL_STATE = 1;
        HAZARD_STATE = 0;
        DRY_STATE = 0;
    }

    status_led_CCR0(HAZARD_STATE, DRY_STATE);
}



//--------------------------------------------------------------
// TIMER 2 (LED ALERT) ISR
//--------------------------------------------------------------
#pragma vector = TIMER2_B0_VECTOR
__interrupt void  ISR_TB2_CCR0(void){
    status_led_CCR0(HAZARD_STATE, DRY_STATE);
}

#pragma vector = TIMER2_B1_VECTOR
__interrupt void  ISR_TB2_CCR1(void){
    status_led_CCR1();
}




//--------------------------------------------------------------
// I2C ISR
//--------------------------------------------------------------
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    __disable_interrupt();
    P1OUT ^= 0x01;

    switch(UCB0IV){
    case 0x16:
        dataIn = UCB0RXBUF;
        break;
    case 0x18:
        if(CRITICAL_STATE){
            UCB0TXBUF = 0x10;
        } else if(HAZARD_STATE){
            UCB0TXBUF = 0x01;
        }else if(DRY_STATE){
            UCB0TXBUF = 0x00;
        }
        
        break;
    }
    __enable_interrupt();
}

