/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEmbossMaskFilter_DEFINED
#define SkEmbossMaskFilter_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"

#include <cstdint>

class SkMatrix;
class SkReadBuffer;
class SkWriteBuffer;
struct SkIPoint;

/** \class SkEmbossMaskFilter

    This mask filter creates a 3D emboss look, by specifying a light and blur amount.
*/
class SkEmbossMaskFilter : public SkMaskFilterBase {
public:
    struct Light {
        SkScalar    fDirection[3];  // x,y,z
        uint16_t    fPad;
        uint8_t     fAmbient;
        uint8_t     fSpecular;      // exponent, 4.4 right now
    };

    static sk_sp<SkMaskFilter> Make(SkScalar blurSigma, const Light& light);

    // overrides from SkMaskFilter
    //  This method is not exported to java.
    SkMask::Format getFormat() const override;
    //  This method is not exported to java.
    bool filterMask(SkMaskBuilder* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;
    SkMaskFilterBase::Type type() const override { return SkMaskFilterBase::Type::kEmboss; }

protected:
    SkEmbossMaskFilter(SkScalar blurSigma, const Light& light);
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkEmbossMaskFilter)

    Light       fLight;
    SkScalar    fBlurSigma;

    using INHERITED = SkMaskFilter;
};

#endif
