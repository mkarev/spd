# The MIT License (MIT)
# 
# Copyright (c) 2019 Mikhail Karev
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

cmake_minimum_required(VERSION 3.2)

project("io")

add_library(io STATIC
    "include/io/io.h"
    "io.c"
)
if (WIN32)
	target_sources(io PRIVATE "win32/ch341.c")
	target_link_libraries(io PUBLIC kernel32)
endif (WIN32)
set_target_properties(io
	PROPERTIES
		C_STANDARD 99
)
target_include_directories(io
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
)
target_compile_definitions(io
	PRIVATE
		_CRT_SECURE_NO_WARNINGS
)
