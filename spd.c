/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Mikhail Karev
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"),
 * to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "spd.h"

#include <string.h>
#include <stdio.h>

// JEDEC Standard No. 21-C
// Annex K: Serial Presence Detect (SPD) for DDR3 SDRAM Modules

// 2.4 CRC: Bytes 126 ~ 127
static int crc16(const uint8_t *data, size_t size)
{
    int crc = 0;
    while (size--) {
        crc = crc ^ (int)*data++ << 8;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = crc << 1 ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }
    return (crc & 0xFFFF);
}

static size_t crc_size(const SpdInfo *i)
{
    return i->CRC_Coverage ? 117 : 126;
}

static int sdram_capacity(const SpdInfo *i)
{
    return 256 << i->Total_SDRAM_capacity;
}

static int sdram_width(const SpdInfo *i)
{
    switch (i->SDRAM_Device_Width) {
        case 0: return 4;
        case 1: return 8;
        case 2: return 16;
        case 3: return 32;
        default: return 0;
    }
}

static int primary_bus_width(const SpdInfo *i)
{
    switch (i->Primary_bus_width) {
        case 0: return 8;
        case 1: return 16;
        case 2: return 32;
        case 3: return 64;
        default: return 0;
    }
}

static int ranks(const SpdInfo *i)
{
    switch (i->Number_of_Ranks) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 3;
        case 3: return 4;
        case 4: return 8;
        default: return 0;
    }
}

static int bytes_total(const SpdInfo *i)
{
    switch (i->SPD_Bytes_Total) {
        case 1: return 256;
        default: return 0;
    }
}

static int bytes_used(const SpdInfo *i)
{
    switch (i->SPD_Bytes_Used) {
        case 1: return 128;
        case 2: return 176;
        case 3: return 256;
        default: return 0;
    }
}

static const char* device_name(const SpdInfo *i)
{
    switch (i->DRAM_Device_Type)
    {
        case 1: return "Standard FPM DRAM";
        case 2: return "EDO";
        case 3: return "Pipelined Nibble";
        case 4: return "SDRAM";
        case 5: return "ROM";
        case 6: return "DDR SGRAM";
        case 7: return "DDR SDRAM";
        case 8: return "DDR2 SDRAM";
        case 9: return "DDR2 SDRAM FB-DIMM";
        case 10: return "DDR2 SDRAM FB-DIMM PROBE";
        case 11: return "DDR3 SDRAM";
        default: return "Unknown";
    }
}

static const char* module_type(const SpdInfo *i)
{
    switch (i->Module_Type)
    {
        case 1: return "RDIMM (width = 133.35 mm nom)";
        case 2: return "UDIMM (width = 133.35 mm nom)";
        case 3: return "SO-DIMM (width = 67.6 mm nom)";
        case 4: return "Micro-DIMM (width = TBD mm nom)";
        case 5: return "Mini-RDIMM (width = 82.0 mm nom)";
        case 6: return "Mini-UDIMM (width = 82.0 mm nom)";
        case 7: return "Mini-CDIMM (width = 67.6 mm nom)";
        case 8: return "72b-SO-UDIMM (width = 67.6 mm nom)";
        case 9: return "72b-SO-RDIMM (width = 67.6 mm nom)";
        case 10: return "72b-SO-CDIMM (width = 67.6 mm nom)";
        case 11: return "LRDIMM (width = 133.35 mm nom)";
        case 12: return "16b-SO-DIMM (width = 67.6 mm nom)";
        case 13: return "32b-SO-DIMM (width = 67.6 mm nom)";
        default: return "Unknown";
    }
}

static const char* module_voltage(const SpdInfo *i)
{
    switch (i->Module_Minimum_Nominal_Voltage)
    {
        case 0b000: return "1.5 V operable";
        case 0b010: return "1.35/1.5 V operable";
        case 0b011: return "1.35 V operable";
        case 0b100: return "1.25/1.5 V operable";
        case 0b101: return "1.25 V operable";
        case 0b110: return "1.25/1.35/1.5 V operable";
        case 0b111: return "1.25/1.35 V operable";
        default: return "Unknown";
    }
}

bool spd_decode(SpdInfo *i, const uint8_t byte[SPD_SIZE_MAX])
{
    memset(i, 0, sizeof(i[0]));

    i->CRC_Coverage = byte[0] >> 7;
    i->SPD_Bytes_Total = (byte[0] >> 4) & 0b111;
    i->SPD_Bytes_Used = byte[0] & 0b1111;

    i->SPD_Revision.Encoding_Level = byte[1] >> 4;
    i->SPD_Revision.Encoding_Level = byte[1] & 0b1111;

    i->DRAM_Device_Type = byte[2];
    if (i->DRAM_Device_Type != 11) {
        printf("Unsupported device type: %s (%d)", device_name(i), i->DRAM_Device_Type);
        return false;
    }
    i->Module_Type = byte[3] & 0b1111;

    i->Total_SDRAM_capacity = byte[4] & 0b1111;
    i->Bank_Address_Bits = (byte[4] >> 4) & 0b111;
    i->Column_Address_Bits = byte[5] & 0b111;
    i->Row_Address_Bits = (byte[5] >> 3) & 0b111;

    i->Module_Minimum_Nominal_Voltage = byte[6] & 0b111;

    i->SDRAM_Device_Width = byte[7] & 0b111;
    i->Number_of_Ranks = (byte[7] >> 3) & 0b111;
    i->Primary_bus_width = byte[8] & 0b111;
    i->Bus_width_extension = (byte[8] >> 3) & 0b11;
    i->Module_Capacity = (sdram_capacity(i) * primary_bus_width(i) * ranks(i)) / (8 * sdram_width(i));

    memcpy(i->Module_Part_Number, byte + 128, sizeof(i->Module_Part_Number) - 1);

    i->CRC = byte[126] | (byte[127] << 8);

    i->CRC_real = crc16(byte, crc_size(i));
    if (i->CRC != i->CRC_real) {
        printf("CRC invalid: %d(spd) != %d(real)", i->CRC, i->CRC_real);
        return false;
    }
    return true;
}

void spd_print(const SpdInfo *i, bool verbose)
{
    if (verbose) {
        printf(
            "CRC Coverage:                   0...%zd (%d)\n"
            "Bytes total:                    %d bytes (%d)\n"
            "Bytes used:                     %d bytes (%d)\n"
            "Revision:                       %d.%d\n"
            "DRAM Device Type:               %s (%d)\n"
            "Module Type:                    %s (%d)\n"
            "Total SDRAM capacity:           %d Mbits (%d)\n"
            "Bank Address Bits:              %d bits (%d)\n"
            "Row Address Bits:               %d bits (%d)\n"
            "Column Address Bits:            %d bits (%d)\n"
            "Module Minimum Nominal Voltage: %s (%d)\n"
            "SDRAM Device Width:             %d (%d)\n"
            "Number of Ranks:                %d (%d)\n"
            "Primary bus width:              %d (%d)\n"
            "Bus width extension:            %d (%d)\n"
            "Module Capacity:                %d MBytes\n"
            "Module Part Number:             %s\n"
            "CRC:                            0x%04X %s\n"
            , crc_size(i) - 1, i->CRC_Coverage
            , bytes_total(i), i->SPD_Bytes_Total
            , bytes_used(i), i->SPD_Bytes_Used
            , i->SPD_Revision.Encoding_Level, i->SPD_Revision.Additions_Level
            , device_name(i), i->DRAM_Device_Type
            , module_type(i), i->Module_Type
            , sdram_capacity(i), i->Total_SDRAM_capacity
            , 8 << i->Bank_Address_Bits, i->Bank_Address_Bits
            , 12 + i->Row_Address_Bits, i->Row_Address_Bits
            , 9 + i->Column_Address_Bits, i->Column_Address_Bits
            , module_voltage(i), i->Module_Minimum_Nominal_Voltage
            , sdram_width(i), i->SDRAM_Device_Width
            , ranks(i), i->Number_of_Ranks
            , primary_bus_width(i), i->Primary_bus_width
            , i->Bus_width_extension, i->Bus_width_extension
            , i->Module_Capacity
            , i->Module_Part_Number

            , i->CRC, i->CRC == i->CRC_real ? "OK" : "ERR"
        );
    } else {
        printf(
            "SPD Bytes used:                 %d bytes (%d)\n"
            "DRAM Device Type:               %s (%d)\n"
            "Module Type:                    %s (%d)\n"
            "Module Minimum Nominal Voltage: %s (%d)\n"
            "Module Capacity:                %d MBytes\n"
            "Module Part Number:             %s\n"
            "CRC[0...%zd]:                   0x%04X %s\n"
            , bytes_used(i), i->SPD_Bytes_Used
            , device_name(i), i->DRAM_Device_Type
            , module_type(i), i->Module_Type
            , module_voltage(i), i->Module_Minimum_Nominal_Voltage
            , i->Module_Capacity
            , i->Module_Part_Number
            , crc_size(i) - 1, i->CRC, i->CRC == i->CRC_real ? "OK" : "ERR"
        );
    }
}

static const size_t parse_line(uint8_t data[SPD_SIZE_MAX], const char *line, bool verbose)
{
    const char *eol = strstr(line, "\n");
    size_t len = eol ? eol - line + 1 : strlen(line);
    
    bool is_len_valid = (len >= 71);
    if (!is_len_valid) {
        return len;
    }

    // "b0: ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff    ................\n"

    int address, x[16];
    int tokens = sscanf(line, "%x: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x"
        , &address
        , x + 0, x + 1, x + 2, x + 3
        , x + 4, x + 5, x + 6, x + 7
        , x + 8, x + 9, x +10, x +11
        , x +12, x +13, x +14, x +15
    );
    if (tokens != 17 || address > (SPD_SIZE_MAX - 16)) {
        return len;
    }
    data += address;
    if (verbose) printf("%02x: ", address);
    for (int n = 0; n < 16; n++) {
        data[n] = (uint8_t)x[n];
        if (verbose) printf("%02x ", x[n]);
    }
    if (verbose) printf("\n");
    return len;
}

void spd_read_i2cdump(uint8_t data[SPD_SIZE_MAX], const char *dump)
{
    size_t offset = 0;
    while (dump[offset]) {
        bool verbose = false;
        offset += parse_line(data, dump + offset, verbose);
    }
}