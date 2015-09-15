/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrBlend.h"

/**
 * MaskedColor is used to evaluate the color and valid color component flags through the
 * blending equation. Could possibly extend this to be used more broadly.
 */
class MaskedColor {
public:
    MaskedColor(GrColor color, GrColorComponentFlags flags)
        : fColor(color)
        , fFlags(flags) {}

    MaskedColor() {}

    void set(GrColor color, GrColorComponentFlags flags) {
        fColor = color;
        fFlags = flags;
    }

    static MaskedColor Invert(const MaskedColor& in) {
        return MaskedColor(GrInvertColor(in.fColor), in.fFlags);
    }

    static MaskedColor ExtractAlpha(const MaskedColor& in) {
        GrColorComponentFlags flags = (in.fFlags & kA_GrColorComponentFlag) ?
            kRGBA_GrColorComponentFlags : kNone_GrColorComponentFlags;
        return MaskedColor(GrColorPackA4(GrColorUnpackA(in.fColor)), flags);
    }

    static MaskedColor ExtractInverseAlpha(const MaskedColor& in) {
        GrColorComponentFlags flags = (in.fFlags & kA_GrColorComponentFlag) ?
            kRGBA_GrColorComponentFlags : kNone_GrColorComponentFlags;
        return MaskedColor(GrColorPackA4(0xFF - GrColorUnpackA(in.fColor)), flags);
    }

    static MaskedColor Mul(const MaskedColor& a, const MaskedColor& b) {
        GrColorComponentFlags outFlags = (a.fFlags & b.fFlags) | a.componentsWithValue(0) |
                                         b.componentsWithValue(0);
        return MaskedColor(GrColorMul(a.fColor, b.fColor), outFlags);
    }

    static MaskedColor SatAdd(const MaskedColor& a, const MaskedColor& b) {
        GrColorComponentFlags outFlags = (a.fFlags & b.fFlags) | a.componentsWithValue(0xFF) |
                                         b.componentsWithValue(0xFF);
        return MaskedColor(GrColorSatAdd(a.fColor, b.fColor), outFlags);
    }

    GrColor color() const { return fColor; }

    GrColorComponentFlags validFlags () const { return fFlags; }

private:
    GrColorComponentFlags componentsWithValue(unsigned value) const {
        GrColorComponentFlags flags = kNone_GrColorComponentFlags;
        if ((kR_GrColorComponentFlag & fFlags) && value == GrColorUnpackR(fColor)) {
            flags |= kR_GrColorComponentFlag;
        }
        if ((kG_GrColorComponentFlag & fFlags) && value == GrColorUnpackG(fColor)) {
            flags |= kG_GrColorComponentFlag;
        }
        if ((kB_GrColorComponentFlag & fFlags) && value == GrColorUnpackB(fColor)) {
            flags |= kB_GrColorComponentFlag;
        }
        if ((kA_GrColorComponentFlag & fFlags) && value == GrColorUnpackA(fColor)) {
            flags |= kA_GrColorComponentFlag;
        }
        return flags;
    }

    GrColor                 fColor;
    GrColorComponentFlags   fFlags;
};

static MaskedColor get_term(GrBlendCoeff coeff, const MaskedColor& src, const MaskedColor& dst,
                            const MaskedColor& value) {
    switch (coeff) {
        case kZero_GrBlendCoeff:
            return MaskedColor(0, kRGBA_GrColorComponentFlags);
        case kOne_GrBlendCoeff:
            return value;
        case kDC_GrBlendCoeff:
            return MaskedColor::Mul(dst, value);
        case kIDC_GrBlendCoeff:
            return MaskedColor::Mul(MaskedColor::Invert(dst), value);
        case kDA_GrBlendCoeff:
            return MaskedColor::Mul(MaskedColor::ExtractAlpha(dst), value);
        case kIDA_GrBlendCoeff:
            return MaskedColor::Mul(MaskedColor::ExtractInverseAlpha(dst), value);
        case kSC_GrBlendCoeff:
            return MaskedColor::Mul(src, value);
        case kISC_GrBlendCoeff:
            return MaskedColor::Mul(MaskedColor::Invert(src), value);
        case kSA_GrBlendCoeff:
            return MaskedColor::Mul(MaskedColor::ExtractAlpha(src), value);
        case kISA_GrBlendCoeff:
            return MaskedColor::Mul(MaskedColor::ExtractInverseAlpha(src), value);
        default:
            SkFAIL("Illegal coefficient");
            return MaskedColor();
    }
}

void GrGetCoeffBlendKnownComponents(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff,
                                    GrColor srcColor, GrColorComponentFlags srcColorFlags,
                                    GrColor dstColor, GrColorComponentFlags dstColorFlags,
                                    GrColor* outColor,
                                    GrColorComponentFlags* outFlags) {
    MaskedColor src(srcColor, srcColorFlags);
    MaskedColor dst(dstColor, dstColorFlags);

    MaskedColor srcTerm = get_term(srcCoeff, src, dst, src);
    MaskedColor dstTerm = get_term(dstCoeff, src, dst, dst);

    MaskedColor output = MaskedColor::SatAdd(srcTerm, dstTerm);
    *outColor = output.color();
    *outFlags = output.validFlags();
}
