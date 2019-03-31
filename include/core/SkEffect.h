/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEffect_DEFINED
#define SkEffect_DEFINED

#include "SkBlendMode.h"
#include "SkColor.h"
#include "SkFlattenable.h"

class SkColorMatrix;
struct SkHighContrastConfig;
struct SkStageRec;

enum class SkTileMode {
    kClamp,
    kRepeat,
    kMirror,
    kDecal,

    kLastTileMode = kDecal,
};

struct SkGradientRec {
    SkColorSpace*   fColorSpace;
    const SkColor4f* fColors4f;     // exclusive with fColors
    const SkColor*  fColors;        // exclusive with fColors4f
    const SkScalar* fPos;
    int             fCount;
    SkTileMode      fTiling;
    bool            fInterpInPremul;
};

class SK_API SkEffect : public SkFlattenable {
public:
    //  These are leaf-node effects. Shaders. They take device (x,y) and return a color.
    static sk_sp<SkEffect> Color(SkColor);
    static sk_sp<SkEffect> Color(const SkColor4f&);

    static sk_sp<SkEffect> LinearGradient(const SkGradientRec&, SkPoint p0, SkPoint p1);
    static sk_sp<SkEffect> RadialGradient(const SkGradientRec&, SkPoint center, SkScalar radius);
    static sk_sp<SkEffect> ConicalGradient(const SkGradientRec&, SkPoint p0, SkScalar radius0,
                                           SkPoint p1, SkScalar radius1);
    static sk_sp<SkEffect> SweepGradient(const SkGradientRec&, SkPoint center,
                                         SkScalar startAngle, SkScalar endAngle);

    static sk_sp<SkEffect> PerlinNoise(SkScalar freqX, SkScalar freqY, int numOctaves, SkScalar z);

    // SkImage::makeShader(...)
    // SkPicture::makeShader(...)

    // Modifies the device (x,y) before sending to the effect (shader)
    static sk_sp<SkEffect> LocalMatrix(const SkMatrix&, sk_sp<SkEffect>);

    // These modify the returned color from their effect
    // a null effect is replaced with the paint's color
    static sk_sp<SkEffect> ColorMatrix(const SkColorMatrix&, sk_sp<SkEffect>);
    static sk_sp<SkEffect> HighContrast(const SkHighContrastConfig&, sk_sp<SkEffect>);
    static sk_sp<SkEffect> LumaToAlpha(sk_sp<SkEffect>);
    static sk_sp<SkEffect> Table(const uint8_t table[256], sk_sp<SkEffect>);

    // These combine the returned colors from their effects
    // a null effect is replaced with the paint's color
    static sk_sp<SkEffect> Blend(SkBlendMode, sk_sp<SkEffect>, sk_sp<SkEffect>);
    static sk_sp<SkEffect> LerpT(float t, sk_sp<SkEffect>, sk_sp<SkEffect>);
    static sk_sp<SkEffect> LerpEffect(sk_sp<SkEffect> redChannel, sk_sp<SkEffect>, sk_sp<SkEffect>);

    bool appendStages(const SkStageRec& rec) const;

    static SkFlattenable::Type GetFlattenableType() {
        return kSkEffect_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkEffect_Type;
    }

    static void RegisterFlattenables();

protected:
    virtual bool onAppendStages(const SkStageRec& rec) const = 0;
};

#endif
