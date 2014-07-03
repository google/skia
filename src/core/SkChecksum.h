/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkChecksum_DEFINED
#define SkChecksum_DEFINED

#include "SkTypes.h"

/**
 *  Computes a 32bit checksum from a blob of 32bit aligned data. This is meant
 *  to be very very fast, as it is used internally by the font cache, in
 *  conjuction with the entire raw key. This algorithm does not generate
 *  unique values as well as others (e.g. MD5) but it performs much faster.
 *  Skia's use cases can survive non-unique values (since the entire key is
 *  always available). Clients should only be used in circumstances where speed
 *  over uniqueness is at a premium.
 */
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
     * Calculate 32-bit Murmur hash (murmur3).
     * This should take 2-3x longer than SkChecksum::Compute, but is a considerably better hash.
     * See en.wikipedia.org/wiki/MurmurHash.
     *
     *  @param data Memory address of the data block to be processed. Must be 32-bit aligned.
     *  @param size Size of the data block in bytes. Must be a multiple of 4.
     *  @param seed Initial hash seed. (optional)
     *  @return hash result
     */
    static uint32_t Murmur3(const uint32_t* data, size_t bytes, uint32_t seed=0) {
        SkASSERT(SkIsAlign4(bytes));
        const size_t words = bytes/4;

        uint32_t hash = seed;
        for (size_t i = 0; i < words; i++) {
            uint32_t k = data[i];
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;

            hash ^= k;
            hash = (hash << 13) | (hash >> 19);
            hash *= 5;
            hash += 0xe6546b64;
        }
        hash ^= bytes;
        hash ^= hash >> 16;
        hash *= 0x85ebca6b;
        hash ^= hash >> 13;
        hash *= 0xc2b2ae35;
        hash ^= hash >> 16;
        return hash;
    }

    /**
     *  Compute a 32-bit checksum for a given data block
     *
     *  WARNING: this algorithm is tuned for efficiency, not backward/forward
     *  compatibility.  It may change at any time, so a checksum generated with
     *  one version of the Skia code may not match a checksum generated with
     *  a different version of the Skia code.
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
