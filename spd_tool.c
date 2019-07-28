/* The MIT License (MIT)
 *
 * Copyright (c) 2019 Mikhail Karev
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

#include <spd.h>

#include <getopt.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum Options {
    OP_INPUT = 'i',
    OP_OUTPUT = 'o',
    OP_VERBOSE = 'V',
    OP_HELP = 'h',

    OP_SET_LP = 'z' + 1,
    OP_RESET_LP,
    OP_FIX_CRC
};

typedef struct Args
{
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
        "    --input,-i INPUT_FILE\n"
        "        Input EEPROM binary dump\n"
        "    --output,-o OUTPUT_FILE\n"
        "        Output EEPROM binary dump\n"
        "    --set-lp\n"
        "        Set low power mode\n"
        "        Module minimum nominal voltage 1.35 V\n"
        "     --fix-crc\n"
        "        Fix CRC checksum\n"
        "    --verbose, -v\n"
        "        Verbose output\n"
        "    --help, -h\n"
        "        Print this message\n\n"
        "EXAMPLES\n"
        "    Print detailed SPD info\n"
        "        spd-tool -i dump.bin -V\n"
        "    Fix incorrect CRC checksum and save result to the same file\n"
        "        spd-tool -i dump.bin --fix-crc -o dump.bin\n"
        "    Convert DDR3 to LP-DDR3\n"
        "        spd-tool -i dump_1.5v.bin --set-lp -o dump_1.35v.bin\n"
        "    Convert LP-DDR3 to DDR3\n"
        "        spd-tool -i dump_lp-ddr.bin --reset-lp -o dump_ddr.bin\n"
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
        int c = getopt_long(argc, argv, "i:o:Vh", options, &index);
        if (c == -1) {
            break;
        }
        switch (c) {
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
                exit(EXIT_FAILURE);
        }
    }
    if (!args->in_file) {
        printf("Input file is a required agrument: --input,-i\n");
        exit(EXIT_FAILURE);
    }
    if (args->set_lp && args->reset_lp) {
        printf("Options --set-lp and --reset-lp are mutually exclusive\n");
        exit(EXIT_FAILURE);
    }
}

bool read_input(const Args *args, uint8_t *data, size_t size)
{
    FILE *f = fopen(args->in_file, "rb");
    if (!f) {
        printf("Can't open input file: %s\n", args->in_file);
        return false;
    }
    if (size != fread(data, 1, size, f)) {
        printf("Input file too small\n");
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

bool write_output(const Args *args, uint8_t* data, size_t size)
{
    FILE* f = fopen(args->out_file, "wb");
    if (!f) {
        printf("Can't open output file: %s\n", args->out_file);
        return false;
    }
    if (size != fwrite(data, 1, size, f)) {
        printf("Can't write output file\n");
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

static bool run_tool(const Args *args)
{
    uint8_t spd_data[SPD_SIZE_MAX];
    if (!read_input(args, spd_data, sizeof(spd_data))) {
        printf("Read input file failed\n");
        return false;
    }
    bool data_changed = false;

    SpdInfo i;
    spd_decode(&i, spd_data);
    printf("Input:\n"); spd_print(&i, args->verbose); printf("\n");

    if (args->fix_crc) {
        printf("Fix CRC\n");
        data_changed |= spd_fix_crc(spd_data, &i);
    }
    if (args->set_lp) {
        printf("Set LP-DDR\n");
        data_changed |= spd_enable_lp(spd_data, &i, true);
    } else if (args->reset_lp) {
        printf("Reset LP-DDR\n");
        data_changed |= spd_enable_lp(spd_data, &i, false);
    }

    if (data_changed) {
        printf("\nOutput:\n"); spd_print(&i, args->verbose);
    }

    if (args->out_file) {
        if (!write_output(args, spd_data, sizeof(spd_data))) {
            printf("Write output file failed\n");
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
