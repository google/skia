/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ResourceProvider_DEFINED
#define skgpu_ResourceProvider_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class CommandBuffer;
class Pipeline;
class PipelineDesc;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    std::unique_ptr<CommandBuffer> createCommandBuffer();
    Pipeline* findOrCreatePipeline(const PipelineDesc&);

protected:
    ResourceProvider();

    virtual std::unique_ptr<CommandBuffer> onCreateCommandBuffer() { return nullptr; }
    virtual Pipeline* onCreatePipeline() { return nullptr; }

private:
};

} // namespace skgpu

#endif // skgpu_ResourceProvider_DEFINED
