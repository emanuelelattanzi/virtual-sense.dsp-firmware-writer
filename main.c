/*
 * $$$MODULE_NAME csl_usb_iso_fullspeed_example.c
 *
 * $$$MODULE_DESC csl_usb_iso_fullspeed_example.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  This software is licensed under the  standard terms and conditions in the Texas Instruments  Incorporated
 *  Technology and Software Publicly Available Software License Agreement , a copy of which is included in the
 *  software download.
*/

/** @file csl_usb_iso_fullspeed_example.c
 *
 *  @brief USB Audio Class functional layer full speed mode example source file
 *
 *  This example tests the operation of VC5505 usb in full speed mode.
 *  NOTE: For Testing Audio class module a macro CSL_AC_TEST needs to be defined
 *  This includes some code in csl_usbAux.h file which is essential for Audio class
 *  operation and not required for MUSB stand alone testing.
 *  define this macro in pre defined symbols in project biuld options
 *  (Defined in the current usb audio class example pjt).
 *  Semaphores and mail boxes are used in the Audio class example code as the USB operation
 *  is not possible with out OS calls. DSP BIOS version 5.32.03 is used for this purpose.
 *  Definig Start transfer and complete transfer call back functions is must
 *  and Audio class module does not work if they are not implemeted properly. A call back
 *  is sent to this functions from MUSB module.
 *
 *  NOTE: Message boxes and semaphores are reused from MSC module.
 *  Name MSC is not replaced with Auidio class at some places for quick reusability
 *
 * NOTE: THIS TEST HAS BEEN DEVELOPED TO WORK WITH CHIP VERSIONS C5505 AND
 * C5515. MAKE SURE THAT PROPER CHIP VERSION MACRO CHIP_5505/CHIP_5515 IS
 * DEFINED IN THE FILE c55xx_csl\inc\csl_general.h.
 *
 *  Path: \(CSLPATH)\example\usb\example5
 */

/* ============================================================================
 * Revision History
 * ================
 * 20-Dec-2008 Created
 * ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>



#include "csl_spi.h"



#include <csl_mmcsd.h>
#include "csl_sysctrl.h"
#include <csl_rtc.h>

#include "csl_spi.h"
#include "csl_general.h"
#include "spi_flash.h"

#include "rtc.h"
#include "csl_types.h"
#include "csl_error.h"
#include "csl_intc.h"
#include "csl_gpio.h"
#include "csl_usb.h"
#include "csl_audioClass.h"

#include "soc.h"
//#include "psp_i2s.h"
#include "dda_dma.h"
#include "i2s_sample.h"
#include "gpio_control.h"
#include "pll_control.h"
#include "app_globals.h"
#include "app_usb.h"
#include "app_usbac.h"
#include "app_usbac_descs.h"
#include "codec_aic3254.h"
#include "user_interface.h"
#include "app_asrc.h"
#include "sample_rate.h"
#include "i2c_display.h"

#ifdef C5535_EZDSP_DEMO
#include "lcd_osd.h"
#include "dsplib.h"
#include "soc.h"
#include "cslr.h"
#include "cslr_sysctrl.h"

#include "wdt.h"
#include "main_config.h"
#include "circular_buffer.h"

#include "ff.h"
#include "make_wav.h"

#undef ENABLE_REC_ASRC
#undef ENABLE_ASRC


extern CSL_Status pll_sample_freq(Uint16 freq);

void init_all_peripheral(void);
Int16 program_firmware(void);
void C5515_reset(void);

FATFS fatfs;			/* File system object */
FIL file_config;
FIL rtc_time_file;
#endif



#include "VirtualSense_ACIcfg.h"

 /* Debug: enable run-time storage of data to SDRAM */
//#define STORE_PARAMETERS_TO_SDRAM

/**
 *  \brief  CSL Audio Class main function
 *
 *  \param  None
 *
 *  \return None
 */
void main(void)
{
    CSL_Status status;
    CSL_Status mmcStatus;
    Uint32 gpioIoDir;

    EZDSP5535_init( );

    // turn on led to turn on oscillator
    CSL_CPU_REGS->ST1_55 |= CSL_CPU_ST1_55_XF_MASK;

    mmcStatus = MMC_init();
	if (mmcStatus != CSL_SOK)
	{
		debug_printf("API: MMC_init Failed!\r\n");

	}


	/* Initialize GPIO module */

	/* GPIO02 and GPIO04 for debug */
	/* GPIO10 for AIC3204 reset */
	gpioIoDir = ((((Uint32)CSL_GPIO_DIR_OUTPUT)<<CSL_GPIO_PIN16)| // 16 is SD1_ENABLE
		        (((Uint32)CSL_GPIO_DIR_OUTPUT)<<CSL_GPIO_PIN17)); // 17 is OSCILLATOR ENABLE

	gpioInit(gpioIoDir, 0x00000000, 0x00000000);



    /* Initialize DSP PLL */
    status = pll_sample_freq(100);
    if (status != CSL_SOK)
    {
        exit(EXIT_FAILURE);
    }

       SYS_setEBSR(CSL_EBSR_FIELD_SP0MODE,
                                       CSL_EBSR_SP0MODE_0);
        SYS_setEBSR(CSL_EBSR_FIELD_SP1MODE,
                                        CSL_EBSR_SP1MODE_0);
        SYS_setEBSR(CSL_EBSR_FIELD_PPMODE,
        	                         CSL_EBSR_PPMODE_1);

        /* PP Mode 1 (SPI, GPIO[17:12], UART, and I2S2) */

    init_debug(100);

    init_all_peripheral();

}

/**
 *  \brief  Audio Class intialization function
 *
 *  \param  None
 *
 *  \return None
 */
void init_all_peripheral(void)
{
    I2sInitPrms i2sInitPrms;
    PSP_Result result;
    Int16 status;
    FRESULT rc;
    FRESULT rc_fat;
	//UINT bw;

	FIL null_file;
	Uint16 current_pc = 0;


	//Initialize RTC
    //initRTC();


    debug_printf("Starting device configuration\r\n");
    debug_printf("enable gpios\r\n");

    debug_printf("enable gpios\r\n");
    debug_printf("enable gpios\r\n");
    debug_printf("Starting device configuration\r\n");
    debug_printf("enable gpios\r\n");

    debug_printf("enable gpios\r\n");
    debug_printf("enable gpios\r\n");

    debug_printf("enable gpios\r\n");
      debug_printf("enable gpios\r\n");


      debug_printf("Starting device configuration\r\n");
        debug_printf("enable gpios\r\n");

        debug_printf("enable gpios\r\n");
        debug_printf("enable gpios\r\n");
        debug_printf("Starting device configuration\r\n");
        debug_printf("enable gpios\r\n");

        debug_printf("enable gpios\r\n");
        debug_printf("enable gpios\r\n");

        debug_printf("enable gpios\r\n");
          debug_printf("enable gpios\r\n");
    dbgGpio1Write(1); // ENABLE SD_1
    dbgGpio2Write(1); // ENABLE OSCILLATOR
    //mount sdcard: must be High capacity(>4GB), standard capacity have a problem
    rc = f_mount(0, &fatfs);
    if(rc){

    	debug_printf("Error mounting volume\r\n");

    }
    else{

    	debug_printf("Mounting volume\r\n");

    }

    rc_fat = f_open(&null_file, "null.void", FA_READ);

	debug_printf(" try to open null.void\r\n");

	if(rc_fat){
		debug_printf("null.void doesn't exist\r\n");
	}


	 rc_fat = f_open(&null_file, "null2.void", FA_READ);

	debug_printf(" try to open null2.void\r\n");
	if(rc_fat){
		debug_printf("null2.void doesn't exist\r\n");

	}

	start_log();

	debug_printf("\r\n");
	debug_printf("Firmware version:");
	debug_printf(FW_VER);
	debug_printf("\r\n");
	debug_printf("\r\n");
	debug_printf("VirtualSenseDSP flash writer:\r\n");
	debug_printf(" Copyright Emanuele Lattanzi 2014\r\n");
	debug_printf(" Department of Basic Sciences and Foundations \r\n");
	debug_printf(" University of Urbino - Urbino Italy \r\n");
	debug_printf(" Contact: emanuele.lattanzi@uniurb.it \r\n");
	debug_printf("\r\n");

	debug_printf("Start configuration\r\n");

    // init lcd
	LCD_Write("Firmware update... please wait\n");


	if(program_firmware()){
		debug_printf(" ************ Error programming firmware ***************");
		LCD_Write("Firmware update... ERROR\n");
	}else {
		debug_printf(" ************ Firmware successfully programmed -- please reboot ***************");
	}
	LCD_Write("Firmware update... successfully\n");
	while(1);

        debug_printf("Initialization completed\r\n");
    debug_printf("\r\n");

}

Int16 program_firmware(void)
{

	/** Buffer to hold contents used for writing by SPI_Write() */
	Uint16 spiWriteBuff[32];
	/** Buffer to hold contents Read using SPI_Read() */
	Uint16 spiReadBuff[32];
	Uint16 i = 0;
	FRESULT fatRes;
	Uint16 field = 0;
	UINT bw;
	Uint32 index;
	Uint32 address = 0x00000000;


	for(i = 0; i < 32; i++){
		spiWriteBuff[i] = 10+i;
	}

	init_spi_flash();
	erase_spi_flash();


	fatRes = f_open(&file_config, FILE_FIRMWARE, FA_READ);
	if(!fatRes) {
		for(index = 0; index < FIRMWARE_SIZE; index++){
			fatRes = f_read(&file_config,  &field, 1, &bw);
			if(index < 32)
				debug_printf("-- 0x%x\r\n",field);
			spi_flash_write_byte(field, address);
			address++;
		}
		//debug_printf(" PC is %d \r\n", field);
	}

	//spi_flash_write(&spiWriteBuff[0], 0x0000, 32);
	spi_flash_read(0x0000, &spiReadBuff[0], 32);

	for(i = 0; i < 32; i++){
			debug_printf("0x%x\r\n",spiReadBuff[i]);
	}
	return fatRes;
}
