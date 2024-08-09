/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_TextureInfoPriv_DEFINED
#define skgpu_graphite_TextureInfoPriv_DEFINED

#include "include/core/SkString.h"
#include "include/gpu/graphite/TextureInfo.h"

#include <cstdint>

namespace skgpu::graphite {

class TextureInfoData {
public:
    virtual ~TextureInfoData();

#if defined(SK_DEBUG)
    virtual skgpu::BackendApi type() const = 0;
#endif
protected:
    TextureInfoData() = default;
    TextureInfoData(const TextureInfoData&) = default;

    using AnyTextureInfoData = TextureInfo::AnyTextureInfoData;

private:
    friend class TextureInfo;

    virtual size_t bytesPerPixel() const = 0;
    virtual SkTextureCompressionType compressionType() const = 0;
    virtual bool isMemoryless() const = 0;
    virtual SkString toString() const = 0;
    virtual SkString toRPAttachmentString(uint32_t sampleCount) const = 0;

    virtual void copyTo(AnyTextureInfoData&) const = 0;
    virtual bool equal(const TextureInfoData* that) const = 0;
    virtual bool isCompatible(const TextureInfoData* that) const = 0;
};

class TextureInfoPriv {
public:
    template <typename SomeTextureInfoData>
    static TextureInfo Make(BackendApi backend,
                            uint32_t sampleCount,
                            skgpu::Mipmapped mipped,
                            skgpu::Protected p,
                            const SomeTextureInfoData& data) {
        return TextureInfo(backend, sampleCount, mipped, p, data);
    }

    static const TextureInfoData* GetData(const TextureInfo& info) {
        return info.fTextureInfoData.get();
    }
};

}  // namespace skgpu::graphite

#endif
