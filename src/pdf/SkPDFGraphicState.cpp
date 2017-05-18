/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkPaint.h"
#include "SkPDFCanon.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFUtils.h"

static const char* as_blend_mode(SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kSrcOver:
            return "Normal";
        case SkBlendMode::kMultiply:
            return "Multiply";
        case SkBlendMode::kScreen:
            return "Screen";
        case SkBlendMode::kOverlay:
            return "Overlay";
        case SkBlendMode::kDarken:
            return "Darken";
        case SkBlendMode::kLighten:
            return "Lighten";
        case SkBlendMode::kColorDodge:
            return "ColorDodge";
        case SkBlendMode::kColorBurn:
            return "ColorBurn";
        case SkBlendMode::kHardLight:
            return "HardLight";
        case SkBlendMode::kSoftLight:
            return "SoftLight";
        case SkBlendMode::kDifference:
            return "Difference";
        case SkBlendMode::kExclusion:
            return "Exclusion";
        case SkBlendMode::kHue:
            return "Hue";
        case SkBlendMode::kSaturation:
            return "Saturation";
        case SkBlendMode::kColor:
            return "Color";
        case SkBlendMode::kLuminosity:
            return "Luminosity";

        // These are handled in SkPDFDevice::setUpContentEntry.
        case SkBlendMode::kClear:
        case SkBlendMode::kSrc:
        case SkBlendMode::kDst:
        case SkBlendMode::kDstOver:
        case SkBlendMode::kSrcIn:
        case SkBlendMode::kDstIn:
        case SkBlendMode::kSrcOut:
        case SkBlendMode::kDstOut:
        case SkBlendMode::kSrcATop:
        case SkBlendMode::kDstATop:
        case SkBlendMode::kModulate:
            return "Normal";

        // TODO(vandebo): Figure out if we can support more of these modes.
        case SkBlendMode::kXor:
        case SkBlendMode::kPlus:
            return nullptr;
    }
    return nullptr;
}

// If a SkXfermode is unsupported in PDF, this function returns
// SrcOver, otherwise, it returns that Xfermode as a Mode.
static SkBlendMode mode_for_pdf(SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kSrcOver:
        case SkBlendMode::kMultiply:
        case SkBlendMode::kScreen:
        case SkBlendMode::kOverlay:
        case SkBlendMode::kDarken:
        case SkBlendMode::kLighten:
        case SkBlendMode::kColorDodge:
        case SkBlendMode::kColorBurn:
        case SkBlendMode::kHardLight:
        case SkBlendMode::kSoftLight:
        case SkBlendMode::kDifference:
        case SkBlendMode::kExclusion:
        case SkBlendMode::kHue:
        case SkBlendMode::kSaturation:
        case SkBlendMode::kColor:
        case SkBlendMode::kLuminosity:
            // Mode is suppported and handled by pdf graphics state.
            return mode;
        default:
            return SkBlendMode::kSrcOver;  // Default mode.
    }
}

SkPDFGraphicState::SkPDFGraphicState(const SkPaint& p)
    : fStrokeWidth(p.getStrokeWidth())
    , fStrokeMiter(p.getStrokeMiter())
    , fAlpha(p.getAlpha())
    , fStrokeCap(SkToU8(p.getStrokeCap()))
    , fStrokeJoin(SkToU8(p.getStrokeJoin()))
    , fMode(SkToU8((unsigned)mode_for_pdf(p.getBlendMode()))) {}

sk_sp<SkPDFGraphicState> SkPDFGraphicState::GetGraphicStateForPaint(SkPDFCanon* canon,
                                                                    const SkPaint& paint) {
    SkASSERT(canon);
    SkPDFGraphicState key(paint);
    if (const SkPDFGraphicState* canonGS = canon->findGraphicState(key)) {
        // The returned SkPDFGraphicState must be made non-const,
        // since the emitObject() interface is non-const.  But We
        // promise that there is no way to mutate this object from
        // here on out.
        return sk_sp<SkPDFGraphicState>(SkRef(const_cast<SkPDFGraphicState*>(canonGS)));
    }
    sk_sp<SkPDFGraphicState> pdfGraphicState(new SkPDFGraphicState(paint));
    canon->addGraphicState(pdfGraphicState.get());
    return pdfGraphicState;
}

static sk_sp<SkPDFStream> make_invert_function() {
    // Acrobat crashes if we use a type 0 function, kpdf crashes if we use
    // a type 2 function, so we use a type 4 function.
    auto domainAndRange = sk_make_sp<SkPDFArray>();
    domainAndRange->reserve(2);
    domainAndRange->appendInt(0);
    domainAndRange->appendInt(1);

    static const char psInvert[] = "{1 exch sub}";
    // Do not copy the trailing '\0' into the SkData.
    auto invertFunction = sk_make_sp<SkPDFStream>(
            SkData::MakeWithoutCopy(psInvert, strlen(psInvert)));
    invertFunction->dict()->insertInt("FunctionType", 4);
    invertFunction->dict()->insertObject("Domain", domainAndRange);
    invertFunction->dict()->insertObject("Range", std::move(domainAndRange));
    return invertFunction;
}

sk_sp<SkPDFDict> SkPDFGraphicState::GetSMaskGraphicState(
        sk_sp<SkPDFObject> sMask,
        bool invert,
        SkPDFSMaskMode sMaskMode,
        SkPDFCanon* canon) {
    // The practical chances of using the same mask more than once are unlikely
    // enough that it's not worth canonicalizing.
    auto sMaskDict = sk_make_sp<SkPDFDict>("Mask");
    if (sMaskMode == kAlpha_SMaskMode) {
        sMaskDict->insertName("S", "Alpha");
    } else if (sMaskMode == kLuminosity_SMaskMode) {
        sMaskDict->insertName("S", "Luminosity");
    }
    sMaskDict->insertObjRef("G", std::move(sMask));
    if (invert) {
        // Instead of calling SkPDFGraphicState::MakeInvertFunction,
        // let the canon deduplicate this object.
        sMaskDict->insertObjRef(
                "TR", SkPDFUtils::GetCachedT(&canon->fInvertFunction, &make_invert_function));
    }

    auto result = sk_make_sp<SkPDFDict>("ExtGState");
    result->insertObject("SMask", std::move(sMaskDict));
    return result;
}

sk_sp<SkPDFDict> SkPDFGraphicState::MakeNoSmaskGraphicState() {
    auto noSMaskGS = sk_make_sp<SkPDFDict>("ExtGState");
    noSMaskGS->insertName("SMask", "None");
    return noSMaskGS;
}

void SkPDFGraphicState::emitObject(
        SkWStream* stream,
        const SkPDFObjNumMap& objNumMap) const {
    auto dict = sk_make_sp<SkPDFDict>("ExtGState");
    dict->insertName("Type", "ExtGState");

    SkScalar alpha = SkIntToScalar(fAlpha) / 0xFF;
    dict->insertScalar("CA", alpha);
    dict->insertScalar("ca", alpha);

    SkPaint::Cap strokeCap = (SkPaint::Cap)fStrokeCap;
    SkPaint::Join strokeJoin = (SkPaint::Join)fStrokeJoin;

    static_assert(SkPaint::kButt_Cap == 0, "paint_cap_mismatch");
    static_assert(SkPaint::kRound_Cap == 1, "paint_cap_mismatch");
    static_assert(SkPaint::kSquare_Cap == 2, "paint_cap_mismatch");
    static_assert(SkPaint::kCapCount == 3, "paint_cap_mismatch");
    SkASSERT(strokeCap >= 0 && strokeCap <= 2);
    dict->insertInt("LC", strokeCap);

    static_assert(SkPaint::kMiter_Join == 0, "paint_join_mismatch");
    static_assert(SkPaint::kRound_Join == 1, "paint_join_mismatch");
    static_assert(SkPaint::kBevel_Join == 2, "paint_join_mismatch");
    static_assert(SkPaint::kJoinCount == 3, "paint_join_mismatch");
    SkASSERT(strokeJoin >= 0 && strokeJoin <= 2);
    dict->insertInt("LJ", strokeJoin);

    dict->insertScalar("LW", fStrokeWidth);
    dict->insertScalar("ML", fStrokeMiter);
    dict->insertBool("SA", true);  // SA = Auto stroke adjustment.
    dict->insertName("BM", as_blend_mode((SkBlendMode)fMode));
    dict->emitObject(stream, objNumMap);
}
