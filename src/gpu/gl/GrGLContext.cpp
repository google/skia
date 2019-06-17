/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLContext.h"
#include "src/gpu/gl/GrGLGLSL.h"
#include "src/sksl/SkSLCompiler.h"

#ifdef SK_BUILD_FOR_ANDROID
#include <sys/system_properties.h>
#endif

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

    args.fRenderer = GrGLGetRendererFromStrings(renderer, interface->fExtensions);

    GrGLGetANGLEInfoFromString(renderer, &args.fANGLEBackend, &args.fANGLEVendor,
                               &args.fANGLERenderer);

    /*
     * Qualcomm drivers for the 3xx series have a horrendous bug with some drivers. Though they
     * claim to support GLES 3.00, some perfectly valid GLSL300 shaders will only compile with
     * #version 100, and will fail to compile with #version 300 es.  In the long term, we
     * need to lock this down to a specific driver version.
     * ?????/2019 - Qualcomm has fixed this for Android O+ devices (API 26+)
     * ?????/2015 - This bug is still present in Lollipop pre-mr1
     * 06/18/2015 - This bug does not affect the nexus 6 (which has an Adreno 4xx).
     */
#ifdef SK_BUILD_FOR_ANDROID
    if (!options.fDisableDriverCorrectnessWorkarounds &&
        kAdreno3xx_GrGLRenderer == args.fRenderer) {
        char androidAPIVersion[PROP_VALUE_MAX];
        int strLength = __system_property_get("ro.build.version.release", androidAPIVersion);
        if (strLength == 0 || atoi(androidAPIVersion) < 26) {
            args.fGLSLGeneration = k110_GrGLSLGeneration;
        }
    }
#endif

    // Many ES3 drivers only advertise the ES2 image_external extension, but support the _essl3
    // extension, and require that it be enabled to work with ESSL3. Other devices require the ES2
    // extension to be enabled, even when using ESSL3. Some devices appear to only support the ES2
    // extension. As an extreme (optional) solution, we can fallback to using ES2 shading language
    // if we want to prioritize external texture support. skbug.com/7713
    if (GR_IS_GR_GL_ES(interface->fStandard) &&
        options.fPreferExternalImagesOverES3 &&
        !options.fDisableDriverCorrectnessWorkarounds &&
        interface->hasExtension("GL_OES_EGL_image_external") &&
        args.fGLSLGeneration >= k330_GrGLSLGeneration &&
        !interface->hasExtension("GL_OES_EGL_image_external_essl3") &&
        !interface->hasExtension("OES_EGL_image_external_essl3")) {
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
