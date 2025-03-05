/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_TextureInfoPriv_DEFINED
#define skgpu_graphite_TextureInfoPriv_DEFINED

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/TextureFormat.h"

#include <cstdint>

namespace skgpu::graphite {

class Caps;

// NOTE: This is a class so that it can be friended by TextureInfo and the backend info classes.
class TextureInfoPriv {
public:
    static TextureFormat ViewFormat(const TextureInfo& info) {
        return info.fViewFormat;
    }
    static uint32_t ChannelMask(const TextureInfo& info) {
        return TextureFormatChannelMask(ViewFormat(info));
    }

    static SkString GetAttachmentLabel(const TextureInfo&);

    template <typename BackendTextureInfo>
    static TextureInfo Make(const BackendTextureInfo& data) {
        return TextureInfo(data);
    }

    template <typename BackendTextureInfo>
    static const BackendTextureInfo& Get(const TextureInfo& info) {
        SkASSERT(info.isValid() && info.backend() == BackendTextureInfo::kBackend);
        return *(static_cast<const BackendTextureInfo*>(info.fData.get()));
    }

    template <typename BackendTextureInfo>
    static bool Copy(const TextureInfo& info, BackendTextureInfo* out) {
        if (!info.isValid() || info.backend() != BackendTextureInfo::kBackend) {
            return false;
        }

        SkASSERT(out);
        *out = Get<BackendTextureInfo>(info);
        return true;
    }

    template <typename BackendTextureInfo>
    static bool Serialize(const TextureInfo& info, SkWStream* out) {
        if (info.isValid()) {
            if (!out->write32(GetInfoTag(info))) {
                return false;
            }
            return Get<BackendTextureInfo>(info).serialize(out);
        } else {
            // Write a 0 to signal an empty TextureInfo
            return out->write32(0);
        }
    }

    template <typename BackendTextureInfo>
    static bool Deserialize(SkStream* in, TextureInfo* out) {
        uint32_t tag;
        if (!in->readU32(&tag)) {
            return false;
        }
        if (tag == 0) {
            // Allow deserializing an empty info
            *out = TextureInfo();
            return true;
        }

        auto [backend, mipmapped, sampleCount] = ParseInfoTag(tag);
        if (backend != BackendTextureInfo::kBackend) {
            return false;
        }

        BackendTextureInfo info;
        info.fMipmapped = mipmapped;
        info.fSampleCount = sampleCount;
        if (info.deserialize(in)) {
            *out = Make(info);
            return true;
        }

        return false;
    }

private:
    TextureInfoPriv() = delete;
    TextureInfoPriv(const TextureInfoPriv&) = delete;

    static uint32_t GetInfoTag(const TextureInfo&);
    static std::tuple<BackendApi, Mipmapped, int> ParseInfoTag(uint32_t);
};

}  // namespace skgpu::graphite

#endif
