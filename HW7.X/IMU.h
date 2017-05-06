#ifndef IMU_H
#define	IMU_H

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

void IMU_init(void);
void IMU_read_multiple(unsigned char reg, unsigned char *data, int length);
unsigned char IMU_check(void);

#endif