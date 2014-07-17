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
 * version, the GrGLStandard type of the context, and GLSL version.
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

    GrGLContextInfo(const GrGLContextInfo& that) {
        fGLCaps.reset(SkNEW(GrGLCaps));
        *this = that;
    }

    GrGLContextInfo& operator= (const GrGLContextInfo&);

    /**
     * Initializes a GrGLContextInfo from a GrGLInterface and the currently
     * bound OpenGL context accessible by the GrGLInterface.
     */
    bool initialize(const GrGLInterface* interface);
    bool isInitialized() const;

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
        if (!this->isInitialized()) {
            return false;
        }
        return fInterface->hasExtension(ext);
    }

    const GrGLExtensions& extensions() const { return fInterface->fExtensions; }

    /**
     * Reset the information
     */
    void reset();

protected:
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
    explicit GrGLContext(const GrGLInterface* interface) {
        this->initialize(interface);
    }

    GrGLContext(const GrGLContext& that) : INHERITED(that) {}

    GrGLContext& operator= (const GrGLContext& that) {
        this->INHERITED::operator=(that);
        return *this;
    }

    const GrGLInterface* interface() const { return fInterface.get(); }

private:
    typedef GrGLContextInfo INHERITED;
};

#endif
