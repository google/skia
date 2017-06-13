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
#include "GrPipeline.h"

class SkDescriptor;
class SkTypeface;
class GrGpu;
class GrPath;
class GrStencilSettings;
class GrStyle;

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

    typedef GrPathRange::PathIndexType PathIndexType;

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

    // No native support for inverse at this time
    enum FillType {
        /** Specifies that "inside" is computed by a non-zero sum of signed
            edge crossings
        */
        kWinding_FillType,
        /** Specifies that "inside" is computed by an odd number of edge
            crossings
        */
        kEvenOdd_FillType,
    };

    static const GrUserStencilSettings& GetStencilPassSettings(FillType);

    /**
     * Creates a new gpu path, based on the specified path and stroke and returns it.
     *
     * @param SkPath    the geometry.
     * @param GrStyle   the style applied to the path. Styles with non-dash path effects are not
     *                  allowed.
     * @return a new GPU path object.
     */
    virtual sk_sp<GrPath> createPath(const SkPath&, const GrStyle&) = 0;

    /**
     * Creates a range of gpu paths with a common style.
     *
     * @param PathGenerator class that generates SkPath objects for each path in the range.
     * @param GrStyle   the common style applied to each path in the range. Styles with non-dash
     *                  path effects are not allowed.
     * @return a new path range.
     */
    virtual sk_sp<GrPathRange> createPathRange(GrPathRange::PathGenerator*, const GrStyle&) = 0;

    /**
     * Creates a range of glyph paths, indexed by glyph id. The glyphs will have an
     * inverted y-direction in order to match the raw font path data.
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
     * @param GrStyle      Common style that the GPU will apply to every path. Note that
     *                     if the glyph outlines contain baked-in styles from the font
     *                     descriptor, the GPU style will be applied on top of those
     *                     outlines.
     *
     * @return a new path range populated with glyphs.
     */
    sk_sp<GrPathRange> createGlyphs(const SkTypeface*, const SkScalerContextEffects&,
                                    const SkDescriptor*, const GrStyle&);

    /** None of these params are optional, pointers used just to avoid making copies. */
    struct StencilPathArgs {
        StencilPathArgs(bool useHWAA,
                        GrRenderTarget* renderTarget,
                        const SkMatrix* viewMatrix,
                        const GrScissorState* scissor,
                        const GrStencilSettings* stencil)
            : fUseHWAA(useHWAA)
            , fRenderTarget(renderTarget)
            , fViewMatrix(viewMatrix)
            , fScissor(scissor)
            , fStencil(stencil) {
        }
        bool fUseHWAA;
        GrRenderTarget* fRenderTarget;
        const SkMatrix* fViewMatrix;
        const GrScissorState* fScissor;
        const GrStencilSettings* fStencil;
    };

    void stencilPath(const StencilPathArgs& args, const GrPath* path);

    void drawPath(const GrPipeline& pipeline,
                  const GrPrimitiveProcessor& primProc,
                  const GrStencilSettings& stencilPassSettings, // Cover pass settings in pipeline.
                  const GrPath* path);

    void drawPaths(const GrPipeline& pipeline,
                   const GrPrimitiveProcessor& primProc,
                   const GrStencilSettings& stencilPassSettings, // Cover pass settings in pipeline.
                   const GrPathRange* pathRange,
                   const void* indices,
                   PathIndexType indexType,
                   const float transformValues[],
                   PathTransformType transformType,
                   int count);

protected:
    GrPathRendering(GrGpu* gpu) : fGpu(gpu) { }

    virtual void onStencilPath(const StencilPathArgs&, const GrPath*) = 0;
    virtual void onDrawPath(const GrPipeline&,
                            const GrPrimitiveProcessor&,
                            const GrStencilSettings&,
                            const GrPath*) = 0;
    virtual void onDrawPaths(const GrPipeline&,
                             const GrPrimitiveProcessor&,
                             const GrStencilSettings&,
                             const GrPathRange*,
                             const void* indices,
                             PathIndexType,
                             const float transformValues[],
                             PathTransformType,
                             int count) = 0;

    GrGpu* fGpu;
private:
    GrPathRendering& operator=(const GrPathRendering&);
};

#endif
