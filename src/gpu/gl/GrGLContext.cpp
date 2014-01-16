/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLContext.h"

////////////////////////////////////////////////////////////////////////////////
GrGLContextInfo& GrGLContextInfo::operator= (const GrGLContextInfo& ctxInfo) {
    fStandard = ctxInfo.fStandard;
    fGLVersion = ctxInfo.fGLVersion;
    fGLSLGeneration = ctxInfo.fGLSLGeneration;
    fVendor = ctxInfo.fVendor;
    fRenderer = ctxInfo.fRenderer;
    fExtensions = ctxInfo.fExtensions;
    fIsMesa = ctxInfo.fIsMesa;
    fIsChromium = ctxInfo.fIsChromium;
    *fGLCaps = *ctxInfo.fGLCaps.get();
    return *this;
}

bool GrGLContextInfo::initialize(const GrGLInterface* interface) {
    this->reset();
    // We haven't validated the GrGLInterface yet, so check for GetString
    // function pointer
    if (interface->fGetString) {
        const GrGLubyte* verUByte;
        GR_GL_CALL_RET(interface, verUByte, GetString(GR_GL_VERSION));
        const char* ver = reinterpret_cast<const char*>(verUByte);

        const GrGLubyte* rendererUByte;
        GR_GL_CALL_RET(interface, rendererUByte, GetString(GR_GL_RENDERER));
        const char* renderer = reinterpret_cast<const char*>(rendererUByte);

        if (interface->validate() && fExtensions.init(interface)) {

            fGLVersion = GrGLGetVersionFromString(ver);

            fGLSLGeneration = GrGetGLSLGeneration(interface);

            fVendor = GrGLGetVendor(interface);

            fRenderer = GrGLGetRendererFromString(renderer);

            fIsMesa = GrGLIsMesaFromVersionString(ver);

            fIsChromium = GrGLIsChromiumFromRendererString(renderer);

            // This must be done before calling GrGLCaps::init()
            fStandard = interface->fStandard;

            fGLCaps->init(*this, interface);

            return true;
        }
    }
    return false;
}

bool GrGLContextInfo::isInitialized() const {
    return kNone_GrGLStandard != fStandard;
}

void GrGLContextInfo::reset() {
    fStandard = kNone_GrGLStandard;
    fGLVersion = GR_GL_VER(0, 0);
    fGLSLGeneration = static_cast<GrGLSLGeneration>(0);
    fVendor = kOther_GrGLVendor;
    fRenderer = kOther_GrGLRenderer;
    fIsMesa = false;
    fIsChromium = false;
    fExtensions.reset();
    fGLCaps->reset();
}

////////////////////////////////////////////////////////////////////////////////
GrGLContext::GrGLContext(const GrGLInterface* interface) {
    fInterface = NULL;
    this->initialize(interface);
}

GrGLContext::GrGLContext(const GrGLContext& ctx) {
    fInterface = NULL;
    *this = ctx;
}

GrGLContext& GrGLContext::operator = (const GrGLContext& ctx) {
    SkRefCnt_SafeAssign(fInterface, ctx.fInterface);
    fInfo = ctx.fInfo;
    return *this;
}

void GrGLContext::reset() {
    SkSafeSetNull(fInterface);
    fInfo.reset();
}

bool GrGLContext::initialize(const GrGLInterface* interface) {
    if (fInfo.initialize(interface)) {
        fInterface = interface;
        interface->ref();
        return true;
    }
    return false;
}
