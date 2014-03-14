/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkComposeImageFilter_DEFINED
#define SkComposeImageFilter_DEFINED

#include "SkImageFilter.h"

class SK_API SkComposeImageFilter : public SkImageFilter {
public:
    virtual ~SkComposeImageFilter();

    static SkComposeImageFilter* Create(SkImageFilter* outer, SkImageFilter* inner) {
        return SkNEW_ARGS(SkComposeImageFilter, (outer, inner));
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeImageFilter)

protected:
    explicit SkComposeImageFilter(SkReadBuffer& buffer);

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkComposeImageFilter(SkImageFilter* outer, SkImageFilter* inner) : INHERITED(outer, inner) {}

private:
    typedef SkImageFilter INHERITED;
};

#endif
