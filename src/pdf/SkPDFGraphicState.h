/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFGraphicState_DEFINED
#define SkPDFGraphicState_DEFINED

#include "SkPDFTypes.h"
#include "SkOpts.h"

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

SK_BEGIN_REQUIRE_DENSE
struct SkPDFStrokeGraphicState {
    SkScalar fStrokeWidth;
    SkScalar fStrokeMiter;
    uint8_t fStrokeCap;   // SkPaint::Cap
    uint8_t fStrokeJoin;  // SkPaint::Join
    uint8_t fAlpha;
    uint8_t fBlendMode;
    bool operator==(const SkPDFStrokeGraphicState& o) const { return !memcmp(this, &o, sizeof(o)); }
    bool operator!=(const SkPDFStrokeGraphicState& o) const { return !(*this == o); }
};
SK_END_REQUIRE_DENSE

SK_BEGIN_REQUIRE_DENSE
struct SkPDFFillGraphicState {
    uint8_t fAlpha;
    uint8_t fBlendMode;
    bool operator==(const SkPDFFillGraphicState& o) const { return !memcmp(this, &o, sizeof(o)); }
    bool operator!=(const SkPDFFillGraphicState& o) const { return !(*this == o); }
};
SK_END_REQUIRE_DENSE

#endif
