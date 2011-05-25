/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkPDFFormXObject.h"
#include "SkPDFGraphicState.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkTypes.h"

static const char* blend_mode_from_xfermode(SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kSrcOver_Mode:    return "Normal";
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

        // TODO(vandebo) Figure out if we can support more of these modes.
        case SkXfermode::kSrcATop_Mode:
        case SkXfermode::kDstATop_Mode:
        case SkXfermode::kXor_Mode:
        case SkXfermode::kPlus_Mode:
            return NULL;
    }
    return NULL;
}

SkPDFGraphicState::~SkPDFGraphicState() {
    SkAutoMutexAcquire lock(canonicalPaintsMutex());
    if (!fSMask) {
        int index = find(fPaint);
        SkASSERT(index >= 0);
        canonicalPaints().removeShuffle(index);
    }
    fResources.unrefAll();
}

void SkPDFGraphicState::getResources(SkTDArray<SkPDFObject*>* resourceList) {
    resourceList->setReserve(resourceList->count() + fResources.count());
    for (int i = 0; i < fResources.count(); i++) {
        resourceList->push(fResources[i]);
        fResources[i]->ref();
        fResources[i]->getResources(resourceList);
    }
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
SkPDFGraphicState::canonicalPaints() {
    // This initialization is only thread safe with gcc.
    static SkTDArray<SkPDFGraphicState::GSCanonicalEntry> gCanonicalPaints;
    return gCanonicalPaints;
}

// static
SkMutex& SkPDFGraphicState::canonicalPaintsMutex() {
    // This initialization is only thread safe with gcc.
    static SkMutex gCanonicalPaintsMutex;
    return gCanonicalPaintsMutex;
}

// static
SkPDFGraphicState* SkPDFGraphicState::getGraphicStateForPaint(
        const SkPaint& paint) {
    SkAutoMutexAcquire lock(canonicalPaintsMutex());
    int index = find(paint);
    if (index >= 0) {
        canonicalPaints()[index].fGraphicState->ref();
        return canonicalPaints()[index].fGraphicState;
    }
    GSCanonicalEntry newEntry(new SkPDFGraphicState(paint));
    canonicalPaints().push(newEntry);
    return newEntry.fGraphicState;
}

// static
SkPDFObject* SkPDFGraphicState::GetInvertFunction() {
    // This assumes that canonicalPaintsMutex is held.
    static SkPDFStream* invertFunction = NULL;
    if (!invertFunction) {
        // Acrobat crashes if we use a type 0 function, kpdf crashes if we use
        // a type 2 function, so we use a type 4 function.
        SkRefPtr<SkPDFArray> domainAndRange = new SkPDFArray;
        domainAndRange->unref();  // SkRefPtr and new both took a reference.
        domainAndRange->reserve(2);
        domainAndRange->append(new SkPDFInt(0))->unref();
        domainAndRange->append(new SkPDFInt(1))->unref();

        static const char psInvert[] = "{1 exch sub}";
        SkRefPtr<SkMemoryStream> psInvertStream =
            new SkMemoryStream(&psInvert, strlen(psInvert), true);
        psInvertStream->unref();  // SkRefPtr and new both took a reference.

        invertFunction = new SkPDFStream(psInvertStream.get());
        invertFunction->insert("FunctionType", new SkPDFInt(4))->unref();
        invertFunction->insert("Domain", domainAndRange.get());
        invertFunction->insert("Range", domainAndRange.get());
    }
    return invertFunction;
}

// static
SkPDFGraphicState* SkPDFGraphicState::getSMaskGraphicState(
        SkPDFFormXObject* sMask, bool invert) {
    // The practical chances of using the same mask more than once are unlikely
    // enough that it's not worth canonicalizing.
    SkAutoMutexAcquire lock(canonicalPaintsMutex());

    SkRefPtr<SkPDFDict> sMaskDict = new SkPDFDict("Mask");
    sMaskDict->unref();  // SkRefPtr and new both took a reference.
    sMaskDict->insert("S", new SkPDFName("Alpha"))->unref();
    sMaskDict->insert("G", new SkPDFObjRef(sMask))->unref();

    SkPDFGraphicState* result = new SkPDFGraphicState;
    result->fPopulated = true;
    result->fSMask = true;
    result->insert("Type", new SkPDFName("ExtGState"))->unref();
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
SkPDFGraphicState* SkPDFGraphicState::getNoSMaskGraphicState() {
    SkAutoMutexAcquire lock(canonicalPaintsMutex());
    static SkPDFGraphicState* noSMaskGS = NULL;
    if (!noSMaskGS) {
        noSMaskGS = new SkPDFGraphicState;
        noSMaskGS->fPopulated = true;
        noSMaskGS->fSMask = true;
        noSMaskGS->insert("Type", new SkPDFName("ExtGState"))->unref();
        noSMaskGS->insert("SMask", new SkPDFName("None"))->unref();
    }
    noSMaskGS->ref();
    return noSMaskGS;
}

// static
int SkPDFGraphicState::find(const SkPaint& paint) {
    GSCanonicalEntry search(&paint);
    return canonicalPaints().find(search);
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
        insert("Type", new SkPDFName("ExtGState"))->unref();

        SkRefPtr<SkPDFScalar> alpha =
            new SkPDFScalar(fPaint.getAlpha() * SkScalarInvert(0xFF));
        alpha->unref();  // SkRefPtr and new both took a reference.
        insert("CA", alpha.get());
        insert("ca", alpha.get());

        SK_COMPILE_ASSERT(SkPaint::kButt_Cap == 0, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kRound_Cap == 1, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kSquare_Cap == 2, paint_cap_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kCapCount == 3, paint_cap_mismatch);
        SkASSERT(fPaint.getStrokeCap() >= 0 && fPaint.getStrokeCap() <= 2);
        insert("LC", new SkPDFInt(fPaint.getStrokeCap()))->unref();

        SK_COMPILE_ASSERT(SkPaint::kMiter_Join == 0, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kRound_Join == 1, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kBevel_Join == 2, paint_join_mismatch);
        SK_COMPILE_ASSERT(SkPaint::kJoinCount == 3, paint_join_mismatch);
        SkASSERT(fPaint.getStrokeJoin() >= 0 && fPaint.getStrokeJoin() <= 2);
        insert("LJ", new SkPDFInt(fPaint.getStrokeJoin()))->unref();

        insert("LW", new SkPDFScalar(fPaint.getStrokeWidth()))->unref();
        insert("ML", new SkPDFScalar(fPaint.getStrokeMiter()))->unref();
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
        insert("BM",
               new SkPDFName(blend_mode_from_xfermode(xfermode)))->unref();
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
