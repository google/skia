/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLPathRendering_DEFINED
#define GrGLPathRendering_DEFINED

#include "SkRefCnt.h"
#include "gl/GrGLFunctions.h"

class GrGLNameAllocator;
struct GrGLInterface;

/**
 * This class wraps the NV_path_rendering extension and manages its various
 * API versions. If a method is not present in the GrGLInterface (because the
 * driver version is old), it tries to provide a backup implementation. But if
 * a backup implementation is not practical, it marks the method as not
 * supported.
 */
class GrGLPathRendering {
public:
    /**
     * Create a new GrGLPathRendering object from a given GL interface. Unless
     * otherwise specified in the caps, every method will work properly, even
     * if it did not exist in the GL interface.
     */
    static GrGLPathRendering* Create(const GrGLInterface*);
    virtual ~GrGLPathRendering();

    /**
     * Mark certain functionality as not supported if the driver version is too
     * old and a backup implementation is not practical.
     */
    struct Caps {
        bool fragmentInputGenSupport : 1;
    };
    const Caps& caps() const { return fCaps; }

    /**
     * Called when the GPU resources have been lost and need to be abandoned
     * (for example after a context loss).
     */
    void abandonGpuResources();

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
    GrGLPathRendering(const GrGLInterface*);

    SkAutoTUnref<const GrGLInterface> fGLInterface;
    SkAutoTDelete<GrGLNameAllocator> fPathNameAllocator;
    Caps fCaps;
};

#endif
