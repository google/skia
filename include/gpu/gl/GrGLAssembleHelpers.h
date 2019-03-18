/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLAssembleInterface.h"

#define GET_PROC(F) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F)
#define GET_PROC_SUFFIX(F, S) functions->f##F = (GrGL##F##Fn*)get(ctx, "gl" #F #S)
#define GET_PROC_LOCAL(F) GrGL##F##Fn* F = (GrGL##F##Fn*)get(ctx, "gl" #F)

#define GET_EGL_PROC_SUFFIX(F, S) functions->fEGL##F = (GrEGL##F##Fn*)get(ctx, "egl" #F #S)


void get_egl_query_and_display(GrEGLQueryStringFn** queryString, GrEGLDisplay* display,
                               void* ctx, GrGLGetProc get);
