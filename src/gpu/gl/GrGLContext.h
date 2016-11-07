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
#include "GrGLUtil.h"

struct GrContextOptions;
namespace SkSL {
    class Compiler;
}

/**
 * Encapsulates information about an OpenGL context including the OpenGL
 * version, the GrGLStandard type of the context, and GLSL version.
 */
class GrGLContextInfo : public SkRefCnt {
public:
    GrGLStandard standard() const { return fInterface->fStandard; }
    GrGLVersion version() const { return fGLVersion; }
    GrGLSLGeneration glslGeneration() const { return fGLSLGeneration; }
    GrGLVendor vendor() const { return fVendor; }
    GrGLRenderer renderer() const { return fRenderer; }
    /** What driver is running our GL implementation? This is not necessarily related to the vendor.
        (e.g. Intel GPU being driven by Mesa) */
    GrGLDriver driver() const { return fDriver; }
    GrGLDriverVersion driverVersion() const { return fDriverVersion; }
    const GrGLCaps* caps() const { return fGLCaps.get(); }
    GrGLCaps* caps() { return fGLCaps.get(); }
    bool hasExtension(const char* ext) const {
        return fInterface->hasExtension(ext);
    }

    const GrGLExtensions& extensions() const { return fInterface->fExtensions; }

    virtual ~GrGLContextInfo() {}

protected:
    struct ConstructorArgs {
        const GrGLInterface*                fInterface;
        GrGLVersion                         fGLVersion;
        GrGLSLGeneration                    fGLSLGeneration;
        GrGLVendor                          fVendor;
        GrGLRenderer                        fRenderer;
        GrGLDriver                          fDriver;
        GrGLDriverVersion                   fDriverVersion;
        const  GrContextOptions*            fContextOptions;
    };

    GrGLContextInfo(const ConstructorArgs& args);

    sk_sp<const GrGLInterface> fInterface;
    GrGLVersion                fGLVersion;
    GrGLSLGeneration           fGLSLGeneration;
    GrGLVendor                 fVendor;
    GrGLRenderer               fRenderer;
    GrGLDriver                 fDriver;
    GrGLDriverVersion          fDriverVersion;
    sk_sp<GrGLCaps>            fGLCaps;
};

/**
 * Extension of GrGLContextInfo that also provides access to GrGLInterface and SkSL::Compiler.
 */
class GrGLContext : public GrGLContextInfo {
public:
    /**
     * Creates a GrGLContext from a GrGLInterface and the currently
     * bound OpenGL context accessible by the GrGLInterface.
     */
    static GrGLContext* Create(const GrGLInterface* interface, const GrContextOptions& options);

    const GrGLInterface* interface() const { return fInterface.get(); }

    SkSL::Compiler* compiler() const;

    ~GrGLContext() override;

private:
    GrGLContext(const ConstructorArgs& args) 
    : INHERITED(args)
    , fCompiler(nullptr) {}

    mutable SkSL::Compiler* fCompiler;

    typedef GrGLContextInfo INHERITED;
};

#endif
