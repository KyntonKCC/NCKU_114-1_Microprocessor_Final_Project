#include "setting.h"
#include "config.h"

void Timer0_Initialize(void){
    T0CONbits.T08BIT = 0;   // 16-bit mode
    T0CONbits.T0CS = 0;     // Internal instruction cycle clock
    T0CONbits.PSA = 1;      // Bypass prescaler (1:1 ratio)
    
    // Preload values for overflow logic
    TMR0H = 0xEB;           
    TMR0L = 0xB0;           
    
    T0CONbits.TMR0ON = 1;   // Start Timer0
}

void SYSTEM_Initialize(void){
    // Set all pins to Digital I/O
    // ADCON1 = 0x0F;

    // Timer2 Configuration (Shared by CCP1 and CCP2)
    T2CONbits.TMR2ON = 1;       // Turn on Timer2
    T2CONbits.T2CKPS = 0b11;    // Prescaler = 16

    // Oscillator Configuration
    OSCCONbits.IRCF = 0b011;    // Set Internal Oscillator to 500 kHz
    
    // Disable PSP to ensure PORTD is GPIO
    TRISEbits.PSPMODE = 0;
    
    // PWM Mode Configuration
    CCP1CONbits.CCP1M = 0b1100; // Configure CCP1 (Motor 1)
    CCP2CONbits.CCP2M = 0b1100; // Configure CCP2 (Motor 2)
    
    // I/O Pin Direction Configuration (TRIS: 1=Input, 0=Output)
    TRISBbits.TRISB0 = 1;       // RB0 as Input (Button)
    TRISCbits.TRISC2 = 0;       // RC2 as Output (Motor 1)
    TRISCbits.TRISC1 = 0;       // RC1 as Output (Motor 2)
    TRISDbits.TRISD0 = 0;       // RD0 as Output (Buzzer)
    TRISDbits.TRISD1 = 0;       // RD1 as Output (Motor 3)
    
    // LED Outputs
    TRISDbits.TRISD4 = 0;       
    TRISDbits.TRISD5 = 0;       
    TRISDbits.TRISD6 = 0;       
    TRISDbits.TRISD7 = 0;       
    
    // Set PWM Period
    PR2 = 0x9B;                 // Set PR2 for 50Hz frequency
    
    // Initialize Outputs
    LATC = 0;                   
    LATD = 0;                   
    
    // Interrupt Initialization
    Timer0_Initialize();
    INTCONbits.TMR0IE = 1;      // Enable Timer0 Interrupt
    INTCONbits.GIE = 1;         // Enable Global Interrupts
}
