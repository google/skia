/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkChecksum_DEFINED
#define SkChecksum_DEFINED

#include "SkTypes.h"

#if !defined(SK_PREFER_32BIT_CHECKSUM)
#define SK_PREFER_32BIT_CHECKSUM 0
#endif

enum {
    ChecksumRotateBits = 17
};

#define SkCHECKSUM_MASH(CHECKSUM, NEW_CHUNK) \
    CHECKSUM = (((CHECKSUM) >> (sizeof(CHECKSUM)*8 - ChecksumRotateBits)) + \
        ((CHECKSUM) << ChecksumRotateBits)) ^ (NEW_CHUNK);


/**
 *  Compute a 64-bit checksum for a given data block
 *
 *  @param data Memory address of the data block to be processed. Must be
 *      32-bit aligned
 *  @param size Size of the data block in bytes. Must be a multiple of 8.
 *  @return checksum result
 */
inline uint64_t SkComputeChecksum64(const uint64_t* ptr, size_t size) {
    SkASSERT(SkIsAlign8(size));
    // Strict 8-byte alignment is not required on ptr. On current
    // CPUs there is no measurable performance difference between 32-bit
    // and 64-bit aligned access to uint64_t data
    SkASSERT(SkIsAlign4((intptr_t)ptr));

    const uint64_t* stop = ptr + (size >> 3);
    uint64_t result = 0;
    while (ptr < stop) {
        SkCHECKSUM_MASH(result, *ptr);
        ptr++;
    }
    return result;
}

/**
 *  Compute a 32-bit checksum for a given data block
 *
 *  @param data Memory address of the data block to be processed. Must be
 *      32-bit aligned.
 *  @param size Size of the data block in bytes. Must be a multiple of 4.
 *  @return checksum result
 */
inline uint32_t SkComputeChecksum32(const uint32_t* ptr, size_t size) {
    SkASSERT(SkIsAlign4(size));
    SkASSERT(SkIsAlign4((intptr_t)ptr));

    const uint32_t* stop = ptr + (size >> 2);
    uint32_t result = 0;
    while (ptr < stop) {
        SkCHECKSUM_MASH(result, *ptr);
        ptr++;
    }
    return result;
}

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
        int n4 = size / (sizeof(uintptr_t) << 2);
        for (int i = 0; i < n4; ++i) {
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

