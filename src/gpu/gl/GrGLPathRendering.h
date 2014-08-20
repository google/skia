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
     * Create a new GrGLPathRendering object from a given GrGpuGL. Unless
     * otherwise specified in the caps, every method will work properly, even
     * if it did not exist in the GL interface of the gpu.
     */
    static GrGLPathRendering* Create(GrGpuGL* gpu);
    virtual ~GrGLPathRendering();

    // GrPathRendering implementations.
    virtual GrPath* createPath(const SkPath&, const SkStrokeRec&) SK_OVERRIDE;
    virtual GrPathRange* createPathRange(size_t size, const SkStrokeRec&) SK_OVERRIDE;
    virtual void stencilPath(const GrPath*, SkPath::FillType) SK_OVERRIDE;
    virtual void drawPath(const GrPath*, SkPath::FillType) SK_OVERRIDE;
    virtual void drawPaths(const GrPathRange*, const uint32_t indices[], int count,
                           const float transforms[], PathTransformType,
                           SkPath::FillType) SK_OVERRIDE;

    /**
     * Mark certain functionality as not supported if the driver version is too
     * old and a backup implementation is not practical.
     */
    struct Caps {
        bool fragmentInputGenSupport : 1;
    };
    const Caps& caps() const { return fCaps; }


    /* Called when the 3D context state is unknown. */
    void resetContext();

    /**
     * Called when the GPU resources have been lost and need to be abandoned
     * (for example after a context loss).
     */
    void abandonGpuResources();

    enum PathTexGenComponents {
        kS_PathTexGenComponents = 1,
        kST_PathTexGenComponents = 2,
        kSTR_PathTexGenComponents = 3
    };
    void enablePathTexGen(int unitIdx, PathTexGenComponents, const GrGLfloat* coefficients);
    void enablePathTexGen(int unitIdx, PathTexGenComponents, const SkMatrix& matrix);
    void flushPathTexGenSettings(int numUsedTexCoordSets);
    void setProjectionMatrix(const SkMatrix& matrix,
                             const SkISize& renderTargetSize,
                             GrSurfaceOrigin renderTargetOrigin);


    // NV_path_rendering
    GrGLuint genPaths(GrGLsizei range);
    GrGLvoid deletePaths(GrGLuint path, GrGLsizei range);
    GrGLvoid pathCommands(GrGLuint path, GrGLsizei numCommands, const GrGLubyte *commands,
                          GrGLsizei numCoords, GrGLenum coordType, const GrGLvoid *coords);
    GrGLvoid pathCoords(GrGLuint path, GrGLsizei numCoords,
                        GrGLenum coordType, const GrGLvoid *coords);
    GrGLvoid pathParameteri(GrGLuint path, GrGLenum pname, GrGLint value);
    GrGLvoid pathParameterf(GrGLuint path, GrGLenum pname, GrGLfloat value);
    GrGLboolean isPath(GrGLuint path);
    GrGLvoid pathStencilFunc(GrGLenum func, GrGLint ref, GrGLuint mask);
    GrGLvoid stencilFillPath(GrGLuint path, GrGLenum fillMode, GrGLuint mask);
    GrGLvoid stencilStrokePath(GrGLuint path, GrGLint reference, GrGLuint mask);
    GrGLvoid stencilFillPathInstanced(GrGLsizei numPaths, GrGLenum pathNameType,
                                      const GrGLvoid *paths, GrGLuint pathBase, GrGLenum fillMode,
                                      GrGLuint mask, GrGLenum transformType,
                                      const GrGLfloat *transformValues);
    GrGLvoid stencilStrokePathInstanced(GrGLsizei numPaths, GrGLenum pathNameType,
                                        const GrGLvoid *paths, GrGLuint pathBase,
                                        GrGLint reference, GrGLuint mask, GrGLenum transformType,
                                        const GrGLfloat *transformValues);
    GrGLvoid pathTexGen(GrGLenum texCoordSet, GrGLenum genMode,
                        GrGLint components, const GrGLfloat *coeffs);
    GrGLvoid coverFillPath(GrGLuint path, GrGLenum coverMode);
    GrGLvoid coverStrokePath(GrGLuint name, GrGLenum coverMode);
    GrGLvoid coverFillPathInstanced(GrGLsizei numPaths, GrGLenum pathNameType,
                                    const GrGLvoid *paths, GrGLuint pathBase, GrGLenum coverMode,
                                    GrGLenum transformType, const GrGLfloat *transformValues);
    GrGLvoid coverStrokePathInstanced(GrGLsizei numPaths, GrGLenum pathNameType,
                                      const GrGLvoid *paths, GrGLuint pathBase, GrGLenum coverMode,
                                      GrGLenum transformType, const GrGLfloat* transformValues);

    // NV_path_rendering v1.2
    virtual GrGLvoid stencilThenCoverFillPath(GrGLuint path, GrGLenum fillMode,
                                              GrGLuint mask, GrGLenum coverMode);
    virtual GrGLvoid stencilThenCoverStrokePath(GrGLuint path, GrGLint reference,
                                                GrGLuint mask, GrGLenum coverMode);
    virtual GrGLvoid stencilThenCoverFillPathInstanced(
                         GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
                         GrGLuint pathBase, GrGLenum fillMode, GrGLuint mask, GrGLenum coverMode,
                         GrGLenum transformType, const GrGLfloat *transformValues);
    virtual GrGLvoid stencilThenCoverStrokePathInstanced(
                         GrGLsizei numPaths, GrGLenum pathNameType, const GrGLvoid *paths,
                         GrGLuint pathBase, GrGLint reference, GrGLuint mask, GrGLenum coverMode,
                         GrGLenum transformType, const GrGLfloat *transformValues);

    // NV_path_rendering v1.3
    virtual GrGLvoid programPathFragmentInputGen(GrGLuint program, GrGLint location,
                                                 GrGLenum genMode, GrGLint components,
                                                 const GrGLfloat *coeffs);

protected:
    GrGLPathRendering(GrGpuGL* gpu);

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

private:
    void flushPathStencilSettings(SkPath::FillType fill);

};

#endif
