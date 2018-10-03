/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRGBA4f_DEFINED
#define SkRGBA4f_DEFINED

// This file probably won't work unless #included from SkColor.h.

struct SkPM4f;

template <SkAlphaType kAT>
struct SkRGBA4f {
    float fR;
    float fG;
    float fB;
    float fA;

    bool operator==(const SkRGBA4f& other) const {
        return fA == other.fA && fR == other.fR && fG == other.fG && fB == other.fB;
    }
    bool operator!=(const SkRGBA4f& other) const {
        return !(*this == other);
    }

    SkRGBA4f operator*(float scale) const {
        return { fR * scale, fG * scale, fB * scale, fA * scale };
    }

    SkRGBA4f operator*(const SkRGBA4f& scale) const {
        return { fR * scale.fR, fG * scale.fG, fB * scale.fB, fA * scale.fA };
    }

    const float* vec() const { return &fR; }
          float* vec()       { return &fR; }

    float operator[](int index) const {
        SkASSERT(index >= 0 && index < 4);
        return this->vec()[index];
    }

    float& operator[](int index) {
        SkASSERT(index >= 0 && index < 4);
        return this->vec()[index];
    }

    static SkRGBA4f Pin(float r, float g, float b, float a);  // impl. depends on kAT
    SkRGBA4f pin() const { return Pin(fR, fG, fB, fA); }

    static SkRGBA4f FromColor(SkColor);  // impl. depends on kAT
    SkColor toSkColor() const;  // impl. depends on kAT

    SkRGBA4f<kPremul_SkAlphaType> premul() const {
        static_assert(kAT == kUnpremul_SkAlphaType, "");
        return { fR * fA, fG * fA, fB * fA, fA };
    }

    SkRGBA4f<kUnpremul_SkAlphaType> unpremul() const {
        static_assert(kAT == kPremul_SkAlphaType, "");

        if (fA == 0.0f) {
            return { 0, 0, 0, 0 };
        } else {
            float invAlpha = 1 / fA;
            return { fR * invAlpha, fG * invAlpha, fB * invAlpha, fA };
        }
    }

    // TODO: remove?
    SkPM4f toPM4f() const;  // impl. depends on kAT
};

#endif
