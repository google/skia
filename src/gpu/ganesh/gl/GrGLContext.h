/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLContext_DEFINED
#define GrGLContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#include <memory>
#include <utility>

class GrGLExtensions;
struct GrContextOptions;

namespace SkSL {
enum class GLSLGeneration;
}

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
    /**
     * We've accumlated a lot of GL driver workarounds and performance preferences based on vendor
     * and renderer. When we have GL sitting on top of Angle it is not clear which of these are
     * necessary and which are handle by Angle. Thus to be safe we get the underlying GL vendor and
     * renderer from Angle so we can enable these workarounds. It may mean that the same workaround
     * is implemented both in Skia and Angle, but that is better than missing out on one.
     */
    GrGLVendor vendor() const {
        if (this->angleBackend() == GrGLANGLEBackend::kOpenGL) {
            return this->angleVendor();
        } else {
            return fDriverInfo.fVendor;
        }
    }
    GrGLRenderer renderer() const {
        if (this->angleBackend() == GrGLANGLEBackend::kOpenGL) {
            return this->angleRenderer();
        } else {
            return fDriverInfo.fRenderer;
        }
    }
    GrGLANGLEBackend angleBackend() const { return fDriverInfo.fANGLEBackend; }
    GrGLDriver angleDriver() const { return fDriverInfo.fANGLEDriver; }
    GrGLDriverVersion angleDriverVersion() const { return fDriverInfo.fANGLEDriverVersion; }
    GrGLVendor angleVendor() const { return fDriverInfo.fANGLEVendor; }
    GrGLRenderer angleRenderer() const { return fDriverInfo.fANGLERenderer; }

    GrGLVendor webglVendor() const { return fDriverInfo.fWebGLVendor; }
    GrGLRenderer webglRenderer() const { return fDriverInfo.fWebGLRenderer; }

    /** What driver is running our GL implementation? This is not necessarily related to the vendor.
        (e.g. Intel GPU being driven by Mesa) */
    GrGLDriver driver() const { return fDriverInfo.fDriver; }
    GrGLDriverVersion driverVersion() const { return fDriverInfo.fDriverVersion; }
    bool isOverCommandBuffer() const { return fDriverInfo.fIsOverCommandBuffer; }
    bool isRunningOverVirgl() const { return fDriverInfo.fIsRunningOverVirgl; }

    const GrGLCaps* caps() const { return fGLCaps.get(); }
    GrGLCaps* caps() { return fGLCaps.get(); }

    bool hasExtension(const char* ext) const {
        return fInterface->hasExtension(ext);
    }

    const GrGLExtensions& extensions() const { return fInterface->fExtensions; }

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
