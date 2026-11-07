/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMD5_DEFINED
#define SkMD5_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkTo.h"

#include <array>
#include <cstdint>
#include <cstring>

/* Calculate a 128-bit MD5 message-digest of the bytes sent to this stream. */
class SkMD5 : public SkWStream {
public:
    SkMD5();

    /** Processes input, adding it to the digest.
        Calling this after finish is undefined. */
    bool write(SkSpan<const uint8_t> input);

    /** DEPRECATED: Prefer the SkSpan overload above. This pointer overload is kept for
        SkWStream compatibility and existing clients, and forwards to the SkSpan overload.

        PRECONDITIONS: buffer must point to at least size readable bytes (or size == 0).
        Calling this after finish is undefined. */
    bool write(const void* buffer, size_t size) final;

    size_t bytesWritten() const final { return SkToSizeT(this->byteCount); }

    struct Digest {
        SkString toHexString() const;
        SkString toLowercaseHexString() const;
        bool operator==(Digest const& other) const {
            return data == other.data;
        }
        bool operator!=(Digest const& other) const {
            return !(*this == other);
        }

        std::array<uint8_t, 16> data;
    };

    /** Computes and returns the digest. */
    Digest finish();

private:
    uint64_t byteCount;  // number of bytes, modulo 2^64
    std::array<uint32_t, 4> state;   // state (ABCD)
    std::array<uint8_t, 64> buffer;  // input buffer
};

#endif
