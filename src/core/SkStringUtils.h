/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStringUtils_DEFINED
#define SkStringUtils_DEFINED

#include "SkScalar.h"

class SkString;

/*
 *  The SkStrAppend... methods will write into the provided buffer, assuming it is large enough.
 *  Each method has an associated const (e.g. SkStrAppendU32_MaxSize) which will be the largest
 *  value needed for that method's buffer.
 *
 *  char storage[SkStrAppendU32_MaxSize];
 *  SkStrAppendU32(storage, value);
 *
 *  Note : none of the SkStrAppend... methods write a terminating 0 to their buffers. Instead,
 *  the methods return the ptr to the end of the written part of the buffer. This can be used
 *  to compute the length, and/or know where to write a 0 if that is desired.
 *
 *  char storage[SkStrAppendU32_MaxSize + 1];
 *  char* stop = SkStrAppendU32(storage, value);
 *  size_t len = stop - storage;
 *  *stop = 0;   // valid, since storage was 1 byte larger than the max.
 */

#define SkStrAppendU32_MaxSize  10
char*   SkStrAppendU32(char buffer[], uint32_t);
#define SkStrAppendU64_MaxSize  20
char*   SkStrAppendU64(char buffer[], uint64_t, int minDigits);

#define SkStrAppendU32Hex_MaxSize 8
char*   SkStrAppendU32Hex(char* dst, uint32_t value, int minDigits = 0);

#define SkStrAppendS32_MaxSize  (SkStrAppendU32_MaxSize + 1)
char*   SkStrAppendS32(char buffer[], int32_t);
#define SkStrAppendS64_MaxSize  (SkStrAppendU64_MaxSize + 1)
char*   SkStrAppendS64(char buffer[], int64_t, int minDigits);

/**
 *  Floats have at most 8 significant digits, so we limit our %g to that.
 *  However, the total string could be 15 characters: -1.2345678e-005
 *
 *  In theory we should only expect up to 2 digits for the exponent, but on
 *  some platforms we have seen 3 (as in the example above).
 */
#define SkStrAppendScalar_MaxSize  15

/**
 *  Write the scaler in decimal format into buffer, and return a pointer to
 *  the next char after the last one written. Note: a terminating 0 is not
 *  written into buffer, which must be at least SkStrAppendScalar_MaxSize.
 *  Thus if the caller wants to add a 0 at the end, buffer must be at least
 *  SkStrAppendScalar_MaxSize + 1 bytes large.
 */
#define SkStrAppendScalar SkStrAppendFloat

char* SkStrAppendFloat(char buffer[], float);

enum SkScalarAsStringType {
    kDec_SkScalarAsStringType,
    kHex_SkScalarAsStringType,
};

void SkAppendScalar(SkString*, SkScalar, SkScalarAsStringType);

static inline void SkAppendScalarDec(SkString* str, SkScalar value) {
    SkAppendScalar(str, value, kDec_SkScalarAsStringType);
}

static inline void SkAppendScalarHex(SkString* str, SkScalar value) {
    SkAppendScalar(str, value, kHex_SkScalarAsStringType);
}

/** Indents every non-empty line of the string by tabCnt tabs */
SkString SkTabString(const SkString& string, int tabCnt);

SkString SkStringFromUTF16(const uint16_t* src, size_t count);

#endif
