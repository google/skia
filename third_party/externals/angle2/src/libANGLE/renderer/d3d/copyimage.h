//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// copyimage.h: Defines image copying functions

#ifndef LIBANGLE_RENDERER_D3D_COPYIMAGE_H_
#define LIBANGLE_RENDERER_D3D_COPYIMAGE_H_

#include "common/mathutil.h"
#include "libANGLE/angletypes.h"

#include <stdint.h>

namespace rx
{

template <typename sourceType, typename colorDataType>
void ReadColor(const uint8_t *source, uint8_t *dest);

template <typename destType, typename colorDataType>
void WriteColor(const uint8_t *source, uint8_t *dest);

template <typename sourceType, typename destType, typename colorDataType>
void CopyPixel(const uint8_t *source, uint8_t *dest);

void CopyBGRA8ToRGBA8(const uint8_t *source, uint8_t *dest);

}

#include "copyimage.inl"

#endif // LIBANGLE_RENDERER_D3D_COPYIMAGE_H_
