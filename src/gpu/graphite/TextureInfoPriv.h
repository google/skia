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

private:
    TextureInfoPriv() = delete;
    TextureInfoPriv(const TextureInfoPriv&) = delete;
};

}  // namespace skgpu::graphite

#endif
