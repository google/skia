/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpotShadowMaskFilter_DEFINED
#define SkSpotShadowMaskFilter_DEFINED

#include "SkMaskFilter.h"
#include "SkShadowFlags.h"

/*
 * This filter implements a shadow for an occluding object
 * representing a displaced shadow from a point light.
 */
class SK_API SkSpotShadowMaskFilter {
public:
    /** Create a shadow maskfilter.
     *  @param occluderHeight Height of occluding object off of ground plane.
     *  @param lightPos       Position of the light applied to this object.
     *  @param lightRadius    Radius of the light (light is assumed to be spherical).
     *  @param spotAlpha      Base opacity of the displaced spot shadow.
     *  @param flags          Flags to use - defaults to none
     *  @return The new shadow maskfilter
     */
    static sk_sp<SkMaskFilter> Make(SkScalar occluderHeight, const SkPoint3& lightPos,
                                    SkScalar lightRadius, SkScalar spotAlpha,
                                    uint32_t flags = SkShadowFlags::kNone_ShadowFlag);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

private:
    SkSpotShadowMaskFilter(); // can't be instantiated
};
#endif
