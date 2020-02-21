/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLContext_DEFINED
#define GrGLContext_DEFINED

#include "include/gpu/gl/GrGLExtensions.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLCaps.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "src/gpu/glsl/GrGLSL.h"

struct GrContextOptions;
namespace SkSL {
    class Compiler;
}

/**
 * Encapsulates information about an OpenGL context including the OpenGL
 * version, the GrGLStandard type of the context, and GLSL version.
 */
class GrGLContextInfo {
public:
    GrGLContextInfo(const GrGLContextInfo&) = delete;
    GrGLContextInfo& operator=(const GrGLContextInfo&) = delete;

    virtual ~GrGLContextInfo() {}

    GrGLStandard standard() const { return fInterface->fStandard; }
    GrGLVersion version() const { return fGLVersion; }
    GrGLSLGeneration glslGeneration() const { return fGLSLGeneration; }
    GrGLVendor vendor() const { return fVendor; }
    GrGLRenderer renderer() const { return fRenderer; }
    GrGLANGLEBackend angleBackend() const { return fANGLEBackend; }
    GrGLANGLEVendor angleVendor() const { return fANGLEVendor; }
    GrGLANGLERenderer angleRenderer() const { return fANGLERenderer; }
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

protected:
    struct ConstructorArgs {
        sk_sp<const GrGLInterface>          fInterface;
        GrGLVersion                         fGLVersion;
        GrGLSLGeneration                    fGLSLGeneration;
        GrGLVendor                          fVendor;
        GrGLRenderer                        fRenderer;
        GrGLDriver                          fDriver;
        GrGLDriverVersion                   fDriverVersion;
        GrGLANGLEBackend                    fANGLEBackend;
        GrGLANGLEVendor                     fANGLEVendor;
        GrGLANGLERenderer                   fANGLERenderer;
        const  GrContextOptions*            fContextOptions;
    };

    GrGLContextInfo(ConstructorArgs&&);

    sk_sp<const GrGLInterface> fInterface;
    GrGLVersion                fGLVersion;
    GrGLSLGeneration           fGLSLGeneration;
    GrGLVendor                 fVendor;
    GrGLRenderer               fRenderer;
    GrGLDriver                 fDriver;
    GrGLDriverVersion          fDriverVersion;
    GrGLANGLEBackend           fANGLEBackend;
    GrGLANGLEVendor            fANGLEVendor;
    GrGLANGLERenderer          fANGLERenderer;
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
    static std::unique_ptr<GrGLContext> Make(sk_sp<const GrGLInterface>, const GrContextOptions&);

    const GrGLInterface* interface() const { return fInterface.get(); }

    SkSL::Compiler* compiler() const;

    ~GrGLContext() override;

private:
    GrGLContext(ConstructorArgs&& args) : INHERITED(std::move(args)), fCompiler(nullptr) {}

    mutable SkSL::Compiler* fCompiler;

    typedef GrGLContextInfo INHERITED;
};

#endif
