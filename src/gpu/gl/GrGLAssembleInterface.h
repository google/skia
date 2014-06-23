
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLInterface.h"

typedef void(*GrGLFuncPtr)();
typedef GrGLFuncPtr (*GrGLGetProc)(void* ctx, const char name[]);

/**
 * Generic function for creating a GrGLInterface for an OpenGL (but not GLES) context. It calls
 * get() to get each function address. ctx is a generic ptr passed to and interpreted by get().
 */
const GrGLInterface* GrGLAssembleGLInterface(void* ctx, GrGLGetProc get);
