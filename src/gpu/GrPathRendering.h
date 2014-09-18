/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRendering_DEFINED
#define GrPathRendering_DEFINED

#include "SkPath.h"
#include "GrPathRange.h"

class SkStrokeRec;
class SkDescriptor;
class SkTypeface;
class GrPath;
class GrGpu;

/**
 * Abstract class wrapping HW path rendering API.
 *
 * The subclasses of this class use the possible HW API to render paths (as opposed to path
 * rendering implemented in Skia on top of a "3d" HW API).
 * The subclasses hold the global state needed to render paths, including shadow of the global HW
 * API state. Similar to GrGpu.
 *
 * It is expected that the lifetimes of GrGpuXX and GrXXPathRendering are the same. The call context
 * interface (eg.  * the concrete instance of GrGpu subclass) should be provided to the instance
 * during construction.
 */
class GrPathRendering {
public:
    virtual ~GrPathRendering() { }

    enum PathTransformType {
        kNone_PathTransformType,        //!< []
        kTranslateX_PathTransformType,  //!< [kMTransX]
        kTranslateY_PathTransformType,  //!< [kMTransY]
        kTranslate_PathTransformType,   //!< [kMTransX, kMTransY]
        kAffine_PathTransformType,      //!< [kMScaleX, kMSkewX, kMTransX, kMSkewY, kMScaleY, kMTransY]

        kLast_PathTransformType = kAffine_PathTransformType
    };

    static inline int PathTransformSize(PathTransformType type) {
        switch (type) {
            case kNone_PathTransformType:
                return 0;
            case kTranslateX_PathTransformType:
            case kTranslateY_PathTransformType:
                return 1;
            case kTranslate_PathTransformType:
                return 2;
            case kAffine_PathTransformType:
                return 6;

            default:
                SkFAIL("Unknown path transform type");
                return 0;
        }
    }

    /**
     * Creates a new gpu path, based on the specified path and stroke and returns it.
     * The caller owns a ref on the returned path which must be balanced by a call to unref.
     *
     * @param skPath the path geometry.
     * @param stroke the path stroke.
     * @return a new path.
     */
    virtual GrPath* createPath(const SkPath&, const SkStrokeRec&) = 0;

    /**
     * Creates a range of gpu paths with a common stroke. The caller owns a ref on the
     * returned path range which must be balanced by a call to unref.
     *
     * @param PathGenerator class that generates SkPath objects for each path in the range.
     * @param SkStrokeRec   the common stroke applied to each path in the range.
     * @return a new path range.
     */
    virtual GrPathRange* createPathRange(GrPathRange::PathGenerator*, const SkStrokeRec&) = 0;

    /**
     * Creates a range of glyph paths, indexed by glyph id. The glyphs will have an
     * inverted y-direction in order to match the raw font path data. The caller owns
     * a ref on the returned path range which must be balanced by a call to unref.
     *
     * @param SkTypeface   Typeface that defines the glyphs.
     *                     If null, the default typeface will be used.
     *
     * @param SkDescriptor Additional font configuration that specifies the font's size,
     *                     stroke, and other flags. This will generally come from an
     *                     SkGlyphCache.
     *
     *                     It is recommended to leave this value null when possible, in
     *                     which case the glyphs will be loaded directly from the font's
     *                     raw path data and sized at SkPaint::kCanonicalTextSizeForPaths.
     *                     This will result in less memory usage and more efficient paths.
     *
     *                     If non-null, the glyph paths will match the font descriptor,
     *                     including with the stroke information baked directly into
     *                     the outlines.
     *
     * @param SkStrokeRec  Common stroke that the GPU will apply to every path. Note that
     *                     if the glyph outlines contain baked-in strokes from the font
     *                     descriptor, the GPU stroke will be applied on top of those
     *                     outlines.
     *
     * @return a new path range populated with glyphs.
     */
    virtual GrPathRange* createGlyphs(const SkTypeface*, const SkDescriptor*, const SkStrokeRec&) = 0;

    virtual void stencilPath(const GrPath*, SkPath::FillType) = 0;
    virtual void drawPath(const GrPath*, SkPath::FillType) = 0;
    virtual void drawPaths(const GrPathRange*, const uint32_t indices[], int count,
                           const float transforms[], PathTransformType, SkPath::FillType) = 0;
protected:
    GrPathRendering() { }

private:
    GrPathRendering& operator=(const GrPathRendering&);
};

#endif
