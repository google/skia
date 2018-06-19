/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Image_DEFINED
#define SkSurface_Image_DEFINED

//
// for now make sure it's defined
//

#if !defined(SK_SUPPORT_GPU_COMPUTE)
#define SK_SUPPORT_GPU_COMPUTE 1
#endif

//
//
//

#if SK_SUPPORT_GPU_COMPUTE

//
//
//

// #include "GrContext.h"
// #include "SkRefCnt.h"
#include "SkImage.h"
#include "gl/GrGLGpu.h"

//
//
//

#include "SkContext_Compute.h"

//
//
//

class SkImage_Compute : public SkImage
{
 public:

  SkImage_Compute(sk_sp<SkContext_Compute> compute,
                  GrGLuint           const snap,
                  int                const width,
                  int                const height);

  ~SkImage_Compute();

  //
  //
  //

 private:

  //
  //
  //

  sk_sp<SkContext_Compute> compute; // reference to compute context
  GrGLuint                 snap;    // fbo

  //
  //
  //
};

//
//
//

#endif

//
//
//

#endif
