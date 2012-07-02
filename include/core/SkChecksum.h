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
#endif

