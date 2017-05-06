#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "i2c2.h"

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
#define SLAVE_ADDR 0b01000000       // last bit: 0 for write, default state

void init_expander(void);
unsigned char get_expander(void);
void set_expander(unsigned char pin, unsigned char value);

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
    
    init_expander();
    
    unsigned char pin_values;
    
    while(1)    {
        
        pin_values = get_expander();
        
        if  ((pin_values >> 7) == 1)   {  // if GP7 is HIGH
            set_expander(0, 1);     // outputs GP0 as HIGH
        }
        else    {
            set_expander(0, 0);     // outputs GP0 as LOW
        }
    }
}

void init_expander(void)    {
    ANSELBbits.ANSB2 = 0;       // turn of analog on B2 and B3
    ANSELBbits.ANSB3 = 0;       // SDA2 (B2) and SCL2 (B3))
    
    i2c2_master_setup();
    
    i2c2_master_start();
    i2c2_master_send(SLAVE_ADDR);       // address is set to write on default
    i2c2_master_send(0x0);              // access IODIR
    i2c2_master_send(0b11110000);       // GP7:GP4 = inputs, GP3:GP0 = outputs
    i2c2_master_stop();                 // stops communication
    
    i2c2_master_start();
    i2c2_master_send(SLAVE_ADDR);       // address is set to write on default
    i2c2_master_send(0x6);              // access GPPU (pullup resistors)
    i2c2_master_send(0b10000000);       // GP7 pullup (for pushbutton)
    i2c2_master_stop();
    
    i2c2_master_start();
    i2c2_master_send(SLAVE_ADDR);       // address is set to write on default
    i2c2_master_send(0x9);              // access GPIO (output)
    i2c2_master_send(0b00000001);       // sets GP7:GP1 as LOW, GP0 (LED) as HIGH
    i2c2_master_stop();
}

unsigned char get_expander(void)    {
    unsigned char val;
    
    i2c2_master_start();
    i2c2_master_send(SLAVE_ADDR);       // address is set to write on default
    i2c2_master_send(0x9);              // access GPIO
    i2c2_master_restart();              // restarts I2C before reading
    i2c2_master_send(SLAVE_ADDR | 1);   // 1 = read
    
    val = i2c2_master_recv();
    i2c2_master_ack(1);                 // stops requesting data from MCP23008
    i2c2_master_stop();                 // stops I2C
    
    return val;                         // returns 8-bit info with pin values
}

void set_expander(unsigned char pin, unsigned char value)  {
    unsigned char val_set = get_expander();
    
    if (value == 0) {
        val_set = (val_set | (1 << pin)); // AND with NOT 1 shifted by "pin", sets 0
    }
    else    {
        val_set = (val_set & ~ (1 << pin)); // OR with 1 shifted by "pin", sets 1
    }
    
    i2c2_master_start();
    i2c2_master_send(SLAVE_ADDR);       // address is set to write on default
    i2c2_master_send(0x9);              // access GPIO
    i2c2_master_send(val_set);          // sets as 8-bit val_set
    i2c2_master_stop();
}