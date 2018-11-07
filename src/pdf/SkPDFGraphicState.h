/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFGraphicState_DEFINED
#define SkPDFGraphicState_DEFINED

#include "SkMacros.h"
#include "SkOpts.h"
#include "SkPDFTypes.h"
#include "SkPaint.h"

class SkPaint;
class SkPDFCanon;

/** \class SkPDFGraphicState
    SkPaint objects roughly correspond to graphic state dictionaries that can
    be installed. So that a given dictionary is only output to the pdf file
    once, we want to canonicalize them.
*/
namespace SkPDFGraphicState {
    enum SkPDFSMaskMode {
        kAlpha_SMaskMode,
        kLuminosity_SMaskMode
    };

    /** Get the graphic state for the passed SkPaint.
     */
    sk_sp<SkPDFDict> GetGraphicStateForPaint(SkPDFCanon*, const SkPaint&);

    /** Make a graphic state that only sets the passed soft mask.
     *  @param sMask     The form xobject to use as a soft mask.
     *  @param invert    Indicates if the alpha of the sMask should be inverted.
     *  @param sMaskMode Whether to use alpha or luminosity for the sMask.
     *
     *  These are not de-duped.
     */
    sk_sp<SkPDFDict> GetSMaskGraphicState(sk_sp<SkPDFObject> sMask,
                                          bool invert,
                                          SkPDFSMaskMode sMaskMode,
                                          SkPDFCanon* canon);

    sk_sp<SkPDFStream> MakeInvertFunction();
}

static_assert(sizeof(SkPaint::Cap)  == sizeof(uint8_t), "");
static_assert(sizeof(SkPaint::Join) == sizeof(uint8_t), "");

SK_BEGIN_REQUIRE_DENSE
struct SkPDFStrokeGraphicState {
    SkScalar fStrokeWidth;
    SkScalar fStrokeMiter;
    SkScalar fAlpha;
    SkPaint::Cap fStrokeCap;
    SkPaint::Join fStrokeJoin;
    uint8_t fBlendMode;   // SkBlendMode
    uint8_t fPADDING = 0;
    bool operator==(const SkPDFStrokeGraphicState& o) const { return !memcmp(this, &o, sizeof(o)); }
    bool operator!=(const SkPDFStrokeGraphicState& o) const { return !(*this == o); }
};
SK_END_REQUIRE_DENSE

SK_BEGIN_REQUIRE_DENSE
struct SkPDFFillGraphicState {
    SkScalar fAlpha;
    uint8_t fBlendMode;
    uint8_t fPADDING[3] = {0, 0, 0};
    bool operator==(const SkPDFFillGraphicState& o) const { return !memcmp(this, &o, sizeof(o)); }
    bool operator!=(const SkPDFFillGraphicState& o) const { return !(*this == o); }
};
SK_END_REQUIRE_DENSE

#endif
