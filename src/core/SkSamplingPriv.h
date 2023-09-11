/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSamplingPriv_DEFINED
#define SkSamplingPriv_DEFINED

#include "include/core/SkSamplingOptions.h"

class SkReadBuffer;
class SkWriteBuffer;

// Given a src rect in texels to be filtered, this number of surrounding texels are needed by
// the kernel in x and y.
static constexpr int kBicubicFilterTexelPad = 2;

// Private copy of SkFilterQuality, just for legacy deserialization
// Matches values in SkFilterQuality
enum SkLegacyFQ {
    kNone_SkLegacyFQ   = 0,    //!< nearest-neighbor; fastest but lowest quality
    kLow_SkLegacyFQ    = 1,    //!< bilerp
    kMedium_SkLegacyFQ = 2,    //!< bilerp + mipmaps; good for down-scaling
    kHigh_SkLegacyFQ   = 3,    //!< bicubic resampling; slowest but good quality

    kLast_SkLegacyFQ = kHigh_SkLegacyFQ,
};

// Matches values in SkSamplingOptions::MediumBehavior
enum SkMediumAs {
    kNearest_SkMediumAs,
    kLinear_SkMediumAs,
};

class SkSamplingPriv {
public:
    static size_t FlatSize(const SkSamplingOptions& options) {
        size_t size = sizeof(uint32_t);  // maxAniso
        if (!options.isAniso()) {
            size += 3 * sizeof(uint32_t);  // bool32 + [2 floats | 2 ints]
        }
        return size;
    }

    // Returns true if the sampling can be ignored when the CTM is identity.
    static bool NoChangeWithIdentityMatrix(const SkSamplingOptions& sampling) {
        // If B == 0, the cubic resampler should have no effect for identity matrices
        // https://entropymine.com/imageworsener/bicubic/
        // We assume aniso has no effect with an identity transform.
        return !sampling.useCubic || sampling.cubic.B == 0;
    }

    // Makes a fallback SkSamplingOptions for cases where anisotropic filtering is not allowed.
    // anisotropic filtering can access mip levels if present, but we don't add mipmaps to non-
    // mipmapped images when the user requests anisotropic. So we shouldn't fall back to a
    // sampling that would trigger mip map creation.
    static SkSamplingOptions AnisoFallback(bool imageIsMipped) {
        auto mm = imageIsMipped ? SkMipmapMode::kLinear : SkMipmapMode::kNone;
        return SkSamplingOptions(SkFilterMode::kLinear, mm);
    }

    static SkSamplingOptions FromFQ(SkLegacyFQ fq, SkMediumAs behavior = kNearest_SkMediumAs) {
        switch (fq) {
            case kHigh_SkLegacyFQ:
                return SkSamplingOptions(SkCubicResampler{1/3.0f, 1/3.0f});
            case kMedium_SkLegacyFQ:
                return SkSamplingOptions(SkFilterMode::kLinear,
                                          behavior == kNearest_SkMediumAs ? SkMipmapMode::kNearest
                                                                          : SkMipmapMode::kLinear);
            case kLow_SkLegacyFQ:
                return SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
            case kNone_SkLegacyFQ:
                break;
        }
        return SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
    }
};

#endif
