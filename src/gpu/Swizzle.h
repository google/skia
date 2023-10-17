/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Swizzle_DEFINED
#define skgpu_Swizzle_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkString.h"

class SkRasterPipeline;

namespace skgpu {

/** Represents a rgba swizzle. It can be converted either into a string or a eight bit int. */
class Swizzle {
public:
    // Equivalent to "rgba", but Clang doesn't always manage to inline this
    // if we're too deep in the inlining already.
    constexpr Swizzle() : Swizzle(0x3210) {}
    explicit constexpr Swizzle(const char c[4]);

    constexpr Swizzle(const Swizzle&) = default;
    constexpr Swizzle& operator=(const Swizzle& that) = default;

    static constexpr Swizzle Concat(const Swizzle& a, const Swizzle& b);

    constexpr bool operator==(const Swizzle& that) const { return fKey == that.fKey; }
    constexpr bool operator!=(const Swizzle& that) const { return !(*this == that); }

    /** Compact representation of the swizzle suitable for a key. */
    constexpr uint16_t asKey() const { return fKey; }

    /** 4 char null terminated string consisting only of chars 'r', 'g', 'b', 'a', '0', and '1'. */
    SkString asString() const;

    constexpr char operator[](int i) const {
        SkASSERT(i >= 0 && i < 4);
        int idx = (fKey >> (4 * i)) & 0xfU;
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

    static constexpr Swizzle RGBA() { return Swizzle("rgba"); }
    static constexpr Swizzle BGRA() { return Swizzle("bgra"); }
    static constexpr Swizzle RRRA() { return Swizzle("rrra"); }
    static constexpr Swizzle RGB1() { return Swizzle("rgb1"); }

    using sk_is_trivially_relocatable = std::true_type;

private:
    explicit constexpr Swizzle(uint16_t key) : fKey(key) {}

    static constexpr float ComponentIndexToFloat(std::array<float, 4>, size_t idx);
    static constexpr int CToI(char c);
    static constexpr char IToC(int idx);

    uint16_t fKey;

    static_assert(::sk_is_trivially_relocatable<decltype(fKey)>::value);
};

constexpr Swizzle::Swizzle(const char c[4])
        : fKey(static_cast<uint16_t>((CToI(c[0]) << 0) | (CToI(c[1]) << 4) | (CToI(c[2]) << 8) |
                                     (CToI(c[3]) << 12))) {}

constexpr std::array<float, 4> Swizzle::applyTo(std::array<float, 4> color) const {
    uint32_t key = fKey;
    // Index of the input color that should be mapped to output r.
    size_t idx = (key & 15);
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

constexpr float Swizzle::ComponentIndexToFloat(std::array<float, 4> color, size_t idx) {
    if (idx <= 3) {
        return color[idx];
    }
    if (idx == static_cast<size_t>(CToI('1'))) {
        return 1.0f;
    }
    if (idx == static_cast<size_t>(CToI('0'))) {
        return 0.0f;
    }
    SkUNREACHABLE;
}

constexpr int Swizzle::CToI(char c) {
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

constexpr char Swizzle::IToC(int idx) {
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

constexpr Swizzle Swizzle::Concat(const Swizzle& a, const Swizzle& b) {
    uint16_t key = 0;
    for (unsigned i = 0; i < 4; ++i) {
        int idx = (b.fKey >> (4U * i)) & 0xfU;
        if (idx != CToI('0') && idx != CToI('1')) {
            SkASSERT(idx >= 0 && idx < 4);
            // Get the index value stored in a at location idx.
            idx = ((a.fKey >> (4 * idx)) & 0xfU);
        }
        key |= (idx << (4U * i));
    }
    return Swizzle(key);
}

} // namespace skgpu
#endif
