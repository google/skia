/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_BlendFormula_DEFINED
#define skgpu_BlendFormula_DEFINED

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/Blend.h"

#include <cstdint>

enum class SkBlendMode;

namespace skgpu {

/**
 * Wraps the shader outputs and HW blend state that comprise a Porter Duff blend mode with coverage.
 */
class BlendFormula {
public:
    /**
     * Values the shader can write to primary and secondary outputs. These are all modulated by
     * coverage. We will ignore the multiplies when not using coverage.
     */
    enum OutputType {
        kNone_OutputType,        //<! 0
        kCoverage_OutputType,    //<! inputCoverage
        kModulate_OutputType,    //<! inputColor * inputCoverage
        kSAModulate_OutputType,  //<! inputColor.a * inputCoverage
        kISAModulate_OutputType, //<! (1 - inputColor.a) * inputCoverage
        kISCModulate_OutputType, //<! (1 - inputColor) * inputCoverage

        kLast_OutputType = kISCModulate_OutputType
    };

    constexpr BlendFormula(OutputType primaryOut,
                           OutputType secondaryOut,
                           skgpu::BlendEquation equation,
                           skgpu::BlendCoeff srcCoeff,
                           skgpu::BlendCoeff dstCoeff)
            : fPrimaryOutputType(primaryOut)
            , fSecondaryOutputType(secondaryOut)
            , fBlendEquation(SkTo<uint8_t>(equation))
            , fSrcCoeff(SkTo<uint8_t>(srcCoeff))
            , fDstCoeff(SkTo<uint8_t>(dstCoeff))
            , fProps(GetProperties(primaryOut, secondaryOut, equation, srcCoeff, dstCoeff)) {}

    BlendFormula(const BlendFormula&) = default;
    BlendFormula& operator=(const BlendFormula&) = default;

    bool operator==(const BlendFormula& that) const {
        return fPrimaryOutputType == that.fPrimaryOutputType &&
               fSecondaryOutputType == that. fSecondaryOutputType &&
               fBlendEquation == that.fBlendEquation &&
               fSrcCoeff == that.fSrcCoeff &&
               fDstCoeff == that.fDstCoeff &&
               fProps == that.fProps;
    }

    bool hasSecondaryOutput() const {
        return kNone_OutputType != fSecondaryOutputType;
    }
    bool modifiesDst() const {
        return SkToBool(fProps & kModifiesDst_Property);
    }
    bool unaffectedByDst() const {
        return SkToBool(fProps & kUnaffectedByDst_Property);
    }
    // We don't always fully optimize the blend formula (e.g., for opaque src-over), so we include
    // an "IfOpaque" variant to help set AnalysisProperties::kUnaffectedByDstValue in those cases.
    bool unaffectedByDstIfOpaque() const {
        return SkToBool(fProps & kUnaffectedByDstIfOpaque_Property);
    }
    bool usesInputColor() const {
        return SkToBool(fProps & kUsesInputColor_Property);
    }
    bool canTweakAlphaForCoverage() const {
        return SkToBool(fProps & kCanTweakAlphaForCoverage_Property);
    }

    skgpu::BlendEquation equation() const {
        return static_cast<skgpu::BlendEquation>(fBlendEquation);
    }

    skgpu::BlendCoeff srcCoeff() const {
        return static_cast<skgpu::BlendCoeff>(fSrcCoeff);
    }

    skgpu::BlendCoeff dstCoeff() const {
        return static_cast<skgpu::BlendCoeff>(fDstCoeff);
    }

    OutputType primaryOutput() const {
        return fPrimaryOutputType;
    }

    OutputType secondaryOutput() const {
        return fSecondaryOutputType;
    }

private:
    enum Properties {
        kModifiesDst_Property              = 1 << 0,
        kUnaffectedByDst_Property          = 1 << 1,
        kUnaffectedByDstIfOpaque_Property  = 1 << 2,
        kUsesInputColor_Property           = 1 << 3,
        kCanTweakAlphaForCoverage_Property = 1 << 4,

        kLast_Property = kCanTweakAlphaForCoverage_Property
    };
    SK_DECL_BITFIELD_OPS_FRIENDS(Properties)

    /**
     * Deduce the properties of a BlendFormula.
     */
    constexpr BlendFormula::Properties GetProperties(OutputType PrimaryOut,
                                                     OutputType SecondaryOut,
                                                     skgpu::BlendEquation BlendEquation,
                                                     skgpu::BlendCoeff SrcCoeff,
                                                     skgpu::BlendCoeff DstCoeff) {
        return
        // The provided formula should already be optimized before a BlendFormula is constructed.
        // Assert that here while setting up the properties in the constexpr constructor.
        SkASSERT((kNone_OutputType == PrimaryOut) ==
                 !skgpu::BlendCoeffsUseSrcColor(SrcCoeff, DstCoeff)),
        SkASSERT(!skgpu::BlendCoeffRefsSrc2(SrcCoeff)),
        SkASSERT((kNone_OutputType == SecondaryOut) == !skgpu::BlendCoeffRefsSrc2(DstCoeff)),
        SkASSERT(PrimaryOut != SecondaryOut || kNone_OutputType == PrimaryOut),
        SkASSERT(kNone_OutputType != PrimaryOut || kNone_OutputType == SecondaryOut),

        static_cast<Properties>(
            (skgpu::BlendModifiesDst(BlendEquation, SrcCoeff, DstCoeff)
                        ? kModifiesDst_Property
                        : 0) |
            (!skgpu::BlendCoeffsUseDstColor(SrcCoeff, DstCoeff, false/*srcColorIsOpaque*/)
                        ? kUnaffectedByDst_Property
                        : 0) |
            (!skgpu::BlendCoeffsUseDstColor(SrcCoeff, DstCoeff, true/*srcColorIsOpaque*/)
                        ? kUnaffectedByDstIfOpaque_Property
                        : 0) |
            ((PrimaryOut >= kModulate_OutputType &&
                                skgpu::BlendCoeffsUseSrcColor(SrcCoeff, DstCoeff)) ||
                                (SecondaryOut >= kModulate_OutputType &&
                                skgpu::BlendCoeffRefsSrc2(DstCoeff))
                        ? kUsesInputColor_Property
                        : 0) |  // We assert later that SrcCoeff doesn't ref src2.
            ((kModulate_OutputType == PrimaryOut || kNone_OutputType == PrimaryOut) &&
                                kNone_OutputType == SecondaryOut &&
                                skgpu::BlendAllowsCoverageAsAlpha(BlendEquation, SrcCoeff, DstCoeff)
                        ? kCanTweakAlphaForCoverage_Property
                        : 0));
    }

    struct {
        // We allot the enums one more bit than they require because MSVC seems to sign-extend
        // them when the top bit is set. (This is in violation of the C++03 standard 9.6/4)
        OutputType fPrimaryOutputType   : 4;
        OutputType fSecondaryOutputType : 4;
        uint32_t   fBlendEquation       : 6;
        uint32_t   fSrcCoeff            : 6;
        uint32_t   fDstCoeff            : 6;
        Properties fProps               : 32 - (4 + 4 + 6 + 6 + 6);
    };

    static_assert(kLast_OutputType                              < (1 << 3));
    static_assert(static_cast<int>(skgpu::BlendEquation::kLast) < (1 << 5));
    static_assert(static_cast<int>(skgpu::BlendCoeff::kLast)    < (1 << 5));
    static_assert(kLast_Property                                < (1 << 6));
};

static_assert(4 == sizeof(BlendFormula));

SK_MAKE_BITFIELD_OPS(BlendFormula::Properties)

BlendFormula GetBlendFormula(bool isOpaque, bool hasCoverage, SkBlendMode xfermode);

BlendFormula GetLCDBlendFormula(SkBlendMode xfermode);

}  // namespace skgpu

#endif  // skgpu_BlendFormula_DEFINED
