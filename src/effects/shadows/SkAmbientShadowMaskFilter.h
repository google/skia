/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAmbientShadowMaskFilter_DEFINED
#define SkAmbientShadowMaskFilter_DEFINED

#include "SkMaskFilter.h"
#include "SkShadowFlags.h"

/*
 * This filter implements a shadow representing ambient occlusion for an occluding object.
 */
class SK_API SkAmbientShadowMaskFilter {
public:
    /** Create a shadow maskfilter.
     *  @param occluderHeight Height of occluding object off of ground plane.
     *  @param ambientAlpha   Base opacity of the ambient occlusion shadow.
     *  @param flags          Flags to use - defaults to none
     *  @return The new shadow maskfilter
     */
    static sk_sp<SkMaskFilter> Make(SkScalar occluderHeight, SkScalar ambientAlpha,
                                    uint32_t flags = SkShadowFlags::kNone_ShadowFlag);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

private:
    SkAmbientShadowMaskFilter(); // can't be instantiated
};
#endif
