/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_AtlasProvider_DEFINED
#define skgpu_graphite_AtlasProvider_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"

#include <memory>
#include <unordered_map>

namespace skgpu::graphite {

class ComputePathAtlas;
class Recorder;
class SoftwarePathAtlas;
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

    enum class PathAtlasFlags : unsigned {
        kNone     = 0b000,
        // ComputePathAtlas is supported
        kCompute  = 0b001,
        // SoftwarePathAtlas is supported
        kSoftware = 0b010,
    };
    SK_DECL_BITMASK_OPS_FRIENDS(PathAtlasFlags);

    // Returns whether a particular atlas type is available
    bool isAvailable(PathAtlasFlags atlasType) {
        return SkToBool(fPathAtlasFlags & atlasType);
    }

    // Creates a new transient atlas handler that uses compute shaders to rasterize coverage masks
    // for path rendering. This method returns nullptr if compute shaders are not supported by the
    // owning Recorder's context.
    std::unique_ptr<ComputePathAtlas> createComputePathAtlas() const;

    // Creates a new atlas handler that uses the CPU pipeline to rasterize coverage masks
    // for path rendering.
    std::unique_ptr<SoftwarePathAtlas> createSoftwarePathAtlas() const;

    // Return a TextureProxy with the given dimensions and color type.
    sk_sp<TextureProxy> getAtlasTexture(
            Recorder*, uint16_t width, uint16_t height, SkColorType, bool requireStorageUsage);

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

    SkEnumBitMask<PathAtlasFlags> fPathAtlasFlags = PathAtlasFlags::kNone;
};

SK_MAKE_BITMASK_OPS(AtlasProvider::PathAtlasFlags)

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_AtlasProvider_DEFINED
