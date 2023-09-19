/*
 * File:   main.c
 * Author: fremsoft
 *
 * Created on 22 maggio 2023, 15.46
 */

#define _XTAL_FREQ 20000000  

// PIC16F628A Configuration Bit Settings
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>

volatile unsigned long t0_ovl;
volatile unsigned long t0_gross;       // TMR0 + t0_ovl
volatile unsigned long t0_gross_prec;  // TMR0 + t0_ovl precedente
volatile unsigned long frequenza_x100;

volatile int   millisecondi;

/* ROUTINES DI INTERRUZIONE */
void __interrupt() tcInt(void)
{
    unsigned long frequenza_x100_temp;
    unsigned long ratio;
    
    if (TMR0IE && TMR0IF) { // any timer 0 interrupts?
        TMR0IF=0;
        
        t0_ovl += 256L;
    }
    
    if (TMR2IE && TMR2IF) { // any timer 2 interrupts?
        TMR2IF=0;
        
        // e' trascorso un millisecondo
        millisecondi ++;
        
        if (millisecondi >= 500) {
            // e' trascorso mezzo secondo
            
            t0_gross = ((unsigned long int)TMR0 + t0_ovl) * 200L; 
            
            // ... do something
            frequenza_x100_temp = (t0_gross - t0_gross_prec);
            ratio = (100 * frequenza_x100_temp) / frequenza_x100;
            
            if ((ratio > 101) || (ratio < 99)) {
                /* se c'è una grossa variazione la segue istantaneamente */
                frequenza_x100 = frequenza_x100_temp;
            }
            else {
                /* altrimenti esegue un filtro digitale */
                frequenza_x100 = ((frequenza_x100*9) + (frequenza_x100_temp)) / 10;
            }
            
            t0_gross_prec = t0_gross;
            millisecondi = 0;
        }   
    }
    
    return;
}

void accendi_cifra( int posizione, int cifra, int virgola ) {
    
    PORTB = 0; // spengo momentaneamente i segmenti
    
    // attivo la 'n'esima cifra
    switch ( posizione ) {
        case 1 : //       12435 
            PORTA = 0b00000111;       // IN LOGICA NEGATIVA
            break;  
        case 2 : //       12435 
            PORTA = 0b00001011;       // IN LOGICA NEGATIVA
            break;  
        case 3 : //       12435 
            PORTA = 0b00001110;       // IN LOGICA NEGATIVA
            break;  
        case 4 : //       12435 
            PORTA = 0b00001101;       // IN LOGICA NEGATIVA
            break;  
        default :
            PORTA = 0b00001111;   // accendo la 5' cifra a destra ( centesimi )
            break;
    }
    
    // accendo i segmenti
    switch ( cifra ) {
        case 0 :   // BAGFECpD
            PORTB = 0b11011101;       
            break;
        case 1 :   // BAGFECpD
            PORTB = 0b10000100;       
            break;
        case 2 :   // BAGFECpD
            PORTB = 0b11101001;       
            break;
        case 3 :   // BAGFECpD
            PORTB = 0b11100101;       
            break;
        case 4 :   // BAGFECpD
            PORTB = 0b10110100;       
            break;
        case 5 :   // BAGFECpD
            PORTB = 0b01110101;       
            break;
        case 6 :   // BAGFECpD
            PORTB = 0b01111101;       
            break;
        case 7 :   // BAGFECpD
            PORTB = 0b11000100;       
            break;
        case 8 :   // BAGFECpD
            PORTB = 0b11111101;       
            break;
        case 9 :   // BAGFECpD
            PORTB = 0b11110101;       
            break;
        default :  // BAGFECpD
            PORTB = 0b00000000;       
            break;
    }
    
    // accendo eventualmente anche la virgola
    if (virgola != 0) {
        //                  BAGFECpD
        //PORTB = PORTB | 0b00000010;
        PORTBbits.RB1 = 1;
    }
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
    
    // spengo i comparatori
    CMCONbits.CM = 7;        // CM = 7 : Comparators Off ingressi digitali

    TRISA = 0b00110000;       // PORTA0-A3 : catodo comune dei display
                              // PORTA4    : ingresso frequenza
                              // PORTA5    : /CLR o PULSANTE (con pullup)
    //            12435 
    PORTA = 0b00001111;       // TUTTO SPENTO (tranne la quinta cifra)
    
    TRISB = 0b00000000;       // PORTB0 : anodo segmento 
                              // PORTB1 : anodo segmento 
                              // PORTB2 : anodo segmento 
                              // PORTB3 : anodo segmento 
                              // PORTB4 : anodo segmento 
                              // PORTB5 : anodo segmento 
                              // PORTB6 : anodo segmento 
                              // PORTB7 : anodo segmento 
           // BAGFECpD
    PORTB = 0b00000000;       // ACCENDO SOLO UN SEGMENTO 

    // il Timer 0 è usato come contatore di eventi esterni
    // quando raggiunge i 256 conteggi TMR0 si azzera e scatta
    // un interrupt che usiamo per incrementare un altro contatore
    
    // il Timer 2 è usato per creare un interrupt ogni millisecondo
    T2CONbits.T2CKPS = 1;     // 01 = 1:4 Prescaler Value
    T2CONbits.TOUTPS = 10-1;  // 9 = 1:10 Postscaler Value
    T2CONbits.TMR2ON = 1;
    PR2 = 125 - 1;            // 20 MHz : 4 : 4 : 10 : (124+1) = 1000 Hz
    
    // azzero le variabili globali
    t0_ovl = 0;
    TMR0 = 0;
    
    t0_gross = 0;       // TMR0 + t0_ovl
    t0_gross_prec = 0;
    
    millisecondi = 0;
    frequenza_x100 = 0;
    
    INTCONbits.GIE = 1;       // Enables all interrupts
}

void loop() {
    
    // in modo asincrono visualizzo il valore di frequenza
    long f;
    int  cifra;
    
    TMR2IE = 0;       // Disables all interrupts    
    f = frequenza_x100;
    // DEBUG : f = 12345;
    TMR2IE = 1;       // Enables all interrupts
     
    // f contiene la frequenza asintotica espressa in centesimi di Hz, 
    //   del segnale presente sul pin 3 A4/ToCKI
    
    // estrarre le 5 cifre significative da f
    cifra = f % 10;
    // visualizzazione delle singole cifre
    accendi_cifra( 5, cifra, 0 );
    __delay_ms( 1 );
    
    f = f / 10;
    cifra = f % 10;
    // visualizzazione delle singole cifre
    accendi_cifra( 4, cifra, 0 );
    __delay_ms( 1 );
    
    f = f / 10;
    cifra = f % 10;
    // visualizzazione delle singole cifre
    accendi_cifra( 3, cifra, 1 );
    __delay_ms( 1 );
    
    f = f / 10;
    cifra = f % 10;
    // visualizzazione delle singole cifre
    accendi_cifra( 2, cifra, 0 );
    __delay_ms( 1 );
    
    f = f / 10;
    cifra = f % 10;
    // visualizzazione delle singole cifre
    accendi_cifra( 1, cifra, 0 );
    __delay_ms( 1 );    
    
}

void main(void) {
    setup();
    
    while ( 1 ) {
       
        loop();
        
    }
}