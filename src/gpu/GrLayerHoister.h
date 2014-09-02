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
struct SkRect;

// This class collects the layer hoisting functionality in one place.
// For each picture rendering:
//  FindLayersToHoist should be called once to collect the required layers
//  DrawLayers should be called once to render them
//  UnlockLayers should be called once to allow the texture resources to be recycled
class GrLayerHoister {
public:
    /** Find the layers in 'gpuData' that need hoisting.
        @param gpuData  Acceleration structure containing layer information for a picture
        @param ops      If a BBH is being used the operations about to be executed; NULL otherwise.
        @param query    The rectangle that is about to be drawn.
        @param pullForward A gpuData->numSaveLayers -sized Boolean array indicating 
                           which layers are to be hoisted
        Return true if any layers are suitable for hoisting; false otherwise
    */
    static bool FindLayersToHoist(const GrAccelData *gpuData,
                                  const SkPicture::OperationList* ops,
                                  const SkRect& query,
                                  bool pullForward[]);

    /** Draw the specified layers of 'picture' into either the atlas or free
        floating textures.
        @param picture  The picture containing the layers
        @param atlased  The layers to be drawn into the atlas
        @param nonAtlased The layers to be drawn into their own textures
    */
    static void DrawLayers(const SkPicture* picture,
                           const SkTDArray<GrCachedLayer*>& atlased,
                           const SkTDArray<GrCachedLayer*>& nonAtlased);

    /** Unlock all the layers associated with picture in the layer cache.
        @param layerCache holder of the locked layers
        @pmara picture    the source of the locked layers
    */
    static void UnlockLayers(GrLayerCache* layerCache, const SkPicture* picture);
};

#endif
