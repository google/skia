/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDither.h"

/*  The base dither matrix we use to derive optimized ones for 565 and 4444

    { 0,  32, 8,  40, 2,  34, 10, 42 },
    { 48, 16, 56, 24, 50, 18, 58, 26 },
    { 12, 44, 4,  36, 14, 46, 6,  38 },
    { 60, 28, 52, 20, 62, 30, 54, 22 },
    { 3,  35, 11, 43, 1,  33, 9,  41 },
    { 51, 19, 59, 27, 49, 17, 57, 25 },
    { 15, 47, 7,  39, 13, 45, 5,  37 },
    { 63, 31, 55, 23, 61, 29, 53, 21 }

    The 4444 version only needs 4 bits, and given that we can reduce its size
    since the other 4x4 sub pieces all look the same once we truncate the bits.

    The 565 version only needs 3 bits for red/blue, and only 2 bits for green.
    For simplicity, we store 3 bits, and have the dither macros for green know
    this, and they shift the dither value down by 1 to make it 2 bits.
 */

#ifdef ENABLE_DITHER_MATRIX_4X4

const uint8_t gDitherMatrix_4Bit_4X4[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 }
};

const uint8_t gDitherMatrix_3Bit_4X4[4][4] = {
    {  0,  4,  1,  5 },
    {  6,  2,  7,  3 },
    {  1,  5,  0,  4 },
    {  7,  3,  6,  2 }
};

#else   // used packed shorts for a scanlines worth of dither values

const uint16_t gDitherMatrix_4Bit_16[4] = {
    0xA280, 0x6E4C, 0x91B3, 0x5D7F
};

const uint16_t gDitherMatrix_3Bit_16[4] = {
    0x5140, 0x3726, 0x4051, 0x2637
};

#endif
