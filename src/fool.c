/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ezpsg.h"
#include <rp6502.h>
#include <stdlib.h>
#include <stdio.h>

#define SPRITE_CONFIG_SIZE sizeof(vga_mode4_sprite_t)
#define SPRITE_CONFIG 0xF000 // sprite config address
#define ALTAIR_DATA 0x0000 // (actually 0x10000)
#define RADIO_DATA 0x8000 // (actually 0x18000)
#define NUM_SPRITES 2

#define PSG_CONFIG 0xFF00 // programmable sound generator config address

struct
{
    int x;
    int y;
    uint16_t config_ptr;
    uint16_t data_ptr;
    uint8_t size;
    uint8_t log_size;
} sprites[NUM_SPRITES];
struct
{
    int xv;
    int yv;
} vectors[NUM_SPRITES];

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
void init_sprite_config(uint8_t i)
{
    uint16_t ptr = sprites[i].config_ptr;
    unsigned x = ((uint32_t)rand() * (320 - sprites[i].size)) >> 15;
    unsigned y = ((uint32_t)rand() * (240 - sprites[i].size)) >> 15;

    sprites[i].x = x;
    sprites[i].y = y;

    if (((uint32_t)rand() * 2) >> 15)
        vectors[i].xv = 1;
    else
        vectors[i].xv = -1;
    if (((uint32_t)rand() * 2) >> 15)
        vectors[i].yv = 1;
    else
        vectors[i].yv = -1;

    xram0_struct_set(ptr, vga_mode4_sprite_t, x_pos_px, x);
    xram0_struct_set(ptr, vga_mode4_sprite_t, y_pos_px, y);
    xram0_struct_set(ptr, vga_mode4_sprite_t, xram_sprite_ptr, sprites[i].data_ptr);
    xram0_struct_set(ptr, vga_mode4_sprite_t, log_size, sprites[i].log_size);
    xram0_struct_set(ptr, vga_mode4_sprite_t, has_opacity_metadata, false);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void init_sprites()
{
    uint8_t i;    // sprite index
    uint16_t ptr; // config address

    // init altair
    i = 0;
    ptr = SPRITE_CONFIG + i*SPRITE_CONFIG_SIZE;
    sprites[i].config_ptr = ptr;
    sprites[i].data_ptr = ALTAIR_DATA;
    sprites[i].size = 128;
    sprites[i].log_size = 7;
    init_sprite_config(i);

    // init radio
    i = 1;
    ptr = SPRITE_CONFIG + i*SPRITE_CONFIG_SIZE;
    sprites[i].config_ptr = ptr;
    sprites[i].data_ptr = RADIO_DATA;
    sprites[i].size = 64;
    sprites[i].log_size = 6;
    init_sprite_config(i);

    // plane=1, sprite mode
    xreg_vga_mode(4, 0, SPRITE_CONFIG, NUM_SPRITES, 1);
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
    uint8_t i;

    _randomize();

    // Erase console
    printf("\f");

    puts("A tribute to the first great microcomputer hack!");
    puts("");
    puts("On April 16, 1975, Homebrew Computer Club member Steve Dompier used an Altair");
    puts("to play the Beatle's 'Fool on the Hill' via EM interference on an AM radio.");
    puts("");
    puts("I wanted my first song for the RP6502 to be 'Fool on the Hill' as well. Enjoy!");
    puts("");

    // Program VGA
    xreg_vga_canvas(1);

    // Initial sprite positions
    init_sprites();

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
            RIA.addr0 = SPRITE_CONFIG;
            RIA.addr1 = SPRITE_CONFIG + 1;
            for (i = 0; i < NUM_SPRITES; i++)
            {
                int val = sprites[i].x;
                RIA.rw0 = val & 0xff;
                RIA.rw1 = val >> 8;
            }
            RIA.addr0 = SPRITE_CONFIG + 2;
            RIA.addr1 = SPRITE_CONFIG + 3;
            for (i = 0; i < NUM_SPRITES; i++)
            {
                int val = sprites[i].y;
                RIA.rw0 = val & 0xff;
                RIA.rw1 = (val >> 8) & 0xff;
            }

            // Update positions
            for (i = 0; i < NUM_SPRITES; i++)
            {
                int x = sprites[i].x = sprites[i].x + vectors[i].xv;
                int y = sprites[i].y = sprites[i].y + vectors[i].yv;
                if (x < 0 || x > 320 - sprites[i].size)
                    vectors[i].xv *= -1;
                if (y < 0 || y > 240 - sprites[i].size)
                    vectors[i].yv *= -1;
            }

            ezpsg_tick(8);
            if (!ezpsg_playing()) {
                break;
            }
        }
    }
}
