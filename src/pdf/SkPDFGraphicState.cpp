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

static bool equivalent(const SkPaint& a, const SkPaint& b) {
    // We're only interested in some fields of the SkPaint, so we have
    // a custom equality function.
    if (SkColorGetA(a.getColor()) != SkColorGetA(b.getColor()) ||
        a.getStrokeCap() != b.getStrokeCap() ||
        a.getStrokeJoin() != b.getStrokeJoin() ||
        a.getStrokeWidth() != b.getStrokeWidth() ||
        a.getStrokeMiter() != b.getStrokeMiter()) {
        return false;
    }

    SkXfermode::Mode aXfermodeName = SkXfermode::kSrcOver_Mode;
    SkXfermode* aXfermode = a.getXfermode();
    if (aXfermode) {
        aXfermode->asMode(&aXfermodeName);
    }
    if (aXfermodeName < 0 || aXfermodeName > SkXfermode::kLastMode ||
        as_blend_mode(aXfermodeName) == NULL) {
        aXfermodeName = SkXfermode::kSrcOver_Mode;
    }
    const char* aXfermodeString = as_blend_mode(aXfermodeName);
    SkASSERT(aXfermodeString != NULL);

    SkXfermode::Mode bXfermodeName = SkXfermode::kSrcOver_Mode;
    SkXfermode* bXfermode = b.getXfermode();
    if (bXfermode) {
        bXfermode->asMode(&bXfermodeName);
    }
    if (bXfermodeName < 0 || bXfermodeName > SkXfermode::kLastMode ||
        as_blend_mode(bXfermodeName) == NULL) {
        bXfermodeName = SkXfermode::kSrcOver_Mode;
    }
    const char* bXfermodeString = as_blend_mode(bXfermodeName);
    SkASSERT(bXfermodeString != NULL);

    return strcmp(aXfermodeString, bXfermodeString) == 0;
}

bool SkPDFGraphicState::equals(const SkPaint& paint) const {
    return equivalent(paint, fPaint);
}

SkPDFGraphicState::~SkPDFGraphicState() {}

void SkPDFGraphicState::emitObject(SkWStream* stream, SkPDFCatalog* catalog) {
    populateDict();
    SkPDFDict::emitObject(stream, catalog);
}

// static
SkPDFGraphicState* SkPDFGraphicState::GetGraphicStateForPaint(
        SkPDFCanon* canon, const SkPaint& paint) {
    SkASSERT(canon);
    SkPDFGraphicState* pdfGraphicState = canon->findGraphicState(paint);
    if (pdfGraphicState) {
        return SkRef(pdfGraphicState);
    }
    pdfGraphicState = new SkPDFGraphicState(paint);
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
    invertFunction->insert("Domain", domainAndRange.get());
    invertFunction->insert("Range", domainAndRange.get());
    return invertFunction;
}

template <typename T> void unref(T* ptr) { ptr->unref(); }
}  // namespace

SK_DECLARE_STATIC_LAZY_PTR(SkPDFObject, invertFunction,
                           create_invert_function, unref<SkPDFObject>);

// static
SkPDFGraphicState* SkPDFGraphicState::GetSMaskGraphicState(
        SkPDFFormXObject* sMask, bool invert, SkPDFSMaskMode sMaskMode) {
    // The practical chances of using the same mask more than once are unlikely
    // enough that it's not worth canonicalizing.
    SkAutoTUnref<SkPDFDict> sMaskDict(new SkPDFDict("Mask"));
    if (sMaskMode == kAlpha_SMaskMode) {
        sMaskDict->insertName("S", "Alpha");
    } else if (sMaskMode == kLuminosity_SMaskMode) {
        sMaskDict->insertName("S", "Luminosity");
    }
    sMaskDict->insert("G", new SkPDFObjRef(sMask))->unref();

    SkPDFGraphicState* result = new SkPDFGraphicState;
    result->fPopulated = true;
    result->insertName("Type", "ExtGState");
    result->insert("SMask", sMaskDict.get());

    if (invert) {
        sMaskDict->insert("TR", new SkPDFObjRef(invertFunction.get()))->unref();
    }

    return result;
}

SkPDFGraphicState* SkPDFGraphicState::CreateNoSMaskGraphicState() {
    SkPDFGraphicState* noSMaskGS = SkNEW(SkPDFGraphicState);
    noSMaskGS->fPopulated = true;
    noSMaskGS->insertName("Type", "ExtGState");
    noSMaskGS->insertName("SMask", "None");
    return noSMaskGS;
}

SK_DECLARE_STATIC_LAZY_PTR(
        SkPDFGraphicState, noSMaskGraphicState,
        SkPDFGraphicState::CreateNoSMaskGraphicState, unref<SkPDFGraphicState>);

// static
SkPDFGraphicState* SkPDFGraphicState::GetNoSMaskGraphicState() {
    return SkRef(noSMaskGraphicState.get());
}

SkPDFGraphicState::SkPDFGraphicState()
    : fPopulated(false) {}

SkPDFGraphicState::SkPDFGraphicState(const SkPaint& paint)
    : fPaint(paint), fPopulated(false) {}

// populateDict and operator== have to stay in sync with each other.
void SkPDFGraphicState::populateDict() {
    if (!fPopulated) {
        fPopulated = true;
        insertName("Type", "ExtGState");

        SkAutoTUnref<SkPDFScalar> alpha(
            new SkPDFScalar(SkScalarDiv(fPaint.getAlpha(), 0xFF)));
        insert("CA", alpha.get());
        insert("ca", alpha.get());

        SK_COMPILE_ASSERT(SkPaint::kButt_Cap == 0, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kRound_Cap == 1, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kSquare_Cap == 2, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kCapCount == 3, paint_cap_mismatch);
        SkASSERT(fPaint.getStrokeCap() >= 0 && fPaint.getStrokeCap() <= 2);
        insertInt("LC", fPaint.getStrokeCap());

        SK_COMPILE_ASSERT(SkPaint::kMiter_Join == 0, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kRound_Join == 1, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kBevel_Join == 2, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kJoinCount == 3, paint_join_mismatch);
        SkASSERT(fPaint.getStrokeJoin() >= 0 && fPaint.getStrokeJoin() <= 2);
        insertInt("LJ", fPaint.getStrokeJoin());

        insertScalar("LW", fPaint.getStrokeWidth());
        insertScalar("ML", fPaint.getStrokeMiter());
        insert("SA", new SkPDFBool(true))->unref();  // Auto stroke adjustment.

        SkXfermode::Mode xfermode = SkXfermode::kSrcOver_Mode;
        // If asMode fails, default to kSrcOver_Mode.
        if (fPaint.getXfermode())
            fPaint.getXfermode()->asMode(&xfermode);
        // If we don't support the mode, just use kSrcOver_Mode.
        if (xfermode < 0 || xfermode > SkXfermode::kLastMode ||
            as_blend_mode(xfermode) == NULL) {
            xfermode = SkXfermode::kSrcOver_Mode;
            NOT_IMPLEMENTED("unsupported xfermode", false);
        }
        insertName("BM", as_blend_mode(xfermode));
    }
}
