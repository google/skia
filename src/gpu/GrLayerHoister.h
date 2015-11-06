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

struct GrCachedLayer;
class GrReplacements;
class SkGpuDevice;
struct SkRect;

class GrHoistedLayer {
public:
    const SkPicture* fPicture;  // the picture that actually contains the layer
                                // (not necessarily the top-most picture)
    GrCachedLayer*   fLayer;
    SkMatrix         fInitialMat;
    SkMatrix         fPreMat;
    SkMatrix         fLocalMat;
};

// This class collects the layer hoisting functionality in one place.
// For each picture rendering:
//  FindLayersToHoist should be called once to collect the required layers
//  DrawLayers should be called once to render them
//  UnlockLayers should be called once to allow the texture resources to be recycled
class GrLayerHoister {
public:
    /** Attempt to reattach layers that may have been atlased in the past
     */
    static void Begin(GrContext* context);

    /** Release cache resources
     */
    static void End(GrContext* context);

    /** Find the layers in 'topLevelPicture' that can be atlased. Note that the discovered
        layers can be inside nested sub-pictures.
        @param context    Owner of the layer cache (the source of new layers)
        @param topLevelPicture The top-level picture that is about to be rendered
        @param initialMat  The CTM of the canvas into which the layers will be drawn
        @param query       The rectangle that is about to be drawn.
        @param atlasedNeedRendering Out parameter storing the layers that 
                                    should be hoisted to the atlas
        @param recycled    Out parameter storing layers that are atlased but do not need rendering
        @param numSamples  The number if MSAA samples required
        */
    static void FindLayersToAtlas(GrContext* context,
                                  const SkPicture* topLevelPicture,
                                  const SkMatrix& initialMat,
                                  const SkRect& query,
                                  SkTDArray<GrHoistedLayer>* atlasedNeedRendering,
                                  SkTDArray<GrHoistedLayer>* recycled,
                                  int numSamples);

    /** Find the layers in 'topLevelPicture' that need hoisting. Note that the discovered
        layers can be inside nested sub-pictures.
        @param context    Owner of the layer cache (the source of new layers)
        @param topLevelPicture The top-level picture that is about to be rendered
        @param initialMat  The CTM of the canvas into which the layers will be drawn
        @param query       The rectangle that is about to be drawn.
        @param needRendering Out parameter storing the layers that need rendering.
                             This should never include atlased layers.
        @param recycled    Out parameter storing layers that need hoisting but not rendering
        @param numSamples  The number if MSAA samples required
    */
    static void FindLayersToHoist(GrContext* context,
                                  const SkPicture* topLevelPicture,
                                  const SkMatrix& initialMat,
                                  const SkRect& query,
                                  SkTDArray<GrHoistedLayer>* needRendering,
                                  SkTDArray<GrHoistedLayer>* recycled,
                                  int numSamples);

    /** Draw the specified layers into the atlas.
        @param context      Owner of the layer cache (and thus the layers)
        @param layers       The layers to be drawn into the atlas
    */
    static void DrawLayersToAtlas(GrContext* context, const SkTDArray<GrHoistedLayer>& layers);

    /** Draw the specified layers into their own individual textures.
        @param context      Owner of the layer cache (and thus the layers)
        @param layers       The layers to be drawn
    */
    static void DrawLayers(GrContext* context, const SkTDArray<GrHoistedLayer>& layers);

    /** Convert all the layers in 'layers' into replacement objects in 'replacements'.
        @param layers       The hoisted layers
        @param replacements Replacement object that will be used for a replacement draw
    */
    static void ConvertLayersToReplacements(const SkPicture* topLevelPicture, 
                                            const SkTDArray<GrHoistedLayer>& layers,
                                            GrReplacements* replacements);

    /** Unlock a group of layers in the layer cache.
        @param context    Owner of the layer cache (and thus the layers)
        @param layers     Unneeded layers in the atlas
    */
    static void UnlockLayers(GrContext* context, const SkTDArray<GrHoistedLayer>& layers);

    /** Forceably remove all cached layers and release the atlas. Useful for debugging and timing.
        This is only functional when GR_CACHE_HOISTED_LAYERS is set to 1 in GrLayerCache.h
        @param context    Owner of the layer cache (and thus the layers)
     */
    static void PurgeCache(GrContext* context);

private:
    /** Update the GrTexture in 'layer' with its filtered version
        @param context    Owner of the layer cache (and thus the layers)
        @param device     Required by the filtering code
        @param info       Layer info for a layer needing filtering prior to being composited
     */
    static void FilterLayer(GrContext* context, SkGpuDevice* device, const GrHoistedLayer& info);

};

#endif
