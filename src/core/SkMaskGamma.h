/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskGamma_DEFINED
#define SkMaskGamma_DEFINED

#include "SkTypes.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkRefCnt.h"

/**
 * SkColorSpaceLuminance is used to convert luminances to and from linear and
 * perceptual color spaces.
 *
 * Luma is used to specify a linear luminance value [0.0, 1.0].
 * Luminance is used to specify a luminance value in an arbitrary color space [0.0, 1.0].
 */
class SkColorSpaceLuminance : SkNoncopyable {
public:
    virtual ~SkColorSpaceLuminance() {};

    /** Converts a color component luminance in the color space to a linear luma. */
    virtual SkScalar toLuma(SkScalar luminance) const = 0;
    /** Converts a linear luma to a color component luminance in the color space. */
    virtual SkScalar fromLuma(SkScalar luma) const = 0;

    /** Converts a color to a luminance value. */
    U8CPU computeLuminance(SkColor c) const {
        SkScalar r = toLuma(SkIntToScalar(SkColorGetR(c)) / 255);
        SkScalar g = toLuma(SkIntToScalar(SkColorGetG(c)) / 255);
        SkScalar b = toLuma(SkIntToScalar(SkColorGetB(c)) / 255);
        SkScalar luma = r * SkFloatToScalar(SK_LUM_COEFF_R) +
                        g * SkFloatToScalar(SK_LUM_COEFF_G) +
                        b * SkFloatToScalar(SK_LUM_COEFF_B);
        SkASSERT(luma <= SK_Scalar1);
        return SkScalarRoundToInt(fromLuma(luma) * 255);
    }
};

class SkSRGBLuminance : public SkColorSpaceLuminance {
public:
    SkScalar toLuma(SkScalar luminance) const SK_OVERRIDE;
    SkScalar fromLuma(SkScalar luma) const SK_OVERRIDE;
};

class SkGammaLuminance : public SkColorSpaceLuminance {
public:
    SkGammaLuminance(SkScalar gamma);
    SkScalar toLuma(SkScalar luminance) const SK_OVERRIDE;
    SkScalar fromLuma(SkScalar luma) const SK_OVERRIDE;
private:
    SkScalar fGamma;
    SkScalar fGammaInverse;
};

class SkLinearLuminance : public SkColorSpaceLuminance {
public:
    SkScalar toLuma(SkScalar luminance) const SK_OVERRIDE;
    SkScalar fromLuma(SkScalar luma) const SK_OVERRIDE;
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

void SkTMaskGamma_build_correcting_lut(uint8_t table[256], U8CPU srcI, SkScalar contrast,
                                       const SkColorSpaceLuminance& srcConvert,
                                       const SkColorSpaceLuminance& dstConvert);

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
    SK_DECLARE_INST_COUNT_TEMPLATE(SkTMaskGamma)

    SkTMaskGamma() : fIsLinear(true) {
    }

    /**
     * Creates tables to convert linear alpha values to gamma correcting alpha
     * values.
     *
     * @param contrast A value in the range [0.0, 1.0] which indicates the
     *                 amount of artificial contrast to add.
     * @param paint The color space in which the paint color was chosen.
     * @param device The color space of the target device.
     */
    SkTMaskGamma(SkScalar contrast,
                 const SkColorSpaceLuminance& paint,
                 const SkColorSpaceLuminance& device) : fIsLinear(false) {
        for (U8CPU i = 0; i < (1 << kLuminanceBits_Max); ++i) {
            U8CPU lum = sk_t_scale255<kLuminanceBits_Max>(i);
            SkTMaskGamma_build_correcting_lut(fGammaTables[i], lum, contrast, paint, device);
        }
    }

    /** Given a color, returns the closest cannonical color. */
    static SkColor cannonicalColor(SkColor color) {
        return SkColorSetRGB(
                   sk_t_scale255<kLuminanceBits_R>(SkColorGetR(color) >> (8 - kLuminanceBits_R)),
                   sk_t_scale255<kLuminanceBits_G>(SkColorGetG(color) >> (8 - kLuminanceBits_G)),
                   sk_t_scale255<kLuminanceBits_B>(SkColorGetB(color) >> (8 - kLuminanceBits_B)));
    }

    /** The type of the mask pre-blend which will be returned from preBlend(SkColor). */
    typedef SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS> PreBlend;

    /**
     * Provides access to the tables appropriate for converting linear alpha
     * values into gamma correcting alpha values when drawing the given color
     * through the mask. The destination color will be approximated.
     */
    PreBlend preBlend(SkColor color);

private:
    enum LuminanceBits {
        kLuminanceBits_R = R_LUM_BITS,
        kLuminanceBits_G = G_LUM_BITS,
        kLuminanceBits_B = B_LUM_BITS,
        kLuminanceBits_Max = B_LUM_BITS > (R_LUM_BITS > G_LUM_BITS ? R_LUM_BITS : G_LUM_BITS)
                           ? B_LUM_BITS
                           : (R_LUM_BITS > G_LUM_BITS ? R_LUM_BITS : G_LUM_BITS)
    };
    uint8_t fGammaTables[1 << kLuminanceBits_Max][256];
    bool fIsLinear;

    typedef SkRefCnt INHERITED;
};


#define MacroComma ,
SK_DEFINE_INST_COUNT_TEMPLATE(
    template <int R_LUM_BITS MacroComma int G_LUM_BITS MacroComma int B_LUM_BITS>,
    SkTMaskGamma<R_LUM_BITS MacroComma G_LUM_BITS MacroComma B_LUM_BITS>);

/**
 * SkTMaskPreBlend is a tear-off of SkTMaskGamma. It provides the tables to
 * convert a linear alpha value for a given channel to a gamma correcting alpha
 * value for that channel. This class is immutable.
 *
 * If fR, fG, or fB is NULL, all of them will be. This indicates that no mask
 * pre blend should be applied.
 */
template <int R_LUM_BITS, int G_LUM_BITS, int B_LUM_BITS> class SkTMaskPreBlend {
private:
    SkTMaskPreBlend(SkTMaskGamma<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>* parent,
                    const uint8_t* r,
                    const uint8_t* g,
                    const uint8_t* b)
    : fParent(parent), fR(r), fG(g), fB(b) {
        SkSafeRef(parent);
    }
    SkAutoTUnref<SkTMaskGamma<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS> > fParent;
    friend class SkTMaskGamma<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>;
public:
    /**
     * This copy contructor exists for correctness, but should never be called
     * when return value optimization is enabled.
     */
    SkTMaskPreBlend(const SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>& that)
    : fParent(that.fParent.get()), fR(that.fR), fG(that.fG), fB(that.fB) {
        SkSafeRef(fParent.get());
    }
    ~SkTMaskPreBlend() { }
    const uint8_t* fR;
    const uint8_t* fG;
    const uint8_t* fB;
};

template <int R_LUM_BITS, int G_LUM_BITS, int B_LUM_BITS>
SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>
SkTMaskGamma<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>::preBlend(SkColor color) {
    return fIsLinear ? SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>(
                          NULL, NULL, NULL, NULL)
                      : SkTMaskPreBlend<R_LUM_BITS, G_LUM_BITS, B_LUM_BITS>(
                          this,
                          fGammaTables[SkColorGetR(color) >> (8 - kLuminanceBits_Max)],
                          fGammaTables[SkColorGetG(color) >> (8 - kLuminanceBits_Max)],
                          fGammaTables[SkColorGetB(color) >> (8 - kLuminanceBits_Max)]);
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
