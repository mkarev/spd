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

#include <io/io.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <sys/stat.h>

static bool is_file_exists(const char *path)
{
#if _WIN32
    struct _stat64 st;
    return 0 == _stat64(path, &st);
#else
    strust stat st;
    return 0 == stat(path, &st);
#endif
}

static char get_answer()
{
    char ans, c;
    (void)scanf("%c", &ans);

    // https://c-faq.com/stdio/stdinflush2.html
    while (ans != '\n' && (c = getchar()) != '\n' && c != EOF)
        /* discard */;

    return ans;
}

bool io_file_write(const char *path, uint8_t *data, size_t size)
{
    if (is_file_exists(path)) {
        printf("The file already exists: %s\nDo you want to overwrite it? (y/N): ", path);
        char ans = get_answer();
        if (ans != 'y' && ans != 'Y')
            return true;
    }
    FILE *f = fopen(path, "wb");
    if (!f) {
        printf("Can't open file: %s\n", path);
        return false;
    }
    if (size != fwrite(data, 1, size, f)) {
        printf("Can't write file: %s\n", path);
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

bool io_file_read(const char *path, uint8_t *data, size_t size)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("Can't open input file: %s\n", path);
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
