/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSDFMaskFilter_DEFINED
#define SkSDFMaskFilter_DEFINED

#include "SkMaskFilterBase.h"

/** \class SkSDFMaskFilter

    This mask filter converts an alpha mask to a signed distance field mask
*/
class SK_API SkSDFMaskFilter : public SkMaskFilterBase {
public:
    static sk_sp<SkMaskFilter> Make();

    // overrides from SkMaskFilter
    //  This method is not exported to java.
    SkMask::Format getFormat() const override;
    //  This method is not exported to java.
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

    void toString(SkString* str) const override;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSDFMaskFilter)

protected:
    SkSDFMaskFilter();
    void flatten(SkWriteBuffer&) const override;

private:
    typedef SkMaskFilter INHERITED;
};

#endif
