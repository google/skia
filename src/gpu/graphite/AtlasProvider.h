/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_AtlasProvider_DEFINED
#define skgpu_graphite_AtlasProvider_DEFINED

#include <memory>

namespace skgpu::graphite {

class ComputePathAtlas;
class Recorder;
class TextAtlasManager;

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

    // Returns the transient atlas handler that uses compute shaders to rasterize coverage masks for
    // path rendering. This method returns nullptr if compute shaders are not supported by the
    // owning Recorder's context.
    ComputePathAtlas* computePathAtlas() const {
#ifdef SK_ENABLE_VELLO_SHADERS
        return fComputePathAtlas.get();
#else
        return nullptr;
#endif
    }

private:
    std::unique_ptr<TextAtlasManager> fTextAtlasManager;
#ifdef SK_ENABLE_VELLO_SHADERS
    std::unique_ptr<ComputePathAtlas> fComputePathAtlas;
#endif
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_AtlasProvider_DEFINED
