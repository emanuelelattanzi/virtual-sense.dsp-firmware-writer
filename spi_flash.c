/*
 * spi_flash.c
 *
 *  Created on: 15/10/2014
 *      Author: Emanuele Lattanzi
 */

#include <stdio.h>
#include <csl_general.h>
#include "spi_flash.h"

#include "csl_sysctrl.h"
#include "main_config.h"
#include "cslr.h"


CSL_SpiHandle	hSpi;
#define CSL_TEST_FAILED         (1)
/**< Error Status to indicate Test has Failed */
#define CSL_TEST_PASSED         (0)
/**< Error Status to indicate Test has Passed */

#define	CSL_SPI_BUF_LEN			(64)
/**< SPI Buffer length used for reading and writing */

#define	SPI_CLK_DIV				(25)
/**< SPI Clock Divisor */

#define	SPI_FRAME_LENGTH		(1)
/**< SPI Frame length */

/** Data to be written to EEPROM */
#define DATA_TO_WRITE (0x17)

/** Buffer length used for reading and writing to EEPROM */
#define CSL_SPI_BUF_LEN_EEPROM (32)


/** Command Buffer */
Uint16 cmdBuffer[6] = {0, 0, 0, 0, 0, 0};
CSL_SpiHandle	hSpi;


void init_spi_flash(void){

	CSL_Status 		status;
	Int16 			result;
	SPI_Config		hwConfig;
	volatile Uint32	looper;
	volatile Uint16 value = 0;
	Uint32 			pageNo;
	Uint16          commandLength;
	Uint16 e = 0;
	volatile Uint16	pollStatus;
	volatile Uint16	fnCnt;

	hSpi = NULL;

	pageNo = 0x0001;

	result = SPI_init();
	if(CSL_SOK != result)
	{
		debug_printf ("SPI init error\n");
		return;
	}

	hSpi = SPI_open(SPI_CS_NUM_0, SPI_POLLING_MODE);
	if(NULL == hSpi)
	{
		debug_printf ("SPI open error\n");
		return;

	}
	status = SYS_setEBSR(CSL_EBSR_FIELD_PPMODE,
						 CSL_EBSR_PPMODE_1);

	if (CSL_SOK != status)
	{
		debug_printf("SYS_setEBSR failed\n");
		return;
	}

	/** Set the hardware configuration 							*/
	hwConfig.spiClkDiv	= 100;
	hwConfig.wLen		= SPI_WORD_LENGTH_8;
	hwConfig.frLen		= SPI_FRAME_LENGTH;
	hwConfig.wcEnable	= SPI_WORD_IRQ_ENABLE;
	hwConfig.fcEnable	= SPI_FRAME_IRQ_DISABLE;
	hwConfig.csNum		= SPI_CS_NUM_0;
	hwConfig.dataDelay	= SPI_DATA_DLY_1;
	hwConfig.csPol		= SPI_CSP_ACTIVE_LOW;
	hwConfig.clkPol		= SPI_CLKP_LOW_AT_IDLE;
	hwConfig.clkPh		= SPI_CLK_PH_RISE_EDGE;

	result = SPI_config(hSpi, &hwConfig);
	if(CSL_SOK != result)
	{
		debug_printf ("SPI config error\n");
		return;

	}


	/* Read device ID */

	CSL_SPI_REGS->SPICMD1 = 0x0000 | 5 - 1;

	cmdBuffer[0] = RDID;
	cmdBuffer[1] = ( Uint8 )( (pageNo & 0xFFFFFF)>> 16 );
	cmdBuffer[2] = ( Uint8 )( (pageNo & 0xFFFF)>> 8 );
	cmdBuffer[3] = ( Uint8 )( pageNo & 0xFF );

	commandLength = 4;

	result = SPI_dataTransaction(hSpi ,cmdBuffer, commandLength, SPI_WRITE);
	result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_READ);

	if(CSL_SOK != result)
	{
		debug_printf ("SPI read device id error\n");
		return;
	}

	debug_printf("device id is 0x%x\n",cmdBuffer[0]);

	// Clear any pre-existing AAI write mode
	// This may occur if the PIC is reset during a write, but the Flash is
	// not tied to the same hardware reset.

	CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;

	cmdBuffer[0] = WRDI;

	result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_WRITE);
	if(CSL_SOK != result)
	{
		debug_printf ("SPI WRDI device error\n");
		return;
	}

	// Execute Enable-Write-Status-Register (EWSR) instruction
	CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;

	cmdBuffer[0] = EWSR;

	result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_WRITE);
	if(CSL_SOK != result)
	{
		debug_printf ("SPI EWSR device error\n");
		return;
	}

	// Clear Write-Protect on all memory locations
	// WRITE 0 on BP0,BP1,BP2,BP3

	CSL_SPI_REGS->SPICMD1 = 0x0000 | 1;

	cmdBuffer[0] = WRSR;
	cmdBuffer[1] = ( Uint8 )( 0x10);

	commandLength = 2;

	result = SPI_dataTransaction(hSpi ,cmdBuffer, commandLength, SPI_WRITE);
	if(CSL_SOK != result)
	{
		debug_printf ("SPI WRSR device error\n");
		return;
	}

	cmdBuffer[0] = RDSR;

	/* Send read status command */

	CSL_SPI_REGS->SPICMD1 = 0x0000 | 1;

	result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_WRITE );
	result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_READ);

	debug_printf ("Status register is 0x%x\n",cmdBuffer[0]);

}

void erase_spi_flash(void){
	// Clear any pre-existing AAI write mode
	// This may occur if the PIC is reset during a write, but the Flash is
	// not tied to the same hardware reset.
	Int16 			result;
	Uint32 writingAddress = 0x00000000;
	Uint16 index = 0;

	for(index = 0; index< 50; index++){
		// Enable writing
		CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;

		cmdBuffer[0] = WREN;

		result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_WRITE);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI EWSR device error\n");
			return;
		}


		CSL_SPI_REGS->SPICMD1 = 0x0000 | 4 - 1;

		cmdBuffer[0] = ERASE_4K;
		/* Flash address - MSB */
		cmdBuffer[1] = (writingAddress >> 0x10 & 0xFF);
		/* Flash address */
		cmdBuffer[2] = ((writingAddress >> 0x08) & 0xFF);
		/* Flash address - LSB */
		cmdBuffer[3] = (writingAddress & 0xFF);


		result = SPI_dataTransaction(hSpi ,cmdBuffer, 4, SPI_WRITE);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI ERASE_ALL error\n");
			return;
		}
		_WaitWhileBusy();
		writingAddress+=4096;
	}
}
void spi_flash_write_byte ( Uint16 byte,
                        Uint32        address){
		Uint32 i = 0;

		Int16 			result;
		Uint16          commandLength;

		Uint32 writingAddress = address;

		// Enable writing
		CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;

		cmdBuffer[0] = WREN;

		result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_WRITE);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI EWSR device error\n");
			return;
		}

		/* Write Command */
		cmdBuffer[0] = WRITE;
		/* Flash address - MSB */
		cmdBuffer[1] = (writingAddress >> 0x10 & 0xFF);
		/* Flash address */
		cmdBuffer[2] = ((writingAddress >> 0x08) & 0xFF);
		/* Flash address - LSB */
		cmdBuffer[3] = (writingAddress & 0xFF);
		/* data byte */
		cmdBuffer[4] = (byte & 0xFF);

		commandLength = 5;



		CSL_SPI_REGS->SPICMD1 = 0x0000 | 5 - 1;


		result = SPI_dataTransaction(hSpi ,cmdBuffer, commandLength, SPI_WRITE);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI error writing bytes\n");
		}
		_WaitWhileBusy();
}

void spi_flash_write ( Uint16 *      src,
                        Uint32        address,
                        Uint16        length){

		Uint16 * data = src;
		Uint32 i = 0;

		Int16 			result;
		Uint16          commandLength;

		Uint32 writingAddress = address;
#if 0
		// Enable writing
		CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;

		cmdBuffer[0] = WREN;

		result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_WRITE);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI WREN device error\n");
			return;
		}


		/* Write Command */
		cmdBuffer[0] = WRITE;
		/* Flash address - MSB */
		cmdBuffer[1] = (writingAddress >> 0x10 & 0xFF);
		/* Flash address */
		cmdBuffer[2] = ((writingAddress >> 0x08) & 0xFF);
		/* Flash address - LSB */
		cmdBuffer[3] = (writingAddress & 0xFF);

		commandLength = 4;


		CSL_SPI_REGS->SPICMD1 = 0x0000 | length + 4 - 1;


		result = SPI_dataTransaction(hSpi ,cmdBuffer, commandLength, SPI_WRITE);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI WRITE device error\n");
			return;
		}
		result = SPI_dataTransaction(hSpi ,&data[0], length, SPI_WRITE);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI WRITE2 device error\n");
						return;;
		}
		_WaitWhileBusy();
#endif
#if 1
	    // Issue WRITE command with address

		for(i=0; i < length; i++){

			// Enable writing
			CSL_SPI_REGS->SPICMD1 = 0x0000 | 0;

			cmdBuffer[0] = WREN;

			result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_WRITE);
			if(CSL_SOK != result)
			{
				debug_printf ("SPI EWSR device error\n");
				return;
			}

			/* Write Command */
			cmdBuffer[0] = WRITE;
			/* Flash address - MSB */
			cmdBuffer[1] = (writingAddress >> 0x10 & 0xFF);
			/* Flash address */
			cmdBuffer[2] = ((writingAddress >> 0x08) & 0xFF);
			/* Flash address - LSB */
			cmdBuffer[3] = (writingAddress & 0xFF);
			/* data byte */
			cmdBuffer[4] = (data[i]& 0xFF);

			commandLength = 5;



			CSL_SPI_REGS->SPICMD1 = 0x0000 | 5 - 1;


			result = SPI_dataTransaction(hSpi ,cmdBuffer, commandLength, SPI_WRITE);
			if(CSL_SOK != result)
			{
				debug_printf ("SPI error writing bytes\n");
			}
			_WaitWhileBusy();
			writingAddress++;
		}
#endif

}

void spi_flash_read (Uint32 address,
                     Uint16 *        dst,
                      Uint16        length){

	    Uint32 srcAddress = address;
		Int16 			result;
		Uint16          commandLength;

	 /* Configure the SPI for read from SPI Flash */
	    /* read command */
	    cmdBuffer[0] = READ;
	    /* Flash address - MSB */
	    cmdBuffer[1] = ((srcAddress >> 0x10) & 0xFF);
	    /* Flash address */
	    cmdBuffer[2] = ((srcAddress >> 0x08) & 0xFF);
		/*Flash address - LSB */
		cmdBuffer[3] = (srcAddress & 0xFF);

		commandLength = 4;




		CSL_SPI_REGS->SPICMD1 = 0x0000 | CSL_SPI_BUF_LEN + 4 - 1;


		result = SPI_dataTransaction(hSpi ,cmdBuffer, commandLength, SPI_WRITE);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI write command error\n");
		}

		/******************************************************************************/
		result = SPI_dataTransaction(hSpi ,dst, CSL_SPI_BUF_LEN, SPI_READ);
		if(CSL_SOK != result)
		{
			debug_printf ("SPI read error\n");
		}


}


void _WaitWhileBusy(void){
	Int16 			result;
	 do
	    {
			cmdBuffer[0] = RDSR;

			/* Send read status command */

			CSL_SPI_REGS->SPICMD1 = 0x0000 | 1;


			result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_WRITE );
			result = SPI_dataTransaction(hSpi ,cmdBuffer, 1, SPI_READ);

		} while((cmdBuffer[0]  & 0x01));
}
