/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLContext.h"
#include "GrGLGLSL.h"
#include "SkSLCompiler.h"

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrGLContext> GrGLContext::Make(sk_sp<const GrGLInterface> interface,
                                               const GrContextOptions& options) {
    if (!interface->validate()) {
        return nullptr;
    }

    const GrGLubyte* verUByte;
    GR_GL_CALL_RET(interface.get(), verUByte, GetString(GR_GL_VERSION));
    const char* ver = reinterpret_cast<const char*>(verUByte);

    const GrGLubyte* rendererUByte;
    GR_GL_CALL_RET(interface.get(), rendererUByte, GetString(GR_GL_RENDERER));
    const char* renderer = reinterpret_cast<const char*>(rendererUByte);

    ConstructorArgs args;
    args.fGLVersion = GrGLGetVersionFromString(ver);
    if (GR_GL_INVALID_VER == args.fGLVersion) {
        return nullptr;
    }

    if (!GrGLGetGLSLGeneration(interface.get(), &args.fGLSLGeneration)) {
        return nullptr;
    }

    args.fVendor = GrGLGetVendor(interface.get());

    args.fRenderer = GrGLGetRendererFromString(renderer);

    GrGLGetANGLEInfoFromString(renderer, &args.fANGLEBackend, &args.fANGLEVendor,
                               &args.fANGLERenderer);
    /*
     * Qualcomm drivers for the 3xx series have a horrendous bug with some drivers. Though they
     * claim to support GLES 3.00, some perfectly valid GLSL300 shaders will only compile with
     * #version 100, and will fail to compile with #version 300 es.  In the long term, we
     * need to lock this down to a specific driver version.
     * ?????/2015 - This bug is still present in Lollipop pre-mr1
     * 06/18/2015 - This bug does not affect the nexus 6 (which has an Adreno 4xx).
     */
    if (kAdreno3xx_GrGLRenderer == args.fRenderer) {
        args.fGLSLGeneration = k110_GrGLSLGeneration;
    }

    GrGLGetDriverInfo(interface->fStandard, args.fVendor, renderer, ver,
                      &args.fDriver, &args.fDriverVersion);

    args.fContextOptions = &options;
    args.fInterface = std::move(interface);

    return std::unique_ptr<GrGLContext>(new GrGLContext(std::move(args)));
}

GrGLContext::~GrGLContext() {
    delete fCompiler;
}

SkSL::Compiler* GrGLContext::compiler() const {
    if (!fCompiler) {
        fCompiler = new SkSL::Compiler();
    }
    return fCompiler;
}

GrGLContextInfo::GrGLContextInfo(ConstructorArgs&& args) {
    fInterface = std::move(args.fInterface);
    fGLVersion = args.fGLVersion;
    fGLSLGeneration = args.fGLSLGeneration;
    fVendor = args.fVendor;
    fRenderer = args.fRenderer;
    fDriver = args.fDriver;
    fDriverVersion = args.fDriverVersion;
    fANGLEBackend = args.fANGLEBackend;
    fANGLEVendor = args.fANGLEVendor;
    fANGLERenderer = args.fANGLERenderer;

    fGLCaps = sk_make_sp<GrGLCaps>(*args.fContextOptions, *this, fInterface.get());
}
