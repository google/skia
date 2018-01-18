
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
     * Deprecated version that uses one color and two alphas.
     *
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
     * @param ambientAlpha  The maximum alpha of the ambient shadow.
     * @param spotAlpha  The maxium alpha of the spot shadow.
     * @param color  The shadow color.
     * @param flags  Options controlling opaque occluder optimizations and shadow appearance. See
     *               SkShadowFlags.
     */
    static void DrawShadow(SkCanvas* canvas, const SkPath& path, const SkPoint3& zPlane,
                           const SkPoint3& lightPos, SkScalar lightRadius, SkScalar ambientAlpha,
                           SkScalar spotAlpha, SkColor color,
                           uint32_t flags = SkShadowFlags::kNone_ShadowFlag) {
        SkColor ambientColor;
        SkColor spotColor;
        if (flags & SkShadowFlags::kDisableTonalColor_ShadowFlag) {
            ambientColor = SkColorSetARGB(ambientAlpha*SkColorGetA(color), SkColorGetR(color),
                                          SkColorGetG(color), SkColorGetB(color));
            spotColor = SkColorSetARGB(spotAlpha*SkColorGetA(color), SkColorGetR(color),
                                       SkColorGetG(color), SkColorGetB(color));
        } else {
            SkColor4f color4f = SkColor4f::FromColor(color);
            SkColor4f ambient4f(color4f);
            ambient4f.fA *= ambientAlpha; // TODO: multiply entire color?
            SkColor4f spot4f(color4f);
            spot4f.fA *= spotAlpha;
            ComputeTonalColors(ambient4f, spot4f, &ambientColor, &spotColor);
        }

        DrawShadow(canvas, path, zPlane, lightPos, lightRadius, ambientColor, spotColor, flags);
    }

   /**
    * Deprecated version with height value (to be removed when Flutter is updated).
    *
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
    */
    static void DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                           const SkPoint3& lightPos, SkScalar lightRadius, SkScalar ambientAlpha,
                           SkScalar spotAlpha, SkColor color,
                           uint32_t flags = SkShadowFlags::kNone_ShadowFlag) {
        SkPoint3 zPlane = SkPoint3::Make(0, 0, occluderHeight);
        DrawShadow(canvas, path, zPlane, lightPos, lightRadius, ambientAlpha, spotAlpha,
                   color, flags);
    }

   /**
    * Helper routine to compute color values for one-pass tonal alpha.
    *
    * @param ambient4f  Original ambient color
    * @param spot4f  Original spot color
    * @param ambientColor  Modified ambient color
    * @param spotColor  Modified spot color
    */
    static void ComputeTonalColors(const SkColor4f& ambient4f, const SkColor4f& spot4f,
                                   SkColor* ambientColor, SkColor* spotColor);
};

#endif
