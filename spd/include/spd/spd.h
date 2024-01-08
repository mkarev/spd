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

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#define SPD_SIZE_MAX 256

typedef struct SpdInfo
{
    bool CRC_Coverage;
    int SPD_Bytes_Total;
    int SPD_Bytes_Used;
    struct {
        int Encoding_Level;
        int Additions_Level;
    } SPD_Revision;
    int DRAM_Device_Type;
    int Module_Type;
    int Total_SDRAM_capacity;
    int Bank_Address_Bits;
    int Row_Address_Bits;
    int Column_Address_Bits;
    int Module_Minimum_Nominal_Voltage;
    int SDRAM_Device_Width;
    int Number_of_Ranks;
    int Primary_bus_width;
    int Bus_width_extension;
    int Module_Capacity;
    char Module_Part_Number[145 - 128 + 1 + 1];

    int CRC;
    int CRC_real;
} SpdInfo;

#ifdef __cplusplus
extern "C" {
#endif

bool spd_decode(SpdInfo *i, const uint8_t data[SPD_SIZE_MAX]);
void spd_print(const SpdInfo* i, bool verbose);

bool spd_fix_crc(uint8_t data[SPD_SIZE_MAX], SpdInfo *i);
bool spd_enable_lp(uint8_t byte[SPD_SIZE_MAX], SpdInfo *i, bool enable);

void spd_parse_i2cdump(uint8_t data[SPD_SIZE_MAX], const char *i2cdump);

#ifdef __cplusplus
}
#endif
