//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils9.h: Queries for GL image formats and their translations to D3D
// formats.

#ifndef LIBANGLE_RENDERER_D3D_FORMATUTILSD3D_H_
#define LIBANGLE_RENDERER_D3D_FORMATUTILSD3D_H_

#include "angle_gl.h"

#include <cstddef>
#include <stdint.h>

namespace rx
{

typedef void (*MipGenerationFunction)(size_t sourceWidth, size_t sourceHeight, size_t sourceDepth,
                                      const uint8_t *sourceData, size_t sourceRowPitch, size_t sourceDepthPitch,
                                      uint8_t *destData, size_t destRowPitch, size_t destDepthPitch);

typedef void (*LoadImageFunction)(size_t width, size_t height, size_t depth,
                                  const uint8_t *input, size_t inputRowPitch, size_t inputDepthPitch,
                                  uint8_t *output, size_t outputRowPitch, size_t outputDepthPitch);

typedef void (*InitializeTextureDataFunction)(size_t width, size_t height, size_t depth,
                                              uint8_t *output, size_t outputRowPitch, size_t outputDepthPitch);

typedef void (*ColorReadFunction)(const uint8_t *source, uint8_t *dest);
typedef void (*ColorWriteFunction)(const uint8_t *source, uint8_t *dest);
typedef void (*ColorCopyFunction)(const uint8_t *source, uint8_t *dest);

typedef void (*VertexCopyFunction)(const uint8_t *input, size_t stride, size_t count, uint8_t *output);

enum VertexConversionType
{
    VERTEX_CONVERT_NONE = 0,
    VERTEX_CONVERT_CPU = 1,
    VERTEX_CONVERT_GPU = 2,
    VERTEX_CONVERT_BOTH = 3
};

ColorWriteFunction GetColorWriteFunction(GLenum format, GLenum type);

}

#endif // LIBANGLE_RENDERER_D3D_FORMATUTILSD3D_H_
