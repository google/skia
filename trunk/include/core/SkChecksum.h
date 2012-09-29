/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkChecksum_DEFINED
#define SkChecksum_DEFINED

#include "SkTypes.h"

class SkChecksum : SkNoncopyable {
private:
    /*
     *  Our Rotate and Mash helpers are meant to automatically do the right
     *  thing depending if sizeof(uintptr_t) is 4 or 8.
     */
    enum {
        ROTR = 17,
        ROTL = sizeof(uintptr_t) * 8 - ROTR,
        HALFBITS = sizeof(uintptr_t) * 4
    };

    static inline uintptr_t Mash(uintptr_t total, uintptr_t value) {
        return ((total >> ROTR) | (total << ROTL)) ^ value;
    }

public:
    /**
     *  Compute a 32-bit checksum for a given data block
     *
     *  @param data Memory address of the data block to be processed. Must be
     *              32-bit aligned.
     *  @param size Size of the data block in bytes. Must be a multiple of 4.
     *  @return checksum result
     */
    static uint32_t Compute(const uint32_t* data, size_t size) {
        SkASSERT(SkIsAlign4(size));

        /*
         *  We want to let the compiler use 32bit or 64bit addressing and math
         *  so we use uintptr_t as our magic type. This makes the code a little
         *  more obscure (we can't hard-code 32 or 64 anywhere, but have to use
         *  sizeof()).
         */
        uintptr_t result = 0;
        const uintptr_t* ptr = reinterpret_cast<const uintptr_t*>(data);

        /*
         *  count the number of quad element chunks. This takes into account
         *  if we're on a 32bit or 64bit arch, since we use sizeof(uintptr_t)
         *  to compute how much to shift-down the size.
         */
        size_t n4 = size / (sizeof(uintptr_t) << 2);
        for (size_t i = 0; i < n4; ++i) {
            result = Mash(result, *ptr++);
            result = Mash(result, *ptr++);
            result = Mash(result, *ptr++);
            result = Mash(result, *ptr++);
        }
        size &= ((sizeof(uintptr_t) << 2) - 1);

        data = reinterpret_cast<const uint32_t*>(ptr);
        const uint32_t* stop = data + (size >> 2);
        while (data < stop) {
            result = Mash(result, *data++);
        }

        /*
         *  smash us down to 32bits if we were 64. Note that when uintptr_t is
         *  32bits, this code-path should go away, but I still got a warning
         *  when I wrote
         *      result ^= result >> 32;
         *  since >>32 is undefined for 32bit ints, hence the wacky HALFBITS
         *  define.
         */
        if (8 == sizeof(result)) {
            result ^= result >> HALFBITS;
        }
        return static_cast<uint32_t>(result);
    }
};

#endif

