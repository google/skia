/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//
//
//

#include "SkImage_Compute.h"

//
//
//

#if SK_SUPPORT_GPU_COMPUTE

//
// C++
//

//
//
//

SkImage_Compute::SkImage_Compute(sk_sp<SkContext_Compute> compute,
                                 GrGLuint           const snap,
                                 int                const width,
                                 int                const height)

  : SkImage(width,height,0), // FIXME -- who provides unique id?
    compute(compute),
    snap(snap)
{
  ;
}

SkImage_Compute::~SkImage_Compute()
{
  // skc_interop_snap_dispose(compute->interop,snap); TODO skc.h
}

//
//
//

#endif
