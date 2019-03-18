/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLAssembleHelpers.h"
#include "gl/GrGLUtil.h"

void GrGetEGLQueryAndDisplay(GrEGLQueryStringFn** queryString, GrEGLDisplay* display,
                             void* ctx, GrGLGetProc get) {
    *queryString = (GrEGLQueryStringFn*)get(ctx, "eglQueryString");
    *display = GR_EGL_NO_DISPLAY;
    if (*queryString) {
        GrEGLGetCurrentDisplayFn* getCurrentDisplay =
                (GrEGLGetCurrentDisplayFn*)get(ctx, "eglGetCurrentDisplay");
        if (getCurrentDisplay) {
            *display = getCurrentDisplay();
        } else {
            *queryString = nullptr;
        }
    }
}
