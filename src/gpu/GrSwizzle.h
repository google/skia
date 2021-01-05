/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSwizzle_DEFINED
#define GrSwizzle_DEFINED

#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "src/gpu/GrColor.h"

class SkRasterPipeline;

/** Represents a rgba swizzle. It can be converted either into a string or a eight bit int. */
class GrSwizzle {
public:
    constexpr GrSwizzle() : GrSwizzle("rgba") {}
    explicit constexpr GrSwizzle(const char c[4]);

    constexpr GrSwizzle(const GrSwizzle&);
    constexpr GrSwizzle& operator=(const GrSwizzle& that);

    static constexpr GrSwizzle Concat(const GrSwizzle& a, const GrSwizzle& b);

    constexpr bool operator==(const GrSwizzle& that) const { return fKey == that.fKey; }
    constexpr bool operator!=(const GrSwizzle& that) const { return !(*this == that); }

    /** Compact representation of the swizzle suitable for a key. */
    constexpr uint16_t asKey() const { return fKey; }

    /** 4 char null terminated string consisting only of chars 'r', 'g', 'b', 'a', '0', and '1'. */
    SkString asString() const;

    constexpr char operator[](int i) const {
        SkASSERT(i >= 0 && i < 4);
        int idx = (fKey >> (4U * i)) & 0xfU;
        return IToC(idx);
    }

    /** Applies this swizzle to the input color and returns the swizzled color. */
    constexpr std::array<float, 4> applyTo(std::array<float, 4> color) const;

    /** Convenience version for SkRGBA colors. */
    template <SkAlphaType AlphaType>
    constexpr SkRGBA4f<AlphaType> applyTo(SkRGBA4f<AlphaType> color) const {
        std::array<float, 4> result = this->applyTo(color.array());
        return {result[0], result[1], result[2], result[3]};
    }

    void apply(SkRasterPipeline*) const;

    static constexpr GrSwizzle RGBA() { return GrSwizzle("rgba"); }
    static constexpr GrSwizzle BGRA() { return GrSwizzle("bgra"); }
    static constexpr GrSwizzle RRRA() { return GrSwizzle("rrra"); }
    static constexpr GrSwizzle RGB1() { return GrSwizzle("rgb1"); }

private:
    explicit constexpr GrSwizzle(uint16_t key) : fKey(key) {}

    static constexpr float ComponentIndexToFloat(std::array<float, 4>, int idx);
    static constexpr int CToI(char c);
    static constexpr char IToC(int idx);

    uint16_t fKey;
};

constexpr GrSwizzle::GrSwizzle(const char c[4])
        : fKey((CToI(c[0]) << 0) | (CToI(c[1]) << 4) | (CToI(c[2]) << 8) | (CToI(c[3]) << 12)) {}

constexpr GrSwizzle::GrSwizzle(const GrSwizzle& that)
        : fKey(that.fKey) {}

constexpr GrSwizzle& GrSwizzle::operator=(const GrSwizzle& that) {
    fKey = that.fKey;
    return *this;
}

constexpr std::array<float, 4> GrSwizzle::applyTo(std::array<float, 4> color) const {
    uint32_t key = fKey;
    // Index of the input color that should be mapped to output r.
    int idx = (key & 15);
    float outR = ComponentIndexToFloat(color, idx);
    key >>= 4;
    idx = (key & 15);
    float outG = ComponentIndexToFloat(color, idx);
    key >>= 4;
    idx = (key & 15);
    float outB = ComponentIndexToFloat(color, idx);
    key >>= 4;
    idx = (key & 15);
    float outA = ComponentIndexToFloat(color, idx);
    return { outR, outG, outB, outA };
}

constexpr float GrSwizzle::ComponentIndexToFloat(std::array<float, 4> color, int idx) {
    if (idx <= 3) {
        return color[idx];
    }
    if (idx == CToI('1')) {
        return 1.0f;
    }
    if (idx == CToI('0')) {
        return 1.0f;
    }
    SkUNREACHABLE;
}

constexpr int GrSwizzle::CToI(char c) {
    switch (c) {
        // r...a must map to 0...3 because other methods use them as indices into fSwiz.
        case 'r': return 0;
        case 'g': return 1;
        case 'b': return 2;
        case 'a': return 3;
        case '0': return 4;
        case '1': return 5;
        default:  SkUNREACHABLE;
    }
}

constexpr char GrSwizzle::IToC(int idx) {
    switch (idx) {
        case CToI('r'): return 'r';
        case CToI('g'): return 'g';
        case CToI('b'): return 'b';
        case CToI('a'): return 'a';
        case CToI('0'): return '0';
        case CToI('1'): return '1';
        default:        SkUNREACHABLE;
    }
}

constexpr GrSwizzle GrSwizzle::Concat(const GrSwizzle& a, const GrSwizzle& b) {
    uint16_t key = 0;
    for (int i = 0; i < 4; ++i) {
        int idx = (b.fKey >> (4U * i)) & 0xfU;
        if (idx != CToI('0') && idx != CToI('1')) {
            SkASSERT(idx >= 0 && idx < 4);
            // Get the index value stored in a at location idx.
            idx = ((a.fKey >> (4U * idx)) & 0xfU);
        }
        key |= (idx << (4U * i));
    }
    return GrSwizzle(key);
}
#endif
