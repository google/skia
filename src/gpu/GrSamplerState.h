/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSamplerState_DEFINED
#define GrSamplerState_DEFINED

#include "include/core/SkSamplingOptions.h"
#include "include/gpu/GrTypes.h"
#include <limits>

/**
 * Represents the filtering and tile modes used to access a texture.
 */
class GrSamplerState {
public:
    using Filter = SkFilterMode;
    using MipmapMode = SkMipmapMode;

    enum class WrapMode : uint8_t {
        kClamp,
        kRepeat,
        kMirrorRepeat,
        kClampToBorder,
        kLast = kClampToBorder
    };

    inline static constexpr int kFilterCount = static_cast<int>(Filter::kLast) + 1;
    inline static constexpr int kWrapModeCount = static_cast<int>(WrapMode::kLast) + 1;

    constexpr GrSamplerState() = default;

    constexpr GrSamplerState(WrapMode wrapXAndY, Filter filter, MipmapMode mm = MipmapMode::kNone)
            : fWrapModes{wrapXAndY, wrapXAndY}, fFilter(filter), fMipmapMode(mm) {}

    constexpr GrSamplerState(WrapMode wrapX,
                             WrapMode wrapY,
                             Filter filter,
                             MipmapMode mm = MipmapMode::kNone)
            : fWrapModes{wrapX, wrapY}, fFilter(filter), fMipmapMode(mm) {}

    constexpr GrSamplerState(const WrapMode wrapModes[2],
                             Filter filter,
                             MipmapMode mm = MipmapMode::kNone)
            : fWrapModes{wrapModes[0], wrapModes[1]}, fFilter(filter), fMipmapMode(mm) {}

    constexpr /*explicit*/ GrSamplerState(Filter filter) : fFilter(filter) {}
    constexpr GrSamplerState(Filter filter, MipmapMode mm) : fFilter(filter), fMipmapMode(mm) {}

    constexpr GrSamplerState(const GrSamplerState&) = default;

    constexpr GrSamplerState& operator=(const GrSamplerState&) = default;

    constexpr WrapMode wrapModeX() const { return fWrapModes[0]; }

    constexpr WrapMode wrapModeY() const { return fWrapModes[1]; }

    constexpr bool isRepeatedX() const {
        return fWrapModes[0] == WrapMode::kRepeat || fWrapModes[0] == WrapMode::kMirrorRepeat;
    }

    constexpr bool isRepeatedY() const {
        return fWrapModes[1] == WrapMode::kRepeat || fWrapModes[1] == WrapMode::kMirrorRepeat;
    }

    constexpr bool isRepeated() const {
        return this->isRepeatedX() || this->isRepeatedY();
    }

    constexpr Filter filter() const { return fFilter; }

    constexpr MipmapMode mipmapMode() const { return fMipmapMode; }

    constexpr GrMipmapped mipmapped() const {
        return GrMipmapped(fMipmapMode != MipmapMode::kNone);
    }

    constexpr void setFilterMode(Filter filterMode) { fFilter = filterMode; }

    constexpr void setMipmapMode(MipmapMode mm) { fMipmapMode = mm; }

    constexpr void setWrapModeX(const WrapMode wrap) { fWrapModes[0] = wrap; }

    constexpr void setWrapModeY(const WrapMode wrap) { fWrapModes[1] = wrap; }

    constexpr bool operator==(GrSamplerState that) const {
        return fWrapModes[0] == that.fWrapModes[0] && fWrapModes[1] == that.fWrapModes[1] &&
               fFilter == that.fFilter && fMipmapMode == that.fMipmapMode;
    }

    constexpr bool operator!=(const GrSamplerState& that) const { return !(*this == that); }

    /**
     * Turn the sampler state into an integer from a tightly packed range for use as an index
     * (or key)
     */
    constexpr uint8_t asIndex() const {
        constexpr int kNumWraps   = static_cast<int>(WrapMode::kLast) + 1;
        constexpr int kNumFilters = static_cast<int>(Filter::kLast  ) + 1;
        int result = static_cast<int>(fWrapModes[0])*1
                   + static_cast<int>(fWrapModes[1])*kNumWraps
                   + static_cast<int>(fFilter)      *kNumWraps*kNumWraps
                   + static_cast<int>(fMipmapMode)  *kNumWraps*kNumWraps*kNumFilters;
        SkASSERT(result <= kNumUniqueSamplers);
        return static_cast<uint8_t>(result);
    }

    inline static constexpr int kNumUniqueSamplers = (static_cast<int>(WrapMode::kLast  ) + 1)
                                                   * (static_cast<int>(WrapMode::kLast  ) + 1)
                                                   * (static_cast<int>(Filter::kLast    ) + 1)
                                                   * (static_cast<int>(MipmapMode::kLast) + 1);
private:
    WrapMode fWrapModes[2] = {WrapMode::kClamp, WrapMode::kClamp};
    Filter fFilter = GrSamplerState::Filter::kNearest;
    MipmapMode fMipmapMode = GrSamplerState::MipmapMode::kNone;
};

static_assert(GrSamplerState::kNumUniqueSamplers <=
              std::numeric_limits<decltype(GrSamplerState{}.asIndex())>::max());

#endif
