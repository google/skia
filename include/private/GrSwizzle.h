/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSwizzle_DEFINED
#define GrSwizzle_DEFINED

#include "GrColor.h"
#include "SkRandom.h"

/** Represents a rgba swizzle. It can be converted either into a string or a eight bit int.
    Currently there is no way to specify an arbitrary swizzle, just some static swizzles and an
    assignment operator. That could be relaxed. */
class GrSwizzle {
private:
    char fSwiz[5];
    uint8_t fKey;

    static constexpr int CToI(char c) {
        return ('r' == c) ? (GrColor_SHIFT_R / 8) :
               ('g' == c) ? (GrColor_SHIFT_G / 8) :
               ('b' == c) ? (GrColor_SHIFT_B / 8) :
               ('a' == c) ? (GrColor_SHIFT_A / 8) : -1;
    }

    static constexpr char IToC(int idx) {
        return (8 * idx) == GrColor_SHIFT_R ? 'r' :
               (8 * idx) == GrColor_SHIFT_G ? 'g' :
               (8 * idx) == GrColor_SHIFT_B ? 'b' :
               (8 * idx) == GrColor_SHIFT_A ? 'a' : 'x';
    }

    constexpr GrSwizzle(const char c[4])
        : fSwiz{c[0], c[1], c[2], c[3], 0}
        , fKey((CToI(c[0]) << 0) | (CToI(c[1]) << 2) | (CToI(c[2]) << 4) | (CToI(c[3]) << 6)) {}

    GR_STATIC_ASSERT(sizeof(char[4]) == sizeof(uint32_t));
    uint32_t* asUIntPtr() { return SkTCast<uint32_t*>(fSwiz); }
    uint32_t asUInt() const { return *SkTCast<const uint32_t*>(fSwiz); }

public:
    GrSwizzle() { *this = RGBA(); }

    GrSwizzle(const GrSwizzle& that) { *this = that; }

    GrSwizzle& operator=(const GrSwizzle& that) {
        memcpy(this, &that, sizeof(GrSwizzle));
        return *this;
    }

    /** Recreates a GrSwizzle from the output of asKey() */
    void setFromKey(uint8_t key) {
        fKey = key;
        for (int i = 0; i < 4; ++i) {
            fSwiz[i] = IToC(key & 3);
            key >>= 2;
        }
        SkASSERT(fSwiz[4] == 0);
    }

    bool operator==(const GrSwizzle& that) const { return this->asUInt() == that.asUInt(); }

    bool operator!=(const GrSwizzle& that) const { return !(*this == that); }

    /** Compact representation of the swizzle suitable for a key. */
    uint8_t asKey() const { return fKey; }

    /** 4 char null terminated string consisting only of chars 'r', 'g', 'b', 'a'. */
    const char* c_str() const { return fSwiz; }

    /** Applies this swizzle to the input color and returns the swizzled color. */
    GrColor applyTo(GrColor color) const {
        int idx;
        uint32_t key = fKey;
        // Index of the input color that should be mapped to output r.
        idx = (key & 3);
        uint32_t outR = (color >> idx * 8)  & 0xFF;
        key >>= 2;
        idx = (key & 3);
        uint32_t outG = (color >> idx * 8)  & 0xFF;
        key >>= 2;
        idx = (key & 3);
        uint32_t outB = (color >> idx * 8)  & 0xFF;
        key >>= 2;
        idx = (key & 3);
        uint32_t outA = (color >> idx * 8)  & 0xFF;
        return GrColorPackRGBA(outR, outG, outB, outA);
    }

    /** Applies this swizzle to the input color and returns the swizzled color. */
    GrColor4f applyTo(const GrColor4f& color) const {
        int idx;
        uint32_t key = fKey;
        // Index of the input color that should be mapped to output r.
        idx = (key & 3);
        float outR = color.fRGBA[idx];
        key >>= 2;
        idx = (key & 3);
        float outG = color.fRGBA[idx];
        key >>= 2;
        idx = (key & 3);
        float outB = color.fRGBA[idx];
        key >>= 2;
        idx = (key & 3);
        float outA = color.fRGBA[idx];
        return GrColor4f(outR, outG, outB, outA);
    }

    static GrSwizzle RGBA() { return GrSwizzle("rgba"); }
    static GrSwizzle AAAA() { return GrSwizzle("aaaa"); }
    static GrSwizzle RRRR() { return GrSwizzle("rrrr"); }
    static GrSwizzle RRRA() { return GrSwizzle("rrra"); }
    static GrSwizzle BGRA() { return GrSwizzle("bgra"); }

    static GrSwizzle CreateRandom(SkRandom* random) {
        switch (random->nextU() % 4) {
            case 0:
                return RGBA();
            case 1:
                return BGRA();
            case 2:
                return RRRR();
            case 3:
                return AAAA();
            default:
                SkFAIL("Mod is broken?!?");
                return RGBA();
        }
    }
};

#endif
