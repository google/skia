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

struct GrContextOptions;

/**
 * Encapsulates information about an OpenGL context including the OpenGL
 * version, the GrGLStandard type of the context, and GLSL version.
 */
class GrGLContextInfo : public SkNoncopyable {
public:
    GrGLStandard standard() const { return fInterface->fStandard; }
    GrGLVersion version() const { return fGLVersion; }
    GrGLSLGeneration glslGeneration() const { return fGLSLGeneration; }
    GrGLVendor vendor() const { return fVendor; }
    GrGLRenderer renderer() const { return fRenderer; }
    /** Is this a mesa-based driver. Does not mean it is the osmesa software rasterizer. */
    bool isMesa() const { return fIsMesa; }
    /** Are we running inside Chromium (using the command buffer)? We make some different tradeoffs
        about what errors to check for because queries are synchronous. We should probably expose
        this as an option for clients other than Chromium. */
    bool isChromium() const { return fIsChromium; }
    const GrGLCaps* caps() const { return fGLCaps.get(); }
    GrGLCaps* caps() { return fGLCaps; }
    bool hasExtension(const char* ext) const {
        return fInterface->hasExtension(ext);
    }

    const GrGLExtensions& extensions() const { return fInterface->fExtensions; }

protected:
    struct ConstructorArgs {
        const GrGLInterface*                fInterface;
        GrGLVersion                         fGLVersion;
        GrGLSLGeneration                    fGLSLGeneration;
        GrGLVendor                          fVendor;
        GrGLRenderer                        fRenderer;
        bool                                fIsMesa;
        bool                                fIsChromium;
        const  GrContextOptions*            fContextOptions;
    };

    GrGLContextInfo(const ConstructorArgs& args);

    SkAutoTUnref<const GrGLInterface>   fInterface;
    GrGLVersion                         fGLVersion;
    GrGLSLGeneration                    fGLSLGeneration;
    GrGLVendor                          fVendor;
    GrGLRenderer                        fRenderer;
    bool                                fIsMesa;
    bool                                fIsChromium;
    SkAutoTUnref<GrGLCaps>              fGLCaps;
};

/**
 * Extension of GrGLContextInfo that also provides access to GrGLInterface.
 */
class GrGLContext : public GrGLContextInfo {
public:
    /**
     * Creates a GrGLContext from a GrGLInterface and the currently
     * bound OpenGL context accessible by the GrGLInterface.
     */
    static GrGLContext* Create(const GrGLInterface* interface, const GrContextOptions& options);

    const GrGLInterface* interface() const { return fInterface; }

private:
    GrGLContext(const ConstructorArgs& args) : INHERITED(args) {}

    typedef GrGLContextInfo INHERITED;
};

#endif
