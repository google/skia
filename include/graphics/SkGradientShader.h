/* include/graphics/SkGradientShader.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkGradientShader_DEFINED
#define SkGradientShader_DEFINED

#include "SkShader.h"

class SkUnitMapper;

/** \class SkGradientShader

    SkGradientShader hosts factories for creating subclasses of SkShader that
    render linear and radial gradients.
*/
class SkGradientShader : public SkShader {
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
        @param  count   Must be >=2. The number of colors (and pos if not NULL) entries. 
        @param  mode    The tiling mode
        @param  mapper  May be NULL. Callback to modify the spread of the colors.
    */
    static SkShader* CreateLinear(  const SkPoint pts[2],
                                    const SkColor colors[], const SkScalar pos[], int count,
                                    TileMode mode,
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
        @param  count   Must be >= 2. The number of colors (and pos if not NULL) entries
        @param  mode    The tiling mode
        @param  mapper  May be NULL. Callback to modify the spread of the colors.
    */
    static SkShader* CreateRadial(  const SkPoint& center, SkScalar radius,
                                    const SkColor colors[], const SkScalar pos[], int count,
                                    TileMode mode,
                                    SkUnitMapper* mapper = NULL);
};

#endif

