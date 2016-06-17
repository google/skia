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
            fSwiz[i] = IdxToChar(key & 3);
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

    static const GrSwizzle& CreateRandom(SkRandom* random) {
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

private:
    char fSwiz[5];
    uint8_t fKey;

    static int CharToIdx(char c) {
        switch (c) {
            case 'r':
                return (GrColor_SHIFT_R / 8);
            case 'g':
                return (GrColor_SHIFT_G / 8);
            case 'b':
                return (GrColor_SHIFT_B / 8);
            case 'a':
                return (GrColor_SHIFT_A / 8);
            default:
                SkFAIL("Invalid swizzle char");
                return 0;
        }
    }

    static /* constexpr */ char IToC(int idx) {
        return (8*idx) == GrColor_SHIFT_R ? 'r' :
               (8*idx) == GrColor_SHIFT_G ? 'g' :
               (8*idx) == GrColor_SHIFT_B ? 'b' : 'a';
    }

    static char IdxToChar(int c) {
        // Hopefully this array gets computed at compile time.
        static const char gStr[4] = { IToC(0), IToC(1), IToC(2), IToC(3) };
        return gStr[c];
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
