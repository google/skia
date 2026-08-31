/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_EndCaps_DEFINED
#define skgpu_graphite_geom_EndCaps_DEFINED

#include <cstdint>
#include <utility>
#include "include/private/SkLog.h"
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

namespace skgpu::graphite {

// Container holding the "EndCaps" produced by SparseStrips rendering. An EndCap stores the
// antialiased portion of the path rendering, and spans a variable non-zero number of tiles.
class EndCaps {
public:
    struct EndCap {
        EndCap(uint16_t x, uint16_t y, uint16_t width, int32_t alphaIndex, uint16_t texPage)
                : fX(x), fY(y), fWidth(width), fAlphaIndex(alphaIndex), fTexPage(texPage),
                  fPadding(0xffffffff) {}

        uint16_t fX;            // Top left coordinates of the EndCap.
        uint16_t fY;            // ``
        uint16_t fWidth;        // The width of the EndCap in pixels
        int32_t  fAlphaIndex;   // The EndCap's offset into the alpha buffer.
        uint16_t fTexPage;      // The associated page in the EndCap's backing texture
        uint32_t fPadding;      // Pad to vec4 alignment
    };

    EndCaps() = default;

    void addCap(uint16_t x,
                uint16_t y,
                uint16_t width,
                int32_t alphaIndex,
                uint16_t texPage) {
        fCaps.push_back(EndCap(x, y, width, alphaIndex, texPage));
    }

    void addProxy(sk_sp<TextureProxy> texture) {
        if (fProxies.size() < SparseStripConfig::kMaxTexturePages) {
            fProxies.push_back(texture);
        } else {
            SKIA_LOG_W("Warning, exceeded max textures per endcap renderstep");
        }
    }

    const skia_private::TArray<EndCap>& caps() const { return fCaps; }
    skia_private::TArray<EndCap>& caps() { return fCaps; }
    const skia_private::TArray<sk_sp<TextureProxy>>& proxies() const { return fProxies; }

    bool empty() const { return fCaps.empty(); }
    size_t size() const { return fCaps.size(); }
    void clear() {
        fCaps.clear();
        fProxies.clear();
    }

private:
    skia_private::TArray<EndCap> fCaps;
    skia_private::STArray<SparseStripConfig::kMaxTexturePages, sk_sp<TextureProxy>> fProxies;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_EndCaps_DEFINED
