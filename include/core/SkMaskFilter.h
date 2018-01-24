/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskFilter_DEFINED
#define SkMaskFilter_DEFINED

#include "SkBlendMode.h"
#include "SkFlattenable.h"

class SkString;

/** \class SkMaskFilter

    SkMaskFilter is the base class for object that perform transformations on
    the mask before drawing it. An example subclass is Blur.
*/
class SK_API SkMaskFilter : public SkFlattenable {
public:
    /**
     *  Construct a maskfilter whose effect is to first apply the inner filter and then apply
     *  the outer filter to the result of the inner's. Returns nullptr on failure.
     */
    static sk_sp<SkMaskFilter> MakeCompose(sk_sp<SkMaskFilter> outer, sk_sp<SkMaskFilter> inner);

    /**
     *  Compose two maskfilters together using a blendmode. Returns nullptr on failure.
     */
    static sk_sp<SkMaskFilter> MakeCombine(sk_sp<SkMaskFilter> dst, sk_sp<SkMaskFilter> src,
                                           SkBlendMode);

    SK_TO_STRING_PUREVIRT()
    SK_DEFINE_FLATTENABLE_TYPE(SkMaskFilter)

private:
    static void InitializeFlattenables();
    friend class SkFlattenable;
};

#endif
