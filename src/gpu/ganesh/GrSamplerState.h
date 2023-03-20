/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSamplerState_DEFINED
#define GrSamplerState_DEFINED

#include "include/core/SkSamplingOptions.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"

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

    static constexpr int kFilterCount     = static_cast<int>(Filter::kLast)     + 1;
    static constexpr int kWrapModeCount   = static_cast<int>(WrapMode::kLast)   + 1;
    static constexpr int kMipmapModeCount = static_cast<int>(MipmapMode::kLast) + 1;

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

    // We require 'viewIsMipped' for APIs that allow MIP filtering to be specified orthogonally
    // to aniso.
    static constexpr GrSamplerState Aniso(WrapMode wrapX,
                                          WrapMode wrapY,
                                          int maxAniso,
                                          skgpu::Mipmapped viewIsMipped) {
        GrSamplerState sampler;
        sampler.fWrapModes[0] = wrapX;
        sampler.fWrapModes[1] = wrapY;
        sampler.fMaxAniso     = SkTPin(maxAniso, 1, kMaxMaxAniso);
        sampler.fFilter       = Filter::kLinear;
        sampler.fMipmapMode   = viewIsMipped == skgpu::Mipmapped::kYes ? MipmapMode::kLinear
                                                                       : MipmapMode::kNone;
        return sampler;
    }

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

    constexpr skgpu::Mipmapped mipmapped() const {
        return skgpu::Mipmapped(fMipmapMode != MipmapMode::kNone);
    }

    int maxAniso() const { return fMaxAniso; }
    bool isAniso() const { return fMaxAniso > 1; }

    constexpr bool operator==(GrSamplerState that) const {
        return fWrapModes[0] == that.fWrapModes[0] && fWrapModes[1] == that.fWrapModes[1] &&
               fFilter == that.fFilter && fMipmapMode == that.fMipmapMode;
    }

    constexpr bool operator!=(const GrSamplerState& that) const { return !(*this == that); }

    /**
     * Turn the sampler state into an integer for use as a key. How that works for aniso depends
     * on whether the underlying API defines aniso as orthogonal to other filter settings or
     * as a replacement for them.
     */
    uint32_t asKey(bool anisoIsOrthogonal) const {
        constexpr int kNumWrapBits       = SkNextLog2_portable(kWrapModeCount);
        constexpr int kNumMaxAnisoBits   = SkNextLog2_portable(kMaxMaxAniso);
        constexpr int kNumFilterBits     = SkNextLog2_portable(kFilterCount);
        constexpr int kNumMipmapModeBits = SkNextLog2_portable(kMipmapModeCount);

        constexpr int kWrapXShift      = 0;
        constexpr int kWrapYShift      = kWrapXShift    + kNumWrapBits;
        constexpr int kMaxAnisoShift   = kWrapYShift    + kNumWrapBits;
        constexpr int kFilterShift     = kMaxAnisoShift + kNumMaxAnisoBits;
        constexpr int kMipmapModeShift = kFilterShift   + kNumFilterBits;

        static_assert(kMipmapModeShift + kNumMipmapModeBits <= 32);

        uint32_t key = (static_cast<uint32_t>(fWrapModes[0]) << kWrapXShift)
                     | (static_cast<uint32_t>(fWrapModes[1]) << kWrapYShift)
                     | (static_cast<uint32_t>(fMaxAniso)     << kMaxAnisoShift);
        if (fMaxAniso == 1 || anisoIsOrthogonal) {
            key |= (static_cast<uint32_t>(fFilter)     << kFilterShift)
                |  (static_cast<uint32_t>(fMipmapMode) << kMipmapModeShift);
        }
        return key;
    }

private:
    // This is here to support turning the sampler state into a key. Presumably this is way larger
    // than any HW limits. Also, WebGPU accepts this as an unsigned short.
    static constexpr int kMaxMaxAniso = 1024;

    WrapMode   fWrapModes[2] = {WrapMode::kClamp, WrapMode::kClamp};
    Filter     fFilter       = GrSamplerState::Filter::kNearest;
    MipmapMode fMipmapMode   = GrSamplerState::MipmapMode::kNone;
    int        fMaxAniso     = 1;
};

#endif
