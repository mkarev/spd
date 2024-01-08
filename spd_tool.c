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

#include <spd/spd.h>
#include <io/io.h>

#include <getopt.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum Options {
    OP_DEVICE = 'd',
    OP_INPUT = 'i',
    OP_OUTPUT = 'o',
    OP_VERBOSE = 'v',
    OP_HELP = 'h',

    OP_SET_LP = 'z' + 1,
    OP_RESET_LP,
    OP_FIX_CRC
};

typedef struct Args
{
    bool use_i2c;
    int device_id;
    const char* in_file;
    const char* out_file;
    bool set_lp;
    bool reset_lp;
    bool fix_crc;
    bool verbose;
} Args;

static void print_usage()
{
    printf(
        "DDR3 SPD helper tool\n"
        "Usage:\n"
        "    spd-tool OPTIONS\n\n"
        "OPTIONS:\n"
        "    --device,-d [DEVICE_ID]\n"
        "        I2C device for reading SPD directly from SO-DIMM module.\n"
        "        DEVICE_ID - optional zero-based device id, default 0\n"
        "    --input,-i INPUT_FILE\n"
        "        An input EEPROM binary file if the device is unspecified.\n"
        "        An original EEPROM dump file if the device is specified.\n"
        "    --output,-o OUTPUT_FILE\n"
        "        An output EEPROM binary file if the device is unspecified.\n"
        "        A modified EEPROM dump file if the device is specified.\n"
        "    --set-lp\n"
        "        Set low power mode\n"
        "        Module minimum nominal voltage 1.35 V\n"
        "    --reset-lp\n"
        "        Reset low power mode\n"
        "        Module minimum nominal voltage 1.35 V\n"
        "     --fix-crc\n"
        "        Fix CRC checksum\n"
        "    --verbose,-v\n"
        "        Verbose output\n"
        "    --help,-h\n"
        "        Print this message\n\n"
        "EXAMPLES\n"
        "    Print detailed SPD info\n"
        "        spd-tool -i dump.bin -v\n"
        "    Fix incorrect CRC checksum and save result to the same file\n"
        "        spd-tool -i dump.bin --fix-crc -o dump.bin\n"
        "    Convert DDR3 to LP-DDR3\n"
        "        spd-tool -i dump_1.5v.bin --set-lp -o dump_1.35v.bin\n"
        "    Convert LP-DDR3 to DDR3\n"
        "        spd-tool -i dump_lp-ddr.bin --reset-lp -o dump_ddr.bin\n"
        "    Convert LP-DDR3 to DDR3 via CH341 programmer\n"
        "        spd-tool -d --reset-lp\n"
    );
}

static void parse_args(Args *args, int argc, char* argv[])
{
    if (argc < 2) {
        print_usage(argv);
        exit(EXIT_FAILURE);
    }
    memset(args, 0, sizeof(*args));
    while (true) {
        static struct option options[] = {
            { "device",             optional_argument, 0, OP_DEVICE },
            { "input",              required_argument, 0, OP_INPUT },
            { "output",             required_argument, 0, OP_OUTPUT },
            { "set-lp",             no_argument,       0, OP_SET_LP },
            { "reset-lp",           no_argument,       0, OP_RESET_LP },
            { "fix-crc",            no_argument,       0, OP_FIX_CRC },
            { "verbose",            no_argument,       0, OP_VERBOSE },
            { "help",               no_argument,       0, OP_HELP },
            { 0, 0, 0, 0 }
        };
        int index = 0;
        int c = getopt_long(argc, argv, "di:o:vh", options, &index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case OP_DEVICE:
                args->use_i2c = io_i2c_init();
                if (!args->use_i2c) {
                    printf("I2C driver isn't available\n");
                    exit(EXIT_FAILURE);
                }
                if (optarg)
                    args->device_id = atoi(optarg);
                break;
            case OP_INPUT:
                args->in_file = optarg;
                break;
            case OP_OUTPUT:
                args->out_file = optarg;
                break;
            case OP_SET_LP:
                args->set_lp = true;
                break;
            case OP_RESET_LP:
                args->reset_lp = true;
                break;
            case OP_FIX_CRC:
                args->fix_crc = true;
                break;
            case OP_VERBOSE:
                args->verbose = true;
                break;
            case OP_HELP:
                print_usage(argv);
                exit(EXIT_SUCCESS);
            default:
                printf("Incorrect option --%s: %s\n", options[index].name, optarg);
                exit(EXIT_FAILURE);
        }
    }

    if (!args->in_file && !args->use_i2c) {
        printf("SPD source is undefined\n");
        exit(EXIT_FAILURE);
    }
    if (args->set_lp && args->reset_lp) {
        printf("Options --set-lp and --reset-lp are mutually exclusive\n");
        exit(EXIT_FAILURE);
    }
}

void print_hex(uint8_t *data, size_t size)
{
    const size_t step = 16;
    printf("        ");
    for (size_t n = 0; n < step; n++)
        printf(" %02llx", n);
    printf("\n");
    for (size_t offset = 0; offset < size; offset += step) {
        printf("%08llx", offset);
        for (size_t n = 0; n < step; n++)
            printf(" %02x", data[offset + n]);
        printf("\n");
    }
}

static bool run_tool(const Args *args)
{
    uint8_t spd_data[SPD_SIZE_MAX];
    if (args->use_i2c) {
        if (!io_i2c_read(args->device_id, spd_data, sizeof(spd_data))) {
            printf("Read I2C device-%d failed\n", args->device_id);
            return false;
        }
    }
    if (args->in_file) {
        if (args->use_i2c) {
            if (!io_file_write(args->in_file, spd_data, sizeof(spd_data)))
                return false;
        } else {
            if (!io_file_read(args->in_file, spd_data, sizeof(spd_data)))
                return false;
        }
    }

    SpdInfo i;
    spd_decode(&i, spd_data);

    if (args->verbose) {
        print_hex(spd_data, sizeof(spd_data));
        printf("\n");
    }
    printf("SPD:\n");
    spd_print(&i, args->verbose);
    printf("\n");

    bool is_spd_changed = false;
    if (args->fix_crc) {
        if (spd_fix_crc(spd_data, &i)) {
            is_spd_changed = true;
            printf("CRC was fixed\n");
        }
    }
    if (args->set_lp) {
        if (spd_enable_lp(spd_data, &i, true)) {
            is_spd_changed = true;
            printf("LP-DDR was set\n");
        }
    }
    if (args->reset_lp) {
        if (spd_enable_lp(spd_data, &i, false)) {
            is_spd_changed = true;
            printf("LP-DDR was reseted\n");
        }
    }

    if (is_spd_changed) {
        printf("Modified SPD:\n");
        spd_print(&i, args->verbose);
        printf("\n");
        if (args->verbose)
            print_hex(spd_data, sizeof(spd_data));
        printf("\n");
    }

    if (args->out_file) {
        if (!io_file_write(args->out_file, spd_data, sizeof(spd_data))) {
            printf("Write output file failed\n");
            return false;
        }
    }
    if (args->use_i2c && is_spd_changed) {
        if (!io_i2c_write(args->device_id, spd_data, sizeof(spd_data))) {
            printf("Write I2C device-%d failed\n", args->device_id);
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
    Args args;
    parse_args(&args, argc, argv);
    return run_tool(&args) ? EXIT_SUCCESS : EXIT_FAILURE;
}
