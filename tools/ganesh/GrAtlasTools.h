/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasManagerTools_DEFINED
#define GrAtlasManagerTools_DEFINED

#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"

#include <cstddef>

class GrDirectContext;

class GrAtlasManagerTools {
public:
    static void Dump(const GrAtlasManager*, GrDirectContext*);
    static void SetAtlasDimensionsToMinimum(GrAtlasManager*);
    static void SetMaxPages(GrAtlasManager*, uint32_t maxPages);
};

class GrDrawOpAtlasTools {
public:
    static int NumAllocated(const GrDrawOpAtlas*);
    static void SetMaxPages(GrDrawOpAtlas*, uint32_t maxPages);
};

#endif
