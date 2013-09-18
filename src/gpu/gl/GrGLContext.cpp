/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLContext.h"

////////////////////////////////////////////////////////////////////////////////
GrGLContextInfo& GrGLContextInfo::operator= (const GrGLContextInfo& ctxInfo) {
    fBindingInUse = ctxInfo.fBindingInUse;
    fGLVersion = ctxInfo.fGLVersion;
    fGLSLGeneration = ctxInfo.fGLSLGeneration;
    fVendor = ctxInfo.fVendor;
    fRenderer = ctxInfo.fRenderer;
    fExtensions = ctxInfo.fExtensions;
    fIsMesa = ctxInfo.fIsMesa;
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
        GrGLBinding binding = GrGLGetBindingInUseFromString(ver);

        if (0 != binding && interface->validate(binding) && fExtensions.init(binding, interface)) {
            fBindingInUse = binding;

            fGLVersion = GrGLGetVersionFromString(ver);

            fGLSLGeneration = GrGetGLSLGeneration(fBindingInUse, interface);

            fVendor = GrGLGetVendor(interface);

            fRenderer = GrGLGetRenderer(interface);

            fIsMesa = GrGLIsMesaFromVersionString(ver);

            fGLCaps->init(*this, interface);
            return true;
        }
    }
    return false;
}

bool GrGLContextInfo::isInitialized() const {
    return kNone_GrGLBinding != fBindingInUse;
}

void GrGLContextInfo::reset() {
    fBindingInUse = kNone_GrGLBinding;
    fGLVersion = GR_GL_VER(0, 0);
    fGLSLGeneration = static_cast<GrGLSLGeneration>(0);
    fVendor = kOther_GrGLVendor;
    fRenderer = kOther_GrGLRenderer;
    fIsMesa = false;
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
