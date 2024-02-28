cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(tools)

add_executable(fool)
rp6502_executable(fool)
target_sources(fool PRIVATE
    src/fool.c
    src/ezpsg.c
)

