#include "config.h"
#include "setting.h"
#include "servo.h"
#include "uart.h"
#include "adc.h"
#include <pic18f4520.h>
#include <stdio.h>

volatile int curr = 15;
volatile int dir = 1;
volatile int run = 1;
volatile int count_msg = 0;
volatile int send_msg = 0;
volatile int fire_condition = 0;

void __interrupt() ISR(void){
    if(INTCONbits.TMR0IF){
        TMR0H = 0xEB;
        TMR0L = 0xB0;

        count_msg++;
        if(count_msg >= 12){ // 12 * 41.6ms approx 0.5s
            count_msg = 0;
            send_msg = 1;
        }

        if(run){
            curr += dir;
            if(curr >= 75){
                curr = 75;
                dir = -1;
            }else if(curr <= 15){
                curr = 15;
                dir = 1;
            }
            setServo1Position(curr);
        }

        INTCONbits.TMR0IF = 0;
    }
}

void main(void){

    SYSTEM_Initialize();
    UART_Initialize();
    ADC_Initialize();

    setServo1Position(curr);
    setServo2Position(15);

    static char json_buf[64]; // Static for non-blocking access

    while(1){
        UART_Task(); // Run non-blocking UART task

        int adc_raw = ADC_Read(0);
        int angle_deg = (curr - 15) * 3 - 90;
        int flame_intensity = adc_raw;

        if (adc_raw < 400) {
            fire_condition = 1;
        } else if (adc_raw > 600) {
            fire_condition = 0;
        }

        if(PORTBbits.RB0 == 0 || fire_condition){
            run = 0;
            setServo2Position(curr);
            LATDbits.LATD0 = 1; // Buzzer ON
            LATDbits.LATD1 = 1; // Motor 3 ON

            // Immediate status update logic could go here if needed,
            // but requirements say "data every 0.5s", implies periodic.
            // If IMMEDIATE "FIRE" alert is needed on top of periodic, could force it.
            // For now, sticking to strict 0.5s periodic logging as requested.
            
        }else{
            run = 1;
            LATD = 0;
        }

        if(send_msg){
            if(PORTBbits.RB0 == 0 || fire_condition){
                sprintf(json_buf, "{\"angle\": %d, \"temp\": %d, \"status\": \"FIRE\"}\r\n", angle_deg, flame_intensity);
            } else {
                sprintf(json_buf, "{\"status\": \"SAFE\", \"angle\": %d, \"temp\": %d}\r\n", angle_deg, flame_intensity);
            }
            UART_Write_Text_NonBlocking(json_buf);
            send_msg = 0;
        }
    }
    return;
}