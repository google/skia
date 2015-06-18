/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextOptions_DEFINED
#define GrContextOptions_DEFINED

#include "SkTypes.h"

struct GrContextOptions {
    GrContextOptions()
        : fDrawPathToCompressedTexture(false)
        , fSuppressPrints(false)
        , fMaxTextureSizeOverride(SK_MaxS32)
        , fMinTextureSizeOverride(0)
        , fSuppressDualSourceBlending(false)
        , fGeometryBufferMapThreshold(-1)
        , fUseDrawInsteadOfPartialRenderTargetWrite(false) {}

    // EXPERIMENTAL
    // May be removed in the future, or may become standard depending
    // on the outcomes of a variety of internal tests.
    bool fDrawPathToCompressedTexture;

    // Suppress prints for the GrContext.
    bool fSuppressPrints;

    /** Overrides: These options override feature detection using backend API queries. These
        overrides can only reduce the feature set or limits, never increase them beyond the
        detected values. */

    int  fMaxTextureSizeOverride;
    int  fMinTextureSizeOverride;
    bool fSuppressDualSourceBlending;

    /** the threshold in bytes above which we will use a buffer mapping API to map vertex and index
        buffers to CPU memory in order to update them.  A value of -1 means the GrContext should
        deduce the optimal value for this platform. */
    int  fGeometryBufferMapThreshold;

    /** some gpus have problems with partial writes of the rendertarget */
    bool fUseDrawInsteadOfPartialRenderTargetWrite;
};

#endif
