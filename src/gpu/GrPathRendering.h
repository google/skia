/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRendering_DEFINED
#define GrPathRendering_DEFINED

#include "include/core/SkPath.h"

class GrGpu;
class GrPath;
class GrProgramInfo;
class GrRenderTarget;
class GrRenderTargetProxy;
class GrScissorState;
class GrStencilSettings;
class GrStyle;
struct GrUserStencilSettings;
struct SkScalerContextEffects;
class SkDescriptor;
class SkTypeface;

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
                SK_ABORT("Unknown path transform type");
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

    /** None of these params are optional, pointers used just to avoid making copies. */
    struct StencilPathArgs {
        StencilPathArgs(bool useHWAA,
                        GrRenderTargetProxy* proxy,
                        GrSurfaceOrigin origin,
                        const SkMatrix* viewMatrix,
                        const GrScissorState* scissor,
                        const GrStencilSettings* stencil)
            : fUseHWAA(useHWAA)
            , fProxy(proxy)
            , fOrigin(origin)
            , fViewMatrix(viewMatrix)
            , fScissor(scissor)
            , fStencil(stencil) {
        }
        bool                     fUseHWAA;
        GrRenderTargetProxy*     fProxy;
        GrSurfaceOrigin          fOrigin;
        const SkMatrix*          fViewMatrix;
        const GrScissorState*    fScissor;
        const GrStencilSettings* fStencil;
    };

    void stencilPath(const StencilPathArgs& args, const GrPath* path);

    void drawPath(GrRenderTarget*,
                  const GrProgramInfo&,
                  const GrStencilSettings& stencilPassSettings,  // Cover pass settings in pipeline.
                  const GrPath* path);

protected:
    GrPathRendering(GrGpu* gpu) : fGpu(gpu) { }

    virtual void onStencilPath(const StencilPathArgs&, const GrPath*) = 0;
    virtual void onDrawPath(const GrStencilSettings&, const GrPath*) = 0;

    GrGpu* fGpu;
private:
    GrPathRendering& operator=(const GrPathRendering&);
};

#endif
