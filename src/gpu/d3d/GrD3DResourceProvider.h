/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DResourceProvider_DEFINED
#define GrD3DResourceProvider_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "include/private/SkTArray.h"

#include <memory>

class GrD3DDirectCommandList;
class GrD3DGpu;

class GrD3DResourceProvider {
public:
    GrD3DResourceProvider(GrD3DGpu*);

    std::unique_ptr<GrD3DDirectCommandList> findOrCreateDirectCommandList();

    void recycleDirectCommandList(std::unique_ptr<GrD3DDirectCommandList>);

private:
    GrD3DGpu* fGpu;

    SkSTArray<4, std::unique_ptr<GrD3DDirectCommandList>> fAvailableDirectCommandLists;
};

#endif
