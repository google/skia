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
class GrGpuGL;

/**
 * This class wraps the NV_path_rendering extension and manages its various
 * API versions. If a method is not present in the GrGLInterface of the GrGpuGL
 * (because the driver version is old), it tries to provide a backup
 * implementation. But if a backup implementation is not practical, it marks the
 * method as not supported.
 */
class GrGLPathRendering : public GrPathRendering {
public:
    /**
     * Create a new GrGLPathRendering object from a given GrGpuGL.
     */
    GrGLPathRendering(GrGpuGL* gpu);
    virtual ~GrGLPathRendering();

    // GrPathRendering implementations.
    virtual GrPath* createPath(const SkPath&, const SkStrokeRec&) SK_OVERRIDE;
    virtual GrPathRange* createPathRange(GrPathRange::PathGenerator*,
                                         const SkStrokeRec&) SK_OVERRIDE;
    virtual GrPathRange* createGlyphs(const SkTypeface*,
                                      const SkDescriptor*,
                                      const SkStrokeRec&) SK_OVERRIDE;
    virtual void stencilPath(const GrPath*, SkPath::FillType) SK_OVERRIDE;
    virtual void drawPath(const GrPath*, SkPath::FillType) SK_OVERRIDE;
    virtual void drawPaths(const GrPathRange*, const uint32_t indices[], int count,
                           const float transforms[], PathTransformType,
                           SkPath::FillType) SK_OVERRIDE;

    /* Called when the 3D context state is unknown. */
    void resetContext();

    /**
     * Called when the GPU resources have been lost and need to be abandoned
     * (for example after a context loss).
     */
    void abandonGpuResources();


    enum TexturingMode {
        FixedFunction_TexturingMode,
        SeparableShaders_TexturingMode
    };

    /** Specifies whether texturing should use fixed fuction pipe or separable shaders
     * Specifies whether texturing should use fixed fuction pipe or whether
     * it is ok to use normal vertex and fragment shaders, and for path rendering
     * populate fragment shaders with setProgramPathFragmentInputTransform.
     * The fixed function mode will be removed once the other mode is more widely
     * available.
     */
    TexturingMode texturingMode() const  {
        return caps().fragmentInputGenSupport ?
            SeparableShaders_TexturingMode : FixedFunction_TexturingMode;
    }

    // Functions for fixed function texturing support.
    enum PathTexGenComponents {
        kS_PathTexGenComponents = 1,
        kST_PathTexGenComponents = 2,
        kSTR_PathTexGenComponents = 3
    };
    void enablePathTexGen(int unitIdx, PathTexGenComponents, const GrGLfloat* coefficients);
    void enablePathTexGen(int unitIdx, PathTexGenComponents, const SkMatrix& matrix);
    void flushPathTexGenSettings(int numUsedTexCoordSets);

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

private:
    /**
     * Mark certain functionality as not supported if the driver version is too
     * old and a backup implementation is not practical.
     */
    struct Caps {
        bool stencilThenCoverSupport : 1;
        bool fragmentInputGenSupport : 1;
        bool glyphLoadingSupport     : 1;
    };
    const Caps& caps() const { return fCaps; }

    void flushPathStencilSettings(SkPath::FillType fill);

    // NV_path_rendering v1.2
    void stencilThenCoverFillPath(GrGLuint path, GrGLenum fillMode,
                                  GrGLuint mask, GrGLenum coverMode);

    void stencilThenCoverStrokePath(GrGLuint path, GrGLint reference,
                                    GrGLuint mask, GrGLenum coverMode);

    void stencilThenCoverFillPathInstanced(
        GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
                         GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode,
                         GrGLenum transformType, const GrGLfloat *transformValues);

    void stencilThenCoverStrokePathInstanced(
                         GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
                         GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode,
                         GrGLenum transformType, const GrGLfloat *transformValues);

    GrGpuGL* fGpu;
    SkAutoTDelete<GrGLNameAllocator> fPathNameAllocator;
    Caps fCaps;
    GrGLProgram::MatrixState fHWProjectionMatrixState;
    GrStencilSettings fHWPathStencilSettings;
    struct PathTexGenData {
        GrGLenum  fMode;
        GrGLint   fNumComponents;
        GrGLfloat fCoefficients[3 * 3];
    };
    int fHWActivePathTexGenSets;
    SkTArray<PathTexGenData, true> fHWPathTexGenSettings;
};

#endif
