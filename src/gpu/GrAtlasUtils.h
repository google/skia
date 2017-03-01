/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasUtils_DEFINED
#define GrAtlasUtils_DEFINED

#include "GrDrawOpAtlas.h"
#include "GrTypes.h"

class GrContext;

/**
 * A namespace to house atlas utilities. 
 */
namespace GrAtlasUtils {
    /**
     * Returns a GrDrawOpAtlas. This function can be called anywhere, but the returned atlas
     * should only be used inside of GrMeshDrawOp::onPrepareDraws.
     *  @param GrPixelConfig    The pixel config which this atlas will store
     *  @param width            width in pixels of the atlas
     *  @param height           height in pixels of the atlas
     *  @param numPlotsX        The number of plots the atlas should be broken up into in the X
     *                          direction
     *  @param numPlotsY        The number of plots the atlas should be broken up into in the Y
     *                          direction
     *  @param func             An eviction function which will be called whenever the atlas has to
     *                          evict data
     *  @param data             User supplied data which will be passed into func whenver an
     *                          eviction occurs
     *  @return                 An initialized GrDrawOpAtlas, or nullptr if creation fails
     */
    std::unique_ptr<GrDrawOpAtlas> MakeAtlas(GrContext*, GrPixelConfig,
                                             int width, int height,
                                             int numPlotsX, int numPlotsY,
                                             GrDrawOpAtlas::EvictionFunc func, void* data);

};

#endif
