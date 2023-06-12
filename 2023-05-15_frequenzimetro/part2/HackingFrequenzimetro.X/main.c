/*
 * File:   main.c
 * Author: fremsoft
 *
 * Created on 22 maggio 2023, 15.46
 */


// PIC16F628A Configuration Bit Settings
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>

/* ROUTINES DI INTERRUZIONE */
void __interrupt() tcInt(void)
{
    if (TMR0IE && TMR0IF) { // any timer 0 interrupts?
        TMR0IF=0;
        //++tick_count;
    }
    if (TMR2IE && TMR2IF) { // any timer 1 interrupts?
        TMR2IF=0;
        //tick_count += 100;
    }
    /*
    
    INTCONbits.T0IF = 0;      // (must be cleared in software)

    PIR1bits.TMR2IF = 0;      // TMR2 to PR2 Match Interrupt Flag bit
                              // (must be cleared in software)
    */
    
    return;
}


void setup() {

    OPTION_REGbits.PS = 7;    // PRESCALER 1:128 PER WDT
    OPTION_REGbits.PSA = 1;   // PRESCALER PER WDT
    OPTION_REGbits.T0SE = 0;  // Increment on low-to-high transition on RA4/T0CKI/CMP2 pin
    OPTION_REGbits.T0CS = 1;  // Transition on T0CKI pin (3)
    OPTION_REGbits.nRBPU = 1; // PORTB pull-ups are disabled
    
    INTCONbits.GIE = 0;       // Disables all interrupts
    
    INTCONbits.PEIE = 1;      // Peripheral Interrupt Enable bit
    INTCONbits.T0IE = 1;      // TMR0 Overflow Interrupt Enable bit
    INTCONbits.T0IF = 0;      // (must be cleared in software)
    
    PIE1bits.TMR2IE = 1;      // TMR2 to PR2 Match Interrupt Enable bit
                              // dopo postscaler
    PIR1bits.TMR2IF = 0;      // TMR2 to PR2 Match Interrupt Flag bit
                              // (must be cleared in software)
    
    TRISA = 0b00110000;       // PORTA0-A3 : catodo comune dei display
                              // PORTA4    : ingresso frequenza
                              // PORTA5    : /CLR o PULSANTE (con pullup)
    PORTA = 0b00001111;       // TUTTO SPENTO (tranne la quinta cifra)
    
    TRISB = 0b00000000;       // PORTB0 : anodo segmento 
                              // PORTB1 : anodo segmento 
                              // PORTB2 : anodo segmento 
                              // PORTB3 : anodo segmento 
                              // PORTB4 : anodo segmento 
                              // PORTB5 : anodo segmento 
                              // PORTB6 : anodo segmento 
                              // PORTB7 : anodo segmento 
    PORTB = 0b00000001;       // ACCENDO SOLO UN SEGMENTO 

    // il Timer 0 è usato come contatore di eventi esterni
    // quando raggiunge i 256 conteggi TMR0 si azzera e scatta
    // un interrupt che usiamo per incrementare un altro contatore
    TMR0 = 0; 
    
    // il Timer 2 è usato per creare un interrupt ogni millisecondo
    T2CONbits.T2CKPS = 1;     // 01 = 1:4 Prescaler Value
    T2CONbits.TOUTPS = 10-1;  // 9 = 1:10 Postscaler Value
    T2CONbits.TMR2ON = 1;
    
    // spengo i comparatori
    CMCONbits.CM = 7;        // CM = 7 : Comparators Off ingressi digitali
    
}

void loop() {
}

void main(void) {
    
    setup();
    
    while ( 1 ) {
      
        loop();
        
    }
    
}

