/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64)

    // A temporary test to make sure I understand __attribute__((section)) and __declspec(allocate).

    #if defined(_MSC_VER)
        #define CODE __declspec(allocate(".text"))
    #elif defined(__MACH__)
        #define CODE __attribute__((section("__TEXT,__text")))
    #else
        #define CODE __attribute__((section(".text")))
    #endif

    CODE static const uint8_t meaning[] = {
        0xb8,0x2a,0x00,0x00,0x00,  // movl $0x2a, %eax
        0xc3,                      // retq
    };

    DEF_TEST(Section, r) {
        auto fn = (int(*)()) &meaning;

        REPORTER_ASSERT(r, fn() == 42);
    }

#endif
