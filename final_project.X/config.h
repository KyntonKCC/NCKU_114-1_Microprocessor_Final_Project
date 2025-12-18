#ifndef CONFIG_H
#define	CONFIG_H

#include <xc.h>

// Configuration Bits
#pragma config OSC = INTIO67    // Internal oscillator
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bits (Enabled)
#pragma config WDT = OFF        // Watchdog Timer Enable bit (Disabled)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (Digital I/O on Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Disabled)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Disabled)
#pragma config CCP2MX = PORTC   // CCP2 is multiplexed to RC1

#define _XTAL_FREQ 500000       // System Frequency = 500 kHz

#endif	/* CONFIG_H */