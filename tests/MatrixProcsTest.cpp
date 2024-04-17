/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "src/core/SkBitmapProcState.h"
#include "src/opts/SkBitmapProcState_opts.h"

#include <cstddef>
#include <cstdint>
#include <string>

uint32_t highBits(uint32_t rv) {
    return (rv >> 18) & ((1 << 14) - 1);
}

uint32_t middleBits(uint32_t rv) {
    return (rv >> 14) & ((1 << 4) - 1);
}

uint32_t lowBits(uint32_t rv) {
    return rv & ((1 << 14) - 1);
}

DEF_TEST(MatrixProcs_pack_clamp, r) {
    struct TestCase {
        std::string name;
        SkFixed input;
        unsigned max;
        uint32_t expectedOutput;
    };
    // The input values are somewhat arbitrary, inspired by real-world values
    // with some edge cases added as well.
    TestCase tests[] = {
        // Negative values keep the fractional part out of convenience, but it is effectively
        // ignored later.
        {"-2.100 => {0x00, 0xe, 0x00}", SkFloatToFixed(-2.100f), 63, 0x38000},
        {"-1.900 => {0x00, 0x1, 0x00}", SkFloatToFixed(-1.900f), 63, 0x04000},
        {"-0.500 => {0x00, 0x8, 0x00}", SkFloatToFixed(-0.500f), 63, 0x20000},
        {"0.0000 => {0x00, 0x0, 0x01}", SkFloatToFixed(0.0000f), 63, 0x000001},
        {"0.0416 => {0x00, 0x0, 0x01}", SkFloatToFixed(0.0416f), 63, 0x000001},
        {"1.8583 => {0x01, 0xd, 0x02}", SkFloatToFixed(1.8583f), 63, 0x074002},
        {"3.6749 => {0x03, 0xa, 0x04}", SkFloatToFixed(3.6749f), 63, 0x0e8004},
        {"5.4916 => {0x05, 0x7, 0x06}", SkFloatToFixed(5.4916f), 63, 0x15c006},
        {"7.3083 => {0x07, 0x4, 0x08}", SkFloatToFixed(7.3083f), 63, 0x1d0008},
        {"9.0000 => {0x09, 0x0, 0x0a}", SkFloatToFixed(9.0000f), 63, 0x24000a},
        {"50.000 => {0x32, 0x0, 0x33}", SkFloatToFixed(50.000f), 63, 0xc80033},
        {"50.875 => {0x32, 0xe, 0x33}", SkFloatToFixed(50.875f), 63, 0xcb8033},
        {"62.123 => {0x3e, 0x1, 0x3f}", SkFloatToFixed(62.123f), 63, 0xf8403f},
        {"62.999 => {0x3e, 0xf, 0x3f}", SkFloatToFixed(62.999f), 63, 0xfbc03f},
        {"63.000 => {0x3f, 0x0, 0x3f}", SkFloatToFixed(63.000f), 63, 0xfc003f},
        // Similarly, overflow keeps the fractional part.
        {"64.500 => {0x3f, 0x8, 0x3f}", SkFloatToFixed(64.500f), 63, 0xfe003f},
        {"127.20 => {0x3f, 0x3, 0x3f}", SkFloatToFixed(127.20f), 63, 0xfcc03f},
        // Maximum has changed from 63 to 256
        {"64.510 => {0x40,  0x8, 0x41}",  SkFloatToFixed(64.510f), 256, 0x1020041},
        {"127.21 => {0x7f,  0x3, 0x80}",  SkFloatToFixed(127.21f), 256, 0x1fcc080},
        {"256.77 => {0x100, 0xc, 0x100}", SkFloatToFixed(256.77f), 256, 0x4030100},
        {"10000. => {0x100, 0x0, 0x100}", SkFloatToFixed(10000.f), 256, 0x4000100},
    };

    constexpr size_t NUM_TESTS = sizeof(tests) / sizeof(TestCase);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        TestCase tc = tests[i];
        uint32_t rv = sktests::pack_clamp(tc.input, tc.max);
        uint32_t exp = tc.expectedOutput;

        REPORTER_ASSERT(r, rv == tc.expectedOutput,
                        "%s | %x != %x | {%x, %x, %x} != {%x, %x, %x}\n",
                        tc.name.c_str(), rv, exp,
                        highBits(rv), middleBits(rv), lowBits(rv),
                        highBits(exp), middleBits(exp), lowBits(exp));
    }
}

DEF_TEST(MatrixProcs_pack_clamp_out_of_range, r) {
    // See https://crbug.com/1357122
    struct TestCase {
        std::string name;
        SkFixed input;
        unsigned max;
        uint32_t expectedOutput;
    };

    // None of these should crash or cause UBSAN errors, although if they were used in drawing,
    // they might be incorrect due to packing 16 bits of SkFixed into 14 bits of space.
    static constexpr unsigned MAX_PACKED_VALUE = (1 << 14) - 1;
    TestCase tests[] = {
        {"16000.42=>{0xff, 6, 0xff}", SkFloatToFixed(16000.42f), 255, 0x3fd80ff},
        {"17000.42=>{0xff, 6, 0xff}", SkFloatToFixed(17000.42f), 255, 0x3fd80ff},
        {"18000.42=>{0xff, 6, 0xff}", SkFloatToFixed(18000.42f), 255, 0x3fd80ff},

        {"16000.42=>{0x3e80, 6, 0x3e81}", SkFloatToFixed(16000.42f), MAX_PACKED_VALUE, 0xfa01be81},
        {"16382.00=>{0x3ffe, 0, 0x3fff}", SkFloatToFixed(16382.00f), MAX_PACKED_VALUE, 0xfff83fff},
        {"16382.51=>{0x3ffe, 8, 0x3fff}}", SkFloatToFixed(16382.51f), MAX_PACKED_VALUE, 0xfffa3fff},
        {"17000.42=>{0x3fff, 6, 0x3fff}", SkFloatToFixed(17000.42f), MAX_PACKED_VALUE, 0xfffdbfff},
        {"18000.42=>{0x3fff, 6, 0x3fff}", SkFloatToFixed(18000.42f), MAX_PACKED_VALUE, 0xfffdbfff},
        // Adding 1 to this would overflow and cause an UBSAN issue, if it were not suppressed.
        // We suppress the warning and it wraps around.
        {"32767.90=>{0x3fff, e, 0x0}", SkFloatToFixed(32767.90f), MAX_PACKED_VALUE, 0xffff8000},
    };

    constexpr size_t NUM_TESTS = sizeof(tests) / sizeof(TestCase);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        TestCase tc = tests[i];
        uint32_t rv = sktests::pack_clamp(tc.input, tc.max);
        uint32_t exp = tc.expectedOutput;

        REPORTER_ASSERT(r, rv == tc.expectedOutput,
                        "%s | %x != %x | {%x, %x, %x} != {%x, %x, %x}\n",
                        tc.name.c_str(), rv, exp,
                        highBits(rv), middleBits(rv), lowBits(rv),
                        highBits(exp), middleBits(exp), lowBits(exp));
    }
}

DEF_TEST(MatrixProcs_pack_repeat, r) {
    struct TestCase {
        std::string name;
        SkFixed input;
        unsigned max;
        uint32_t expectedOutput;
        size_t width;
    };
    TestCase tests[] = {
        // negative values wrap back around
        {"-0.300 => {0x2c, 0xc, 0x2d}", SkFloatToFixed(-0.300f), 63, 0xb3002d, 63},
        {"-0.200 => {0x33, 0x3, 0x34}", SkFloatToFixed(-0.200f), 63, 0xccc034, 63},
        {"-0.100 => {0x39, 0x9, 0x3a}", SkFloatToFixed(-0.100f), 63, 0xe6403a, 63},
        // The domain of the function is primarily [0.0, 1.0)
        {"0.0000 => {0x00, 0x0, 0x01}", SkFloatToFixed(0.0000f), 63, 0x000001, 63},
        {"0.1000 => {0x06, 0x6, 0x07}", SkFloatToFixed(0.1000f), 63, 0x198007, 63},
        {"0.1234 => {0x07, 0xe, 0x08}", SkFloatToFixed(0.1234f), 63, 0x1f8008, 63},
        {"0.2000 => {0x0c, 0xc, 0x0d}", SkFloatToFixed(0.2000f), 63, 0x33000d, 63},
        {"0.3000 => {0x13, 0x3, 0x14}", SkFloatToFixed(0.3000f), 63, 0x4cc014, 63},
        {"0.4000 => {0x19, 0x9, 0x1a}", SkFloatToFixed(0.4000f), 63, 0x66401a, 63},
        {"0.5000 => {0x20, 0x0, 0x21}", SkFloatToFixed(0.5000f), 63, 0x800021, 63},
        {"0.5678 => {0x24, 0x5, 0x25}", SkFloatToFixed(0.5678f), 63, 0x914025, 63},
        {"0.6000 => {0x26, 0x6, 0x27}", SkFloatToFixed(0.6000f), 63, 0x998027, 63},
        {"0.7000 => {0x2c, 0xc, 0x2d}", SkFloatToFixed(0.7000f), 63, 0xb3002d, 63},
        {"0.8000 => {0x33, 0x3, 0x34}", SkFloatToFixed(0.8000f), 63, 0xccc034, 63},
        {"0.9000 => {0x39, 0x9, 0x3a}", SkFloatToFixed(0.9000f), 63, 0xe6403a, 63},
        {"0.9500 => {0x3c, 0xc, 0x3d}", SkFloatToFixed(0.9500f), 63, 0xf3003d, 63},
        {"0.9990 => {0x3f, 0xe, 0x00}", SkFloatToFixed(0.9990f), 63, 0xff8000, 63},
        // As we go past 1.0, we wrap around, conceptually similar to modular arithmetic.
        {"1.0000 => {0x00, 0x0, 0x01}", SkFloatToFixed(1.0000f), 63, 0x000001, 63},
        {"1.1000 => {0x06, 0x6, 0x07}", SkFloatToFixed(1.1000f), 63, 0x198007, 63},
        {"1.1234 => {0x07, 0xe, 0x08}", SkFloatToFixed(1.1234f), 63, 0x1f8008, 63},
        {"1.9500 => {0x3c, 0xc, 0x3d}", SkFloatToFixed(1.9500f), 63, 0xf3003d, 63},
        // Maximum has changed from 63 to 256
        {"0.4567 => {0x75, 0x5, 0x76}", SkFloatToFixed(0.4567f), 256, 0x1d54076, 256},
        {"1.0000 => {0x00, 0x0, 0x01}", SkFloatToFixed(1.0000f), 256, 0x0000001, 256},
        {"1.2345 => {0x3c, 0x4, 0x3d}", SkFloatToFixed(1.2345f), 256, 0x0f1003d, 256},
        // width does not have to match the maximum value (e.g. rescaling)
        {"0.1111 [64,128] => {0x07, 0x3, 0x07}", SkFloatToFixed(0.1111f), 64, 0x1cc007, 128},
        {"0.1111 [64,256] => {0x07, 0x3, 0x07}", SkFloatToFixed(0.1111f), 64, 0x1cc007, 256},
        {"0.1111 [64,512] => {0x07, 0x3, 0x07}", SkFloatToFixed(0.1111f), 64, 0x1cc007, 512},
        {"0.1111 [64, 32] => {0x07, 0x3, 0x09}", SkFloatToFixed(0.1111f), 64, 0x1cc009, 32},
        {"0.1111 [64,  8] => {0x07, 0x3, 0x0f}", SkFloatToFixed(0.1111f), 64, 0x1cc00f, 8},
    };

    constexpr size_t NUM_TESTS = sizeof(tests) / sizeof(TestCase);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        TestCase tc = tests[i];
        uint32_t rv = sktests::pack_repeat(tc.input, tc.max, tc.width);
        uint32_t exp = tc.expectedOutput;

        REPORTER_ASSERT(r, rv == tc.expectedOutput,
                        "%s | %x != %x | {%x, %x, %x} != {%x, %x, %x}\n",
                        tc.name.c_str(), rv, exp,
                        highBits(rv), middleBits(rv), lowBits(rv),
                        highBits(exp), middleBits(exp), lowBits(exp));
    }
}

DEF_TEST(MatrixProcs_pack_mirror, r) {
    struct TestCase {
        std::string name;
        SkFixed input;
        unsigned max;
        uint32_t expectedOutput;
        size_t width;
    };
    TestCase tests[] = {
        // negative values are treated similarly to absolute values, except
        // the first integer is bigger than the last integer.
        {"-0.300 => {0x13, 0xc, 0x12}", SkFloatToFixed(-0.300f), 63, 0x4f0012, 63},
        {"-0.200 => {0x0c, 0x3, 0x0b}", SkFloatToFixed(-0.200f), 63, 0x30c00b, 63},
        {"-0.100 => {0x06, 0x9, 0x05}", SkFloatToFixed(-0.100f), 63, 0x1a4005, 63},
        // The domain of the function is primarily [0.0, 1.0)
        {"0.0000 => {0x00, 0x0, 0x01}", SkFloatToFixed(0.0000f), 63, 0x000001, 63},
        {"0.1000 => {0x06, 0x6, 0x07}", SkFloatToFixed(0.1000f), 63, 0x198007, 63},
        {"0.1234 => {0x07, 0xe, 0x08}", SkFloatToFixed(0.1234f), 63, 0x1f8008, 63},
        {"0.2000 => {0x0c, 0xc, 0x0d}", SkFloatToFixed(0.2000f), 63, 0x33000d, 63},
        {"0.3000 => {0x13, 0x3, 0x14}", SkFloatToFixed(0.3000f), 63, 0x4cc014, 63},
        {"0.4000 => {0x19, 0x9, 0x1a}", SkFloatToFixed(0.4000f), 63, 0x66401a, 63},
        {"0.5000 => {0x20, 0x0, 0x21}", SkFloatToFixed(0.5000f), 63, 0x800021, 63},
        {"0.5678 => {0x24, 0x5, 0x25}", SkFloatToFixed(0.5678f), 63, 0x914025, 63},
        {"0.6000 => {0x26, 0x6, 0x27}", SkFloatToFixed(0.6000f), 63, 0x998027, 63},
        {"0.7000 => {0x2c, 0xc, 0x2d}", SkFloatToFixed(0.7000f), 63, 0xb3002d, 63},
        {"0.8000 => {0x33, 0x3, 0x34}", SkFloatToFixed(0.8000f), 63, 0xccc034, 63},
        {"0.9000 => {0x39, 0x9, 0x3a}", SkFloatToFixed(0.9000f), 63, 0xe6403a, 63},
        {"0.9500 => {0x3c, 0xc, 0x3d}", SkFloatToFixed(0.9500f), 63, 0xf3003d, 63},
        {"0.9990 => {0x3f, 0xe, 0x3f}", SkFloatToFixed(0.9990f), 63, 0xff803f, 63},
        // As we go past 1.0, we bounce back, as off a wall or reflecting off a mirror.
        {"1.0000 => {0x3f, 0x0, 0x3e}", SkFloatToFixed(1.0000f), 63, 0xfc003e, 63},
        {"1.1000 => {0x39, 0x6, 0x38}", SkFloatToFixed(1.1000f), 63, 0xe58038, 63},
        {"1.1234 => {0x38, 0xe, 0x37}", SkFloatToFixed(1.1234f), 63, 0xe38037, 63},
        {"1.9500 => {0x03, 0xc, 0x02}", SkFloatToFixed(1.9500f), 63, 0xf0002, 63},
        // Maximum has changed from 63 to 256
        {"0.4567 => {0x75, 0x5, 0x76}", SkFloatToFixed(0.4567f), 256, 0x1d54076, 256},
        {"1.0000 => {0x100,0x0, 0xff}", SkFloatToFixed(1.0000f), 256, 0x40000ff, 256},
        {"1.2345 => {0xc4, 0x4, 0xc3}", SkFloatToFixed(1.2345f), 256, 0x31100c3, 256},
        // width does not have to match the maximum value (e.g. rescaling)
        {"0.1111 [64,128] => {0x07, 0x3, 0x07}", SkFloatToFixed(0.1111f), 64, 0x1cc007, 128},
        {"0.1111 [64,256] => {0x07, 0x3, 0x07}", SkFloatToFixed(0.1111f), 64, 0x1cc007, 256},
        {"0.1111 [64,512] => {0x07, 0x3, 0x07}", SkFloatToFixed(0.1111f), 64, 0x1cc007, 512},
        {"0.1111 [64, 32] => {0x07, 0x3, 0x09}", SkFloatToFixed(0.1111f), 64, 0x1cc009, 32},
        {"0.1111 [64,  8] => {0x07, 0x3, 0x0f}", SkFloatToFixed(0.1111f), 64, 0x1cc00f, 8},
    };

    constexpr size_t NUM_TESTS = sizeof(tests) / sizeof(TestCase);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        TestCase tc = tests[i];
        uint32_t rv = sktests::pack_mirror(tc.input, tc.max, tc.width);
        uint32_t exp = tc.expectedOutput;

        REPORTER_ASSERT(r, rv == tc.expectedOutput,
                        "%s | %x != %x | {%x, %x, %x} != {%x, %x, %x}\n",
                        tc.name.c_str(), rv, exp,
                        highBits(rv), middleBits(rv), lowBits(rv),
                        highBits(exp), middleBits(exp), lowBits(exp));
    }
}

DEF_TEST(MatrixProcs_unpack_int, r) {
    struct TestCase {
        std::string name;
        uint32_t input;
        uint32_t expectedLowerBound;
        uint32_t expectedLerp;
        uint32_t expectedUpperBound;
    };
    // These are selected from earlier tests to make sure the packed values unpack correctly.
    TestCase tests[] = {
        {"0x000000  => {0x00, 0x0, 0x00}",   0x000000,  0x00,  0x0, 0x00},
        {"0x074002  => {0x01, 0xd, 0x02}",   0x074002,  0x01,  0xd, 0x02},
        {"0x15c006  => {0x05, 0x7, 0x06}",   0x15c006,  0x05,  0x7, 0x06},
        {"0x1d0008  => {0x07, 0x4, 0x08}",   0x1d0008,  0x07,  0x4, 0x08},
        {"0x24000a  => {0x09, 0x0, 0x0a}",   0x24000a,  0x09,  0x0, 0x0a},
        {"0xfc003f  => {0x3f, 0x0, 0x3f}",   0xfc003f,  0x3f,  0x0, 0x3f},
        {"0x4000100 => {0x100, 0x0, 0x100}", 0x4000100, 0x100, 0x0, 0x100},
    };

    constexpr size_t NUM_TESTS = sizeof(tests) / sizeof(TestCase);

    for (size_t i = 0; i < NUM_TESTS; i++) {
        TestCase tc = tests[i];
        uint32_t lower, upper, lerp;
        sktests::decode_packed_coordinates_and_weight(tc.input, &lower, &upper, &lerp);

        REPORTER_ASSERT(r, lower == tc.expectedLowerBound,
                "%s lower %x != %x", tc.name.c_str(), lower, tc.expectedLowerBound);
        REPORTER_ASSERT(r, lerp == tc.expectedLerp,
                "%s lerp %x != %x", tc.name.c_str(), lerp, tc.expectedLerp);
        REPORTER_ASSERT(r, upper == tc.expectedUpperBound,
                "%s upper %x != %x", tc.name.c_str(), upper, tc.expectedUpperBound);
        // Make sure our helpers work as expected.
        SkASSERT(tc.expectedLowerBound == highBits(tc.input));
        SkASSERT(tc.expectedLerp == middleBits(tc.input));
        SkASSERT(tc.expectedUpperBound == lowBits(tc.input));
    }
}
