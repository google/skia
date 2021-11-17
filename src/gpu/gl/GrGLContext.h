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

/**
 * Encapsulates information about an OpenGL context including the OpenGL
 * version, the GrGLStandard type of the context, and GLSL version.
 */
class GrGLContextInfo {
public:
    GrGLContextInfo(GrGLContextInfo&&) = default;
    GrGLContextInfo& operator=(GrGLContextInfo&&) = default;

    virtual ~GrGLContextInfo() {}

    GrGLStandard standard() const { return fInterface->fStandard; }
    GrGLVersion version() const { return fDriverInfo.fVersion; }
    SkSL::GLSLGeneration glslGeneration() const { return fGLSLGeneration; }
    GrGLVendor vendor() const { return fDriverInfo.fVendor; }
    GrGLRenderer renderer() const { return fDriverInfo.fRenderer; }
    GrGLANGLEBackend angleBackend() const { return fDriverInfo.fANGLEBackend; }
    GrGLVendor angleVendor() const { return fDriverInfo.fANGLEVendor; }
    GrGLRenderer angleRenderer() const { return fDriverInfo.fANGLERenderer; }
    /** What driver is running our GL implementation? This is not necessarily related to the vendor.
        (e.g. Intel GPU being driven by Mesa) */
    GrGLDriver driver() const { return fDriverInfo.fDriver; }
    GrGLDriverVersion driverVersion() const { return fDriverInfo.fDriverVersion; }
    bool isOverCommandBuffer() const { return fDriverInfo.fIsOverCommandBuffer; }

    const GrGLCaps* caps() const { return fGLCaps.get(); }
    GrGLCaps* caps() { return fGLCaps.get(); }

    bool hasExtension(const char* ext) const {
        return fInterface->hasExtension(ext);
    }

    const GrGLExtensions& extensions() const { return fInterface->fExtensions; }

    /**
     * Makes a version of this context info that strips the "angle-ness". It will report kUnknown
     * for angleBackend() and report this info's angleRenderer() as renderer() and similiar for
     * driver(), driverVersion(), and vendor().
     */
    GrGLContextInfo makeNonAngle() const;

protected:
    GrGLContextInfo& operator=(const GrGLContextInfo&) = default;
    GrGLContextInfo(const GrGLContextInfo&) = default;

    struct ConstructorArgs {
        sk_sp<const GrGLInterface>          fInterface;
        GrGLDriverInfo                      fDriverInfo;
        SkSL::GLSLGeneration                fGLSLGeneration;
        const  GrContextOptions*            fContextOptions;
    };

    GrGLContextInfo(ConstructorArgs&&);

    sk_sp<const GrGLInterface> fInterface;
    GrGLDriverInfo             fDriverInfo;
    SkSL::GLSLGeneration       fGLSLGeneration;
    sk_sp<GrGLCaps>            fGLCaps;
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
    static std::unique_ptr<GrGLContext> Make(sk_sp<const GrGLInterface>, const GrContextOptions&);

    const GrGLInterface* glInterface() const { return fInterface.get(); }

    ~GrGLContext() override;

private:
    GrGLContext(ConstructorArgs&& args) : INHERITED(std::move(args)) {}

    using INHERITED = GrGLContextInfo;
};

#endif
