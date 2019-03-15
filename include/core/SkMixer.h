/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMixer_DEFINED
#define SkMixer_DEFINED

#include "SkBlendMode.h"
#include "SkColor.h"
#include "SkFlattenable.h"

class SkShader;

/**
 *  A Mixer takes two input colors (A and B) and returns a new color (C)
 *     C = mix(A, B)
 *
 *  Mixers can be used to combine multiple other effect objects: shaders, colorfilters, imagefilters
 */
class SK_API SkMixer : public SkFlattenable {
public:
    /**
     *  Returns the first color
     */
    static sk_sp<SkMixer> MakeFirst();

    /**
     *  Returns the second color
     */
    static sk_sp<SkMixer> MakeSecond();

    /**
     *  Returns the specified color, ignoring the input colors.
     */
    static sk_sp<SkMixer> MakeConst(SkColor);
    static sk_sp<SkMixer> MakeConst(const SkColor4f&);

    /**
     *  Applies the blendmode, treating the 1st color as DST and the 2nd as SRC
     *
     *      C = blendmode(dst, src)
     */
    static sk_sp<SkMixer> MakeBlend(SkBlendMode);

    /**
     *  Returns a lerp of the two inputs:
     *      C = A*(1 - t) + B*t
     */
    static sk_sp<SkMixer> MakeLerp(float t);

    /**
     *  Uses the first channel (e.g. Red) of the shader's output as the lerp coefficient.
     */
    static sk_sp<SkMixer> MakeShaderLerp(sk_sp<SkShader>);

    /**
     *  Returns a new mixer that invokes this mixer, but with its arguments reversed.
     *      C = this(B, A)
     */
    sk_sp<SkMixer> makeReverse() const;

    /**
     *  Returns a new mixer that forwards its inputs (A, B) to two other mixers, and then calls
     *  the original mixer with their results.
     *      C = this(ma(A, B), mb(A, B))
     */
    sk_sp<SkMixer> makeMerge(sk_sp<SkMixer> ma, sk_sp<SkMixer> mb) const;
};

#endif
