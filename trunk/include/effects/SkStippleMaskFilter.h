/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStippleMaskFilter_DEFINED
#define SkStippleMaskFilter_DEFINED

#include "SkMaskFilter.h"

/**
 * Simple MaskFilter that creates a screen door stipple pattern
 */
class SkStippleMaskFilter : public SkMaskFilter {
public:
    SkStippleMaskFilter() : INHERITED() {
    }

    virtual bool filterMask(SkMask* dst, const SkMask& src,
                            const SkMatrix& matrix,
                            SkIPoint* margin) SK_OVERRIDE;

    // getFormat is from SkMaskFilter
    virtual SkMask::Format getFormat() SK_OVERRIDE {
        return SkMask::kA8_Format;
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkStippleMaskFilter);

protected:
    SkStippleMaskFilter(SkFlattenableReadBuffer& buffer)
    : SkMaskFilter(buffer) {
    }

private:
    typedef SkMaskFilter INHERITED;
};

#endif // SkStippleMaskFilter_DEFINED
