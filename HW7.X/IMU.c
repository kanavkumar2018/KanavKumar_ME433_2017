#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "IMU.h"
#include "i2c_master_noint.h"

#define ACCEL_ADDR 0b11010110    // Address of LSM6DS33 (0 = default write)

void IMU_init(void)    {
    ANSELBbits.ANSB2 = 0;           // turn of analog on B2 and B3
    ANSELBbits.ANSB3 = 0;           // SDA2 (B2) and SCL2 (B3))
    
    i2c2_master_setup();            // turns on I2C
    
    i2c2_master_start();
    i2c2_master_send(ACCEL_ADDR);
    i2c2_master_send(0x10);         // access CTRL1_XL register
    i2c2_master_send(0b10000010);   // 1.66 kHz sampling, 2g sensitivity, 100 Hz
    i2c2_master_stop();
    
    i2c2_master_start();
    i2c2_master_send(ACCEL_ADDR);
    i2c2_master_send(0x11);         // access CTRL2_G register
    i2c2_master_send(0b10001000);   // 1.66 kHz sampling, 1000 dps sensitivity 
    i2c2_master_stop();
    
//    i2c2_master_start();
//    i2c2_master_send(ACCEL_ADDR);   // IF_INC default value is 1, no need to write
//    i2c2_master_send(0x12);         // access CTRL3_C register
//    i2c2_master_send(0b00000100);   // IF_INC = 1
//    i2c2_master_stop();
}

void IMU_read_multiple(unsigned char reg, unsigned char *data, int length)  {
    int i;
    
    i2c2_master_start();
    i2c2_master_send(ACCEL_ADDR);
    i2c2_master_send(reg);
    i2c2_master_restart();              // switches to read mode
    i2c2_master_send(ACCEL_ADDR | 1);   // 1 = read
    
    for (i=0;i<length;i++)  {
        data[i] = i2c2_master_recv();
        
        if (i < (length-1))   {
            i2c2_master_ack(0);         // continues to request data
        }
        else    {
            i2c2_master_ack(1);
        }
    }
    
    i2c2_master_stop();
}

unsigned char IMU_check(void)   {
    unsigned char value;
    
    i2c2_master_start();
    i2c2_master_send(ACCEL_ADDR);
    i2c2_master_send(0x0F);             // access WHO_AM_I register
    i2c2_master_restart();
    i2c2_master_send(ACCEL_ADDR | 1);
    value = i2c2_master_recv();
    
    return value;
}
