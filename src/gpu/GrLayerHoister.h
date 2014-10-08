/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrLayerHoister_DEFINED
#define GrLayerHoister_DEFINED

#include "SkPicture.h"
#include "SkTDArray.h"

class GrAccelData;
struct GrCachedLayer;
class GrReplacements;
struct SkRect;

class GrHoistedLayer {
public:
    const SkPicture* fPicture;
    GrCachedLayer*   fLayer;
    SkIPoint         fOffset;
    SkMatrix         fCTM;
};

// This class collects the layer hoisting functionality in one place.
// For each picture rendering:
//  FindLayersToHoist should be called once to collect the required layers
//  DrawLayers should be called once to render them
//  UnlockLayers should be called once to allow the texture resources to be recycled
class GrLayerHoister {
public:

    /** Find the layers in 'topLevelPicture' that need hoisting. Note that the discovered
        layers can be inside nested sub-pictures.
        @param context    Owner of the layer cache (the source of new layers)
        @param topLevelPicture The top-level picture that is about to be rendered
        @param query       The rectangle that is about to be drawn.
        @param atlased     Out parameter storing the layers that should be hoisted to the atlas
        @param nonAtlased  Out parameter storing the layers that should be hoisted stand alone
        @param recycled    Out parameter storing layers that need hoisting but not rendering
        Return true if any layers are suitable for hoisting; false otherwise
    */
    static bool FindLayersToHoist(GrContext* context,
                                  const SkPicture* topLevelPicture,
                                  const SkRect& query,
                                  SkTDArray<GrHoistedLayer>* atlased,
                                  SkTDArray<GrHoistedLayer>* nonAtlased,
                                  SkTDArray<GrHoistedLayer>* recycled);

    /** Draw the specified layers into either the atlas or free floating textures.
        @param atlased      The layers to be drawn into the atlas
        @param nonAtlased   The layers to be drawn into their own textures
        @param recycled     Layers that don't need rendering but do need to go into the 
                            replacements object
        @param replacements The replacement structure to fill in with the rendered layer info
    */
    static void DrawLayers(const SkTDArray<GrHoistedLayer>& atlased,
                           const SkTDArray<GrHoistedLayer>& nonAtlased,
                           const SkTDArray<GrHoistedLayer>& recycled,
                           GrReplacements* replacements);

    /** Unlock unneeded layers in the layer cache.
        @param context    Owner of the layer cache (and thus the layers)
        @param atlased    Unneeded layers in the atlas
        @param nonAtlased Unneeded layers in their own textures
        @param recycled   Unneeded layers that did not require rendering
    */
    static void UnlockLayers(GrContext* context,
                             const SkTDArray<GrHoistedLayer>& atlased,
                             const SkTDArray<GrHoistedLayer>& nonAtlased,
                             const SkTDArray<GrHoistedLayer>& recycled);
};

#endif
