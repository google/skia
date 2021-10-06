/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Caps_DEFINED
#define skgpu_Caps_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"

namespace skgpu {

class TextureInfo;

class Caps : public SkRefCnt {
public:
    ~Caps() override {}

    virtual TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                                     uint32_t sampleCount,
                                                     uint32_t levelCount,
                                                     Protected,
                                                     Renderable) = 0;

    virtual TextureInfo getDefaultDepthStencilTextureInfo(DepthStencilType,
                                                          uint32_t sampleCount,
                                                          Protected) = 0;

protected:
    Caps();

private:
};

} // namespace skgpu

#endif // skgpu_Caps_DEFINED
