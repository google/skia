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

GrGLContext* GrGLContext::Create(const GrGLInterface* interface, const GrContextOptions& options) {
    // We haven't validated the GrGLInterface yet, so check for GetString function pointer
    if (!interface->fFunctions.fGetString) {
        return nullptr;
    }
    ConstructorArgs args;
    args.fInterface = interface;

    SkDebugf("--------------------------------------------------\n");
    const GrGLubyte* verUByte;
    GR_GL_CALL_RET(interface, verUByte, GetString(GR_GL_VERSION));
    const char* ver = reinterpret_cast<const char*>(verUByte);
    SkDebugf("Version: %s\n", ver);

    const GrGLubyte* rendererUByte;
    GR_GL_CALL_RET(interface, rendererUByte, GetString(GR_GL_RENDERER));
    const char* renderer = reinterpret_cast<const char*>(rendererUByte);
    SkDebugf("Renderer: %s\n", renderer);

    {
        const GrGLubyte* vendorUByte;
        GR_GL_CALL_RET(interface, vendorUByte, GetString(GR_GL_VENDOR));
        const char* vendor =  reinterpret_cast<const char*>(vendorUByte);
        SkDebugf("Vendor: %s\n", vendor);

        const GrGLubyte* langUByte;
        GR_GL_CALL_RET(interface, langUByte, GetString(GR_GL_SHADING_LANGUAGE_VERSION));
        const char* lang =  reinterpret_cast<const char*>(langUByte);
        SkDebugf("Lang: %s\n", lang);

        const GrGLubyte* extensionsUByte;
        GR_GL_CALL_RET(interface, extensionsUByte, GetString(GR_GL_EXTENSIONS));
        const char* extensions =  reinterpret_cast<const char*>(extensionsUByte);
        SkDebugf("Extensions: %s\n", extensions);
    }

    if (!interface->validate()) {
        return nullptr;
    }

    args.fGLVersion = GrGLGetVersionFromString(ver);
    if (GR_GL_INVALID_VER == args.fGLVersion) {
        return nullptr;
    }

    if (!GrGLGetGLSLGeneration(interface, &args.fGLSLGeneration)) {
        return nullptr;
    }

    args.fVendor = GrGLGetVendor(interface);

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

    return new GrGLContext(args);
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

GrGLContextInfo::GrGLContextInfo(const ConstructorArgs& args) {
    fInterface.reset(SkRef(args.fInterface));
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
