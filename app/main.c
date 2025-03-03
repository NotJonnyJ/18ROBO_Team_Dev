#include "intrinsics.h"
#include "msp430fr2355.h"
#include <driverlib.h>


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

void startConversion();

int main(void) {

//test
    volatile uint32_t i;

    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    UCB0CTLW0 = UCSWRST;                      // Enable SW reset
    UCB0CTLW0 |= UCMODE_3 + UCSYNC;                      // I2C Master, synchronous mode
    UCB0I2COA0 = 0x22 | UCOAEN;                   // 0x48
    UCB0CTLW0 &= ~UCSWRST;                     // Clear SW reset, resume operation

    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;
    P1SEL1 &= ~BIT3;                     // P3.1,2 option select
    P1SEL0 |= BIT3;



    // LED Pin Def
    P1DIR |= BIT0;
    P1DIR |= BIT1;


    P6DIR &= ~BIT0;
    
    P6DIR &= ~BIT1;
    P6DIR &= ~BIT2;
    P6DIR &= ~BIT3;
    P6DIR &= ~BIT4;
    P6DIR &= ~BIT5;
    

    // ADC Pin Definitions
    P1SEL1 |= BIT4;
    P1SEL0 |= BIT4;

    P1SEL1 |= BIT5;
    P1SEL0 |= BIT5;

    P1SEL1 |= BIT6;
    P1SEL0 |= BIT6;

    P1SEL1 |= BIT7;
    P1SEL0 |= BIT7;
    

    // Take out of Low Power Mode
    PM5CTL0 &= ~LOCKLPM5;

    UCB0IE |= UCTXIE;
    UCB0IE |= UCRXIE;

    
    configureADC();
    __enable_interrupt();
     _BIS_SR(GIE); //Enable global interrupts.


    while(1)
    {
       startConversion();
       UCB0IE |= UCRXIE0; //Enable receive interrupt

       HS1 = P6IN;
       HS1 &= BIT0;

       HS2 = P6IN;
       HS2 &= BIT1;

       HS3 = P6IN;
       HS3 &= BIT2;

       HS4 = P6IN;
       HS4 &= BIT3;

       HS5 = P6IN;
       HS5 &= BIT4;

       HS6 = P6IN;
       HS6 &= BIT5;


       if((HS1 == 0) | (HS2 == 0) | (HS3 == 0) | (HS4 == 0) | (HS5 == 0) | (HS6 == 0)){
            P1OUT |= BIT1;
            
            __delay_cycles(10000); // Small delay between conversions
       } else if ((HS1 == 1) | (HS2 == 2) | (HS3 == 4) | (HS4 == 8) | (HS5 == 16) | (HS6 == 32)) {
            P1OUT &= ~BIT1;
            
             __delay_cycles(1000);
       } else {
            
            P1OUT &= ~BIT1;
            __delay_cycles(1000);
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
        P1OUT |= BIT0;
    } else {
        P1OUT &= ~BIT0;
    }

    switchChannel();  // Move to the next channel for the next conversion

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

