/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_BackendTexture_DEFINED
#define skgpu_BackendTexture_DEFINED

#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/include/TextureInfo.h"
#include "include/core/SkSize.h"

#ifdef SK_METAL
#include "experimental/graphite/include/mtl/MtlTypes.h"
#endif

namespace skgpu {

class BackendTexture {
public:
#ifdef SK_METAL
    BackendTexture(SkISize dimensions, sk_cfp<mtl::Handle> mtlTexture);
#endif

    bool isValid() const { return fInfo.isValid(); }
    BackendApi backend() const { return fInfo.backend(); }

    SkISize dimensions() const { return fDimensions; }

private:
    SkISize fDimensions;
    TextureInfo fInfo;

    union {
#ifdef SK_METAL
        sk_cfp<mtl::Handle> fMtlTexture;
#endif
    };
};

} // namespace skgpu

#endif // skgpu_BackendTexture_DEFINED

