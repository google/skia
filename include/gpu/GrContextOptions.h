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
        , fSuppressDualSourceBlending(false) {}

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
    bool fSuppressDualSourceBlending;
};

#endif
