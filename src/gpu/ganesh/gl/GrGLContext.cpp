/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/gl/GrGLContext.h"

#include "include/gpu/ganesh/GrContextOptions.h"
#include "src/gpu/ganesh/gl/GrGLGLSL.h"
#include "src/sksl/SkSLGLSL.h"

#ifdef SK_BUILD_FOR_ANDROID
#include <sys/system_properties.h>
#endif

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrGLContext> GrGLContext::Make(sk_sp<const GrGLInterface> interface,
                                               const GrContextOptions& options) {
    if (!interface->validate()) {
        return nullptr;
    }

    ConstructorArgs args;
    args.fDriverInfo = GrGLGetDriverInfo(interface.get());
    if (args.fDriverInfo.fVersion == GR_GL_INVALID_VER) {
        return nullptr;
    }

    if (!GrGLGetGLSLGeneration(args.fDriverInfo, &args.fGLSLGeneration)) {
        return nullptr;
    }

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
        args.fDriverInfo.fRenderer == GrGLRenderer::kAdreno3xx) {
        char androidAPIVersion[PROP_VALUE_MAX];
        int strLength = __system_property_get("ro.build.version.sdk", androidAPIVersion);
        if (strLength == 0 || atoi(androidAPIVersion) < 26) {
            args.fGLSLGeneration = SkSL::GLSLGeneration::k100es;
        }
    }
#endif

    // Many ES3 drivers only advertise the ES2 image_external extension, but support the _essl3
    // extension, and require that it be enabled to work with ESSL3. Other devices require the ES2
    // extension to be enabled, even when using ESSL3. Some devices appear to only support the ES2
    // extension. As an extreme (optional) solution, we can fallback to using ES2 shading language
    // if we want to prioritize external texture support. skbug.com/40038974
    if (GR_IS_GR_GL_ES(interface->fStandard) &&
        options.fPreferExternalImagesOverES3 &&
        !options.fDisableDriverCorrectnessWorkarounds &&
        interface->hasExtension("GL_OES_EGL_image_external") &&
        args.fGLSLGeneration >= SkSL::GLSLGeneration::k330 &&
        !interface->hasExtension("GL_OES_EGL_image_external_essl3") &&
        !interface->hasExtension("OES_EGL_image_external_essl3")) {
        args.fGLSLGeneration = SkSL::GLSLGeneration::k100es;
    }

    args.fContextOptions = &options;
    args.fInterface = std::move(interface);

    return std::unique_ptr<GrGLContext>(new GrGLContext(std::move(args)));
}

GrGLContext::~GrGLContext() {}

GrGLContextInfo::GrGLContextInfo(ConstructorArgs&& args) {
    fInterface = std::move(args.fInterface);
    fDriverInfo = args.fDriverInfo;
    fGLSLGeneration = args.fGLSLGeneration;

    fGLCaps = sk_make_sp<GrGLCaps>(*args.fContextOptions, *this, fInterface.get());
}
