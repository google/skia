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
class SkPDFGraphicState final : public SkPDFObject {

public:
    enum SkPDFSMaskMode {
        kAlpha_SMaskMode,
        kLuminosity_SMaskMode
    };

    // Override emitObject so that we can populate the dictionary on
    // demand.
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& objNumMap) const override;

    /** Get the graphic state for the passed SkPaint.
     *  @param paint  The SkPaint to emulate.
     */
    static sk_sp<SkPDFGraphicState> GetGraphicStateForPaint(SkPDFCanon* canon,
                                                            const SkPaint& paint);

    /** Make a graphic state that only sets the passed soft mask.
     *  @param sMask     The form xobject to use as a soft mask.
     *  @param invert    Indicates if the alpha of the sMask should be inverted.
     *  @param sMaskMode Whether to use alpha or luminosity for the sMask.
     *
     *  These are not de-duped.
     */
    static sk_sp<SkPDFDict> GetSMaskGraphicState(sk_sp<SkPDFObject> sMask,
                                                 bool invert,
                                                 SkPDFSMaskMode sMaskMode,
                                                 SkPDFCanon* canon);

    /** Make a graphic state that only unsets the soft mask. */
    static sk_sp<SkPDFDict> MakeNoSmaskGraphicState();
    static sk_sp<SkPDFStream> MakeInvertFunction();

    bool operator==(const SkPDFGraphicState& rhs) const {
        return 0 == memcmp(&fStrokeWidth, &rhs.fStrokeWidth, 12);
    }
    uint32_t hash() const { return SkOpts::hash(&fStrokeWidth, 12); }

private:
    const SkScalar fStrokeWidth;
    const SkScalar fStrokeMiter;
    const uint8_t fAlpha;
    const uint8_t fStrokeCap;   // SkPaint::Cap
    const uint8_t fStrokeJoin;  // SkPaint::Join
    const uint8_t fMode;        // SkBlendMode

    SkPDFGraphicState(const SkPaint&);

    typedef SkPDFDict INHERITED;
};

#endif
