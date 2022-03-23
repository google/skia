/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Recording_DEFINED
#define skgpu_Recording_DEFINED

#include "include/core/SkRefCnt.h"

class SkTextureDataBlock;

namespace skgpu {

class CommandBuffer;
template<typename DataBlockT> class PipelineDataCache;
using TextureDataCache = PipelineDataCache<SkTextureDataBlock>;

class Recording final {
public:
    ~Recording();

protected:
private:
    friend class Context; // for access fCommandBuffer
    friend class Recorder; // for ctor
    Recording(sk_sp<CommandBuffer>, std::unique_ptr<TextureDataCache>);

    sk_sp<CommandBuffer> fCommandBuffer;

    // The TextureDataCache holds all the Textures and Samplers used in this Recording.
    std::unique_ptr<TextureDataCache> fTextureDataCache;
};

} // namespace skgpu

#endif // skgpu_Recording_DEFINED
