/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLContextInfo.h"

GrGLContextInfo::~GrGLContextInfo() {
    GrSafeUnref(fInterface);
}

GrGLContextInfo::GrGLContextInfo() {
    this->reset();
}

GrGLContextInfo::GrGLContextInfo(const GrGLInterface* interface) {
    fInterface = NULL;
    this->initialize(interface);
}

GrGLContextInfo::GrGLContextInfo(const GrGLContextInfo& ctx) {
    fInterface = NULL;
    *this = ctx;
}

GrGLContextInfo& GrGLContextInfo::operator = (const GrGLContextInfo& ctx) {
    GrSafeAssign(fInterface, ctx.fInterface);
    fBindingInUse = ctx.fBindingInUse;
    fGLVersion = ctx.fGLVersion;
    fGLSLGeneration = ctx.fGLSLGeneration;
    fExtensionString = ctx.fExtensionString;
    fGLCaps = ctx.fGLCaps;
    return *this;
}

void GrGLContextInfo::reset() {
    GrSafeSetNull(fInterface);
    fBindingInUse = kNone_GrGLBinding;
    fGLVersion = GR_GL_VER(0, 0);
    fGLSLGeneration = static_cast<GrGLSLGeneration>(0);
    fExtensionString = "";
    fGLCaps.reset();
}

bool GrGLContextInfo::initialize(const GrGLInterface* interface) {
    this->reset();
    // We haven't validated the GrGLInterface yet, so check for GetString
    // function pointer
    if (NULL != interface->fGetString) {

        const GrGLubyte* verUByte;
        GR_GL_CALL_RET(interface, verUByte, GetString(GR_GL_VERSION));
        const char* ver = reinterpret_cast<const char*>(verUByte);
        GrGLBinding binding = GrGLGetBindingInUseFromString(ver);

        if (interface->validate(binding)) {

            fInterface = interface;
            interface->ref();

            fBindingInUse = binding;

            fGLVersion = GrGLGetVersionFromString(ver);

            fGLSLGeneration = GrGetGLSLGeneration(fBindingInUse,
                                                  this->interface());

            const GrGLubyte* ext;
            GR_GL_CALL_RET(interface, ext, GetString(GR_GL_EXTENSIONS));
            fExtensionString = reinterpret_cast<const char*>(ext);

            fGLCaps.init(*this);
            return true;
        }
    }
    return false;
}

bool GrGLContextInfo::isInitialized() const {
    return kNone_GrGLBinding != fBindingInUse;
}

