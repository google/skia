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
    BackendTexture() {}
#ifdef SK_METAL
    // The BackendTexture will not call retain or release on the passed in mtl::Handle. Thus the
    // client must keep the mtl::Handle valid until they are no longer using the BackendTexture.
    BackendTexture(SkISize dimensions, mtl::Handle mtlTexture);
#endif

    BackendTexture(const BackendTexture&);

    ~BackendTexture();

    BackendTexture& operator=(const BackendTexture&);

    bool operator==(const BackendTexture&) const;
    bool operator!=(const BackendTexture& that) const { return !(*this == that); }

    bool isValid() const { return fInfo.isValid(); }
    BackendApi backend() const { return fInfo.backend(); }

    SkISize dimensions() const { return fDimensions; }

    const TextureInfo& info() const { return fInfo; }

#ifdef SK_METAL
    mtl::Handle getMtlTexture() const;
#endif

private:
    SkISize fDimensions;
    TextureInfo fInfo;

    union {
#ifdef SK_METAL
        mtl::Handle fMtlTexture;
#endif
    };
};

} // namespace skgpu

#endif // skgpu_BackendTexture_DEFINED

