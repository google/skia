/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3D12_DEFINED
#define GrD3D12_DEFINED

#include "include/core/SkTypes.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>  // for ComPtr

#if SKIA_IMPLEMENTATION || GR_TEST_UTILS
#undef interface
#undef MemoryBarrier
#undef CreateSemaphore
#undef GetFreeSpace
#undef far
#undef near
#undef small
#endif

// Abbreviate and alias ComPtr
template<typename T>
using gr_cp = Microsoft::WRL::ComPtr<T>;

#endif
