//
// Copyright (c) 2013-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// copyvertex.h: Defines D3D11 vertex buffer copying and conversion functions

#ifndef LIBANGLE_RENDERER_D3D_D3D11_COPYVERTEX_H_
#define LIBANGLE_RENDERER_D3D_D3D11_COPYVERTEX_H_

#include "common/mathutil.h"

namespace rx
{

// 'alphaDefaultValueBits' gives the default value for the alpha channel (4th component)
template <typename T, size_t inputComponentCount, size_t outputComponentCount, uint32_t alphaDefaultValueBits>
inline void CopyNativeVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output);

template <size_t inputComponentCount, size_t outputComponentCount>
inline void Copy8SintTo16SintVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output);

template <size_t componentCount>
inline void Copy8SnormTo16SnormVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output);

template <size_t inputComponentCount, size_t outputComponentCount>
inline void Copy32FixedTo32FVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output);

template <typename T, size_t inputComponentCount, size_t outputComponentCount, bool normalized>
inline void CopyTo32FVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output);

template <bool isSigned, bool normalized, bool toFloat>
inline void CopyXYZ10W2ToXYZW32FVertexData(const uint8_t *input, size_t stride, size_t count, uint8_t *output);

}

#include "copyvertex.inl"

#endif // LIBANGLE_RENDERER_D3D_D3D11_COPYVERTEX_H_
