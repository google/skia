/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_AtlasProvider_DEFINED
#define skgpu_graphite_AtlasProvider_DEFINED

#include "include/core/SkRefCnt.h"

#include <memory>
#include <unordered_map>

namespace skgpu::graphite {

class ComputePathAtlas;
class Recorder;
class TextAtlasManager;
class TextureProxy;

/**
 * AtlasProvider groups various texture atlas management algorithms together.
 */
class AtlasProvider final {
public:
    explicit AtlasProvider(Recorder*);
    ~AtlasProvider() = default;

    // Returns the TextAtlasManager that provides access to persistent DrawAtlas'es used in glyph
    // rendering. This TextAtlasManager is always available.
    TextAtlasManager* textAtlasManager() const { return fTextAtlasManager.get(); }

    // Creates a new transient atlas handler that uses compute shaders to rasterize coverage masks
    // for path rendering. This method returns nullptr if compute shaders are not supported by the
    // owning Recorder's context.
    std::unique_ptr<ComputePathAtlas> createComputePathAtlas(Recorder*) const;

    // Return an Alpha_8 TextureProxy with the given dimensions.
    sk_sp<TextureProxy> getAtlasTexture(Recorder*, uint32_t width, uint32_t height);

    void clearTexturePool();

private:
    std::unique_ptr<TextAtlasManager> fTextAtlasManager;

    // Allocated and cached texture proxies shared by all PathAtlas instances. It is possible for
    // the same texture to be bound to multiple DispatchGroups and DrawPasses across flushes. The
    // owning Recorder must guarantee that any uploads or compute dispatches are scheduled to remain
    // coherent across flushes.
    // TODO: This requirement might change with a more sophisticated reuse scheme for texture
    // allocations. For now our model is simple: all PathAtlases target the same texture and only
    // one of them will render to the texture during a given command submission.
    std::unordered_map<uint64_t, sk_sp<TextureProxy>> fTexturePool;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_AtlasProvider_DEFINED
