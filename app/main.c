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

//Pin descriptions for the off board LED
#define LED1    BIT4  // P2.4
#define LED2    BIT5  // P2.5
#define LED3    BIT7  // P4.7
#define DELAY   250   // 250ms delay for 50% duty cycle

// To indicate the state of the system
int DRY_STATE = 1;
int HAZARD_STATE = 0;
int CRITICAL_STATE = 0;

// Used in the verification timer for sensor detection
int CRITICAL_FLAG = 0;
int HAZARD_FLAG = 0;

unsigned int ADC_Values[4];  // Array to store values for channels A4-A7
unsigned int currentChannel = 4; // Start from channel A4

unsigned int ADC_Value;

int HS1 = 0;
int HS2 = 0;
int HS3 = 0;
int HS4 = 0;
int HS5 = 0;
int HS6 = 0;

unsigned int i;

char dataIn;

void startConversion();


void delay_ms(unsigned int ms) {
    while (ms--) {
        __delay_cycles(10000); // Assuming 1MHz clock, 1ms delay
    }
}

int main(void) {
//test
    volatile uint32_t i;

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // -- ON BOARD LED Pins ------------------------------------------------
    P1DIR |= BIT0;
    P1DIR |= BIT1;
    // -- OFF BOARD LED PINS  -------------------------------------------------------------------
    P2DIR |= LED1 | LED2;   // Configure P2.4 and P2.5 as output
    P2OUT &= ~(LED1 | LED2); // Ensure LEDs are OFF initially
    P4DIR |= LED3;  // Configure P4.7 as output
    P4OUT &= ~LED3; // Ensure LED3 is OFF initially

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
    
    // HAZARD SENSOR PINS --------------------------------------------------
    P6DIR &= ~BIT0;
    P6DIR &= ~BIT1;
    P6DIR &= ~BIT2;
    P6DIR &= ~BIT3;
    P6DIR &= ~BIT4;
    P6DIR &= ~BIT5;
    
    // --ADC PINS ------------------------------------------------------------
    P1SEL1 |= BIT4;
    P1SEL0 |= BIT4;
    P1SEL1 |= BIT5;
    P1SEL0 |= BIT5;
    P1SEL1 |= BIT6;
    P1SEL0 |= BIT6;
    P1SEL1 |= BIT7;
    P1SEL0 |= BIT7;

    // Take out of Low Power Mode ---------------------------------------------
    PM5CTL0 &= ~LOCKLPM5;

    // -- TIMER 1 SETUP (CRITICAL) --------------------------------------------------------
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;
    TB0CTL |= MC__UP;
    TB0CCR0 = 32768;

    TB0CCTL0 &= ~CCIFG;
    TB0CCTL0 |= CCIE;

    // Enable I2C Send and recieve interrupts
    UCB0IE |= UCTXIE;
    UCB0IE |= UCRXIE;

    // ADC Setup Initialization
    configureADC();

    // Enable timers, and ADC interrupts
    __enable_interrupt();
     _BIS_SR(GIE); //Enable global interrupts.

    // --MAIN LOOP -------------------------------------------------------------
    while(1){
       

       UCB0IE |= UCRXIE0; //Enable i2C receive interrupt
       
       // Grab Bit 0 of port 6 to check sensor 1
       HS1 = P6IN;
       HS1 &= BIT0;
       // Grab Bit 1 of port 6 to check sensor 2
       HS2 = P6IN;
       HS2 &= BIT1;
       // Grab Bit 2 of port 6 to check sensor 3
       HS3 = P6IN;
       HS3 &= BIT2;
       // Grab Bit 3 of port 6 to check sensor 4
       HS4 = P6IN;
       HS4 &= BIT3;
       // Grab Bit 4 of port 6 to check sensor 5
       HS5 = P6IN;
       HS5 &= BIT4;
       // Grab Bit 5 of port 6 to check sensor 6
       HS6 = P6IN;
       HS6 &= BIT5;

    // Compare sensors with expected input values from comparator circuit
       if((HS1 == 0) | (HS2 == 0) | (HS3 == 0) | (HS4 == 0) | (HS5 == 0) | (HS6 == 0)){
            //P1OUT |= BIT1;
            P2OUT |= LED2;   // Turn ON LED2
            delay_ms(DELAY);
       } else if ((HS1 == 1) | (HS2 == 2) | (HS3 == 4) | (HS4 == 8) | (HS5 == 16) | (HS6 == 32)) {
            //P1OUT &= ~BIT1;
            P2OUT &= ~LED2;  // Turn OFF LED2
            delay_ms(DELAY);
       } else {
            P2OUT &= ~LED2;  // Turn OFF LED2
            delay_ms(DELAY);
       }        
    }
}


//--------------------------------------------------------------
// Starts the MSP430 ADC Conversion and triggers the ADC ISR
//--------------------------------------------------------------
void startConversion() {
    ADCCTL0 &= ~ADCENC;        // Clear ADCENC before changing channel
    ADCMCTL0 &= ~ADCINCH;      // Clear previous channel selection
    ADCMCTL0 |= currentChannel; // Set current channel (A4 to A7)
    
    ADCCTL0 |= ADCENC | ADCSC; // Re-enable and start conversion
}

//--------------------------------------------------------------
// Configures the MSP430 on chip ADC
//--------------------------------------------------------------
void configureADC() {
    ADCCTL0 &= ~ADCSHT;  
    ADCCTL0 |= ADCSHT_2;    // Sample-and-hold time
    ADCCTL0 |= ADCON;       // Turn ADC ON

    ADCCTL1 |= ADCSSEL_2;   // Set ADC clock source to SMCLK
    ADCCTL1 |= ADCSHP;      // Sampling signal from sampling timer

    ADCCTL2 &= ~ADCRES;
    ADCCTL2 |= ADCRES_2;    // Set 12-bit resolution

    ADCIE |= ADCIE0;        // Enable ADC interrupt
}

//--------------------------------------------------------------
// Changes the ADC channel (aka the sensors signal channel)
//--------------------------------------------------------------
void switchChannel() {
    currentChannel++;   // Move to the next channel
    if (currentChannel > 7) {
        currentChannel = 4;  // Wrap around back to A4
    }
}

//--------------------------------------------------------------
// ADC ISR
//--------------------------------------------------------------
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void){
    ADC_Values[currentChannel - 4] = ADCMEM0; // Store result in array

    // Example: Turn LED on if any channel reads between 11 and 3000
    if (ADC_Values[currentChannel - 4] < 3000 && ADC_Values[currentChannel - 4] > 11) {
        //P1OUT |= BIT0;
        P2OUT |= LED1;  // Turn ON LED1
        delay_ms(DELAY);
    } else {
        //P1OUT &= ~BIT0;
        P2OUT &= ~LED1;  // Turn OFF LED1
        delay_ms(DELAY); // Keep it OFF for the same duration
    }

    switchChannel();  // Move to the next channel for the next conversion

}

//--------------------------------------------------------------
// TIMER 1 ISR
//--------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void  ISR_TB0_CCR0(void){
    CRITICAL_FLAG++;
    // Start ADC converstion
    startConversion();
    if (CRITICAL_FLAG == 2) {
        CRITICAL_STATE = 1;
        DRY_STATE = 0;
    }
    TB0CCTL0 &= ~CCIFG;
}
//--------------------------------------------------------------
// I2C ISR
//--------------------------------------------------------------
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {
    P1OUT ^= 0x01;

    switch(UCB0IV){
    case 0x16:
        dataIn = UCB0RXBUF;
        break;
    case 0x18:
        UCB0TXBUF = 0x11;
        break;
    }
}

