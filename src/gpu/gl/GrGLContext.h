/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLContext_DEFINED
#define GrGLContext_DEFINED

#include "gl/GrGLExtensions.h"
#include "gl/GrGLInterface.h"
#include "GrGLCaps.h"
#include "GrGLSL.h"
#include "GrGLUtil.h"

#include "SkString.h"

/**
 * Encapsulates information about an OpenGL context including the OpenGL
 * version, the GrGLBinding type of the context, and GLSL version.
 */
class GrGLContextInfo {
public:
    /**
     * Default constructor
     */
    GrGLContextInfo() {
        fGLCaps.reset(SkNEW(GrGLCaps));
        this->reset();
    }

    /**
     * Copies a GrGLContextInfo
     */
    GrGLContextInfo& operator= (const GrGLContextInfo& ctxInfo);

    /**
     * Initializes a GrGLContextInfo from a GrGLInterface and the currently
     * bound OpenGL context accessible by the GrGLInterface.
     */
    bool initialize(const GrGLInterface* interface);
    bool isInitialized() const;

    GrGLBinding binding() const { return fBindingInUse; }
    GrGLVersion version() const { return fGLVersion; }
    GrGLSLGeneration glslGeneration() const { return fGLSLGeneration; }
    GrGLVendor vendor() const { return fVendor; }
    GrGLRenderer renderer() const { return fRenderer; }
    /** Is this a mesa-based driver. Does not mean it is the osmesa software rasterizer. */
    bool isMesa() const { return fIsMesa; }
    const GrGLCaps* caps() const { return fGLCaps.get(); }
    GrGLCaps* caps() { return fGLCaps; }
    const GrGLExtensions& extensions() const { return fExtensions; }

    /**
     * Shortcut for extensions().has(ext);
     */
    bool hasExtension(const char* ext) const {
        if (!this->isInitialized()) {
            return false;
        }
        return fExtensions.has(ext);
    }

    /**
     * Reset the information
     */
    void reset();

private:

    GrGLBinding             fBindingInUse;
    GrGLVersion             fGLVersion;
    GrGLSLGeneration        fGLSLGeneration;
    GrGLVendor              fVendor;
    GrGLRenderer            fRenderer;
    GrGLExtensions          fExtensions;
    bool                    fIsMesa;
    SkAutoTUnref<GrGLCaps>  fGLCaps;
};

/**
 * Encapsulates the GrGLInterface used to make GL calls plus information
 * about the context (via GrGLContextInfo).
 */
class GrGLContext {
public:
    /**
     * Default constructor
     */
    GrGLContext() { this->reset(); }

    /**
     * Creates a GrGLContext from a GrGLInterface and the currently
     * bound OpenGL context accessible by the GrGLInterface.
     */
    explicit GrGLContext(const GrGLInterface* interface);

    /**
     * Copies a GrGLContext
     */
    GrGLContext(const GrGLContext& ctx);

    ~GrGLContext() { SkSafeUnref(fInterface); }

    /**
     * Copies a GrGLContext
     */
    GrGLContext& operator= (const GrGLContext& ctx);

    /**
     * Initializes a GrGLContext from a GrGLInterface and the currently
     * bound OpenGL context accessible by the GrGLInterface.
     */
    bool initialize(const GrGLInterface* interface);
    bool isInitialized() const { return fInfo.isInitialized(); }

    const GrGLInterface* interface() const { return fInterface; }
    const GrGLContextInfo& info() const { return fInfo; }
    GrGLContextInfo& info() { return fInfo; }

private:
    void reset();

    const GrGLInterface* fInterface;
    GrGLContextInfo      fInfo;
};

#endif
