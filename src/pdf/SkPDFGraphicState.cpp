/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkTypes.h"

static const char* blend_mode_from_xfermode(SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kSrcOver_Mode:    return "Normal";
        // kModulate is not really like multipy but similar most of the time.
        case SkXfermode::kModulate_Mode:
        case SkXfermode::kMultiply_Mode:   return "Multiply";
        case SkXfermode::kScreen_Mode:     return "Screen";
        case SkXfermode::kOverlay_Mode:    return "Overlay";
        case SkXfermode::kDarken_Mode:     return "Darken";
        case SkXfermode::kLighten_Mode:    return "Lighten";
        case SkXfermode::kColorDodge_Mode: return "ColorDodge";
        case SkXfermode::kColorBurn_Mode:  return "ColorBurn";
        case SkXfermode::kHardLight_Mode:  return "HardLight";
        case SkXfermode::kSoftLight_Mode:  return "SoftLight";
        case SkXfermode::kDifference_Mode: return "Difference";
        case SkXfermode::kExclusion_Mode:  return "Exclusion";
        case SkXfermode::kHue_Mode:        return "Hue";
        case SkXfermode::kSaturation_Mode: return "Saturation";
        case SkXfermode::kColor_Mode:      return "Color";
        case SkXfermode::kLuminosity_Mode: return "Luminosity";

        // These are handled in SkPDFDevice::setUpContentEntry.
        case SkXfermode::kClear_Mode:
        case SkXfermode::kSrc_Mode:
        case SkXfermode::kDst_Mode:
        case SkXfermode::kDstOver_Mode:
        case SkXfermode::kSrcIn_Mode:
        case SkXfermode::kDstIn_Mode:
        case SkXfermode::kSrcOut_Mode:
        case SkXfermode::kDstOut_Mode:
            return "Normal";

        // TODO(vandebo): Figure out if we can support more of these modes.
        case SkXfermode::kSrcATop_Mode:
        case SkXfermode::kDstATop_Mode:
        case SkXfermode::kXor_Mode:
        case SkXfermode::kPlus_Mode:
            return NULL;
    }
    return NULL;
}

SkPDFGraphicState::~SkPDFGraphicState() {
    SkAutoMutexAcquire lock(CanonicalPaintsMutex());
    if (!fSMask) {
        int index = Find(fPaint);
        SkASSERT(index >= 0);
        SkASSERT(CanonicalPaints()[index].fGraphicState == this);
        CanonicalPaints().removeShuffle(index);
    }
    fResources.unrefAll();
}

void SkPDFGraphicState::getResources(
        const SkTSet<SkPDFObject*>& knownResourceObjects,
        SkTSet<SkPDFObject*>* newResourceObjects) {
    GetResourcesHelper(&fResources, knownResourceObjects, newResourceObjects);
}

void SkPDFGraphicState::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                                   bool indirect) {
    populateDict();
    SkPDFDict::emitObject(stream, catalog, indirect);
}

// static
size_t SkPDFGraphicState::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    populateDict();
    return SkPDFDict::getOutputSize(catalog, indirect);
}

// static
SkTDArray<SkPDFGraphicState::GSCanonicalEntry>&
SkPDFGraphicState::CanonicalPaints() {
    // This initialization is only thread safe with gcc.
    static SkTDArray<SkPDFGraphicState::GSCanonicalEntry> gCanonicalPaints;
    return gCanonicalPaints;
}

// static
SkBaseMutex& SkPDFGraphicState::CanonicalPaintsMutex() {
    // This initialization is only thread safe with gcc or when
    // POD-style mutex initialization is used.
    SK_DECLARE_STATIC_MUTEX(gCanonicalPaintsMutex);
    return gCanonicalPaintsMutex;
}

// static
SkPDFGraphicState* SkPDFGraphicState::GetGraphicStateForPaint(
        const SkPaint& paint) {
    SkAutoMutexAcquire lock(CanonicalPaintsMutex());
    int index = Find(paint);
    if (index >= 0) {
        CanonicalPaints()[index].fGraphicState->ref();
        return CanonicalPaints()[index].fGraphicState;
    }
    GSCanonicalEntry newEntry(new SkPDFGraphicState(paint));
    CanonicalPaints().push(newEntry);
    return newEntry.fGraphicState;
}

// static
SkPDFObject* SkPDFGraphicState::GetInvertFunction() {
    // This assumes that canonicalPaintsMutex is held.
    static SkPDFStream* invertFunction = NULL;
    if (!invertFunction) {
        // Acrobat crashes if we use a type 0 function, kpdf crashes if we use
        // a type 2 function, so we use a type 4 function.
        SkAutoTUnref<SkPDFArray> domainAndRange(new SkPDFArray);
        domainAndRange->reserve(2);
        domainAndRange->appendInt(0);
        domainAndRange->appendInt(1);

        static const char psInvert[] = "{1 exch sub}";
        SkAutoTUnref<SkMemoryStream> psInvertStream(
            new SkMemoryStream(&psInvert, strlen(psInvert), true));

        invertFunction = new SkPDFStream(psInvertStream.get());
        invertFunction->insertInt("FunctionType", 4);
        invertFunction->insert("Domain", domainAndRange.get());
        invertFunction->insert("Range", domainAndRange.get());
    }
    return invertFunction;
}

// static
SkPDFGraphicState* SkPDFGraphicState::GetSMaskGraphicState(
        SkPDFFormXObject* sMask, bool invert) {
    // The practical chances of using the same mask more than once are unlikely
    // enough that it's not worth canonicalizing.
    SkAutoMutexAcquire lock(CanonicalPaintsMutex());

    SkAutoTUnref<SkPDFDict> sMaskDict(new SkPDFDict("Mask"));
    sMaskDict->insertName("S", "Alpha");
    sMaskDict->insert("G", new SkPDFObjRef(sMask))->unref();

    SkPDFGraphicState* result = new SkPDFGraphicState;
    result->fPopulated = true;
    result->fSMask = true;
    result->insertName("Type", "ExtGState");
    result->insert("SMask", sMaskDict.get());
    result->fResources.push(sMask);
    sMask->ref();

    if (invert) {
        SkPDFObject* invertFunction = GetInvertFunction();
        result->fResources.push(invertFunction);
        invertFunction->ref();
        sMaskDict->insert("TR", new SkPDFObjRef(invertFunction))->unref();
    }

    return result;
}

// static
SkPDFGraphicState* SkPDFGraphicState::GetNoSMaskGraphicState() {
    SkAutoMutexAcquire lock(CanonicalPaintsMutex());
    static SkPDFGraphicState* noSMaskGS = NULL;
    if (!noSMaskGS) {
        noSMaskGS = new SkPDFGraphicState;
        noSMaskGS->fPopulated = true;
        noSMaskGS->fSMask = true;
        noSMaskGS->insertName("Type", "ExtGState");
        noSMaskGS->insertName("SMask", "None");
    }
    noSMaskGS->ref();
    return noSMaskGS;
}

// static
int SkPDFGraphicState::Find(const SkPaint& paint) {
    GSCanonicalEntry search(&paint);
    return CanonicalPaints().find(search);
}

SkPDFGraphicState::SkPDFGraphicState()
    : fPopulated(false),
      fSMask(false) {
}

SkPDFGraphicState::SkPDFGraphicState(const SkPaint& paint)
    : fPaint(paint),
      fPopulated(false),
      fSMask(false) {
}

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
                blend_mode_from_xfermode(xfermode) == NULL) {
            xfermode = SkXfermode::kSrcOver_Mode;
            NOT_IMPLEMENTED("unsupported xfermode", false);
        }
        insertName("BM", blend_mode_from_xfermode(xfermode));
    }
}

// We're only interested in some fields of the SkPaint, so we have a custom
// operator== function.
bool SkPDFGraphicState::GSCanonicalEntry::operator==(
        const SkPDFGraphicState::GSCanonicalEntry& gs) const {
    const SkPaint* a = fPaint;
    const SkPaint* b = gs.fPaint;
    SkASSERT(a != NULL);
    SkASSERT(b != NULL);

    if (SkColorGetA(a->getColor()) != SkColorGetA(b->getColor()) ||
           a->getStrokeCap() != b->getStrokeCap() ||
           a->getStrokeJoin() != b->getStrokeJoin() ||
           a->getStrokeWidth() != b->getStrokeWidth() ||
           a->getStrokeMiter() != b->getStrokeMiter()) {
        return false;
    }

    SkXfermode::Mode aXfermodeName = SkXfermode::kSrcOver_Mode;
    SkXfermode* aXfermode = a->getXfermode();
    if (aXfermode) {
        aXfermode->asMode(&aXfermodeName);
    }
    if (aXfermodeName < 0 || aXfermodeName > SkXfermode::kLastMode ||
            blend_mode_from_xfermode(aXfermodeName) == NULL) {
        aXfermodeName = SkXfermode::kSrcOver_Mode;
    }
    const char* aXfermodeString = blend_mode_from_xfermode(aXfermodeName);
    SkASSERT(aXfermodeString != NULL);

    SkXfermode::Mode bXfermodeName = SkXfermode::kSrcOver_Mode;
    SkXfermode* bXfermode = b->getXfermode();
    if (bXfermode) {
        bXfermode->asMode(&bXfermodeName);
    }
    if (bXfermodeName < 0 || bXfermodeName > SkXfermode::kLastMode ||
            blend_mode_from_xfermode(bXfermodeName) == NULL) {
        bXfermodeName = SkXfermode::kSrcOver_Mode;
    }
    const char* bXfermodeString = blend_mode_from_xfermode(bXfermodeName);
    SkASSERT(bXfermodeString != NULL);

    return strcmp(aXfermodeString, bXfermodeString) == 0;
}
