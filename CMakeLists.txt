cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(tools)

add_executable(fool)
rp6502_asset(fool 0x10000 images/altair.bin)
rp6502_executable(fool altair.bin.rp6502)
target_sources(fool PRIVATE
    src/fool.c
    src/ezpsg.c
    src/colors.c
)

