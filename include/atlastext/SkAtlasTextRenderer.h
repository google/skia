/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPoint.h"

#ifndef SkAtlasTextRenderer_DEFINED
#define SkAtlasTextRenderer_DEFINED

class SkAtlasTextRenderer {
public:
    enum class AtlasFormat {
        /** Unsigned normalized 8 bit single channel format. */
        kA8
    };

    struct SDFVertex {
        SkPoint fPosition;
        SkPoint fTextureCoord;
    };

    virtual ~SkAtlasTextRenderer() = default;

    /**
     * Create a texture of the provided format with dimensions 'width' x 'height'
     * and return a unique handle.
     */
    virtual void* createTexture(AtlasFormat, int width, int height) = 0;

    /**
     * Delete this texture with the passed handle.
     */
    virtual void deleteTexture(void* textureHandle) = 0;

    /**
     * Place the pixel data specified by 'data' in the texture with handle
     * 'textureHandle' in the rectangle ['x', 'x' + 'width') x ['y', 'y' + 'height').
     * 'rowBytes' specifies the byte offset between successive rows in 'data' and will always be
     * a multiple of the number of bytes per pixel.
     * The pixel format of data is the same as that of 'texture'.
     */
    virtual void setTextureData(void* textureHandle, void* data, int x, int y,
                                int width, int height, size_t rowBytes) = 0;

    /**
     * Draws glyphs using SDFs. The SDF data resides in 'textureHandle'. The array
     * 'vertices' provides interleaved device-space positions and normalized
     * texture coordinates. There are are 4 * 'quadCnt' entries in 'vertices'.
     */
    virtual void drawSDFGlyphs(void* targetHandle, void* textureHandle,
                               const SDFVertex vertices[], int quadCnt) = 0;
};

#endif
