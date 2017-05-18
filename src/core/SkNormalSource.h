/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNormalSource_DEFINED
#define SkNormalSource_DEFINED

#include "SkFlattenable.h"
#include "SkShader.h"

class SkMatrix;
struct SkPoint3;

#if SK_SUPPORT_GPU
class GrFragmentProcessor;
#endif

/** Abstract class that generates or reads in normals for use by SkLightingShader.
*/
class SK_API SkNormalSource : public SkFlattenable {
public:
    virtual ~SkNormalSource() override;

#if SK_SUPPORT_GPU
    /** Returns a fragment processor that takes no input and outputs a normal (already rotated)
        as its output color. To be used as a child fragment processor.
    */
    virtual sk_sp<GrFragmentProcessor> asFragmentProcessor(const SkShader::AsFPArgs&) const = 0;
#endif

    class Provider {
    public:
        virtual ~Provider() {}

        /** Called for each span of the object being drawn on the CPU. Your subclass should set
            the appropriate normals that correspond to the specified device coordinates.
        */
        virtual void fillScanLine(int x, int y, SkPoint3 output[], int count) const = 0;
    };

    /** Returns an instance of 'Provider' that provides normals for the CPU pipeline. The
        necessary data will be initialized in place at 'storage'.
    */
    virtual Provider* asProvider(const SkShader::ContextRec&, SkArenaAlloc*) const = 0;

    /** Returns a normal source that provides normals sourced from the the normal map argument.

        @param  map  a shader that outputs the normal map
        @param  ctm  the current canvas' total matrix, used to rotate normals when necessary.

        nullptr will be returned if 'map' is null

        The normal map is currently assumed to be an 8888 image where the normal at a texel
        is retrieved by:
            N.x = R-127;
            N.y = G-127;
            N.z = B-127;
            N.normalize();
        The +Z axis is thus encoded in RGB as (127, 127, 255) while the -Z axis is
        (127, 127, 0).
    */
    static sk_sp<SkNormalSource> MakeFromNormalMap(sk_sp<SkShader> map, const SkMatrix& ctm);

    /** Returns a normal source that provides straight-up normals only <0, 0, 1>.
    */
    static sk_sp<SkNormalSource> MakeFlat();

    /** This enum specifies the shape of the bevel. All bevels output <0, 0, 1> as the surface
     *  normal for any point more than 'width' away from any edge.
     *
     *  Mathematical details:
     *  For the purpose of describing the shape of the bevel, we define 'w' to be the given width of
     *  the bevel, and 'h' to be the given height. We will assume the shape is rotated such that the
     *  point being shaded as well as the closest point in the shape's edge to that point are in the
     *  x-axis, and the shape is translated so that the aforementioned point in the edge is at
     *  coordinates (w, 0, 0) and the end of the bevel is at (0, 0, h).
     *
     */
    enum class BevelType {
        /* This bevel simulates a surface that is slanted from the shape's edges inwards, linearly.
         *
         * Mathematical details:
         * This bevel follows a straight line from (w, 0, 0) to (0, 0, h).
         */
        kLinear,
        /* This bevel simulates a surface that rounds off at the shape's edges, smoothly becoming
         * perpendicular to the x-y plane.
         *
         * Mathematical details:
         * This bevel follows the only quadratic bezier curve whose start point is at (w, 0, 0),
         * control point is at (w, 0, h), and end point is at (0, 0, h).
         */
        kRoundedOut,
        /* This bevel simulates a surface that sharply becomes perpendicular to the x-y plane when
         * at 'width' units from the nearest edge, and then rounds off towards the shape's
         * edge, smoothly becoming parallel to the x-y plane.
         *
         * Mathematical details:
         * This bevel follows the only quadratic bezier curve whose start point is at (w, 0, 0),
         * control point is at (0, 0, 0), and end point is at (0, 0, h).
         */
        kRoundedIn
    };

    /** Returns a normal source that generates a bevel for the shape being drawn. Currently this is
        not implemented on CPU rendering. On GPU this currently only works for anti-aliased circles
        and rectangles.

        @param  type   the type of bevel to add.
        @param  width  the width of the bevel, in source space. Must be positive.
        @param  height the height of the plateau, in source space. Can be positive, negative,
                       or zero. A negative height means the simulated bevels slope downwards.
    */
    static sk_sp<SkNormalSource> MakeBevel(BevelType, SkScalar width, SkScalar height);

    SK_DEFINE_FLATTENABLE_TYPE(SkNormalSource)
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
