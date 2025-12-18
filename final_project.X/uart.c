#include "uart.h"

char *tx_ptr = 0; // Pointer to the text being transmitted

void UART_Initialize(void) {
    // 1. I/O Pin Configuration
    TRISCbits.TRISC6 = 0;   // TX (Pin 25) set as Output
    TRISCbits.TRISC7 = 1;   // RX (Pin 26) set as Input

    // 2. Baud Rate Setting (Target: 9600 Baud @ 500kHz Fosc)
    // To achieve low error at low frequency, we use 16-bit Baud Rate Generator.
    // Formula: Baud = Fosc / (4 * (SPBRG + 1))
    // Calculation: 500000 / (4 * (12 + 1)) = 9615 Baud (Error ~0.16%)
    
    BAUDCONbits.BRG16 = 1;  // Enable 16-bit Baud Rate Generator
    TXSTAbits.BRGH = 1;     // High Speed mode
    
    SPBRG = 12;             // Calculated value for 9600 Baud
    SPBRGH = 0;             // High byte is 0

    // 3. Enable Serial Port
    TXSTAbits.SYNC = 0;     // Asynchronous mode
    RCSTAbits.SPEN = 1;     // Enable Serial Port (Configures RC6/RC7)
    
    // 4. Enable Transmission
    TXSTAbits.TXEN = 1;     // Enable transmission
}

void UART_Write(char data) {
    while(!TXSTAbits.TRMT); // Wait until the Transmit Buffer is empty
    TXREG = data;           // Write data to register to send
}

void UART_Write_Text(char *text) {
    int i;
    for(i=0; text[i]!='\0'; i++) // Loop until null terminator
        UART_Write(text[i]);
}

void UART_Write_Text_NonBlocking(char *text) {
    tx_ptr = text;
}

void UART_Task(void) {
    if (tx_ptr != 0 && *tx_ptr != '\0') {
        if (PIR1bits.TXIF) {    // If transmit buffer is empty (Interrupt Flag is set)
            TXREG = *tx_ptr;    // Send current char
            tx_ptr++;           // Move to next char
        }
    } else {
        tx_ptr = 0; // Reset pointer when done or null
    }
}
