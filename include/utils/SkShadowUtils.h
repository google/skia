
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
#include "../private/SkShadowFlags.h"
#include <functional>

class SkCanvas;
class SkPath;
class SkResourceCache;

class SkShadowUtils {
public:
    /**
     * Draw an offset spot shadow and outlining ambient shadow for the given path using a disc
     * light.
     *
     * @param canvas  The canvas on which to draw the shadows.
     * @param path  The occluder used to generate the shadows.
     * @param occluderHeight  The vertical offset of the occluder from the canvas. This is
     *  independent of the canvas's current matrix.
     * @param lightPos  The 3D position of the light relative to the canvas plane. This is
     *  independent of the canvas's current matrix.
     * @param lightRadius  The radius of the disc light.
     * @param ambientAlpha  The maximum alpha of the ambient shadow.
     * @param spotAlpha  The maxium alpha of the spot shadow.
     * @param color  The shadow color.
     * @param flags  Options controlling opaque occluder optimizations and shadow appearance. See
     *               SkShadowFlags.
     * @param cache  Used for testing purposes. Clients should pass nullptr (default).
     */
    static void DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                           const SkPoint3& lightPos, SkScalar lightRadius, SkScalar ambientAlpha,
                           SkScalar spotAlpha, SkColor color,
                           uint32_t flags = SkShadowFlags::kNone_ShadowFlag,
                           SkResourceCache* cache = nullptr);

   /**
    * Draw an offset spot shadow and outlining ambient shadow for the given path using a disc
    * light. Takes a function to vary the z value based on the transformed x and y position.
    * This shadow will not be cached, as the assumption is that this will be used for animation.
    *
    * @param canvas  The canvas on which to draw the shadows.
    * @param path  The occluder used to generate the shadows.
    * @param heightFunc  A function which returns the vertical offset of the occluder from the
    *  canvas based on local x and y values (the current matrix is not applied).
    * @param lightPos  The 3D position of the light relative to the canvas plane. This is
    *  independent of the canvas's current matrix.
    * @param lightRadius  The radius of the disc light.
    * @param ambientAlpha  The maximum alpha of the ambient shadow.
    * @param spotAlpha  The maxium alpha of the spot shadow.
    * @param color  The shadow color.
    * @param flags  Options controlling opaque occluder optimizations and shadow appearance. See
    *               SkShadowFlags.
    */
    static void DrawUncachedShadow(SkCanvas* canvas, const SkPath& path,
                                   std::function<SkScalar(SkScalar, SkScalar)> heightFunc,
                                   const SkPoint3& lightPos, SkScalar lightRadius,
                                   SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                                   uint32_t flags = SkShadowFlags::kNone_ShadowFlag);
};

#endif
