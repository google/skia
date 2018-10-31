/*
 * Copyright 2016 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "gl/GrGLInterface.h"

//
//
//

#ifdef __cplusplus
extern "C" {
#endif

#include "skc.h"

#ifdef __cplusplus
}
#endif

//
//
//

class SkContext_Compute : public SkNVRefCnt<SkContext_Compute>
{
 public:

  //
  //
  //

  SkContext_Compute(GrGLInterface const * fInterface);

  ~SkContext_Compute();

  //
  //
  //

  skc_context_t context;
  skc_interop_t interop;
  skc_surface_t surface;

  //
  //
  //

 private:

  GrGLInterface const * fInterface;

  //
  //
  //
};

//
//
//
