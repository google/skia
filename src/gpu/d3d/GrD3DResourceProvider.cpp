/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DResourceProvider.h"

#include "src/gpu/d3d/GrD3DCommandList.h"
#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DResourceProvider::GrD3DResourceProvider(GrD3DGpu* gpu) : fGpu(gpu) {
}

std::unique_ptr<GrD3DDirectCommandList> GrD3DResourceProvider::findOrCreateDirectCommandList() {
    if (fAvailableDirectCommandLists.count()) {
        std::unique_ptr<GrD3DDirectCommandList> list =
                std::move(fAvailableDirectCommandLists.back());
        fAvailableDirectCommandLists.pop_back();
        return list;
    }
    return GrD3DDirectCommandList::Make(fGpu->device());
}

void GrD3DResourceProvider::recycleDirectCommandList(
        std::unique_ptr<GrD3DDirectCommandList> commandList) {
    commandList.reset();
    fAvailableDirectCommandLists.push_back(std::move(commandList));
}
