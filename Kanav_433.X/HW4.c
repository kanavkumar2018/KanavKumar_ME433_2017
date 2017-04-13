#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>         // Math library
#include<stdio.h>

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable secondary osc
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 00011010 // (26) some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

// Definitions
#define CLOCK 48000000
#define CS LATAbits.LATA4       // chip select (CS) pin

void initSPI1(void);
unsigned char spi_io(unsigned char o);
void write_dac(unsigned int channel, unsigned int voltage);


int main() {

    __builtin_disable_interrupts();

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    initSPI1();
    
    unsigned int i = 0;
    
    unsigned int sinewave[100];
    unsigned int rampwave[100];
    double temp;
    
    for (i=0;i<100;i++)    {
        temp = 255.0/2.0 + 255.0/2.0*sin(2.0*M_PI*i/100.0);
        sinewave[i] = temp;
        temp = 2.5*(i+225.0/100.0);
        rampwave[i] = temp;
    }
    
    __builtin_enable_interrupts();
    
    i = 0;
    
    while(1) {
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < CLOCK/2/1000)  {;}
        write_dac(1, sinewave[i]);
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT() < CLOCK/2/10000)  {;}
        write_dac(0, rampwave[i]);
        i++;
        if (i == 100)  {
            i = 0;
        }
    }
}

void initSPI1() {
    TRISAbits.TRISA4 = 0;     // A4 is CS pin
    CS = 1;
    
    RPA1Rbits.RPA1R = 0b0011; // sets A1 as SDO1 of PIC32

    
    SPI1CON = 0;              // turn off the spi module and reset it
    SPI1BUF;                  // clear the rx buffer by reading from it
    SPI1BRG = 0x1000;         // baud rate to 10 MHz [SPI4BRG = (80000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0;  // clear the overflow bit
    SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1;    // master operation
    SPI1CONbits.ON = 1;       // turn on spi 1
}


unsigned char spi_io(unsigned char o) {
    SPI1BUF = o;
    while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
    }
    return SPI1BUF;
}

void write_dac(unsigned int channel, unsigned int voltage)  {
    unsigned char b1 = 0;
    unsigned char b2 = 0;
    
    b1 = (channel<<7);
    b1 = b1 | (0b1110000);
    b1 = b1 | (voltage>>4);
    
    b2 = voltage<<4;
    
    CS = 0;
    spi_io(b1);
    spi_io(b2);
    CS = 1;
}