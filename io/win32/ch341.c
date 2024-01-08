/* The MIT License (MIT)
 *
 * Copyright (c) 2024 Mikhail Karev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// Light-weight CH341 I2C EEPROM programmer based on https://www.onetransistor.eu/2017/09/ch341a-usb-i2c-programming.html

#ifdef WIN32

#include <io/io.h>

//#define _WIN32_WINNT 0x0600
#include <windows.h>

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/* Get the DLL version number, return the version number */
static ULONG(WINAPI *CH341GetVersion)();

/* Get the driver version number, return the version number, or return 0 if there is an error */
static ULONG(WINAPI *CH341GetDrvVersion)();

/* Open the CH341 device, return the handle, if an error occurs, it will be invalid */
/* iIndex - Specify the device serial number of CH341, 0 corresponds to the first device */
static HANDLE(WINAPI *CH341OpenDevice)(ULONG iIndex);

/* Close the CH341 device */
/* iIndex - Specify the serial number of the CH341 device */
static VOID(WINAPI *CH341CloseDevice)(ULONG iIndex);

/* Reset USB device */
/* iIndex - Specify the serial number of the CH341 device */
static BOOL(WINAPI *CH341ResetDevice)(ULONG iIndex);

/* Set the serial port flow mode */
/* iMode - Specify the CH341 device number */
/* iMode - To specify the mode, see Downlink */
/*         Bit 1-bit 0: I2C interface speed /SCL frequency, 00= low speed /20KHz,01= standard /100KHz(default),10= fast /400KHz,11= high speed /750KHz */
/*         Bit 2: SPI I/O number /IO pins, 0= single in/single out (D3 clock /D5 out /D7 in)(default),1= double in/double out (D3 clock /D5 out D4 out /D7 in D6 in) */
/*         Bit 7: Bit order in SPI bytes, 0= low first, 1= high first */
/*         All other reservations must be 0 */
static BOOL(WINAPI *CH341SetStream)(ULONG iIndex, ULONG iMode);

/* Read one byte of data from the I2C interface */
/* iIndex - Specify the serial number of the CH341 device */
/* iDevice - The lower 7 bits specify the I2C device address */
/* iAddr - Address of specified data unit */
/* oByte - Address of specified data unit */
static BOOL(WINAPI *CH341ReadI2C)(ULONG iIndex, UCHAR iDevice, UCHAR iAddr, PUCHAR oByte);

/* Write a byte of data to the I2C interface */
/* iIndex - Specify the serial number of the CH341 device */
/* iDevice - The lower 7 bits specify the I2C device address */
/* iAddr - Address of specified data unit */
/* iByte - Byte data to be written */
static BOOL(WINAPI *CH341WriteI2C)(ULONG iIndex, UCHAR iDevice, UCHAR iAddr, UCHAR iByte);

/* Process I2C data stream, 2-wire interface, clock line for SCL pin, data line for SDA pin (quasi-bidirectional I/O), speed of about 56K bytes */
/* iIndex - Specify the CH341 device number */
/* iWriteLength - Number of bytes of data to write out */
/* iWriteBuffer - Points to a buffer to place data to be written, usually with the I2C device address and read/write direction bits as the first byte */
/* iReadLength - Number of bytes of data to be read */
/* oReadBuffer - Points to a buffer and returns the data read in */
static BOOL(WINAPI *CH341StreamI2C)(ULONG iIndex, ULONG iWriteLength, PVOID iWriteBuffer, ULONG iReadLength, PVOID oReadBuffer);

// EEPROM type
typedef enum _EEPROM_TYPE
{
    ID_24C01,
    ID_24C02,
    ID_24C04,
    ID_24C08,
    ID_24C16,
    ID_24C32,
    ID_24C64,
    ID_24C128,
    ID_24C256,
    ID_24C512,
    ID_24C1024,
    ID_24C2048,
    ID_24C4096
} EEPROM_TYPE;

// Reads data blocks from EEPROM at a speed of about 56 KB
// iIndex - Specify the CH341 device number
// iEepromID - Specifies the EEPROM model
// iAddr - Specifies the address of the data unit
// iLength - Number of bytes of data to be read
// oBuffer - Points to a buffer and returns the data read in
static BOOL (WINAPI *CH341ReadEEPROM)(ULONG iIndex, EEPROM_TYPE iEepromID, ULONG iAddr, ULONG iLength, PUCHAR oBuffer);

// Writes a data block to the EEPROM
// iIndex - Specify the CH341 device number
// iEepromID - Specifies the EEPROM model
// iAddr - Specifies the address of the data unit
// iLength - Number of bytes of data to write out
// iBuffer - Points to a buffer to place data ready to be written out
static BOOL (WINAPI *CH341WriteEEPROM)(ULONG iIndex, EEPROM_TYPE iEepromID, ULONG iAddr, ULONG iLength, PUCHAR iBuffer);

static INIT_ONCE g_once = INIT_ONCE_STATIC_INIT;

static BOOL CALLBACK InitCH341(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext)
{
    HMODULE lib = LoadLibrary("CH341DLLA64.DLL");
    if (!lib) {
        DWORD err = GetLastError();
        printf("LoadLibraryA failed: %u\n", err);
        return FALSE;
    }
    
#define DLSYM(symbol, type) \
do { \
    if (!(symbol = (type)GetProcAddress(lib, # symbol))) \
        goto fail; \
} while (0)

    DLSYM(CH341GetVersion, ULONG(WINAPI *)());
    DLSYM(CH341GetDrvVersion, ULONG(WINAPI *)());
    DLSYM(CH341OpenDevice, HANDLE(WINAPI *)(ULONG));
    DLSYM(CH341CloseDevice, VOID(WINAPI *)(ULONG));
    DLSYM(CH341ResetDevice, BOOL(WINAPI *)(ULONG));
    DLSYM(CH341SetStream, BOOL(WINAPI *)(ULONG, ULONG));
    DLSYM(CH341ReadI2C, BOOL(WINAPI *)(ULONG, UCHAR, UCHAR, PUCHAR));
    DLSYM(CH341WriteI2C, BOOL(WINAPI *)(ULONG, UCHAR, UCHAR, UCHAR));
    DLSYM(CH341StreamI2C, BOOL(WINAPI *)(ULONG, ULONG, PVOID, ULONG, PVOID));
    DLSYM(CH341ReadEEPROM, BOOL(WINAPI *)(ULONG, EEPROM_TYPE, ULONG, ULONG, PUCHAR));
    DLSYM(CH341WriteEEPROM, BOOL(WINAPI *)(ULONG, EEPROM_TYPE, ULONG, ULONG, PUCHAR));

    return TRUE;

fail:
    FreeLibrary(lib);
    return FALSE;
}

bool io_i2c_init(void)
{
    return !!InitOnceExecuteOnce(&g_once, InitCH341, NULL, NULL);
}

size_t io_i2c_proc(uint32_t id, uint8_t *data, size_t size, BOOL(WINAPI *io_proc)(ULONG, EEPROM_TYPE, ULONG, ULONG, PUCHAR))
{
    if (!io_i2c_init())
        return 0;

    ULONG iIndex = (ULONG)id;
    size_t bytes_processed = 0;

    if (!CH341OpenDevice(iIndex))
        return bytes_processed;

    if (!CH341ResetDevice(iIndex))
        goto done;

    if (!io_proc(iIndex, ID_24C02, 0, (ULONG)size, (PUCHAR)data))
        goto done;

    bytes_processed = size;

done:
    CH341CloseDevice(iIndex);
    return bytes_processed;
}

size_t io_i2c_read(uint32_t id, uint8_t *data, size_t size)
{
    return io_i2c_proc(id, data, size, CH341ReadEEPROM);
}

size_t io_i2c_write(uint32_t id, const uint8_t *data, size_t size)
{
    return io_i2c_proc(id, (uint8_t *)data, size, CH341WriteEEPROM);
}

#endif
