/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkShadowMaskFilter_DEFINED
#define SkShadowMaskFilter_DEFINED

// we include this since our callers will need to at least be able to ref/unref
#include "SkMaskFilter.h"

class SK_API SkShadowMaskFilter {
public:
    /** Create a shadow maskfilter.
    *  @return The new shadow maskfilter
    */
    static sk_sp<SkMaskFilter> Make(SkScalar occluderHeight, const SkPoint3& lightPos,
                                    SkScalar lightRadius, SkScalar ambientAlpha,
                                    SkScalar spotAlpha);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

private:
    SkShadowMaskFilter(); // can't be instantiated
};
#endif
