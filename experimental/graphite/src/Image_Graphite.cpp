/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Image_Graphite.h"

#if SK_SUPPORT_GPU
// TODO: move onAsFragmentProcessor off of SkImage_Base and remove this include
#include "src/gpu/GrFragmentProcessor.h"
#endif

namespace skgpu {

Image_Graphite::Image_Graphite(const SkImageInfo& ii)
    : SkImage_Base(ii, kNeedNewImageUniqueID) {
}

Image_Graphite::~Image_Graphite() {}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> Image_Graphite::onAsFragmentProcessor(
        GrRecordingContext*,
        SkSamplingOptions,
        const SkTileMode[2],
        const SkMatrix&,
        const SkRect* subset,
        const SkRect* domain) const {
    return nullptr;
}
#endif

} // namespace skgpu
