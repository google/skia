/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCppUtil_DEFINED
#define GrMtlCppUtil_DEFINED

#include "mtl/GrMtlTypes.h"

// Utilities that can be used from cpp files (rather than .mm).

GrMTLPixelFormat GrGetMTLPixelFormatFromMtlTextureInfo(const GrMtlTextureInfo&);


#endif
