/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3D12_DEFINED
#define GrD3D12_DEFINED

#include <d3d12.h>
#include <wrl/client.h>  // for ComPtr

// Abbreviate and alias ComPtr
template<typename T>
using gr_cp = Microsoft::WRL::ComPtr<T>;

#endif
