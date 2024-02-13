/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SmallPathAtlas_DEFINED
#define skgpu_graphite_SmallPathAtlas_DEFINED

#include "src/base/SkTInternalLList.h"
#include "src/core/SkTHash.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/PathAtlas.h"

namespace skgpu::graphite {

class UploadList;

/**
 * PathAtlas class that rasterizes coverage masks on the CPU and caches them in a
 * DrawAtlas. Only masks below a certain size will be stored in this atlas; larger
 * masks should be stored in the RasterPathAtlas.
 *
 * When a new shape gets added, its path is rasterized in preparation for upload. These
 * uploads are recorded by `recordUploads()` and subsequently added to an UploadTask.
 *
 */
class SmallPathAtlas : public PathAtlas {
public:
    explicit SmallPathAtlas(Recorder*);
    ~SmallPathAtlas() override {}

    bool recordUploads(UploadList*) { /*TODO*/ return false; }

protected:
    const TextureProxy* onAddShape(const Shape&,
                                   const Transform& transform,
                                   const SkStrokeRec&,
                                   skvx::half2 maskSize,
                                   skvx::half2* outPos) override;

private:
    // TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?
    static constexpr int kDefaultAtlasDim = 2048;

    std::unique_ptr<DrawAtlas> fDrawAtlas;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_SmallPathAtlas_DEFINED
