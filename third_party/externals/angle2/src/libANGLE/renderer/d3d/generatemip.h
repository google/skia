//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// generatemip.h: Defines the GenerateMip function, templated on the format
// type of the image for which mip levels are being generated.

#ifndef LIBANGLE_RENDERER_D3D_GENERATEMIP_H_
#define LIBANGLE_RENDERER_D3D_GENERATEMIP_H_

#include "libANGLE/renderer/d3d/imageformats.h"
#include "libANGLE/angletypes.h"

namespace rx
{

template <typename T>
inline void GenerateMip(size_t sourceWidth, size_t sourceHeight, size_t sourceDepth,
                        const uint8_t *sourceData, size_t sourceRowPitch, size_t sourceDepthPitch,
                        uint8_t *destData, size_t destRowPitch, size_t destDepthPitch);

}

#include "generatemip.inl"

#endif // LIBANGLE_RENDERER_D3D_GENERATEMIP_H_
