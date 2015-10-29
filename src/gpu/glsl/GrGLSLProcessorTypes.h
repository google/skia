/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLProcessorTypes_DEFINED
#define GrGLSLProcessorTypes_DEFINED

#include "GrShaderVar.h"

/**
 * These are meant to only be used by GrGLSL*Processors so they can add transformed coordinates
 * to their shader code.
 */
typedef GrShaderVar GrGLSLTransformedCoords;
typedef SkTArray<GrShaderVar> GrGLSLTransformedCoordsArray;

#endif
