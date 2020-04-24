/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint3.h"
#include "include/core/SkRefCnt.h"

#ifndef SkAtlasTextRenderer_DEFINED
#define SkAtlasTextRenderer_DEFINED

/**
 * This is the base class for a renderer implemented by the SkAtlasText client. The
 * SkAtlasTextContext issues texture creations, deletions, uploads, and vertex draws to the
 * renderer. The renderer must perform those actions in the order called to correctly render
 * the text drawn to SkAtlasTextTargets.
 */
class SK_API SkAtlasTextRenderer : public SkRefCnt {
public:
    enum class AtlasFormat {
        /** Unsigned normalized 8 bit single channel format. */
        kA8
    };

    struct SDFVertex {
        /** Position in device space (not normalized). The third component is w (not z). */
        SkPoint3 fPosition;
        /** Color, same value for all four corners of a glyph quad. */
        uint32_t fColor;
        /** Texture coordinate (in texel units, not normalized). */
        int16_t fTextureCoordX;
        int16_t fTextureCoordY;
    };

    virtual ~SkAtlasTextRenderer() = default;

    /**
     * Create a texture of the provided format with dimensions 'width' x 'height'
     * and return a unique handle.
     */
    virtual void* createTexture(AtlasFormat, int width, int height) = 0;

    /**
     * Delete the texture with the passed handle.
     */
    virtual void deleteTexture(void* textureHandle) = 0;

    /**
     * Place the pixel data specified by 'data' in the texture with handle
     * 'textureHandle' in the rectangle ['x', 'x' + 'width') x ['y', 'y' + 'height').
     * 'rowBytes' specifies the byte offset between successive rows in 'data' and will always be
     * a multiple of the number of bytes per pixel.
     * The pixel format of data is the same as that of 'textureHandle'.
     */
    virtual void setTextureData(void* textureHandle, const void* data, int x, int y, int width,
                                int height, size_t rowBytes) = 0;

    /**
     * Draws glyphs using SDFs. The SDF data resides in 'textureHandle'. The array
     * 'vertices' provides interleaved device-space positions, colors, and
     * texture coordinates. There are are 4 * 'quadCnt' entries in 'vertices'.
     */
    virtual void drawSDFGlyphs(void* targetHandle, void* textureHandle, const SDFVertex vertices[],
                               int quadCnt) = 0;

    /** Called when a SkAtlasTextureTarget is destroyed. */
    virtual void targetDeleted(void* targetHandle) = 0;
};

#endif
