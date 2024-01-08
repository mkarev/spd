# The MIT License (MIT)
# 
# Copyright (c) 2024 Mikhail Karev
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

project("spd-tool")

option (SPD_TESTS "SPD enable tests" ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_cmake")

add_subdirectory(spd)
add_subdirectory(io)

add_executable(spd-tool "spd_tool.c")
target_link_libraries(spd-tool PRIVATE spd io)
if (MSVC)
	target_compile_definitions(spd-tool
        PRIVATE
			_CRT_SECURE_NO_WARNINGS
	)
	add_subdirectory(getopt)
	target_link_libraries(spd-tool PRIVATE getopt)
endif (MSVC)

if (SPD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif (SPD_TESTS)
