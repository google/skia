/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSamplerState_DEFINED
#define GrSamplerState_DEFINED

#include "include/gpu/GrTypes.h"

/**
 * Represents the filtering and tile modes used to access a texture.
 */
class GrSamplerState {
public:
    enum class Filter : uint8_t { kNearest, kBilerp, kMipMap, kLast = kMipMap };
    enum class WrapMode : uint8_t { kClamp, kRepeat, kMirrorRepeat, kClampToBorder,
                                    kLast = kClampToBorder };

    static constexpr int kFilterCount = static_cast<int>(Filter::kLast) + 1;
    static constexpr int kWrapModeCount = static_cast<int>(WrapMode::kLast) + 1;

    constexpr GrSamplerState() = default;

    constexpr GrSamplerState(WrapMode wrapXAndY, Filter filter)
            : fWrapModes{wrapXAndY, wrapXAndY}, fFilter(filter) {}

    constexpr GrSamplerState(WrapMode wrapX, WrapMode wrapY, Filter filter)
            : fWrapModes{wrapX, wrapY}, fFilter(filter) {}

    constexpr GrSamplerState(const WrapMode wrapModes[2], Filter filter)
            : fWrapModes{wrapModes[0], wrapModes[1]}, fFilter(filter) {}

    constexpr /*explicit*/ GrSamplerState(Filter filter) : fFilter(filter) {}

    constexpr GrSamplerState(const GrSamplerState&) = default;

    constexpr GrSamplerState& operator=(const GrSamplerState&) = default;

    constexpr Filter filter() const { return fFilter; }

    constexpr void setFilterMode(Filter filterMode) { fFilter = filterMode; }

    constexpr void setWrapModeX(const WrapMode wrap) { fWrapModes[0] = wrap; }
    constexpr void setWrapModeY(const WrapMode wrap) { fWrapModes[1] = wrap; }

    constexpr WrapMode wrapModeX() const { return fWrapModes[0]; }
    constexpr WrapMode wrapModeY() const { return fWrapModes[1]; }

    constexpr bool isRepeated() const {
        return (WrapMode::kClamp != fWrapModes[0] && WrapMode::kClampToBorder != fWrapModes[0]) ||
               (WrapMode::kClamp != fWrapModes[1] && WrapMode::kClampToBorder != fWrapModes[1]);
    }

    constexpr bool operator==(GrSamplerState that) const {
        return fWrapModes[0] == that.fWrapModes[0] && fWrapModes[1] == that.fWrapModes[1] &&
               fFilter == that.fFilter;
    }

    constexpr bool operator!=(const GrSamplerState& that) const { return !(*this == that); }

    constexpr static uint8_t GenerateKey(GrSamplerState samplerState) {
        const int kTileModeXShift = 2;
        const int kTileModeYShift = 4;

        SkASSERT(static_cast<int>(samplerState.filter()) <= 3);
        uint8_t key = static_cast<uint8_t>(samplerState.filter());

        SkASSERT(static_cast<int>(samplerState.wrapModeX()) <= 3);
        key |= (static_cast<uint8_t>(samplerState.wrapModeX()) << kTileModeXShift);

        SkASSERT(static_cast<int>(samplerState.wrapModeY()) <= 3);
        key |= (static_cast<uint8_t>(samplerState.wrapModeY()) << kTileModeYShift);

        return key;
    }

private:
    WrapMode fWrapModes[2] = {WrapMode::kClamp, WrapMode::kClamp};
    Filter fFilter = GrSamplerState::Filter::kNearest;
};

#endif
