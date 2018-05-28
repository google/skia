/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "SkRefCnt.h"
#include "gl/GrGLFunctions.h"
#include "gl/GrGLInterface.h"
#include "GrContextFactory.h"
#include "gl/GrGLExtensions.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <sstream>

GrContextOptions grContextOpts;

// create_grcontext implementation for EGL.
sk_sp<GrContext> create_grcontext(std::ostringstream &driverinfo) {
    sk_gpu_test::GrContextFactory* fContextFactory = new sk_gpu_test::GrContextFactory(grContextOpts);

    GrContext* result = fContextFactory->get(sk_gpu_test::GrContextFactory::kGL_ContextType,
                                             sk_gpu_test::GrContextFactory::ContextOverrides::kNone);
    if (!result) {
        result = fContextFactory->get(sk_gpu_test::GrContextFactory::kGLES_ContextType,
                                      sk_gpu_test::GrContextFactory::ContextOverrides::kNone);
    }
    driverinfo << "GL Versionr: " << getString(GL_VERSION) << "\n";
    driverinfo << "GL Vendor: " << getString(GL_VENDOR) << "\n";
    driverinfo << "GL Renderer: " << getString(GL_RENDERER) << "\n";
    driverinfo << "GL Extensions: " << getString(GL_EXTENSIONS) << "\n";

    return sk_sp<GrContext>(result);
}
