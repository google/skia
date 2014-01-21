/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLContext.h"

////////////////////////////////////////////////////////////////////////////////

GrGLContextInfo& GrGLContextInfo::operator= (const GrGLContextInfo& that) {
    fInterface.reset(SkSafeRef(that.fInterface.get()));
    fGLVersion      = that.fGLVersion;
    fGLSLGeneration = that.fGLSLGeneration;
    fVendor         = that.fVendor;
    fRenderer       = that.fRenderer;
    fIsMesa         = that.fIsMesa;
    fIsChromium     = that.fIsChromium;
    *fGLCaps        = *that.fGLCaps.get();
    return *this;
}

bool GrGLContextInfo::initialize(const GrGLInterface* interface) {
    this->reset();
    // We haven't validated the GrGLInterface yet, so check for GetString
    // function pointer
    if (interface->fFunctions.fGetString) {
        const GrGLubyte* verUByte;
        GR_GL_CALL_RET(interface, verUByte, GetString(GR_GL_VERSION));
        const char* ver = reinterpret_cast<const char*>(verUByte);

        const GrGLubyte* rendererUByte;
        GR_GL_CALL_RET(interface, rendererUByte, GetString(GR_GL_RENDERER));
        const char* renderer = reinterpret_cast<const char*>(rendererUByte);

        if (interface->validate()) {

            fGLVersion = GrGLGetVersionFromString(ver);

            fGLSLGeneration = GrGetGLSLGeneration(interface);

            fVendor = GrGLGetVendor(interface);

            fRenderer = GrGLGetRendererFromString(renderer);

            fIsMesa = GrGLIsMesaFromVersionString(ver);

            fIsChromium = GrGLIsChromiumFromRendererString(renderer);

            // This must occur before caps init.
            fInterface.reset(SkRef(interface));

            fGLCaps->init(*this, interface);

            return true;
        }
    }
    return false;
}

bool GrGLContextInfo::isInitialized() const {
    return NULL != fInterface.get();
}

void GrGLContextInfo::reset() {
    fInterface.reset(NULL);
    fGLVersion = GR_GL_VER(0, 0);
    fGLSLGeneration = static_cast<GrGLSLGeneration>(0);
    fVendor = kOther_GrGLVendor;
    fRenderer = kOther_GrGLRenderer;
    fIsMesa = false;
    fIsChromium = false;
    fGLCaps->reset();
}
