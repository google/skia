
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkGradientShader_DEFINED
#define SkGradientShader_DEFINED

#include "SkShader.h"

class SkUnitMapper;

/** \class SkGradientShader

    SkGradientShader hosts factories for creating subclasses of SkShader that
    render linear and radial gradients.
*/
class SK_API SkGradientShader {
public:
    /** Returns a shader that generates a linear gradient between the two
        specified points.
        <p />
        CreateLinear returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for count to be < 2.
        @param  pts The start and end points for the gradient.
        @param  colors  The array[count] of colors, to be distributed between the two points
        @param  pos     May be NULL. array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the start and end point.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >=2. The number of colors (and pos if not NULL) entries. 
        @param  mode    The tiling mode
        @param  mapper  May be NULL. Callback to modify the spread of the colors.
    */
    static SkShader* CreateLinear(  const SkPoint pts[2],
                                    const SkColor colors[], const SkScalar pos[], int count,
                                    SkShader::TileMode mode,
                                    SkUnitMapper* mapper = NULL);

    /** Returns a shader that generates a radial gradient given the center and radius.
        <p />
        CreateRadial returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for colorCount to be < 2, or for radius to be <= 0.
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
        @param  mapper  May be NULL. Callback to modify the spread of the colors.
    */
    static SkShader* CreateRadial(  const SkPoint& center, SkScalar radius,
                                    const SkColor colors[], const SkScalar pos[], int count,
                                    SkShader::TileMode mode,
                                    SkUnitMapper* mapper = NULL);

    /** Returns a shader that generates a radial gradient given the start position, start radius, end position and end radius.
        <p />
        CreateTwoPointRadial returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for colorCount to be < 2, for startRadius or endRadius to be < 0, or for
        startRadius to be equal to endRadius.
        @param  start   The center of the start circle for this gradient
        @param  startRadius  Must be positive.  The radius of the start circle for this gradient.
        @param  end     The center of the end circle for this gradient
        @param  endRadius  Must be positive. The radius of the end circle for this gradient.
        @param  colors  The array[count] of colors, to be distributed between the center and edge of the circle
        @param  pos     May be NULL. The array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the center and edge of the circle.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >= 2. The number of colors (and pos if not NULL) entries
        @param  mode    The tiling mode
        @param  mapper  May be NULL. Callback to modify the spread of the colors.
    */
    static SkShader* CreateTwoPointRadial(const SkPoint& start,
                                          SkScalar startRadius,
                                          const SkPoint& end,
                                          SkScalar endRadius,
                                          const SkColor colors[],
                                          const SkScalar pos[], int count,
                                          SkShader::TileMode mode,
                                          SkUnitMapper* mapper = NULL);
    /** Returns a shader that generates a sweep gradient given a center.
        <p />
        CreateSweep returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for colorCount to be < 2.
        @param  cx      The X coordinate of the center of the sweep
        @param  cx      The Y coordinate of the center of the sweep
        @param  colors  The array[count] of colors, to be distributed around the center.
        @param  pos     May be NULL. The array[count] of SkScalars, or NULL, of the relative position of
                        each corresponding color in the colors array. If this is NULL,
                        the the colors are distributed evenly between the center and edge of the circle.
                        If this is not null, the values must begin with 0, end with 1.0, and
                        intermediate values must be strictly increasing.
        @param  count   Must be >= 2. The number of colors (and pos if not NULL) entries
        @param  mapper  May be NULL. Callback to modify the spread of the colors.
    */
    static SkShader* CreateSweep(SkScalar cx, SkScalar cy,
                                 const SkColor colors[], const SkScalar pos[],
                                 int count, SkUnitMapper* mapper = NULL);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif

