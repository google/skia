
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkShadowUtils_DEFINED
#define SkShadowUtils_DEFINED

#include "SkColor.h"
#include "SkScalar.h"

class SkCanvas;
class SkPath;

class SkShadowUtils {
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

    // Draw an offset spot shadow and outlining ambient shadow for the given path.
    static void DrawShadow(SkCanvas*, const SkPath& path, SkScalar occluderHeight,
                           const SkPoint3& lightPos, SkScalar lightRadius,
                           SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                           uint32_t flags = kNone_ShadowFlag);
};

#endif
