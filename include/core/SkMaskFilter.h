/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskFilter_DEFINED
#define SkMaskFilter_DEFINED

#include "include/core/SkBlurTypes.h"
#include "include/core/SkCoverageMode.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkScalar.h"

class SkMatrix;
struct SkRect;
class SkString;

/** \class SkMaskFilter

    SkMaskFilter is the base class for object that perform transformations on
    the mask before drawing it. An example subclass is Blur.
*/
class SK_API SkMaskFilter : public SkFlattenable {
public:
    /** Create a blur maskfilter.
     *  @param style      The SkBlurStyle to use
     *  @param sigma      Standard deviation of the Gaussian blur to apply. Must be > 0.
     *  @param respectCTM if true the blur's sigma is modified by the CTM.
     *  @return The new blur maskfilter
     */
    static sk_sp<SkMaskFilter> MakeBlur(SkBlurStyle style, SkScalar sigma,
                                        bool respectCTM = true);

    /**
     *  Construct a maskfilter whose effect is to first apply the inner filter and then apply
     *  the outer filter to the result of the inner's. Returns nullptr on failure.
     */
    static sk_sp<SkMaskFilter> MakeCompose(sk_sp<SkMaskFilter> outer, sk_sp<SkMaskFilter> inner);

    /**
     *  Compose two maskfilters together using a coverage mode. Returns nullptr on failure.
     */
    static sk_sp<SkMaskFilter> MakeCombine(sk_sp<SkMaskFilter> filterA, sk_sp<SkMaskFilter> filterB,
                                           SkCoverageMode mode);

    /**
     *  Construct a maskfilter with an additional transform.
     *
     *  Note: unlike shader local matrices, this transform composes next to the CTM.
     *
     *    TotalMatrix = CTM x MaskFilterMatrix x (optional/downstream) ShaderLocalMatrix
     */
    sk_sp<SkMaskFilter> makeWithMatrix(const SkMatrix&) const;

    static SkFlattenable::Type GetFlattenableType() {
        return kSkMaskFilter_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkMaskFilter_Type;
    }

    static sk_sp<SkMaskFilter> Deserialize(const void* data, size_t size,
                                          const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkMaskFilter>(static_cast<SkMaskFilter*>(
                                  SkFlattenable::Deserialize(
                                  kSkMaskFilter_Type, data, size, procs).release()));
    }

private:
    static void RegisterFlattenables();
    friend class SkFlattenable;
};

#endif
