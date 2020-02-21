/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3D12_DEFINED
#define GrD3D12_DEFINED

#include <d3d12.h>
#include <wrl/client.h>  // for ComPtr
// Because Microsoft defines this as '__STRUCT__' for some reason
#ifdef interface
#undef interface
#endif

// Abbreviate and alias ComPtr
template<typename T>
using gr_cp = Microsoft::WRL::ComPtr<T>;

#endif
