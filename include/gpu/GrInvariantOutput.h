/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInvariantOutput_DEFINED
#define GrInvariantOutput_DEFINED

#include "GrColor.h"

struct GrInitInvariantOutput {
    GrInitInvariantOutput()
        : fValidFlags(0)
        , fColor(0)
        , fIsSingleComponent(false)
        , fIsLCDCoverage(false) {}

    void setKnownFourComponents(GrColor color) {
        fColor = color;
        fValidFlags = kRGBA_GrColorComponentFlags;
        fIsSingleComponent = false;
    }

    void setUnknownFourComponents() {
        fValidFlags = 0;
        fIsSingleComponent = false;
    }

    void setUnknownOpaqueFourComponents() {
        fColor = 0xff << GrColor_SHIFT_A;
        fValidFlags = kA_GrColorComponentFlag;
        fIsSingleComponent = false;
    }

    void setKnownSingleComponent(uint8_t alpha) {
        fColor = GrColorPackRGBA(alpha, alpha, alpha, alpha);
        fValidFlags = kRGBA_GrColorComponentFlags;
        fIsSingleComponent = true;
    }

    void setUnknownSingleComponent() {
        fValidFlags = 0;
        fIsSingleComponent = true;
    }

    void setUsingLCDCoverage() { fIsLCDCoverage = true; }

    uint32_t fValidFlags;
    GrColor fColor;
    bool fIsSingleComponent;
    bool fIsLCDCoverage; // Temorary data member until texture pixel configs are updated
};

class GrInvariantOutput {
public:
    GrInvariantOutput(GrColor color, GrColorComponentFlags flags, bool isSingleComponent)
        : fColor(color)
        , fValidFlags(flags)
        , fIsSingleComponent(isSingleComponent)
        , fNonMulStageFound(false)
        , fWillUseInputColor(true)
        , fIsLCDCoverage(false) {}

    GrInvariantOutput(const GrInitInvariantOutput& io)
        : fColor(io.fColor)
        , fValidFlags(io.fValidFlags)
        , fIsSingleComponent(io.fIsSingleComponent)
        , fNonMulStageFound(false)
        , fWillUseInputColor(false)
        , fIsLCDCoverage(io.fIsLCDCoverage) {}

    virtual ~GrInvariantOutput() {}

    enum ReadInput {
        kWill_ReadInput,
        kWillNot_ReadInput,
    };

    void mulByUnknownOpaqueFourComponents() {
        if (this->isOpaque()) {
            fValidFlags = kA_GrColorComponentFlag;
            fIsSingleComponent = false;
        } else {
            // Since the current state is not opaque we no longer care if the color being
            // multiplied is opaque.
            this->mulByUnknownFourComponents();
        }
    }

    void mulByUnknownFourComponents() {
        if (this->hasZeroAlpha()) {
            this->internalSetToTransparentBlack();
        } else {
            this->internalSetToUnknown();
        }
    }

    void mulByUnknownSingleComponent() {
        if (this->hasZeroAlpha()) {
            this->internalSetToTransparentBlack();
        } else {
            // We don't need to change fIsSingleComponent in this case
            fValidFlags = 0;
        }
    }

    void mulByKnownSingleComponent(uint8_t alpha) {
        if (this->hasZeroAlpha() || 0 == alpha) {
            this->internalSetToTransparentBlack();
        } else {
            if (alpha != 255) {
                // Multiply color by alpha
                fColor = GrColorPackRGBA(SkMulDiv255Round(GrColorUnpackR(fColor), alpha),
                                         SkMulDiv255Round(GrColorUnpackG(fColor), alpha),
                                         SkMulDiv255Round(GrColorUnpackB(fColor), alpha),
                                         SkMulDiv255Round(GrColorUnpackA(fColor), alpha));
            }
        }
    }

    void invalidateComponents(uint8_t invalidateFlags, ReadInput readsInput) {
        fValidFlags &= ~invalidateFlags;
        fIsSingleComponent = false;
        fNonMulStageFound = true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
        }
    }

    void setToOther(uint8_t validFlags, GrColor color, ReadInput readsInput) {
        fValidFlags = validFlags;
        fColor = color;
        fIsSingleComponent = false;
        fNonMulStageFound = true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
        }
    }

    void setToUnknown(ReadInput readsInput) {
        this->internalSetToUnknown();
        fNonMulStageFound= true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
        }
    }

    // Temporary setter to handle LCD text correctly until we improve texture pixel config queries
    // and thus can rely solely on number of coverage components for RGA vs single channel coverage.
    void setUsingLCDCoverage() {
        fIsLCDCoverage = true;
    }

    GrColor color() const { return fColor; }
    uint8_t validFlags() const { return fValidFlags; }

    /**
     * If isSingleComponent is true, then the flag values for r, g, b, and a must all be the
     * same. If the flags are all set then all color components must be equal.
     */
    SkDEBUGCODE(void validate() const;)

private:
    friend class GrProcOptInfo;

    void reset(GrColor color, GrColorComponentFlags flags, bool isSingleComponent) {
        fColor = color;
        fValidFlags = flags;
        fIsSingleComponent = isSingleComponent;
        fNonMulStageFound = false;
        fWillUseInputColor = true;
    }

    void reset(const GrInitInvariantOutput& io) {
        fColor = io.fColor;
        fValidFlags = io.fValidFlags;
        fIsSingleComponent = io.fIsSingleComponent;
        fNonMulStageFound = false;
        fWillUseInputColor = true;
        fIsLCDCoverage = io.fIsLCDCoverage;
    }

    void internalSetToTransparentBlack() {
        fValidFlags = kRGBA_GrColorComponentFlags;
        fColor = 0;
        fIsSingleComponent = true;
    }

    void internalSetToUnknown() {
        fValidFlags = 0;
        fIsSingleComponent = false;
    }

    bool hasZeroAlpha() const {
        return ((fValidFlags & kA_GrColorComponentFlag) && 0 == GrColorUnpackA(fColor));
    }

    bool isOpaque() const {
        return ((fValidFlags & kA_GrColorComponentFlag) && 0xFF == GrColorUnpackA(fColor));
    }

    bool isSolidWhite() const {
        return (fValidFlags == kRGBA_GrColorComponentFlags && 0xFFFFFFFF == fColor);
    }

    bool isSingleComponent() const { return fIsSingleComponent; }

    bool willUseInputColor() const { return fWillUseInputColor; }
    void resetWillUseInputColor() { fWillUseInputColor = true; }

    bool allStagesMulInput() const { return !fNonMulStageFound; }
    void resetNonMulStageFound() { fNonMulStageFound = false; }

    bool isLCDCoverage() const { return fIsLCDCoverage; }

    SkDEBUGCODE(bool colorComponentsAllEqual() const;)
    /**
     * If alpha is valid, check that any valid R,G,B values are <= A
     */
    SkDEBUGCODE(bool validPreMulColor() const;)

    GrColor fColor;
    uint32_t fValidFlags;
    bool fIsSingleComponent;
    bool fNonMulStageFound;
    bool fWillUseInputColor;
    bool fIsLCDCoverage; // Temorary data member until texture pixel configs are updated

};

#endif

