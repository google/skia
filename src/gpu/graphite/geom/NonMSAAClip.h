/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_NonMSAAClip_DEFINED
#define skgpu_graphite_geom_NonMSAAClip_DEFINED

#if defined(SK_GRAPHITE_USE_LEGACY_RRECT_CLIP_SHADER)
#include "src/gpu/graphite/geom/Rect.h"
#else
#include "include/core/SkM44.h"
#include "include/core/SkRect.h"
#endif

#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

/**
 * Represents a rect or rrect clip with any circular or rectangular corners under an affine
 * transformation.
 */
struct AnalyticClip {
#if defined(SK_GRAPHITE_USE_LEGACY_RRECT_CLIP_SHADER)
    // Indicate which edges are adjacent to circular corners.
    enum EdgeFlags {
        kLeft_EdgeFlag   = 0b0001,
        kTop_EdgeFlag    = 0b0010,
        kRight_EdgeFlag  = 0b0100,
        kBottom_EdgeFlag = 0b1000,

        kNone_EdgeFlag   = 0b0000,
        kAll_EdgeFlag    = 0b1111,
    };
    // These defaults will produce no clip
    Rect     fBounds = { 0, 0, 0, 0 }; // Bounds of clip
    float    fRadius = 0;              // Circular radius, if any
    uint32_t fEdgeFlags = kNone_EdgeFlag;
    bool     fInverted = true;

    bool isEmpty() const { return fBounds.isEmptyNegativeOrNaN(); }
    SkRect edgeSelectRect() const {
        return { fEdgeFlags & kLeft_EdgeFlag   ? 1.f : 0.f,
                 fEdgeFlags & kTop_EdgeFlag    ? 1.f : 0.f,
                 fEdgeFlags & kRight_EdgeFlag  ? 1.f : 0.f,
                 fEdgeFlags & kBottom_EdgeFlag ? 1.f : 0.f };
    }
#else
    // These defaults will produce no clip
    SkRect fBounds = { 0, 0, 0, 0 }; // Bounds of clip
    SkV4   fRadii = {0.f, 0.f, 0.f, 0.f};
    SkV4   fXform = {1.f, 0.f, 0.f, 1.f}; // device-to-clip's local 2x2
    // Inversion matches the ClipStack's convention for depth-only draws: an inverse fill is used
    // for intersection clips and a regular fill is for difference clips.
    bool   fInverted = false;

    // When this is true, the shader will always evaluate to full coverage
    bool isEmpty() const { return fBounds.isEmpty() && !fInverted; }
#endif
};

/**
 * Represents a clip that uses a mask in an atlas
 */
struct AtlasClip {
    SkIRect             fMaskBounds = {0, 0, 0, 0};
    SkIPoint            fOutPos = {0, 0};
    sk_sp<TextureProxy> fAtlasTexture = nullptr;

    bool isEmpty() const { return !SkToBool(fAtlasTexture.get()); }
};

/**
 * Combined non-MSAA clip structure
 */
struct NonMSAAClip {
    AnalyticClip fAnalyticClip;
    AtlasClip    fAtlasClip;

    bool isEmpty() const { return fAnalyticClip.isEmpty() && fAtlasClip.isEmpty(); }
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_NonMSAAClip_DEFINED
