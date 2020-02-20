
/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DTypes_DEFINED
#define GrD3DTypes_DEFINED

#include "include/core/SkTypes.h"

// TODO: move these to something like GrD3D12.h
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>
#include <wrl/client.h>  // for ComPtr
// Because Microsoft typedefs this as 'struct' for some reason
#ifdef interface
#undef interface
#endif

#include <functional>
#include "include/gpu/GrTypes.h"



#endif
