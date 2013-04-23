/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMD5_DEFINED
#define SkMD5_DEFINED

#include "SkTypes.h"
#include "SkEndian.h"
#include "SkStream.h"

//The following macros can be defined to affect the MD5 code generated.
//SK_MD5_CLEAR_DATA causes all intermediate state to be overwritten with 0's.
//SK_CPU_LENDIAN allows 32 bit <=> 8 bit conversions without copies (if alligned).
//SK_CPU_FAST_UNALIGNED_ACCESS allows 32 bit <=> 8 bit conversions without copies if SK_CPU_LENDIAN.

class SkMD5 : public SkWStream {
public:
    SkMD5();

    /** Processes input, adding it to the digest.
     *  Note that this treats the buffer as a series of uint8_t values.
     */
    virtual bool write(const void* buffer, size_t size) SK_OVERRIDE {
        update(reinterpret_cast<const uint8_t*>(buffer), size);
        return true;
    }

    /** Processes input, adding it to the digest. Calling this after finish is undefined. */
    void update(const uint8_t* input, size_t length);

    struct Digest {
        uint8_t data[16];
    };

    /** Computes and returns the digest. */
    void finish(Digest& digest);

private:
    // number of bytes, modulo 2^64
    uint64_t byteCount;

    // state (ABCD)
    uint32_t state[4];

    // input buffer
    uint8_t buffer[64];
};

#endif
