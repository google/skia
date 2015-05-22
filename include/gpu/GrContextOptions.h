/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextOptions_DEFINED
#define GrContextOptions_DEFINED

#include "GrTypes.h"

struct GrContextOptions {
    GrContextOptions() : fDrawPathToCompressedTexture(false), fSuppressPrints(false) {}

    // EXPERIMENTAL
    // May be removed in the future, or may become standard depending
    // on the outcomes of a variety of internal tests.
    bool fDrawPathToCompressedTexture;
    // Suppress prints for the GrContext.
    bool fSuppressPrints;
};

#endif
