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
     *  Applies the blendmode, treating the 2st color as SRC and the 2nd as DST
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
     *  Returns the first color
     */
    static sk_sp<SkMixer> MakeFirst();

    /**
     *  Returns the second color
     */
    static sk_sp<SkMixer> MakeSecond();

    /**
     *  Reverses the two input colors, and then calls the specified mixer
     */
    static sk_sp<SkMixer> MakeReverse(sk_sp<SkMixer> proxy);

    /**
     *  Forwards its inputs (A, B) to two other mixers, and then combines their results.
     *      C = combiner(ma(A, B), mb(A, B))
     */
    static sk_sp<SkMixer> MakeSplit(sk_sp<SkMixer> ma, sk_sp<SkMixer> mb, sk_sp<SkMixer> combiner);

    /**
     *  Returns a new mixer that applies a colorfilter to the output of this mixer.
     *      C = filter(this(A, B))
     */
    sk_sp<SkMixer> makeFilterOutput(sk_sp<SkColorFilter> filter);

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
    sk_sp<SkMixer> makeFilterInputs(sk_sp<SkColorFilter> filterA, sk_sp<SkColorFilter> filterB);

    // Mostly for testing
    SkColor mix(SkColor a, SkColor b) const;
    SkColor4f mix(const SkColor4f& a, const SkColor4f& b) const;
};

#endif
