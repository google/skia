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

class SkColorFilter;

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
     *  Applies the blendmode, treating the 1st color as SRC and the 2nd as DST
     */
    static sk_sp<SkMixer> MakeBlend(SkBlendMode);

    /**
     *  Returns a lerp of the two inputs:
     *      C = A*(1 - t) + B*t
     */
    static sk_sp<SkMixer> MakeLerp(float t);

    /**
     *  Returns the 'arithmetic' combination of two colors
     *      C = k1 * A * B + k2 * A + k3 * B + k4
     */
    static sk_sp<SkMixer> MakeArithmetic(float k1, float k2, float k3, float k4);

    /**
     *  Returns a new mixer that applies a colorfilter to the output of this mixer.
     *      C = filter(this(A, B))
     */
    sk_sp<SkMixer> makeFilterOutput(sk_sp<SkColorFilter> filter) const;

    /**
     *  Returns a new mixer that applies a colorfitler to each input color,
     *  and then calls this mixer. If either filter is null, it just returns its input.
     *
     *      C = this(filterA(A), filterB(B))
     *
     *  If filterA is null, then this becomes   C = this(A, filterB(B))
     *  If filterB is null, then this becomes   C = this(filterA(A), B)
     *  If both filters are null, then this just returns this mixer.
     */
    sk_sp<SkMixer> makeFilterInputs(sk_sp<SkColorFilter> filterA,
                                    sk_sp<SkColorFilter> filterB) const;

    /**
     *  Returns a new mixer that invokes this mixer, but with its arguments reversed.
     *      C = original_mixer(B, A)
     */
    sk_sp<SkMixer> makeReverse() const;

    /**
     *  Returns a new mixer that forwards its inputs (A, B) to two other mixers, and then calls
     *  the original mixer with their results.
     *      C = origianl_mixer(ma(A, B), mb(A, B))
     */
    sk_sp<SkMixer> makeSplit(sk_sp<SkMixer> ma, sk_sp<SkMixer> mb) const;
};

#endif
