#include "adc.h"

void ADC_Initialize(void){
    // 1. Configure ADCON1
    // PCFG = 1110 (AN0 set as Analog, others as Digital)
    // Vref = VDD/VSS
    ADCON1 = 0x0E; 
    
    // 2. Configure TRISA (Set RA0 as Input)
    TRISAbits.TRISA0 = 1;

    // 3. Configure ADCON2
    // ADFM = 1: Result Right Justified (Range 0-1023)
    // ACQT = 010: Acquisition Time = 4 Tad
    // ADCS = 000: Conversion Clock = Fosc/2 (Suitable for 500kHz, Tad=4us)
    ADCON2bits.ADFM = 1;
    ADCON2bits.ACQT = 0b010; 
    ADCON2bits.ADCS = 0b000; 
    
    // 4. Enable ADC Module
    ADCON0bits.ADON = 1;
}

int ADC_Read(int channel){
    // Select ADC Channel (e.g., 0 for AN0)
    ADCON0bits.CHS = channel;
    
    // Start Conversion
    ADCON0bits.GO = 1;
    
    // Wait for conversion to complete
    while(ADCON0bits.GO);
    
    // Return 10-bit result (Combine High byte and Low byte)
    return ((ADRESH << 8) + ADRESL);
}