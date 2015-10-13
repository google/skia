//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_gl.h:
//   Includes all necessary GL headers and definitions for ANGLE.
//

#ifndef ANGLEGL_H_
#define ANGLEGL_H_

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "GLES3/gl3.h"
#include "GLES3/gl31.h"
#include "GLES3/gl32.h"

// The following enum is used in ANGLE, but is from desktop GL
#ifndef GL_SAMPLER_2D_RECT_ARB
#define GL_SAMPLER_2D_RECT_ARB 0x8B63
#endif

#endif // ANGLEGL_H_
