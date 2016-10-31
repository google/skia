/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadowMaskFilter_DEFINED
#define SkShadowMaskFilter_DEFINED

#include "SkMaskFilter.h"


/*
 * This filter implements a pair of shadows for an occluding object-- one representing
 * ambient occlusion, and one representing a displaced shadow from a point light.
 */
class SK_API SkShadowMaskFilter {
public:
    enum ShadowFlags {
        kNone_ShadowFlag = 0x00,
        /** The occluding object is not opaque. Knowing that the occluder is opaque allows
          * us to cull shadow geometry behind it and improve performance. */
        kTransparentOccluder_ShadowFlag = 0x01,
        /** Use a larger umbra for a darker shadow */
        kLargerUmbra_ShadowFlag = 0x02,
        /** Use a Gaussian for the edge function rather than smoothstep */
        kGaussianEdge_ShadowFlag = 0x04,
        /** mask for all shadow flags */
        kAll_ShadowFlag = 0x07
    };

    /** Create a shadow maskfilter.
     *  @param occluderHeight Height of occluding object off of ground plane.
     *  @param lightPos       Position of the light applied to this object.
     *  @param lightRadius    Radius of the light (light is assumed to be spherical).
     *  @param ambientAlpha   Base opacity of the ambient occlusion shadow.
     *  @param spotAlpha      Base opacity of the displaced spot shadow.
     *  @param flags          Flags to use - defaults to none
     *  @return The new shadow maskfilter
     */
    static sk_sp<SkMaskFilter> Make(SkScalar occluderHeight, const SkPoint3& lightPos,
                                    SkScalar lightRadius, SkScalar ambientAlpha,
                                    SkScalar spotAlpha, uint32_t flags = kNone_ShadowFlag);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

private:
    SkShadowMaskFilter(); // can't be instantiated
};
#endif
