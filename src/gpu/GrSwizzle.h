/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSwizzle_DEFINED
#define GrSwizzle_DEFINED

#include "GrColor.h"
#include "SkColorData.h"

/** Represents a rgba swizzle. It can be converted either into a string or a eight bit int.
    Currently there is no way to specify an arbitrary swizzle, just some static swizzles and an
    assignment operator. That could be relaxed. */
class GrSwizzle {
public:
    constexpr GrSwizzle() : GrSwizzle("rgba") {}

    constexpr GrSwizzle(const GrSwizzle& that)
            : fSwiz{that.fSwiz[0], that.fSwiz[1], that.fSwiz[2], that.fSwiz[3], '\0'}
            , fKey(that.fKey) {}

    constexpr GrSwizzle& operator=(const GrSwizzle& that) {
        fSwiz[0] = that.fSwiz[0];
        fSwiz[1] = that.fSwiz[1];
        fSwiz[2] = that.fSwiz[2];
        fSwiz[3] = that.fSwiz[3];
        SkASSERT(fSwiz[4] == '\0');
        fKey = that.fKey;
        return *this;
    }

    /** Recreates a GrSwizzle from the output of asKey() */
    constexpr void setFromOutputSwizzleKey(uint8_t key) {
        fKey = key;
        for (int i = 0; i < 4; ++i) {
            fSwiz[i] = IToC(key & 3);
            key >>= 2;
        }
        SkASSERT(fSwiz[4] == 0);
    }

    constexpr bool operator==(const GrSwizzle& that) const { return fKey == that.fKey; }
    constexpr bool operator!=(const GrSwizzle& that) const { return !(*this == that); }

    /** Compact representation of the swizzle suitable for a key. */
    constexpr uint16_t asKey() const { return fKey; }

    /** Compact representation of the swizzle suitable for an output color key. */
    constexpr uint8_t asOutputKey() const {
        // Make sure the swizzle doesn't contain a 0 or 1 as the last value.
        SkASSERT(fKey <= 255);
        return fKey;
    }

    /** 4 char null terminated string consisting only of chars 'r', 'g', 'b', 'a'. */
    const char* c_str() const { return fSwiz; }

    char operator[](int i) const {
        SkASSERT(i >= 0 && i < 4);
        return fSwiz[i];
    }

    // The normal component swizzles map to key values 0-3. We set the keys for constant 0 and
    // constant 1 to the next two ints.
    static const int k0KeyValue = 4;
    static const int k1KeyValue = 5;

    static float component_idx_to_float(const SkPMColor4f& color, int idx) {
        if (idx <= 3) {
            return color[idx];
        }
        if (idx == 4) {
            return 0.0f;
        }
        if (idx == 5) {
            return 1.0f;
        }
        SK_ABORT("Unexpected swizzle component indx");
        return -1.0f;
    }

    /** Applies this swizzle to the input color and returns the swizzled color. */
    SkPMColor4f applyTo(const SkPMColor4f& color) const {
        int idx;
        uint32_t key = fKey;
        // Index of the input color that should be mapped to output r.
        idx = (key & 3);
        float outR = component_idx_to_float(color, idx);
        key >>= 2;
        idx = (key & 3);
        float outG = component_idx_to_float(color, idx);
        key >>= 2;
        idx = (key & 3);
        float outB = component_idx_to_float(color, idx);
        key >>= 2;
        SkASSERT(key <= 7);
        idx = (key & 7);
        float outA = component_idx_to_float(color, idx);
        return { outR, outG, outB, outA };
    }

    static constexpr GrSwizzle RGBA() { return GrSwizzle("rgba"); }
    static constexpr GrSwizzle AAAA() { return GrSwizzle("aaaa"); }
    static constexpr GrSwizzle RRRR() { return GrSwizzle("rrrr"); }
    static constexpr GrSwizzle RRRA() { return GrSwizzle("rrra"); }
    static constexpr GrSwizzle BGRA() { return GrSwizzle("bgra"); }
    static constexpr GrSwizzle RGRG() { return GrSwizzle("rgrg"); }
    static constexpr GrSwizzle RGB1() { return GrSwizzle("rgb1"); }

private:
    char fSwiz[5];
    // The key uses 2 bits for the first three components since they can only be swizzled to
    // r,g,b,a. For the last component we all it to be swizzled to r,g,b,a,1,0 so we need an
    // 3 bits to handle all combinations of swizzles.
    uint16_t fKey;

    static constexpr int CToI(char c) {
        switch (c) {
            case 'r': return (GrColor_SHIFT_R / 8);
            case 'g': return (GrColor_SHIFT_G / 8);
            case 'b': return (GrColor_SHIFT_B / 8);
            case 'a': return (GrColor_SHIFT_A / 8);
            case '0': SkASSERT(false); // fallthrough, not supported unless it's the alpha component
            case '1': SkASSERT(false); // fallthrough, not supported unless it's the alpha component
            default:  return -1;
        }
    }

    static constexpr int CToIAlpha(char c) {
        switch (c) {
            case 'r': return (GrColor_SHIFT_R / 8);
            case 'g': return (GrColor_SHIFT_G / 8);
            case 'b': return (GrColor_SHIFT_B / 8);
            case 'a': return (GrColor_SHIFT_A / 8);
            case '0': return k0KeyValue;
            case '1': return k1KeyValue;
            default:  return -1;
        }
    }

    static constexpr char IToC(int idx) {
        switch (8 * idx) {
            case GrColor_SHIFT_R  : return 'r';
            case GrColor_SHIFT_G  : return 'g';
            case GrColor_SHIFT_B  : return 'b';
            case GrColor_SHIFT_A  : return 'a';
            case (k0KeyValue * 8) : SkASSERT(false); // fallthrough, not supported
            case (k1KeyValue * 8) : SkASSERT(false); // fallthrough, not supported
            default:                return -1;
        }
    }

    constexpr GrSwizzle(const char c[4])
            : fSwiz{c[0], c[1], c[2], c[3], '\0'}
            , fKey((CToI(c[0]) << 0) | (CToI(c[1]) << 2) |
                   (CToI(c[2]) << 3) | (CToIAlpha(c[3]) << 4)) {}
};

#endif
