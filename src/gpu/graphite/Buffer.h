/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Buffer_DEFINED
#define skgpu_graphite_Buffer_DEFINED

#include "include/gpu/GpuTypes.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

class Buffer : public Resource {
public:
    size_t size() const { return fSize; }

    // TODO(b/262249983): Separate into mapRead(), mapWrite() methods.
    void* map();
    void unmap();

    bool isMapped() const { return fMapPtr; }

    const char* getResourceType() const override { return "Buffer"; }

protected:
    Buffer(const SharedContext* sharedContext, size_t size)
            : Resource(sharedContext,
                       Ownership::kOwned,
                       skgpu::Budgeted::kYes,
                       size,
                       /*label=*/"Buffer")
            , fSize(size) {}

    void* fMapPtr = nullptr;

private:
    virtual void onMap() = 0;
    virtual void onUnmap() = 0;

    size_t             fSize;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Buffer_DEFINED

