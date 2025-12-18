#ifndef ADC_H
#define	ADC_H

#include <xc.h>

void ADC_Initialize(void);
int ADC_Read(int channel);

#endif	/* ADC_H */