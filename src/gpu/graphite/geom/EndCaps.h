/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_EndCaps_DEFINED
#define skgpu_graphite_geom_EndCaps_DEFINED

#include <array>
#include <cstdint>
#include <type_traits>
#include <utility>
#include "include/private/SkLog.h"
#include "include/private/SkTArray.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"

namespace skgpu::graphite {

// TODO (thomsmit): Rename "NullCaps" to "Overflow" caps for clarity
// Container holding the "EndCaps" produced by SparseStrips rendering. An EndCap stores the
// antialiased portion of the path rendering, and spans a variable non-zero number of tiles.
class EndCaps {
public:
    struct EndCap {
        EndCap(uint16_t x, uint16_t y, uint16_t width, int32_t alphaIndex, uint16_t texPage)
                : fX(x), fY(y), fWidth(width), fTexPage(texPage), fAlphaIndex(alphaIndex)
                , fPadding(0xffffffff) {}

        uint16_t fX;            // Top left coordinates of the EndCap.
        uint16_t fY;            // ``
        uint16_t fWidth;        // The width of the EndCap in pixels
        uint16_t fTexPage;      // The associated page in the EndCap's backing texture
        int32_t  fAlphaIndex;   // The EndCap's offset into the alpha buffer.
        uint32_t fPadding;      // Pad to vec4 alignment
    };
    static_assert(sizeof(EndCap) == 16);
    static_assert(std::is_trivially_copyable_v<EndCap>);

    EndCaps() = default;

    void addCap(uint16_t x,
                uint16_t y,
                uint16_t width,
                int32_t alphaIndex,
                uint16_t texPage) {
        fCaps.push_back(EndCap(x, y, width, alphaIndex, texPage));
    }

    void markFirstNullCap() {
        if (fFirstNullCapIndex == kInvalidIndex) {
            fFirstNullCapIndex = static_cast<int32_t>(fCaps.size());
        }
    }

    void setProxy(int slot, sk_sp<TextureProxy> proxy) {
        SkASSERT(slot >= 0 && slot < SparseStripConfig::kMaxTexturePages);
        fProxies[slot] = std::move(proxy);
    }

    const skia_private::TArray<EndCap>& caps() const { return fCaps; }
    skia_private::TArray<EndCap>& caps() { return fCaps; }
    const std::array<sk_sp<TextureProxy>, SparseStripConfig::kMaxTexturePages>& proxies() const {
        return fProxies;
    }

    bool empty() const { return fCaps.empty(); }
    size_t size() const { return fCaps.size(); }
    bool hasNullCaps() const { return fFirstNullCapIndex != kInvalidIndex; }
    int32_t firstNullCapIndex() const { return fFirstNullCapIndex; }
    void setFirstNullCapIndex(int32_t index) { fFirstNullCapIndex = index; }
    void clearNullCaps() { fFirstNullCapIndex = kInvalidIndex; }

    int32_t drawStartIndex() const { return fDrawStartIndex; }
    int32_t drawEndIndex() const {
        return this->hasNullCaps() ? fFirstNullCapIndex : static_cast<int32_t>(fCaps.size());
    }
    void setDrawStartIndex(int32_t index) { fDrawStartIndex = index; }

    void clear() {
        fCaps.clear();
        for (auto& proxy : fProxies) {
            proxy.reset();
        }
        fDrawStartIndex = 0;
        fFirstNullCapIndex = kInvalidIndex;
    }

private:
    static constexpr int32_t kInvalidIndex = -1;

    skia_private::TArray<EndCap> fCaps;
    std::array<sk_sp<TextureProxy>, SparseStripConfig::kMaxTexturePages> fProxies;
    int32_t fDrawStartIndex = 0;
    int32_t fFirstNullCapIndex = kInvalidIndex;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_EndCaps_DEFINED
