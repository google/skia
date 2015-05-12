/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkLazyPtr.h"
#include "SkPDFCanon.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFUtils.h"
#include "SkTypes.h"

static const char* as_blend_mode(SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kSrcOver_Mode:
            return "Normal";
        case SkXfermode::kMultiply_Mode:
            return "Multiply";
        case SkXfermode::kScreen_Mode:
            return "Screen";
        case SkXfermode::kOverlay_Mode:
            return "Overlay";
        case SkXfermode::kDarken_Mode:
            return "Darken";
        case SkXfermode::kLighten_Mode:
            return "Lighten";
        case SkXfermode::kColorDodge_Mode:
            return "ColorDodge";
        case SkXfermode::kColorBurn_Mode:
            return "ColorBurn";
        case SkXfermode::kHardLight_Mode:
            return "HardLight";
        case SkXfermode::kSoftLight_Mode:
            return "SoftLight";
        case SkXfermode::kDifference_Mode:
            return "Difference";
        case SkXfermode::kExclusion_Mode:
            return "Exclusion";
        case SkXfermode::kHue_Mode:
            return "Hue";
        case SkXfermode::kSaturation_Mode:
            return "Saturation";
        case SkXfermode::kColor_Mode:
            return "Color";
        case SkXfermode::kLuminosity_Mode:
            return "Luminosity";

        // These are handled in SkPDFDevice::setUpContentEntry.
        case SkXfermode::kClear_Mode:
        case SkXfermode::kSrc_Mode:
        case SkXfermode::kDst_Mode:
        case SkXfermode::kDstOver_Mode:
        case SkXfermode::kSrcIn_Mode:
        case SkXfermode::kDstIn_Mode:
        case SkXfermode::kSrcOut_Mode:
        case SkXfermode::kDstOut_Mode:
        case SkXfermode::kSrcATop_Mode:
        case SkXfermode::kDstATop_Mode:
        case SkXfermode::kModulate_Mode:
            return "Normal";

        // TODO(vandebo): Figure out if we can support more of these modes.
        case SkXfermode::kXor_Mode:
        case SkXfermode::kPlus_Mode:
            return NULL;
    }
    return NULL;
}

// If a SkXfermode is unsupported in PDF, this function returns
// SrcOver, otherwise, it returns that Xfermode as a Mode.
static SkXfermode::Mode mode_for_pdf(const SkXfermode* xfermode) {
    SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode;
    if (xfermode) {
        xfermode->asMode(&mode);
    }
    switch (mode) {
        case SkXfermode::kSrcOver_Mode:
        case SkXfermode::kMultiply_Mode:
        case SkXfermode::kScreen_Mode:
        case SkXfermode::kOverlay_Mode:
        case SkXfermode::kDarken_Mode:
        case SkXfermode::kLighten_Mode:
        case SkXfermode::kColorDodge_Mode:
        case SkXfermode::kColorBurn_Mode:
        case SkXfermode::kHardLight_Mode:
        case SkXfermode::kSoftLight_Mode:
        case SkXfermode::kDifference_Mode:
        case SkXfermode::kExclusion_Mode:
        case SkXfermode::kHue_Mode:
        case SkXfermode::kSaturation_Mode:
        case SkXfermode::kColor_Mode:
        case SkXfermode::kLuminosity_Mode:
            // Mode is suppported and handled by pdf graphics state.
            return mode;
        default:
            return SkXfermode::kSrcOver_Mode;  // Default mode.
    }
}

SkPDFGraphicState::SkPDFGraphicState(const SkPaint& p)
    : fStrokeWidth(p.getStrokeWidth())
    , fStrokeMiter(p.getStrokeMiter())
    , fAlpha(p.getAlpha())
    , fStrokeCap(SkToU8(p.getStrokeCap()))
    , fStrokeJoin(SkToU8(p.getStrokeJoin()))
    , fMode(SkToU8(mode_for_pdf(p.getXfermode()))) {}

// static
SkPDFGraphicState* SkPDFGraphicState::GetGraphicStateForPaint(
        SkPDFCanon* canon, const SkPaint& paint) {
    SkASSERT(canon);
    SkPDFGraphicState key(paint);
    if (const SkPDFGraphicState* canonGS = canon->findGraphicState(key)) {
        // The returned SkPDFGraphicState must be made non-const,
        // since the emitObject() interface is non-const.  But We
        // promise that there is no way to mutate this object from
        // here on out.
        return SkRef(const_cast<SkPDFGraphicState*>(canonGS));
    }
    SkPDFGraphicState* pdfGraphicState = new SkPDFGraphicState(paint);
    canon->addGraphicState(pdfGraphicState);
    return pdfGraphicState;
}

namespace {
SkPDFObject* create_invert_function() {
    // Acrobat crashes if we use a type 0 function, kpdf crashes if we use
    // a type 2 function, so we use a type 4 function.
    SkAutoTUnref<SkPDFArray> domainAndRange(new SkPDFArray);
    domainAndRange->reserve(2);
    domainAndRange->appendInt(0);
    domainAndRange->appendInt(1);

    static const char psInvert[] = "{1 exch sub}";
    // Do not copy the trailing '\0' into the SkData.
    SkAutoTUnref<SkData> psInvertStream(
            SkData::NewWithoutCopy(psInvert, strlen(psInvert)));

    SkPDFStream* invertFunction = SkNEW_ARGS(
            SkPDFStream, (psInvertStream.get()));
    invertFunction->insertInt("FunctionType", 4);
    invertFunction->insertObject("Domain", SkRef(domainAndRange.get()));
    invertFunction->insertObject("Range", domainAndRange.detach());
    return invertFunction;
}

template <typename T> void unref(T* ptr) { ptr->unref(); }
}  // namespace

SK_DECLARE_STATIC_LAZY_PTR(SkPDFObject,
                           invertFunction,
                           create_invert_function,
                           unref<SkPDFObject>);

// static
SkPDFDict* SkPDFGraphicState::GetSMaskGraphicState(SkPDFFormXObject* sMask,
                                                   bool invert,
                                                   SkPDFSMaskMode sMaskMode) {
    // The practical chances of using the same mask more than once are unlikely
    // enough that it's not worth canonicalizing.
    SkAutoTUnref<SkPDFDict> sMaskDict(new SkPDFDict("Mask"));
    if (sMaskMode == kAlpha_SMaskMode) {
        sMaskDict->insertName("S", "Alpha");
    } else if (sMaskMode == kLuminosity_SMaskMode) {
        sMaskDict->insertName("S", "Luminosity");
    }
    sMaskDict->insertObjRef("G", SkRef(sMask));
    if (invert) {
        sMaskDict->insertObjRef("TR", SkRef(invertFunction.get()));
    }

    SkPDFDict* result = new SkPDFDict("ExtGState");
    result->insertObject("SMask", sMaskDict.detach());
    return result;
}

namespace {
SkPDFDict* create_no_smask_graphic_state() {
    SkPDFDict* noSMaskGS = new SkPDFDict("ExtGState");
    noSMaskGS->insertName("SMask", "None");
    return noSMaskGS;
}
} // namespace
SK_DECLARE_STATIC_LAZY_PTR(SkPDFDict,
                           noSMaskGraphicState,
                           create_no_smask_graphic_state,
                           unref<SkPDFDict>);

// static
SkPDFDict* SkPDFGraphicState::GetNoSMaskGraphicState() {
    return SkRef(noSMaskGraphicState.get());
}

void SkPDFGraphicState::emitObject(SkWStream* stream,
                                   const SkPDFObjNumMap& objNumMap,
                                   const SkPDFSubstituteMap& substitutes) {
    SkAutoTUnref<SkPDFDict> dict(SkNEW_ARGS(SkPDFDict, ("ExtGState")));
    dict->insertName("Type", "ExtGState");

    SkScalar alpha = SkIntToScalar(fAlpha) / 0xFF;
    dict->insertScalar("CA", alpha);
    dict->insertScalar("ca", alpha);

    SkPaint::Cap strokeCap = (SkPaint::Cap)fStrokeCap;
    SkPaint::Join strokeJoin = (SkPaint::Join)fStrokeJoin;
    SkXfermode::Mode xferMode = (SkXfermode::Mode)fMode;

    SK_COMPILE_ASSERT(SkPaint::kButt_Cap == 0, paint_cap_mismatch);
    SK_COMPILE_ASSERT(SkPaint::kRound_Cap == 1, paint_cap_mismatch);
    SK_COMPILE_ASSERT(SkPaint::kSquare_Cap == 2, paint_cap_mismatch);
    SK_COMPILE_ASSERT(SkPaint::kCapCount == 3, paint_cap_mismatch);
    SkASSERT(strokeCap >= 0 && strokeCap <= 2);
    dict->insertInt("LC", strokeCap);

    SK_COMPILE_ASSERT(SkPaint::kMiter_Join == 0, paint_join_mismatch);
    SK_COMPILE_ASSERT(SkPaint::kRound_Join == 1, paint_join_mismatch);
    SK_COMPILE_ASSERT(SkPaint::kBevel_Join == 2, paint_join_mismatch);
    SK_COMPILE_ASSERT(SkPaint::kJoinCount == 3, paint_join_mismatch);
    SkASSERT(strokeJoin >= 0 && strokeJoin <= 2);
    dict->insertInt("LJ", strokeJoin);

    dict->insertScalar("LW", fStrokeWidth);
    dict->insertScalar("ML", fStrokeMiter);
    dict->insertBool("SA", true);  // SA = Auto stroke adjustment.
    dict->insertName("BM", as_blend_mode(xferMode));
    dict->emitObject(stream, objNumMap, substitutes);
}
