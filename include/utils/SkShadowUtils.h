
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkShadowUtils_DEFINED
#define SkShadowUtils_DEFINED

#include "SkColor.h"
#include "SkPoint3.h"
#include "SkScalar.h"
#include "../private/SkShadowFlags.h"

class SkCanvas;
class SkPath;
class SkResourceCache;

class SK_API SkShadowUtils {
public:
    /**
     * Draw an offset spot shadow and outlining ambient shadow for the given path using a disc
     * light. The shadow may be cached, depending on the path type and canvas matrix. If the
     * matrix is perspective or the path is volatile, it will not be cached.
     *
     * @param canvas  The canvas on which to draw the shadows.
     * @param path  The occluder used to generate the shadows.
     * @param zPlaneParams  Values for the plane function which returns the Z offset of the
     *  occluder from the canvas based on local x and y values (the current matrix is not applied).
     * @param lightPos  The 3D position of the light relative to the canvas plane. This is
     *  independent of the canvas's current matrix.
     * @param lightRadius  The radius of the disc light.
     * @param ambientColor  The color of the ambient shadow.
     * @param spotColor  The color of the spot shadow.
     * @param flags  Options controlling opaque occluder optimizations and shadow appearance. See
     *               SkShadowFlags.
     */
    static void DrawShadow(SkCanvas* canvas, const SkPath& path, const SkPoint3& zPlaneParams,
                           const SkPoint3& lightPos, SkScalar lightRadius,
                           SkColor ambientColor, SkColor spotColor,
                           uint32_t flags = SkShadowFlags::kNone_ShadowFlag);

    /**
     * Helper routine to compute color values for one-pass tonal alpha.
     *
     * @param inAmbientColor  Original ambient color
     * @param inSpotColor  Original spot color
     * @param outAmbientColor  Modified ambient color
     * @param outSpotColor  Modified spot color
     */
    static void ComputeTonalColors(SkColor inAmbientColor, SkColor inSpotColor,
                                   SkColor* outAmbientColor, SkColor* outSpotColor);
};

#endif
