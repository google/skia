/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStreamPriv_DEFINED
#define SkStreamPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/base/SkEndian.h"

#include <cstdint>

class SkData;

/**
 *  Copy the provided stream to an SkData variable.
 *
 *  Note: Assumes the stream is at the beginning. If it has a length,
 *  but is not at the beginning, this call will fail (return NULL).
 *
 *  @param stream SkStream to be copied into data.
 *  @return The resulting SkData after the copy, nullptr on failure.
 */
sk_sp<SkData> SkCopyStreamToData(SkStream* stream);

/**
 *  Copies the input stream from the current position to the end.
 *  Does not rewind the input stream.
 */
bool SkStreamCopy(SkWStream* out, SkStream* input);

/** A SkWStream that writes all output to SkDebugf, for debugging purposes. */
class SkDebugfStream final : public SkWStream {
public:
    bool write(const void* buffer, size_t size) override;
    size_t bytesWritten() const override;

private:
    size_t fBytesWritten = 0;
};

/**
 * Helper functions to write big-endian values to a stream.
 */
inline bool SkWStreamWriteU16BE(SkWStream* s, uint16_t value) {
    value = SkEndian_SwapBE16(value);
    return s->write(&value, sizeof(value));
}

inline bool SkWStreamWriteU32BE(SkWStream* s, uint32_t value) {
    value = SkEndian_SwapBE32(value);
    return s->write(&value, sizeof(value));
}

inline bool SkWStreamWriteS32BE(SkWStream* s, int32_t value) {
    value = SkEndian_SwapBE32(value);
    return s->write(&value, sizeof(value));
}

// If the stream supports identifying the current position and total length, this returns
// true if there are not enough bytes in the stream to fulfill a read of the given length.
// Otherwise, it returns false.
// False does *not* mean a read will succeed of the given length, but true means we are
// certain it will fail.
bool StreamRemainingLengthIsBelow(SkStream* stream, size_t len);

#endif  // SkStreamPriv_DEFINED
