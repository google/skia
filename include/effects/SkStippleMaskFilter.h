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
 * Simple MaskFilter that creates a screen door stipple pattern.
 */
class SK_API SkStippleMaskFilter : public SkMaskFilter {
public:
    static SkStippleMaskFilter* Create() {
        return SkNEW(SkStippleMaskFilter);
    }

    virtual bool filterMask(SkMask* dst, const SkMask& src,
                            const SkMatrix& matrix,
                            SkIPoint* margin) const SK_OVERRIDE;

    // getFormat is from SkMaskFilter
    virtual SkMask::Format getFormat() const SK_OVERRIDE {
        return SkMask::kA8_Format;
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkStippleMaskFilter);

protected:
    SkStippleMaskFilter(SkReadBuffer& buffer)
    : SkMaskFilter(buffer) {
    }

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkStippleMaskFilter() : INHERITED() {
    }

private:
    typedef SkMaskFilter INHERITED;
};

#endif // SkStippleMaskFilter_DEFINED
