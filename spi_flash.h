/* ============================================================================
 * Copyright (c) 2008-2012 Texas Instruments Incorporated.
 * Except for those rights granted to you in your license from TI, all rights
 * reserved.
 *
 * Software License Agreement
 * Texas Instruments (TI) is supplying this software for use solely and
 * exclusively on TI devices. The software is owned by TI and/or its suppliers,
 * and is protected under applicable patent and copyright laws.  You may not
 * combine this software with any open-source software if such combination would
 * cause this software to become subject to any of the license terms applicable
 * to such open source software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
 * NO WARRANTIES APPLY TO THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY.
 * EXAMPLES OF EXCLUDED WARRANTIES ARE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE AND WARRANTIES OF NON-INFRINGEMENT,
 * BUT ALL OTHER WARRANTY EXCLUSIONS ALSO APPLY. FURTHERMORE, TI SHALL NOT,
 * UNDER ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, CONSEQUENTIAL
 * OR PUNITIVE DAMAGES, FOR ANY REASON WHATSOEVER.
 * ============================================================================
 */

#include "csl_general.h"
#include "csl_spi.h"
#include <stdio.h>

/* SPI Commands */
#define SPI_CMD_WRSR            (0x01)
#define SPI_CMD_EWRSR           (0x50)
#define SPI_CMD_WRITE           (0x02)
#define SPI_CMD_WRITE_AAI       (0xAD)
#define SPI_CMD_READ            (0x03)
#define SPI_CMD_WRDI            (0x04)
#define SPI_CMD_RDSR            (0x05)
#define SPI_CMD_WREN            (0x06)
#define SPI_CMD_ERASE           (0x20)


#define READ                            0x03    // SPI Flash opcode: Read up up to 25MHz
#define READ_FAST                       0x0B    // SPI Flash opcode: Read up to 50MHz with 1 dummy byte
#define ERASE_4K                        0x20    // SPI Flash opcode: 4KByte sector erase
#define ERASE_32K                       0x52    // SPI Flash opcode: 32KByte block erase
#define ERASE_SECTOR            0xD8    // SPI Flash opcode: 64KByte block erase
#define ERASE_ALL                       0x60    // SPI Flash opcode: Entire chip erase
#define WRITE                           0x02    // SPI Flash opcode: Write one byte (or a page of up to 256 bytes, depending on device)
#define WRITE_WORD_STREAM       0xAD    // SPI Flash opcode: Write continuous stream of 16-bit words (AAI mode); available on SST25VF016B (but not on SST25VF010A)
#define WRITE_BYTE_STREAM       0xAF    // SPI Flash opcode: Write continuous stream of bytes (AAI mode); available on SST25VF010A (but not on SST25VF016B)
#define RDSR                            0x05    // SPI Flash opcode: Read Status Register
#define EWSR                            0x50    // SPI Flash opcode: Enable Write Status Register
#define WRSR                            0x01    // SPI Flash opcode: Write Status Register
#define WREN                            0x06    // SPI Flash opcode: Write Enable
#define WRDI                            0x04    // SPI Flash opcode: Write Disable / End AAI mode
#define RDID                            0x90    // SPI Flash opcode: Read ID
#define JEDEC_ID                        0x9F    // SPI Flash opcode: Read JEDEC ID
#define EBSY                            0x70    // SPI Flash opcode: Enable write BUSY status on SO pin
#define DBSY                            0x80    // SPI Flash opcode: Disable write BUSY status on SO pin

#define BUSY    0x01    // Mask for Status Register BUSY (Internal Write Operaiton in Progress status) bit
#define WEL     0x02    // Mask for Status Register WEL (Write Enable status) bit
#define BP0     0x04    // Mask for Status Register BP0 (Block Protect 0) bit
#define BP1     0x08    // Mask for Status Register BP1 (Block Protect 1) bit
#define BP2     0x10    // Mask for Status Register BP2 (Block Protect 2) bit
#define BP3     0x20    // Mask for Status Register BP3 (Block Protect 3) bit
#define AAI     0x40    // Mask for Status Register AAI (Auto Address Increment Programming status) bit
#define BPL     0x80    // Mask for Status Register BPL (BPx block protect bit read-only protect) bit



void init_spi_flash(void);
void erase_spi_flash(void);
void _WaitWhileBusy(void);

void spi_flash_read (Uint32 address,
                     Uint16 *        dst,
                      Uint16        length);

void spi_flash_write ( Uint16 *      src,
                        Uint32        address,
                        Uint16        length);

void spi_flash_write_byte ( Uint16 byte,
                        Uint32        address);




