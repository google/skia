/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSwizzle_DEFINED
#define GrSwizzle_DEFINED

#include "GrTypes.h"

/** Represents a rgba swizzle. It can be converted either into a string or a eight bit int.
    Currently there is no way to specify an arbitrary swizzle, just some static swizzles and an
    assignment operator. That could be relaxed. */
class GrSwizzle {
public:
    GrSwizzle() { *this = RGBA(); }

    GrSwizzle& operator=(const GrSwizzle& that) {
        memcpy(this, &that, sizeof(GrSwizzle));
        return *this;
    }

    bool operator==(const GrSwizzle& that) const { return this->asUInt() == that.asUInt(); }

    bool operator!=(const GrSwizzle& that) const { return !(*this == that); }

    /** Compact representation of the swizzle suitable for a key. */
    uint8_t asKey() const { return fKey; }

    /** 4 char null terminated string consisting only of chars 'r', 'g', 'b', 'a'. */
    const char* c_str() const { return fSwiz; }

    static const GrSwizzle& RGBA() {
        static GrSwizzle gRGBA("rgba");
        return gRGBA;
    }

    static const GrSwizzle& AAAA() {
        static GrSwizzle gAAAA("aaaa");
        return gAAAA;
    }

    static const GrSwizzle& RRRR() {
        static GrSwizzle gRRRR("rrrr");
        return gRRRR;
    }

    static const GrSwizzle& BGRA() {
        static GrSwizzle gBGRA("bgra");
        return gBGRA;
    }

private:
    char fSwiz[5];
    uint8_t fKey;

    static int CharToIdx(char c) {
        switch (c) {
            case 'r':
                return 0;
            case 'g':
                return 1;
            case 'b':
                return 2;
            case 'a':
                return 3;
            default:
                SkFAIL("Invalid swizzle char");
                return 0;
        }
    }

    explicit GrSwizzle(const char* str) {
        SkASSERT(strlen(str) == 4);
        fSwiz[0] = str[0];
        fSwiz[1] = str[1];
        fSwiz[2] = str[2];
        fSwiz[3] = str[3];
        fSwiz[4] = 0;
        fKey = SkToU8(CharToIdx(fSwiz[0]) | (CharToIdx(fSwiz[1]) << 2) |
                      (CharToIdx(fSwiz[2]) << 4) | (CharToIdx(fSwiz[3]) << 6));
    }

    uint32_t* asUIntPtr() { return SkTCast<uint32_t*>(fSwiz); }
    uint32_t asUInt() const { return *SkTCast<const uint32_t*>(fSwiz); }

    GR_STATIC_ASSERT(sizeof(char[4]) == sizeof(uint32_t));
};

#endif
