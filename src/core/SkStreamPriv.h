/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStreamPriv_DEFINED
#define SkStreamPriv_DEFINED

#include "SkRefCnt.h"
#include "SkString.h"
#include "SkStream.h"

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

static inline bool SkWStreamWriteDecAsText(SkWStream* out, int32_t dec) {
    char buffer[SkStrAppendS32_MaxSize];
    return out->write(buffer, SkStrAppendS32(buffer, dec) - buffer);
}

static inline bool SkWStreamWriteBigDecAsText(SkWStream* out, int64_t dec, int minDigits = 0) {
    char buffer[SkStrAppendU64_MaxSize];
    return out->write(buffer, SkStrAppendU64(buffer, dec, minDigits) - buffer);
}

static inline bool SkWStreamWriteHexAsText(SkWStream* out, uint32_t hex, int minDigits = 0) {
    char buffer[SkStrAppendU32Hex_MaxSize];
    return out->write(buffer, SkStrAppendU32Hex(buffer, hex, minDigits) - buffer);
}

static inline bool SkWStreamWriteScalarAsText(SkWStream* out, SkScalar value) {
    char buffer[SkStrAppendScalar_MaxSize];
    return out->write(buffer, SkStrAppendScalar(buffer, value) - buffer);
}

static inline bool SkWStreamWriteScalar(SkWStream* out, SkScalar value) {
    return out->write(&value, sizeof(value));
}

static inline bool SkWStreamWriteBool(SkWStream* out, bool v) { return out->write8(v); }

#endif  // SkStreamPriv_DEFINED
