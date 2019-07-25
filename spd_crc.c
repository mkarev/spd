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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static void print_usage()
{
    printf(
        "Usage:\n"
        "  spd-scr eeprom_dump.bin\n"
    );
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        print_usage();
        return EXIT_FAILURE;
    }

    FILE* dump = fopen(argv[1], "rb");
    if (!dump) {
        printf("Can't open dump: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    uint8_t data[SPD_SIZE_MAX];
    if (sizeof(data) == fread(data, 1, sizeof(data), dump)) {
        SpdInfo i;
        if (spd_decode(&i, data)) {
            spd_print(&i, false);
        }
    }
    fclose(dump);

    return EXIT_SUCCESS;
}
