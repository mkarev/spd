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

#ifdef __cplusplus
extern "C" {
#endif

bool io_file_write(const char *path, uint8_t *data, size_t size);
bool io_file_read(const char *path, uint8_t *data, size_t size);

#if _WIN32
bool io_i2c_init(void);
size_t io_i2c_read(uint32_t id, uint8_t *data, size_t size);
size_t io_i2c_write(uint32_t id, const uint8_t *data, size_t size);
#else
// TODO
static inline bool io_i2c_init(void) { return fasle; }
static inline size_t io_i2c_read(uint32_t, uint8_t *, size_t) { return 0; }
static inline size_t io_i2c_write(uint32_t, const uint8_t *, size_t) { return 0; }
#endif

#ifdef __cplusplus
}
#endif
