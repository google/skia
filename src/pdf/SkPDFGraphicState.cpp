/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFGraphicState.h"

#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/docs/SkPDFDocument.h"
#include "include/private/SkTo.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFFormXObject.h"
#include "src/pdf/SkPDFUtils.h"

static const char* as_pdf_blend_mode_name(SkBlendMode mode) {
    const char* name = SkPDFUtils::BlendModeName(mode);
    SkASSERT(name);
    return name;
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
    if (!SkPDFUtils::BlendModeName(mode)
        || SkBlendMode::kXor  == mode
        || SkBlendMode::kPlus == mode)
    {
        mode = SkBlendMode::kSrcOver;
    }
    return SkToU8((unsigned)mode);
}

SkPDFIndirectReference SkPDFGraphicState::GetGraphicStateForPaint(SkPDFDocument* doc,
                                                                  const SkPaint& p) {
    SkASSERT(doc);
    if (SkPaint::kFill_Style == p.getStyle()) {
        SkPDFFillGraphicState fillKey = {p.getColor4f().fA, pdf_blend_mode(p.getBlendMode())};
        auto& fillMap = doc->fFillGSMap;
        if (SkPDFIndirectReference* statePtr = fillMap.find(fillKey)) {
            return *statePtr;
        }
        SkPDFDict state;
        state.reserve(2);
        state.insert("ca", SkPDFColorComponentF{fillKey.fAlpha});
        state.insert("BM", SkPDFName(as_pdf_blend_mode_name((SkBlendMode)fillKey.fBlendMode)));
        SkPDFIndirectReference ref = doc->emit(state);
        fillMap.set(fillKey, ref);
        return ref;
    } else {
        SkPDFStrokeGraphicState strokeKey = {
            p.getStrokeWidth(),
            p.getStrokeMiter(),
            p.getColor4f().fA,
            SkToU8(p.getStrokeCap()),
            SkToU8(p.getStrokeJoin()),
            pdf_blend_mode(p.getBlendMode())
        };
        auto& sMap = doc->fStrokeGSMap;
        if (SkPDFIndirectReference* statePtr = sMap.find(strokeKey)) {
            return *statePtr;
        }
        SkPDFDict state;
        state.reserve(8);
        state.insert("CA", SkPDFColorComponentF{strokeKey.fAlpha});
        state.insert("ca", SkPDFColorComponentF{strokeKey.fAlpha});
        state.insert("LC", to_stroke_cap(strokeKey.fStrokeCap));
        state.insert("LJ", to_stroke_join(strokeKey.fStrokeJoin));
        state.insert("LW", strokeKey.fStrokeWidth);
        state.insert("ML", strokeKey.fStrokeMiter);
        state.insert("SA", true);  // SA = Auto stroke adjustment.
        state.insert("BM", SkPDFName(as_pdf_blend_mode_name((SkBlendMode)strokeKey.fBlendMode)));
        SkPDFIndirectReference ref = doc->emit(state);
        sMap.set(strokeKey, ref);
        return ref;
    }
}

////////////////////////////////////////////////////////////////////////////////

static SkPDFIndirectReference make_invert_function(SkPDFDocument* doc) {
    // Acrobat crashes if we use a type 0 function, kpdf crashes if we use
    // a type 2 function, so we use a type 4 function.
    static const char psInvert[] = "{1 exch sub}";
    // Do not copy the trailing '\0' into the SkData.
    auto invertFunction = SkData::MakeWithoutCopy(psInvert, strlen(psInvert));

    auto dict = std::make_unique<SkPDFDict>();
    dict->insert("FunctionType", 4);
    dict->insert("Domain", SkPDFMakeArray(0, 1));
    dict->insert("Range", SkPDFMakeArray(0, 1));
    return SkPDFStreamOut(std::move(dict), SkMemoryStream::Make(std::move(invertFunction)), doc);
}

SkPDFIndirectReference SkPDFGraphicState::GetSMaskGraphicState(SkPDFIndirectReference sMask,
                                                               bool invert,
                                                               SkPDFSMaskMode sMaskMode,
                                                               SkPDFDocument* doc) {
    // The practical chances of using the same mask more than once are unlikely
    // enough that it's not worth canonicalizing.
    auto sMaskDict = std::make_unique<SkPDFDict>();
    sMaskDict->insert("Type", SkPDFName("Mask"));
    if (sMaskMode == kAlpha_SMaskMode) {
        sMaskDict->insert("S", SkPDFName("Alpha"));
    } else if (sMaskMode == kLuminosity_SMaskMode) {
        sMaskDict->insert("S", SkPDFName("Luminosity"));
    }
    sMaskDict->insert("G", sMask);
    if (invert) {
        // let the doc deduplicate this object.
        if (doc->fInvertFunction == SkPDFIndirectReference()) {
            doc->fInvertFunction = make_invert_function(doc);
        }
        sMaskDict->insert("TR", doc->fInvertFunction);
    }
    SkPDFDict result;
    result.insert("Type", SkPDFName("ExtGState"));
    result.insert("SMask", std::move(sMaskDict));
    return doc->emit(result);
}
