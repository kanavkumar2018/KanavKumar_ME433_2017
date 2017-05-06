#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "ILI9163C.h"
#include "i2c_master_noint.h"
#include "IMU.h"

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
#define BCKGRND BLUE             // Background LCD color is "BLUE"   (0xF81F)
#define TEXT WHITE               // Text LCD color is "WHITE"        (0xFFFF)
#define MAX_VAL 32768            // 2^16 / 2 = 65536 / 2 = 32768
#define VAL 256                  
#define CHECK 0b01101001         // Default WHO_AM_I register value (105))

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
    
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;

    SPI1_init();    // initializes SPI1 peripheral
    LCD_init();     // initializes LCD screen
    IMU_init();    // initializes I2C2 peripheral
    
    __builtin_enable_interrupts();
    
    LCD_clearScreen(BCKGRND);   // sets LCD screen to color BCKGRND
    
    unsigned char msg1[100];     // debugging message arrays
    unsigned char msg2[100];

    unsigned char IMU_data[14];
    unsigned char value;
    signed short ACC_data[7]; // (temperature, gyroX, gyroY, gyroZ, accelX, accelY, accelZ))
    signed short len = MAX_VAL / VAL;
    signed short barx, bary;


    while(1)    {
        while (_CP0_GET_COUNT() < CLOCK / 4800000)  {;} // 10 Hz refresh speed
        _CP0_SET_COUNT(0);
        
        value = IMU_check();    // checks WHO_AM_I to ensure valid connection

        if (value != CHECK)   {
            while (_CP0_GET_COUNT() < CLOCK/2)  {;}
            _CP0_SET_COUNT(0);
            
            LATAbits.LATA4 = !LATAbits.LATA4;
        }
        else    {
            IMU_read_multiple(0x20, IMU_data, 14);

            int i;

            for (i=0;i<7;i++)   {
                ACC_data[i] = ((IMU_data[(2*i)+1] << 8) | (IMU_data[2*i]));
            }

//            sprintf(msg3, " X:  %d    ", ACC_data[4]);
//            sprintf(msg4, " Y:  %d    ", ACC_data[5]);
//            LCD_writeString(msg1, 20, 20, TEXT, BCKGRND);
//            LCD_writeString(msg2, 20, 40, TEXT, BCKGRND);
            
            LCD_writeBar(60, 60, TEXT, 4, 4);
            
            if (ACC_data[4] < 0)    {
                barx = (-1)*ACC_data[4]/VAL;
                
                LCD_writeBar(64, 60, TEXT, barx, 4);
                LCD_writeBar(64+barx, 60, BCKGRND, len-barx, 4);
                LCD_writeBar(59-len, 60, BCKGRND, len, 4);
            }
            else {
                barx = ACC_data[4]/VAL;
                
                LCD_writeBar(60-barx, 60, TEXT, barx, 4);
                LCD_writeBar(60-len, 60, BCKGRND, len-barx, 4);
                LCD_writeBar(65, 60, BCKGRND, len, 4);
            }
            
            if (ACC_data[5] < 0)    {
                bary = (-1)*ACC_data[5]/VAL;
                
                LCD_writeBar(60, 64, TEXT, 4, bary);
                LCD_writeBar(60, 64+bary, BCKGRND, 4, len-bary);
                LCD_writeBar(60, 59-len, BCKGRND, 4, len);
            }
            else    {
                bary = ACC_data[5]/VAL;
                
                LCD_writeBar(60, 60-bary, TEXT, 4, bary);
                LCD_writeBar(60, 60-len, BCKGRND, 4, len-bary);
                LCD_writeBar(60, 65, BCKGRND, 4, len);
            }
        }
    }
}
