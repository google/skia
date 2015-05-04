/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceProvider_DEFINED
#define GrResourceProvider_DEFINED

#include "GrTextureProvider.h"

/**
 * An extension of the texture provider for arbitrary resource types. This class is intended for
 * use within the Gr code base, not by clients or extensions (e.g. third party GrProcessor
 * derivatives).
 */
class GrResourceProvider : public GrTextureProvider {
public:

    GrResourceProvider(GrGpu* gpu, GrResourceCache* cache) : INHERITED(gpu, cache) {}

    using GrTextureProvider::assignUniqueKeyToResource;
    using GrTextureProvider::findAndRefResourceByUniqueKey;
    using GrTextureProvider::abandon;

    typedef GrTextureProvider INHERITED;
};

#endif
