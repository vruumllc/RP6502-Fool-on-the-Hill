/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ezpsg.h"
#include <rp6502.h>
#include <stdlib.h>
#include <stdio.h>

#define SPRITE_SIZE 128
#define LOG_SPRITE_SIZE 7 // 2^7 = 128

#define SPRITE_CONFIG 0xF000 // sprite config address
#define ALTAIR_DATA 0x0000 // (actually 0x10000)

#define PSG_CONFIG 0xFF00 // programmable sound generator config address

struct
{
    int x;
    int y;
} sprite;
struct
{
    int xv;
    int yv;
} vector;

#define wait(duration) (duration)
#define piano(note, duration) (-1), (note), (duration)
#define end() (0)

#define bar_0                                                           wait(8)
#define bar_1   piano(b4, 1), piano(g3, 1), piano(g2, 1),               wait(1),\
                piano(b4, 2), piano(g3, 2),                             wait(2),\
                piano(b4, 1), piano(g3, 1), piano(g2, 1),               wait(1),\
                piano(b4, 2), piano(g3, 2), piano(g2, 2),               wait(2),\
                piano(g3, 2), piano(d3, 2), piano(g2, 2),               wait(2)
#define bar_2   piano(b4, 2), piano(g3, 2), piano(g2, 2),               wait(2),\
                piano(g3, 2), piano(d3, 2), piano(g2, 2),               wait(2),\
                piano(b4, 2), piano(g3, 2), piano(g2, 2),               wait(2),\
                piano(b4, 2), piano(g3, 2), piano(d3, 2),               wait(2)
#define bar_3   piano(c4, 2), piano(g3, 2), piano(d3, 2), piano(g2, 2), wait(2),\
                piano(d4, 1),                                           wait(1),\
                piano(e4, 1), piano(g2, 1),                             wait(1),\
                piano(e4, 2), piano(c4, 2), piano(g3, 2), piano(d3, 2), wait(2),\
                piano(b3, 2),                                           wait(2)
#define bar_4   piano(e4, 2), piano(c4, 2), piano(g3, 2), piano(c3, 2), wait(2),\
                piano(b3, 2),                                           wait(2),\
                piano(c3, 2),                                           wait(2),\
                piano(e4, 2), piano(c3, 2), piano(g3, 2), piano(a2, 2), wait(2)
#define bar_5   piano(d4, 2), piano(b4, 2), piano(g2, 2),               wait(2),\
                piano(e4, 1),                                           wait(1),\
                piano(fs4,1), piano(g2, 1),                             wait(1),\
                piano(g4, 2), piano(b4, 2), piano(g3, 2), piano(d3, 2), wait(2),\
                piano(g4, 2), piano(d3, 2),                             wait(2)
#define bar_6   piano(g4, 2), piano(b4, 2), piano(g3, 2), piano(g2, 2), wait(2),\
                piano(g4, 2), piano(g2, 2),                             wait(2),\
                piano(fs4,2), piano(b4, 2), piano(g3, 2), piano(d3, 2), wait(2),\
                piano(d4, 2), piano(b4, 2),                             wait(2)
#define bar_7   piano(e4, 2), piano(c4, 2), piano(c3, 2),               wait(2),\
                piano(e4, 1), piano(c4, 1), piano(a4, 1),               wait(1),\
                piano(e4, 1), piano(c4, 1), piano(a4, 1), piano(c3, 1), wait(1),\
                piano(e4, 2), piano(c4, 2), piano(a4, 2),               wait(2),\
                piano(b4, 2),                                           wait(2)
#define bar_8   piano(c4, 2), piano(e3, 2),                             wait(2),\
                piano(a4, 2),                                           wait(2),\
                piano(e3, 2),                                           wait(2),\
                piano(d4, 2), piano(d3, 2),                             wait(2)
#define bar_9   piano(e4, 2), piano(c4, 2), piano(c3, 2),               wait(2),\
                piano(a4, 1),                                           wait(1),\
                piano(a4, 1), piano(c3, 1),                             wait(1),\
                piano(e4, 2), piano(c4, 2), piano(a4, 2), piano(e2, 2), wait(2),\
                piano(g4, 2), piano(e2, 2),                             wait(2)
#define bar_10  piano(fs4,2), piano(c4, 2), piano(b4, 2), piano(d3, 2), wait(2),\
                piano(b4, 2), piano(d3, 2),                             wait(2),\
                piano(a4, 2), piano(d3, 2),                             wait(2),\
                piano(e4, 1),                                           wait(1),\
                piano(d4, 1), piano(d3, 1),                             wait(1)
#define bar_11  piano(e4, 2), piano(b4, 2), piano(g3, 2), piano(g2, 2), wait(2),\
                piano(g4, 1),                                           wait(1),\
                piano(g4, 1), piano(g2, 1),                             wait(1),\
                piano(g4, 2), piano(b4, 2), piano(g3, 2), piano(d3, 2), wait(2),\
                piano(e4, 2), piano(g2, 2),                             wait(2)
#define bar_12  piano(g4, 2), piano(b3, 2), piano(g3, 2), piano(e2, 2), wait(2),\
                piano(e4, 2), piano(e2, 2),                             wait(2),\
                piano(b3, 2),                                           wait(2),\
                piano(d4, 2), piano(g3, 2), piano(b3, 2),               wait(2)
#define bar_13  piano(e4, 1), piano(c4, 1), piano(a3, 1),               wait(1),\
                piano(a4, 2),                                           wait(2),\
                piano(g4, 1), piano(a3, 1),                             wait(1),\
                piano(e4, 2), piano(c4, 2), piano(a4, 2), piano(a3, 2), wait(2),\
                piano(g4, 2), piano(a3, 2),                             wait(2)
#define bar_14  piano(e4, 2), piano(c4, 2), piano(a4, 2), piano(d3, 2), wait(2),\
                piano(g4, 2), piano(d3, 2),                             wait(2),\
                piano(g4, 2), piano(c4, 2), piano(a4, 2), piano(d3, 2), wait(2),\
                piano(e4, 2), piano(c4, 2), piano(g3, 2),               wait(2)
#define bar_15  piano(g4, 2), piano(as4,2), piano(g2, 2),               wait(2),\
                piano(d4, 2), piano(g2, 2),                             wait(2),\
                piano(ds4,2), piano(as4,2), piano(g3, 2),               wait(2),\
                piano(c4, 1),                                           wait(1),\
                piano(d4, 1), piano(as4,1),                             wait(1)
#define bar_16  piano(d4, 2), piano(g2, 2),                             wait(2),\
                piano(d4, 2), piano(d3, 2), piano(g2, 2),               wait(2),\
                piano(c4, 2),                                           wait(2),\
                piano(d4, 1), piano(g2, 1),                             wait(1),\
                piano(ds4,1), piano(as4,1),                             wait(1)
#define bar_17  piano(ds4,2), piano(c3, 2),                             wait(2),\
                piano(ds4,2), piano(g3, 2),                             wait(2),\
                piano(d4, 2),                                           wait(2),\
                piano(c4, 1), piano(c3, 1),                             wait(1),\
                piano(c4, 1), piano(g3, 1),                             wait(1)
#define bar_18  piano(c4, 2), piano(c3, 2),                             wait(2),\
                piano(c4, 2), piano(g3, 2),                             wait(2),\
                piano(d4, 1),                                           wait(1),\
                piano(ds4,2), piano(c3, 2),                             wait(2),\
                piano(f4, 1), piano(c4, 1), piano(a4, 1),               wait(1)
#define bar_19  piano(f2, 2),                                           wait(2),\
                piano(f4, 2), piano(ds3,2),                             wait(2),\
                piano(ds4,1), piano(a4, 1),                             wait(1),\
                piano(d4, 2), piano(f2, 2),                             wait(2),\
                piano(ds4,1), piano(a4, 1),                             wait(1)
#define bar_20  piano(ds4,2), piano(a4, 2), piano(f2, 2),               wait(2),\
                piano(ds4,2), piano(ds3,2),                             wait(2),\
                piano(f4, 1), piano(a4, 1), piano(f3, 1),               wait(1),\
                piano(g4, 2), piano(a4, 2), piano(f2, 2),               wait(2),\
                piano(d4, 1), piano(as4,1),                             wait(1)
#define bar_21  piano(d4, 2), piano(a4, 2), piano(g2, 2),               wait(2),\
                piano(d4, 2), piano(a4, 2), piano(g3, 2),               wait(2),\
                piano(as4,1),                                           wait(1),\
                piano(a4, 2), piano(g2, 2),                             wait(2),\
                piano(g4, 1), piano(d4, 1), piano(as4,1),               wait(1)
#define bar_22  piano(g4, 2), piano(d4, 2), piano(as4,2), piano(g2, 2), wait(2),\
                piano(d4, 2), piano(as4,2),                             wait(2),\
                piano(g4, 2), piano(d4, 2),                             wait(2),\
                piano(d4, 2), piano(as4,2), piano(g2, 2),               wait(2)
#define bar_23  piano(g4, 2), piano(b4, 2), piano(g2, 2),               wait(2),\
                piano(g4, 2), piano(d4, 2),                             wait(2),\
                piano(g4, 2),                                           wait(2),\
                piano(d4, 2), piano(g2, 2),                             wait(2)
#define bar_24  piano(g4, 2), piano(b4, 2), piano(g2, 2),               wait(2),\
                piano(g4, 2), piano(d4, 2),                             wait(2),\
                piano(g4, 2),                                           wait(2),\
                piano(d4, 2), piano(g2, 2),                             wait(2)

static const uint8_t song[] = {
    bar_1,
    bar_2,
    bar_3,
    bar_4,
    bar_5,
    bar_6,
    bar_7,
    bar_8,
    bar_9,
    bar_10,
    bar_11,
    bar_12,
    bar_13,
    bar_14,
    bar_15,
    bar_16,
    bar_17,
    bar_18,
    bar_19,
    bar_20,
    bar_21,
    bar_22,
    bar_23,
    bar_24,
    end()};


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void init_asprite()
{
    unsigned x = ((uint32_t)rand() * (320 - SPRITE_SIZE)) >> 15;
    unsigned y = ((uint32_t)rand() * (240 - SPRITE_SIZE)) >> 15;
    sprite.x = x;
    sprite.y = y;
    if (((uint32_t)rand() * 2) >> 15)
        vector.xv = 1;
    else
        vector.xv = -1;
    if (((uint32_t)rand() * 2) >> 15)
        vector.yv = 1;
    else
        vector.yv = -1;

    xram0_struct_set(SPRITE_CONFIG, vga_mode4_asprite_t, x_pos_px, x);
    xram0_struct_set(SPRITE_CONFIG, vga_mode4_asprite_t, y_pos_px, y);
    xram0_struct_set(SPRITE_CONFIG, vga_mode4_asprite_t, xram_sprite_ptr, ALTAIR_DATA);
    xram0_struct_set(SPRITE_CONFIG, vga_mode4_asprite_t, log_size, LOG_SPRITE_SIZE);
    xram0_struct_set(SPRITE_CONFIG, vga_mode4_asprite_t, has_opacity_metadata, true);

    // plane=1, normal sprite mode
    xreg_vga_mode(4, 0, SPRITE_CONFIG, 1, 1);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void ezpsg_instruments(const uint8_t **data)
{
    uint8_t note, vol_decay;
    switch ((int8_t) * (*data)++) // instrument
    {
    case -1: // piano
        // higher notes decay faster
        note = *(*data)++;
        vol_decay = 0xF9;
        if (note < c3)
            vol_decay = 0xFA;
        if (note > c6)
            vol_decay = 0xF8;
        ezpsg_play_note(note,       // note
                        *(*data)++, // duration
                        1,          // release
                        200,        // duty
                        0x11,       // vol_attack
                        vol_decay,  // vol_decay
                        0x34,       // wave_release
                        0);         // pan
        break;
    default:
        // The instrumment you just added probably isn't
        // consuming the correct number of paramaters.
        puts("Unknown instrument.");
        exit(1);
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void main(void)
{
    uint8_t v = RIA.vsync;
    int16_t x, y;

    // Erase console
    printf("\f");

    puts("A tribute to the first great microcomputer hack!");
    puts("");
    puts("On April 16, 1975, Homebrew Computer Club member Steve Dompier used an Altair");
    puts("to play the Beatle's 'Fool in the Hill' via EM interference on an AM radio.");
    puts("");
    puts("I wanted my first song for the RP6502 to be 'Fool on the Hill' as well. Enjoy!");
    puts("");

    // Initial sprite positions
    init_asprite();

    // Program VGA
    xreg_vga_canvas(1);
    xreg_vga_mode(4, 1, SPRITE_CONFIG, 1);

    ezpsg_init(PSG_CONFIG);
    while(true) {
        ezpsg_play_song(song);
        while (true) {
            if (RIA.vsync == v) {
                continue;
            } else {
                v = RIA.vsync;
            }

            // Copy positions during vblank
            RIA.step0 = sizeof(vga_mode4_sprite_t);
            RIA.step1 = sizeof(vga_mode4_sprite_t);

            RIA.addr0 = SPRITE_CONFIG + 12;
            RIA.addr1 = SPRITE_CONFIG + 13;
            RIA.rw0 = sprite.x & 0xff;
            RIA.rw1 = sprite.x >> 8;

            RIA.addr0 = SPRITE_CONFIG + 14;
            RIA.addr1 = SPRITE_CONFIG + 15;
            RIA.rw0 = sprite.y & 0xff;
            RIA.rw1 = (sprite.y  >> 8) & 0xff;

            // Update positions
            x = sprite.x = sprite.x + vector.xv;
            y = sprite.y = sprite.y + vector.yv;
            if (x < 0 || x > 320 - SPRITE_SIZE + 3) {
                vector.xv *= -1;
            }
            if (y < -12 || y > 240 - SPRITE_SIZE + 10) {
                vector.yv *= -1;
            }

            ezpsg_tick(8);
            if (!ezpsg_playing()) {
                break;
            }
        }
    }
}
