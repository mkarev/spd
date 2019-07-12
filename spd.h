#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

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

bool spd_decode_i2cdump(SpdInfo *i, const char *i2cdump);

void spd_print(const SpdInfo *i, bool verbose);

#ifdef __cplusplus
}
#endif