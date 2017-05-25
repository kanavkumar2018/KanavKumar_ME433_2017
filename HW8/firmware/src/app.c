/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include "../../../HW7.X/ILI9163C.h"
#include "../../../HW7.X/IMU.h"
#include "../../../HW7.X/i2c_master_noint.h"

// Definitions
#define CLOCK 48000000
#define BCKGRND BLUE             // Background LCD color is "BLUE"   (0xF81F)
#define TEXT WHITE               // Text LCD color is "WHITE"        (0xFFFF)
#define MAX_VAL 32768            // 2^16 / 2 = 65536 / 2 = 32768
#define VAL 256                  
#define CHECK 0b01101001         // Default WHO_AM_I register value (105))




void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;

    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
            
        
            if (appInitialized)
            {
            
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
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
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

 

/*******************************************************************************
 End of File
 */
