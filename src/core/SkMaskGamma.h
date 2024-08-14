/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskGamma_DEFINED
#define SkMaskGamma_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTo.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>

/**
 * SkColorSpaceLuminance is used to convert luminances to and from linear and
 * perceptual color spaces.
 *
 * Luma is used to specify a linear luminance value [0.0, 1.0].
 * Luminance is used to specify a luminance value in an arbitrary color space [0.0, 1.0].
 */
class SkColorSpaceLuminance : SkNoncopyable {
public:
    virtual ~SkColorSpaceLuminance() { }

    /** Converts a color component luminance in the color space to a linear luma. */
    virtual SkScalar toLuma(SkScalar gamma, SkScalar luminance) const = 0;
    /** Converts a linear luma to a color component luminance in the color space. */
    virtual SkScalar fromLuma(SkScalar gamma, SkScalar luma) const = 0;

    /** Converts a color to a luminance value. */
    static U8CPU computeLuminance(SkScalar gamma, SkColor c) {
        const SkColorSpaceLuminance& luminance = Fetch(gamma);
        SkScalar r = luminance.toLuma(gamma, SkIntToScalar(SkColorGetR(c)) / 255);
        SkScalar g = luminance.toLuma(gamma, SkIntToScalar(SkColorGetG(c)) / 255);
        SkScalar b = luminance.toLuma(gamma, SkIntToScalar(SkColorGetB(c)) / 255);
        SkScalar luma = r * SK_LUM_COEFF_R +
                        g * SK_LUM_COEFF_G +
                        b * SK_LUM_COEFF_B;
        SkASSERT(luma <= SK_Scalar1);
        return SkScalarRoundToInt(luminance.fromLuma(gamma, luma) * 255);
    }

    /** Retrieves the SkColorSpaceLuminance for the given gamma. */
    static const SkColorSpaceLuminance& Fetch(SkScalar gamma);
};

///@{
/**
 * Scales base <= 2^N-1 to 2^8-1
 * @param N [1, 8] the number of bits used by base.
 * @param base the number to be scaled to [0, 255].
 */
template<U8CPU N> static inline U8CPU sk_t_scale255(U8CPU base) {
    base <<= (8 - N);
    U8CPU lum = base;
    for (unsigned int i = N; i < 8; i += N) {
        lum |= base >> i;
    }
    return lum;
}
template<> /*static*/ inline U8CPU sk_t_scale255<1>(U8CPU base) {
    return base * 0xFF;
}
template<> /*static*/ inline U8CPU sk_t_scale255<2>(U8CPU base) {
    return base * 0x55;
}
template<> /*static*/ inline U8CPU sk_t_scale255<4>(U8CPU base) {
    return base * 0x11;
}
template<> /*static*/ inline U8CPU sk_t_scale255<8>(U8CPU base) {
    return base;
}
///@}

template <int R_LUM_BITS, int G_LUM_BITS, int B_LUM_BITS> class SkTMaskPreBlend;

void SkTMaskGamma_build_correcting_lut(uint8_t* table, U8CPU srcI, SkScalar contrast,
                                       const SkColorSpaceLuminance& dstConvert, SkScalar dstGamma);

/**
 * A regular mask contains linear alpha values. A gamma correcting mask
 * contains non-linear alpha values in an attempt to create gamma correct blits
 * in the presence of a gamma incorrect (linear) blend in the blitter.
 *
 * SkMaskGamma creates and maintains tables which convert linear alpha values
 * to gamma correcting alpha values.
 * @param R The number of luminance bits to use [1, 8] from the red channel.
 * @param G The number of luminance bits to use [1, 8] from the green channel.
 * @param B The number of luminance bits to use [1, 8] from the blue channel.
 */
template <int R_LUM_BITS, int G_LUM_BITS, int B_LUM_BITS> class SkTMaskGamma : public SkRefCnt {

public:

    /** Creates a linear SkTMaskGamma. */
    constexpr SkTMaskGamma() {}

    /**
     * Creates tables to convert linear alpha values to gamma correcting alpha
     * values.
     *
     * @param contrast A value in the range [0.0, 1.0] which indicates the
     *                 amount of artificial contrast to add.
     * @param device The color space of the target device.
     */
    SkTMaskGamma(SkScalar contrast, SkScalar deviceGamma)
        : fGammaTables(std::make_unique<uint8_t[]>(kTableNumElements))
    {
        const SkColorSpaceLuminance& deviceConvert = SkColorSpaceLuminance::Fetch(deviceGamma);
        for (U8CPU i = 0; i < kNumTables; ++i) {
            U8CPU lum = sk_t_scale255<kMaxLumBits>(i);
            SkTMaskGamma_build_correcting_lut(&fGammaTables[i * kTableWidth], lum, contrast,
                                              deviceConvert, deviceGamma);
        }
    }

    /** Given a color, returns the closest canonical color. */
    static SkColor CanonicalColor(SkColor color) {
        return SkColorSetRGB(
                   sk_t_scale255<R_LUM_BITS>(SkColorGetR(color) >> (8 - R_LUM_BITS)),
                   sk_t_scale255<G_LUM_BITS>(SkColorGetG(color) >> (8 - G_LUM_BITS)),
                   sk_t_scale255<B_LUM_BITS>(SkColorGetB(color) >> (8 - B_LUM_BITS)));
    }

    /** The type of the mask pre-blend which will be returned from preBlend(SkColor). */
    typedef SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS> PreBlend;

    /**
     * Provides access to the tables appropriate for converting linear alpha
     * values into gamma correcting alpha values when drawing the given color
     * through the mask. The destination color will be approximated.
     */
    PreBlend preBlend(SkColor color) const;

    /**
     * Get dimensions for the full table set, so it can be allocated as a block. Linear
     * tables should report the full table size.
     */
    void getGammaTableDimensions(int* tableWidth, int* numTables) const {
        *tableWidth = kTableWidth;
        *numTables = kNumTables;
    }

    /**
     * Returns the size for the full table set in bytes, so it can be allocated as a block.
     * Linear tables should report the full table size.
     */
    constexpr size_t getGammaTableSizeInBytes() const {
        return kTableNumElements * sizeof(uint8_t);
    }

    /**
     * Provides direct access to the full table set, so it can be uploaded
     * into a texture or analyzed in other ways.
     * Returns nullptr if fGammaTables hasn't been initialized.
     */
    const uint8_t* getGammaTables() const {
        return fGammaTables.get();
    }

private:
    static constexpr int kMaxLumBits = std::max({B_LUM_BITS, R_LUM_BITS, G_LUM_BITS});
    static constexpr size_t kNumTables = 1 << kMaxLumBits;
    static constexpr size_t kTableWidth = 256;
    static constexpr size_t kTableNumElements = kNumTables * kTableWidth;

    constexpr bool isLinear() const {
        return fGammaTables == nullptr;
    }

    /**
     * fGammaTables is a flattened 2-D array. Accessing rows requires accounting
     * for the width dimension (via kTableWidth).
     */
    std::unique_ptr<uint8_t[]> fGammaTables;

    using INHERITED = SkRefCnt;
};

/**
 * SkTMaskPreBlend is a tear-off of SkTMaskGamma. It provides the tables to
 * convert a linear alpha value for a given channel to a gamma correcting alpha
 * value for that channel. This class is immutable.
 *
 * If fR, fG, or fB is nullptr, all of them will be. This indicates that no mask
 * pre blend should be applied. SkTMaskPreBlend::isApplicable() is provided as
 * a convenience function to test for the absence of this case.
 */
template <int R_LUM_BITS, int G_LUM_BITS, int B_LUM_BITS> class SkTMaskPreBlend {
private:
    SkTMaskPreBlend(sk_sp<const SkTMaskGamma<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>> parent,
                    const uint8_t* r, const uint8_t* g, const uint8_t* b)
    : fParent(std::move(parent)), fR(r), fG(g), fB(b) { }

    sk_sp<const SkTMaskGamma<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>> fParent;
    friend class SkTMaskGamma<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>;
public:
    /** Creates a non applicable SkTMaskPreBlend. */
    SkTMaskPreBlend() : fParent(), fR(nullptr), fG(nullptr), fB(nullptr) { }

    /**
     * This copy contructor exists for correctness, but should never be called
     * when return value optimization is enabled.
     */
    SkTMaskPreBlend(const SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>& that)
    : fParent(that.fParent), fR(that.fR), fG(that.fG), fB(that.fB) { }

    ~SkTMaskPreBlend() { }

    /** True if this PreBlend should be applied. When false, fR, fG, and fB are nullptr. */
    bool isApplicable() const { return SkToBool(this->fG); }

    const uint8_t* fR;
    const uint8_t* fG;
    const uint8_t* fB;
};

template <int R_LUM_BITS, int G_LUM_BITS, int B_LUM_BITS>
SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>
SkTMaskGamma<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>::preBlend(SkColor color) const {
    if (isLinear()) {
        return SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>();
    }
    constexpr size_t lum_shift = 8 - kMaxLumBits;
    const size_t r_index = (SkColorGetR(color) >> lum_shift) * kTableWidth;
    const size_t g_index = (SkColorGetG(color) >> lum_shift) * kTableWidth;
    const size_t b_index = (SkColorGetB(color) >> lum_shift) * kTableWidth;
    SkASSERT(r_index < kTableNumElements &&
             g_index < kTableNumElements &&
             b_index < kTableNumElements);
    return SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>(sk_ref_sp(this),
                         &fGammaTables[r_index],
                         &fGammaTables[g_index],
                         &fGammaTables[b_index]);
}

///@{
/**
 *  If APPLY_LUT is false, returns component unchanged.
 *  If APPLY_LUT is true, returns lut[component].
 *  @param APPLY_LUT whether or not the look-up table should be applied to component.
 *  @component the initial component.
 *  @lut a look-up table which transforms the component.
 */
template<bool APPLY_LUT> static inline U8CPU sk_apply_lut_if(U8CPU component, const uint8_t*) {
    return component;
}
template<> /*static*/ inline U8CPU sk_apply_lut_if<true>(U8CPU component, const uint8_t* lut) {
    return lut[component];
}
///@}

#endif
