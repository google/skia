/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShadowFlags_DEFINED
#define SkShadowFlags_DEFINED

// A set of flags shared between the SkAmbientShadowMaskFilter and the SkSpotShadowMaskFilter
enum SkShadowFlags {
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

#endif
