
#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "HW6.h"

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
#define BCKGRND BLUE        // Background LCD color is "BLUE"   (0x001F)
#define TEXT WHITE          // Text LCD color is "WHITE"        (0xFFFF)


int main() {
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);
    
    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    SPI1_init();    // initializes SPI1 peripheral
    LCD_init();     // initializes LCD screen
    
    __builtin_enable_interrupts();
    
    LCD_clearScreen(BCKGRND);   // sets LCD screen to color BCKGRND
    
    char msg[100];
    char num[20];
    char fps[20];
    sprintf(msg, "Hello World!");
    
    unsigned int msgLen = 6*LCD_msgLength(msg); // sets spacing for numbers later
    
    int i = 0;
    int fps_count;
    
    LCD_writeString(msg, 28, 32, TEXT, BCKGRND);
    
    _CP0_SET_COUNT(0);
    
    while(1)    {
        _CP0_SET_COUNT(0);
        sprintf(num, "Bar Length: %d   ", (i-50));
        LCD_writeString(num, 24, 44, TEXT, BCKGRND);
        LCD_writeBar(64, 57, TEXT, 1, 5);           // draws center bar
        if (i < 50)  {
            LCD_writeBar(14+i, 57, TEXT, (50-i), 5);    // draws bar color TEXT
            LCD_writeBar(14, 57, BCKGRND, i, 5);      // draws bar color BCKGRND over old bars
        }
        else    {
            LCD_writeBar(64, 57, TEXT, (i-50), 5);
        }

        i++;
        if (i == 100)   {
            i = 0;
            LCD_writeBar(64, 57, BCKGRND, 50, 5);       // clears bar on the right
        }
        
        fps_count = (CLOCK/2) / _CP0_GET_COUNT();
        sprintf(fps, "FPS:  %d   ", fps_count);
        LCD_writeString(fps, 44, 70, TEXT, BCKGRND);
        
    }
}
