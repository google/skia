/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/BackendTexture.h"

namespace skgpu {

#ifdef SK_METAL
BackendTexture::BackendTexture(SkISize dimensions, sk_cfp<mtl::Handle> mtlTexture)
        : fDimensions(dimensions)
        , fInfo(mtl::TextureInfo(mtlTexture.get()))
        , fMtlTexture(std::move(mtlTexture)) {}
#endif

} // namespace skgpu

