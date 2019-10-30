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
    enum class Filter : uint8_t { kNearest, kBilerp, kMipMap };
    enum class WrapMode : uint8_t { kClamp, kRepeat, kMirrorRepeat, kClampToBorder };

    static constexpr GrSamplerState ClampNearest() { return GrSamplerState(); }
    static constexpr GrSamplerState ClampBilerp() {
        return GrSamplerState(WrapMode::kClamp, Filter::kBilerp);
    }

    constexpr GrSamplerState() : GrSamplerState(WrapMode::kClamp, Filter::kNearest) {}

    constexpr GrSamplerState(WrapMode wrapXAndY, Filter filter)
            : fWrapModes{wrapXAndY, wrapXAndY}, fFilter(filter) {}

    constexpr GrSamplerState(const WrapMode wrapModes[2], Filter filter)
            : fWrapModes{wrapModes[0], wrapModes[1]}, fFilter(filter) {}

    constexpr GrSamplerState(const GrSamplerState&) = default;

    GrSamplerState& operator=(const GrSamplerState& that) {
        fWrapModes[0] = that.fWrapModes[0];
        fWrapModes[1] = that.fWrapModes[1];
        fFilter = that.fFilter;
        return *this;
    }

    Filter filter() const { return fFilter; }

    void setFilterMode(Filter filterMode) { fFilter = filterMode; }

    void setWrapModeX(const WrapMode wrap) { fWrapModes[0] = wrap; }
    void setWrapModeY(const WrapMode wrap) { fWrapModes[1] = wrap; }

    WrapMode wrapModeX() const { return fWrapModes[0]; }
    WrapMode wrapModeY() const { return fWrapModes[1]; }

    bool isRepeated() const {
        return (WrapMode::kClamp != fWrapModes[0] && WrapMode::kClampToBorder != fWrapModes[0]) ||
               (WrapMode::kClamp != fWrapModes[1] && WrapMode::kClampToBorder != fWrapModes[1]);
    }

    bool operator==(const GrSamplerState& that) const {
        return fWrapModes[0] == that.fWrapModes[0] && fWrapModes[1] == that.fWrapModes[1] &&
               fFilter == that.fFilter;
    }

    bool operator!=(const GrSamplerState& that) const { return !(*this == that); }

    static uint8_t GenerateKey(const GrSamplerState& samplerState) {
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
    WrapMode fWrapModes[2];
    Filter fFilter;
};

#endif
