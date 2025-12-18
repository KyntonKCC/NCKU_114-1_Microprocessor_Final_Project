#include "servo.h"

// Set Motor 1 Position (CCP1 -> RC2)
void setServo1Position(int dutyCycle){
    CCPR1L = dutyCycle >> 2;                // High 8 bits
    CCP1CONbits.DC1B = dutyCycle & 0x03;    // Low 2 bits
}

// Set Motor 2 Position (CCP2 -> RC1)
void setServo2Position(int dutyCycle){
    CCPR2L = dutyCycle >> 2;                // High 8 bits
    CCP2CONbits.DC2B = dutyCycle & 0x03;    // Low 2 bits
    
    if(dutyCycle < 30){
        LATDbits.LATD7 = 1;
    }else if(dutyCycle < 45){
        LATDbits.LATD6 = 1;
    }else if(dutyCycle < 60){
        LATDbits.LATD5 = 1;
    }else if(dutyCycle <= 75){
        LATDbits.LATD4 = 1;
    }
}