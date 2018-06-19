/*
 * Copyright 2016 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "SkContext_Compute.h"

//
//
//

//
//
//

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#define TARGET_PLATFORM_SUBSTRING   "TO BE SET"
#define TARGET_DEVICE_SUBSTRING     "TO BE SET"

//
//
//

//
//
//

SkContext_Compute::SkContext_Compute(GrGLInterface const * fInterface)
  : fInterface(fInterface)
{
  //
  // Make sure fInterface destruction occurs after compute
  //
  SkSafeRef(fInterface);

  skc_err err;

  //
  // CREATE A NEW SPINEL CONTEXT AND ATTACH TO WINDOW
  //
  err = skc_context_create(&context, TARGET_PLATFORM_SUBSTRING, TARGET_DEVICE_SUBSTRING);
  SKC_ERR_CHECK(err);

  //
  // CREATE A NEW REUSABLE INTEROP OBJECT
  //
  // interop = skc_interop_create(fInterface,1); TODO have this in skc.h

  //
  // CREATE A NEW REUSABLE SURFACE OBJECT
  //
  err = skc_surface_create(context,
			   interop,
			   &surface);
  SKC_ERR_CHECK(err);
}

//
//
//

SkContext_Compute::~SkContext_Compute()
{
  skc_err err;

  // dispose of surface
  err = skc_surface_dispose(surface);
  SKC_ERR_CHECK(err);

  // dispose of interop
  // skc_interop_dispose(interop); TODO have this in skc.h

  // dispose of context
  err = skc_context_release(context);
  SKC_ERR_CHECK(err);

  // unref GL interface
  SkSafeUnref(fInterface);
}

//
//
//

