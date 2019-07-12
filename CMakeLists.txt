cmake_minimum_required(VERSION 3.2)

project("SPD")

add_executable(spd
    "spd.c"
    "spd.h"
    "main.cpp"
)
set_target_properties(spd PROPERTIES
    C_STANDARD 99
    CXX_STANDARD 11
)
target_compile_definitions(spd
	PRIVATE
	-D_CRT_SECURE_NO_WARNINGS
)