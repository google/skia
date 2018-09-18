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

static const char* as_pdf_blend_mode_name(SkBlendMode mode) {
    // PDF32000.book section 11.3.5 "Blend Mode"
    switch (mode) {
        case SkBlendMode::kScreen:      return "Screen";
        case SkBlendMode::kOverlay:     return "Overlay";
        case SkBlendMode::kDarken:      return "Darken";
        case SkBlendMode::kLighten:     return "Lighten";
        case SkBlendMode::kColorDodge:  return "ColorDodge";
        case SkBlendMode::kColorBurn:   return "ColorBurn";
        case SkBlendMode::kHardLight:   return "HardLight";
        case SkBlendMode::kSoftLight:   return "SoftLight";
        case SkBlendMode::kDifference:  return "Difference";
        case SkBlendMode::kExclusion:   return "Exclusion";
        case SkBlendMode::kMultiply:    return "Multiply";
        case SkBlendMode::kHue:         return "Hue";
        case SkBlendMode::kSaturation:  return "Saturation";
        case SkBlendMode::kColor:       return "Color";
        case SkBlendMode::kLuminosity:  return "Luminosity";
        // Other blendmodes are either unsupported or handled in
        // SkPDFDevice::setUpContentEntry.
        default:                        return "Normal";
    }
}

static int to_stroke_cap(uint8_t cap) {
    // PDF32000.book section 8.4.3.3 "Line Cap Style"
    switch ((SkPaint::Cap)cap) {
        case SkPaint::kButt_Cap:   return 0;
        case SkPaint::kRound_Cap:  return 1;
        case SkPaint::kSquare_Cap: return 2;
        default: SkASSERT(false);  return 0;
    }
}

static int to_stroke_join(uint8_t join) {
    // PDF32000.book section 8.4.3.4 "Line Join Style"
    switch ((SkPaint::Join)join) {
        case SkPaint::kMiter_Join: return 0;
        case SkPaint::kRound_Join: return 1;
        case SkPaint::kBevel_Join: return 2;
        default: SkASSERT(false);  return 0;
    }
}

// If a SkXfermode is unsupported in PDF, this function returns
// SrcOver, otherwise, it returns that Xfermode as a Mode.
static uint8_t pdf_blend_mode(SkBlendMode mode) {
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
            return SkToU8((unsigned)mode);
        default:
            return SkToU8((unsigned)SkBlendMode::kSrcOver);
    }
}

sk_sp<SkPDFDict> SkPDFGraphicState::GetGraphicStateForPaint(SkPDFCanon* canon,
                                                            const SkPaint& p) {
    SkASSERT(canon);
    if (SkPaint::kFill_Style == p.getStyle()) {
        SkPDFFillGraphicState fillKey = {p.getAlpha(), pdf_blend_mode(p.getBlendMode())};
        auto& fillMap = canon->fFillGSMap;
        if (sk_sp<SkPDFDict>* statePtr = fillMap.find(fillKey)) {
            return *statePtr;
        }
        auto state = sk_make_sp<SkPDFDict>();
        state->reserve(2);
        state->insertScalar("ca", fillKey.fAlpha / 255.0f);
        state->insertName("BM", as_pdf_blend_mode_name((SkBlendMode)fillKey.fBlendMode));
        fillMap.set(fillKey, state);
        return state;
    } else {
        SkPDFStrokeGraphicState strokeKey = {
            p.getStrokeWidth(), p.getStrokeMiter(),
            SkToU8(p.getStrokeCap()), SkToU8(p.getStrokeJoin()),
            p.getAlpha(), pdf_blend_mode(p.getBlendMode())};
        auto& sMap = canon->fStrokeGSMap;
        if (sk_sp<SkPDFDict>* statePtr = sMap.find(strokeKey)) {
            return *statePtr;
        }
        auto state = sk_make_sp<SkPDFDict>();
        state->reserve(8);
        state->insertScalar("CA", strokeKey.fAlpha / 255.0f);
        state->insertScalar("ca", strokeKey.fAlpha / 255.0f);
        state->insertInt("LC", to_stroke_cap(strokeKey.fStrokeCap));
        state->insertInt("LJ", to_stroke_join(strokeKey.fStrokeJoin));
        state->insertScalar("LW", strokeKey.fStrokeWidth);
        state->insertScalar("ML", strokeKey.fStrokeMiter);
        state->insertBool("SA", true);  // SA = Auto stroke adjustment.
        state->insertName("BM", as_pdf_blend_mode_name((SkBlendMode)strokeKey.fBlendMode));
        sMap.set(strokeKey, state);
        return state;
    }
}

////////////////////////////////////////////////////////////////////////////////

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
        sk_sp<SkPDFStream>& invertFunction = canon->fInvertFunction;
        if (!invertFunction) {
            invertFunction = make_invert_function();
        }
        sMaskDict->insertObjRef("TR", invertFunction);
    }
    auto result = sk_make_sp<SkPDFDict>("ExtGState");
    result->insertObject("SMask", std::move(sMaskDict));
    return result;
}
