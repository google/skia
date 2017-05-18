/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientShader_DEFINED
#define SkGradientShader_DEFINED

#include "SkShader.h"

/** \class SkGradientShader

    SkGradientShader hosts factories for creating subclasses of SkShader that
    render linear and radial gradients.
*/
class SK_API SkGradientShader {
public:
    enum Flags {
        /** By default gradients will interpolate their colors in unpremul space
         *  and then premultiply each of the results. By setting this flag, the
         *  gradients will premultiply their colors first, and then interpolate
         *  between them.
         */
        kInterpolateColorsInPremul_Flag = 1 << 0,
    };

    /** Returns a shader that generates a linear gradient between the two specified points.
        <p />
        @param  pts     The start and end points for the gradient.
        @param  colors  The array[count] of colors, to be distributed between the two points
        @param  pos     May be NULL. array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the start and end point.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >=2. The number of colors (and pos if not NULL) entries.
        @param  mode    The tiling mode
    */
    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2],
                                      const SkColor colors[], const SkScalar pos[], int count,
                                      SkShader::TileMode mode,
                                      uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2],
                                      const SkColor colors[], const SkScalar pos[], int count,
                                      SkShader::TileMode mode) {
        return MakeLinear(pts, colors, pos, count, mode, 0, NULL);
    }

    /** Returns a shader that generates a linear gradient between the two specified points.
        <p />
        @param  pts     The start and end points for the gradient.
        @param  colors  The array[count] of colors, to be distributed between the two points
        @param  pos     May be NULL. array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the start and end point.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >=2. The number of colors (and pos if not NULL) entries.
        @param  mode    The tiling mode
    */
    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2],
                                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                      const SkScalar pos[], int count, SkShader::TileMode mode,
                                      uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeLinear(const SkPoint pts[2],
                                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                      const SkScalar pos[], int count, SkShader::TileMode mode) {
        return MakeLinear(pts, colors, std::move(colorSpace), pos, count, mode, 0, NULL);
    }

    /** Returns a shader that generates a radial gradient given the center and radius.
        <p />
        @param  center  The center of the circle for this gradient
        @param  radius  Must be positive. The radius of the circle for this gradient
        @param  colors  The array[count] of colors, to be distributed between the center and edge of the circle
        @param  pos     May be NULL. The array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the center and edge of the circle.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >= 2. The number of colors (and pos if not NULL) entries
        @param  mode    The tiling mode
    */
    static sk_sp<SkShader> MakeRadial(const SkPoint& center, SkScalar radius,
                                      const SkColor colors[], const SkScalar pos[], int count,
                                      SkShader::TileMode mode,
                                      uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeRadial(const SkPoint& center, SkScalar radius,
                                      const SkColor colors[], const SkScalar pos[], int count,
                                      SkShader::TileMode mode) {
        return MakeRadial(center, radius, colors, pos, count, mode, 0, NULL);
    }

    /** Returns a shader that generates a radial gradient given the center and radius.
        <p />
        @param  center  The center of the circle for this gradient
        @param  radius  Must be positive. The radius of the circle for this gradient
        @param  colors  The array[count] of colors, to be distributed between the center and edge of the circle
        @param  pos     May be NULL. The array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the center and edge of the circle.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >= 2. The number of colors (and pos if not NULL) entries
        @param  mode    The tiling mode
    */
    static sk_sp<SkShader> MakeRadial(const SkPoint& center, SkScalar radius,
                                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                      const SkScalar pos[], int count, SkShader::TileMode mode,
                                      uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeRadial(const SkPoint& center, SkScalar radius,
                                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                      const SkScalar pos[], int count, SkShader::TileMode mode) {
        return MakeRadial(center, radius, colors, std::move(colorSpace), pos, count, mode, 0, NULL);
    }

    /**
     *  Returns a shader that generates a conical gradient given two circles, or
     *  returns NULL if the inputs are invalid. The gradient interprets the
     *  two circles according to the following HTML spec.
     *  http://dev.w3.org/html5/2dcontext/#dom-context-2d-createradialgradient
     */
    static sk_sp<SkShader> MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                               const SkPoint& end, SkScalar endRadius,
                                               const SkColor colors[], const SkScalar pos[],
                                               int count, SkShader::TileMode mode,
                                               uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                               const SkPoint& end, SkScalar endRadius,
                                               const SkColor colors[], const SkScalar pos[],
                                               int count, SkShader::TileMode mode) {
        return MakeTwoPointConical(start, startRadius, end, endRadius, colors, pos, count, mode,
                                   0, NULL);
    }

    /**
     *  Returns a shader that generates a conical gradient given two circles, or
     *  returns NULL if the inputs are invalid. The gradient interprets the
     *  two circles according to the following HTML spec.
     *  http://dev.w3.org/html5/2dcontext/#dom-context-2d-createradialgradient
     */
    static sk_sp<SkShader> MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                               const SkPoint& end, SkScalar endRadius,
                                               const SkColor4f colors[],
                                               sk_sp<SkColorSpace> colorSpace, const SkScalar pos[],
                                               int count, SkShader::TileMode mode,
                                               uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeTwoPointConical(const SkPoint& start, SkScalar startRadius,
                                               const SkPoint& end, SkScalar endRadius,
                                               const SkColor4f colors[],
                                               sk_sp<SkColorSpace> colorSpace, const SkScalar pos[],
                                               int count, SkShader::TileMode mode) {
        return MakeTwoPointConical(start, startRadius, end, endRadius, colors,
                                   std::move(colorSpace), pos, count, mode, 0, NULL);
    }

    /** Returns a shader that generates a sweep gradient given a center.
        <p />
        @param  cx      The X coordinate of the center of the sweep
        @param  cx      The Y coordinate of the center of the sweep
        @param  colors  The array[count] of colors, to be distributed around the center.
        @param  pos     May be NULL. The array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the center and edge of the circle.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >= 2. The number of colors (and pos if not NULL) entries
    */
    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor colors[], const SkScalar pos[], int count,
                                     uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor colors[], const SkScalar pos[], int count) {
        return MakeSweep(cx, cy, colors, pos, count, 0, NULL);
    }

    /** Returns a shader that generates a sweep gradient given a center.
        <p />
        @param  cx      The X coordinate of the center of the sweep
        @param  cx      The Y coordinate of the center of the sweep
        @param  colors  The array[count] of colors, to be distributed around the center.
        @param  pos     May be NULL. The array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the center and edge of the circle.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >= 2. The number of colors (and pos if not NULL) entries
    */
    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                     const SkScalar pos[], int count,
                                     uint32_t flags, const SkMatrix* localMatrix);
    static sk_sp<SkShader> MakeSweep(SkScalar cx, SkScalar cy,
                                     const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                                     const SkScalar pos[], int count) {
        return MakeSweep(cx, cy, colors, std::move(colorSpace), pos, count, 0, NULL);
    }

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
