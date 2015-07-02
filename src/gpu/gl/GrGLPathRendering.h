/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPathRendering_DEFINED
#define GrGLPathRendering_DEFINED

#include "SkRefCnt.h"
#include "GrPathRendering.h"
#include "GrStencil.h"
#include "gl/GrGLFunctions.h"
#include "gl/GrGLProgram.h"

class GrGLNameAllocator;
class GrGLGpu;

/**
 * This class wraps the NV_path_rendering extension and manages its various
 * API versions. If a method is not present in the GrGLInterface of the GrGLGpu
 * (because the driver version is old), it tries to provide a backup
 * implementation. But if a backup implementation is not practical, it marks the
 * method as not supported.
 */
class GrGLPathRendering : public GrPathRendering {
public:
    /**
     * Create a new GrGLPathRendering object from a given GrGLGpu.
     */
    GrGLPathRendering(GrGLGpu* gpu);
    virtual ~GrGLPathRendering();

    // GrPathRendering implementations.
    GrPath* createPath(const SkPath&, const GrStrokeInfo&) override;
    virtual GrPathRange* createPathRange(GrPathRange::PathGenerator*,
                                         const GrStrokeInfo&) override;

    /* Called when the 3D context state is unknown. */
    void resetContext();

    /**
     * Called when the GPU resources have been lost and need to be abandoned
     * (for example after a context loss).
     */
    void abandonGpuResources();

    bool shouldBindFragmentInputs() const {
        return fCaps.bindFragmentInputSupport;
    }

    // Functions for "separable shader" texturing support.
    void setProgramPathFragmentInputTransform(GrGLuint program, GrGLint location,
                                              GrGLenum genMode, GrGLint components,
                                              const SkMatrix&);

    /* Sets the projection matrix for path rendering */
    void setProjectionMatrix(const SkMatrix& matrix,
                             const SkISize& renderTargetSize,
                             GrSurfaceOrigin renderTargetOrigin);

    GrGLuint genPaths(GrGLsizei range);
    GrGLvoid deletePaths(GrGLuint path, GrGLsizei range);

protected:
    void onStencilPath(const StencilPathArgs&, const GrPath*) override;
    void onDrawPath(const DrawPathArgs&, const GrPath*) override;
    void onDrawPaths(const DrawPathArgs&, const GrPathRange*, const void* indices, PathIndexType,
                     const float transformValues[], PathTransformType, int count) override;
private:
    /**
     * Mark certain functionality as not supported.
     */
    struct Caps {
        bool bindFragmentInputSupport : 1;
    };

    void flushPathStencilSettings(const GrStencilSettings&);

    struct MatrixState {
        SkMatrix        fViewMatrix;
        SkISize         fRenderTargetSize;
        GrSurfaceOrigin fRenderTargetOrigin;

        MatrixState() { this->invalidate(); }
        void invalidate() {
            fViewMatrix = SkMatrix::InvalidMatrix();
            fRenderTargetSize.fWidth = -1;
            fRenderTargetSize.fHeight = -1;
            fRenderTargetOrigin = (GrSurfaceOrigin) -1;
        }

        /**
         * Gets a matrix that goes from local coordinates to GL normalized device coords.
         */
        template<int Size> void getRTAdjustedGLMatrix(GrGLfloat* destMatrix) {
            SkMatrix combined;
            if (kBottomLeft_GrSurfaceOrigin == fRenderTargetOrigin) {
                combined.setAll(SkIntToScalar(2) / fRenderTargetSize.fWidth, 0, -SK_Scalar1,
                                0, -SkIntToScalar(2) / fRenderTargetSize.fHeight, SK_Scalar1,
                                0, 0, 1);
            } else {
                combined.setAll(SkIntToScalar(2) / fRenderTargetSize.fWidth, 0, -SK_Scalar1,
                                0, SkIntToScalar(2) / fRenderTargetSize.fHeight, -SK_Scalar1,
                                0, 0, 1);
            }
            combined.preConcat(fViewMatrix);
            GrGLGetMatrix<Size>(destMatrix, combined);
        }
    };
    GrGLGpu* gpu();

    SkAutoTDelete<GrGLNameAllocator> fPathNameAllocator;
    MatrixState fHWProjectionMatrixState;
    GrStencilSettings fHWPathStencilSettings;
    Caps fCaps;
};

#endif
