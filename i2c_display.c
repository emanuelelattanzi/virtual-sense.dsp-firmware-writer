/*
 * $$$MODULE_NAME i2c_display.c
 *
 * $$$MODULE_DESC i2c_display.c
 *
 * Copyright (C) 2014 Emanuele Lattanzi
 *
 /

/**
 *  \file   i2c_display.c
 *
 *  \brief  MIDAS i2c display functions
 *
 *   This file contains the APIs for I2C MIDA LCD Display
 *
 *  (C) Copyright 2014, Emanuele Lattanzi
 *
 *  \author     Emanuele Lattanzi
 *
 *  \version    1.0
 *
 */

#include <stdio.h>
#include "psp_i2c.h"
//#include "dda_i2c.h"
#include "csl_i2c.h"
#include "psp_common.h"

#include "main_config.h"
#include "i2c_display.h"

#define I2C_DISPLAY_ADDRESS		0x3E
#define I2C_READ    1
/** defines the data direction (writing to I2C device) in i2c_start(),i2c_rep_start() */
#define I2C_WRITE   0

#define CSL_I2C_DATA_SIZE        (64)
#define CSL_I2C_OWN_ADDR         (0x2F)
//#define CSL_I2C_SYS_CLK          (12.228)
//#define CSL_I2C_SYS_CLK          (40)
//#define CSL_I2C_SYS_CLK          (60)
//#define CSL_I2C_SYS_CLK          (75)
#define CSL_I2C_SYS_CLK          (40)
#define CSL_I2C_BUS_FREQ         (10)
#define CSL_I2C_EEPROM_ADDR		 (0x50)
#define CSL_I2C_CODEC_ADDR		 (0x18)


extern PSP_Handle    hi2c;
CSL_I2cSetup     i2cSetup;
CSL_I2cConfig    i2cConfig;


static char buffer[80];



void lcd_Delay() {
	int d,e;

	for (d=0; d<0x555; d++)
		for (e=0; e<0xAAA; e++)
			asm(" NOP ");
}



void lcd_Init() {
	Uint16 startStop = ((CSL_I2C_START) | (CSL_I2C_STOP));
	CSL_Status  status;

	// Check if I2C display is just initialized
	Uint16 s = 0x00;
	status = I2C_write(&s, 1, I2C_DISPLAY_ADDRESS, TRUE, startStop, CSL_I2C_MAX_TIMEOUT);
	lcd_Delay();

	if(status == -200) {
		status = I2C_init(CSL_I2C0);

		if(status != CSL_SOK) {
			debug_printf("I2C Init Failed!!\n");
		}

		/* Setup I2C module */
		i2cSetup.addrMode    = CSL_I2C_ADDR_7BIT;
		i2cSetup.bitCount    = CSL_I2C_BC_8BITS;
		i2cSetup.loopBack    = CSL_I2C_LOOPBACK_DISABLE;
		i2cSetup.freeMode    = CSL_I2C_FREEMODE_DISABLE;
		i2cSetup.repeatMode  = CSL_I2C_REPEATMODE_DISABLE;
		i2cSetup.ownAddr     = CSL_I2C_OWN_ADDR;
		i2cSetup.sysInputClk = CSL_I2C_SYS_CLK;
		i2cSetup.i2cBusFreq  = CSL_I2C_BUS_FREQ;

		status = I2C_setup(&i2cSetup);
		if(status != CSL_SOK) {
			debug_printf("I2C Setup Failed!!\n");
		}

		Uint16 b = 0x00;
		status = I2C_write(&b, 1, 0x00, TRUE, startStop, CSL_I2C_MAX_TIMEOUT);
		lcd_Delay();

		Uint16 bToWrite[] = {0x00, 0x38, 0x39, 0x14, 0x74, 0x54, 0x6F, 0x0F, 0x01};
		status = I2C_write(bToWrite, 9, I2C_DISPLAY_ADDRESS, TRUE, startStop, CSL_I2C_MAX_TIMEOUT);
	}
	else {
		// Clear display
		Uint16 l[] = {0x00, 0x01};
		status = I2C_write(l, 2, I2C_DISPLAY_ADDRESS, TRUE, startStop, CSL_I2C_MAX_TIMEOUT);
	}
	lcd_Delay();
}



/*************************************************************************
 Writes a string or char to the Midas MCCOG21605B6W-SPTLYI LCD display

 Input:   String to display as "Hello World!"

 Return:

*************************************************************************/
void LCD_Write(const char *format, ...){

	va_list aptr;
	int len;
	Int16 ret;

	va_start(aptr, format);
	len = vsprintf(buffer, format, aptr);

	Uint16 startStop = ((CSL_I2C_START) | (CSL_I2C_STOP));
	CSL_Status  status;

	lcd_Init();

	int t;
	Uint16 ustr[sizeof(buffer)+1];
	ustr[0] = 0x40;
	for(t = 0; t < len; t++)
	{
		ustr[t+1] = (Uint16)buffer[t];
	}

	status = I2C_write(ustr, (len <= 16)?(len+1):17, I2C_DISPLAY_ADDRESS, TRUE, startStop, CSL_I2C_MAX_TIMEOUT);
	lcd_Delay();

	if(len > 16) {
		Uint16 l[] = {0x00, 0xC0};
		status = I2C_write(l, 2, I2C_DISPLAY_ADDRESS, TRUE, startStop, CSL_I2C_MAX_TIMEOUT);
		lcd_Delay();

		ustr[16] = 0x40;
		status = I2C_write(ustr+16, len-15, I2C_DISPLAY_ADDRESS, TRUE, startStop, CSL_I2C_MAX_TIMEOUT);
		lcd_Delay();
	}

}

/*************************************************************************
 Clear the Midas MCCOG21605B6W-SPTLYI LCD display

 Input:

 Return:

*************************************************************************/
void LCD_Clear() {
	lcd_Init();
}


