/* 
 * File:   hw4.c
 * Author: kanavkumar
 *
 * Created on April 12, 2017, 2:23 PM
 */

#include "NU32.h"       // constants, funcs for startup and UART
#include <math.h>
#include <stdio.h>


// Demonstrates spi by accessing external ram
// PIC is the master, ram is the slave
// Uses microchip 23K256 ram chip (see the data sheet for protocol details)
// SDO4 -> SI (pin F5 -> pin 5)
// SDI4 -> SO (pin F4 -> pin 2)
// SCK4 -> SCK (pin B14 -> pin 6)
// SS4 -> CS (pin B8 -> pin 1)
// Additional SRAM connections
// Vss (Pin 4) -> ground
// Vcc (Pin ? -> 3.3 V
// Hold (pin 7) -> 3.3 V (we don't use the hold function)
// 
// Only uses the SRAM's sequential mode
//
#define CS LATBbits.LATB7       // chip select pin

// send a byte via spi and return the response
unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

// initialize spi4 and the ram module
void ram_init() {
  // set up the chip select pin as an output
  // the chip select pin is used by the sram to indicate
  // when a command is beginning (clear CS to low) and when it
  // is ending (set CS high)
  TRISBbits.TRISB7 = 0;
  CS = 1;

  // Master - SPI4, pins are: SDI4(F4), SDO4(F5), SCK4(F13).  
  // we manually control SS4 as a digital output (F12)
  // since the pic is just starting, we know that spi is off. We rely on defaults here
  TRISAbits.TRISA4 = 0;
  CS = 1;
  
  RPA1Rbits.RPA1R = 0b0011 //example
  // setup spi4
  SPI1CON = 0;              // turn off the spi module and reset it
  SPI1BUF;                  // clear the rx buffer by reading from it
  SPI1BRG = 0x1000;            // baud rate to 10 MHz [SPI4BRG = (80000000/(2*desired))-1]
  SPI1STATbits.SPIROV = 0;  // clear the overflow bit
  SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
  SPI1CONbits.MSTEN = 1;    // master operation
  SPI1CONbits.ON = 1;       // turn on spi 4
  
                            // send a ram set status command.
  CS = 0;                   // enable the ram
  spi_io(0x01);             // ram write status
  spi_io(0x41);             // sequential mode (mode = 0b01), hold disabled (hold = 0)
  CS = 1;                   // finish the command
}


// read len bytes from ram, starting at the address addr
/*void ram_read(unsigned short addr, char data[], int len) {
  int i = 0;
  CS = 0;
  spi_io(0x3);                   // ram read operation
  spi_io((addr & 0xFF00) >> 8);  // most significant address byte
  spi_io(addr & 0x00FF);         // least significant address byte
  for(i = 0; i < len; ++i) {
    data[i] = spi_io(0);         // read in the data
  }
  CS = 1;
}*/

int main(void) {
  unsigned short addr1 = 0x1234;                  // the address for writing the ram
  char data[] = "Help, I'm stuck in the RAM!";    // the test message
  char read[] = "***************************";    // buffer for reading from ram
  char buf[100];                                  // buffer for comm. with the user
  unsigned char status;                           // used to verify we set the status 
  NU32_Startup();   // cache on, interrupts on, LED/button init, UART init
  ram_init(); 
  unsigned int i = 0;
  
  unsigned int sinewave[100];
  unsigned int rampwave[100];
  double temp;
  
  for (i=0; i<100; i++){
      temp = 255.0/2.0+255/2*sin(2.0*3.14*i/100.0);
      sinewave{i} = temp;
      temp = i*255.0/100.0;
      rampwave[i] = temp;
  }
  TRISBbits.TRISB9 = 0;
  TRISBbits.TRISB8 = 1;
  
  // check the ram status
  CS = 0;
  spi_io(0x5);                                      // ram read status command
  status = spi_io(0);                               // the actual status
  CS = 1;

  sprintf(buf, "Status 0x%x\r\n",status);
  NU32_WriteUART3(buf);

  sprintf(buf,"Writing \"%s\" to ram at address 0x%x\r\n", data, addr1);
  NU32_WriteUART3(buf);
                                                    // write the data to the ram
  ram_write(addr1, data, strlen(data) + 1);         // +1, to send the '\0' character
  ram_read(addr1, read, strlen(data) + 1);          // read the data back
  sprintf(buf,"Read \"%s\" from ram at address 0x%x\r\n", read, addr1);
  NU32_WriteUART3(buf);

  while(1) {
      _CP0_SET_COUNT(0);
      while(_CP0_GET_COUNT() < 48000000/2/1000){}
      write_dac(),sinewave[i];
      _CP0_SET_COUNT(0);
      while (_CP0_SET_COUNT() < 48000000/2/1000){}
      write_dac(0,rampwave[i]);
      i++;
      if(i==100){
          i=0;
          
      }
  }
  
}
void write_dac(unsigned int channel, unsigned int voltage){
    unsigned char b1=0, b2=0;
    
    b1 = (channel<<7);
    b1 = b1 | (0b1110000);
    b1 = b1 | (voltage>>4);
    
    b2 = voltage<<4;
    
    CS = 0;
    spi_io(b1);
    spi_io(b2);
    CS = 1;
    
}