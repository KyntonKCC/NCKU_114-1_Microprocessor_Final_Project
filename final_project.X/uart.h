#ifndef UART_H
#define	UART_H

#include <xc.h>

// Function Prototypes
void UART_Initialize(void);
void UART_Write(char data);
void UART_Write_Text(char *text);
void UART_Write_Text_NonBlocking(char *text);
void UART_Task(void);

#endif	/* UART_H */