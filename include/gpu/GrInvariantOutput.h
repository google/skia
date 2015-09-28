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
        : fValidFlags(kNone_GrColorComponentFlags)
        , fColor(0)
        , fIsSingleComponent(false)
        , fIsLCDCoverage(false) {}

    void setKnownFourComponents(GrColor color) {
        fColor = color;
        fValidFlags = kRGBA_GrColorComponentFlags;
        fIsSingleComponent = false;
    }

    void setUnknownFourComponents() {
        fValidFlags = kNone_GrColorComponentFlags;
        fIsSingleComponent = false;
    }

    void setUnknownOpaqueFourComponents() {
        fColor = 0xffU << GrColor_SHIFT_A;
        fValidFlags = kA_GrColorComponentFlag;
        fIsSingleComponent = false;
    }

    void setKnownSingleComponent(uint8_t alpha) {
        fColor = GrColorPackRGBA(alpha, alpha, alpha, alpha);
        fValidFlags = kRGBA_GrColorComponentFlags;
        fIsSingleComponent = true;
    }

    void setUnknownSingleComponent() {
        fValidFlags = kNone_GrColorComponentFlags;
        fIsSingleComponent = true;
    }

    void setUsingLCDCoverage() { fIsLCDCoverage = true; }

    GrColorComponentFlags   fValidFlags;
    GrColor                 fColor;
    bool                    fIsSingleComponent;
    bool                    fIsLCDCoverage; // Temorary data member until texture pixel configs are
                                            // updated
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
        SkDEBUGCODE(this->validate());
        if (this->isOpaque()) {
            fValidFlags = kA_GrColorComponentFlag;
            fIsSingleComponent = false;
        } else {
            // Since the current state is not opaque we no longer care if the color being
            // multiplied is opaque.
            this->mulByUnknownFourComponents();
        }
        SkDEBUGCODE(this->validate());
    }

    void mulByUnknownFourComponents() {
        SkDEBUGCODE(this->validate());
        if (this->hasZeroAlpha()) {
            this->internalSetToTransparentBlack();
        } else {
            this->internalSetToUnknown();
        }
        SkDEBUGCODE(this->validate());
    }

    void mulByUnknownSingleComponent() {
        SkDEBUGCODE(this->validate());
        if (this->hasZeroAlpha()) {
            this->internalSetToTransparentBlack();
        } else {
            // We don't need to change fIsSingleComponent in this case
            fValidFlags = kNone_GrColorComponentFlags;
        }
        SkDEBUGCODE(this->validate());
    }

    void mulByKnownSingleComponent(uint8_t alpha) {
        SkDEBUGCODE(this->validate());
        if (this->hasZeroAlpha() || 0 == alpha) {
            this->internalSetToTransparentBlack();
        } else {
            if (alpha != 255) {
                // Multiply color by alpha
                fColor = GrColorPackRGBA(SkMulDiv255Round(GrColorUnpackR(fColor), alpha),
                                         SkMulDiv255Round(GrColorUnpackG(fColor), alpha),
                                         SkMulDiv255Round(GrColorUnpackB(fColor), alpha),
                                         SkMulDiv255Round(GrColorUnpackA(fColor), alpha));
                // We don't need to change fIsSingleComponent in this case
            }
        }
        SkDEBUGCODE(this->validate());
    }

    void mulByKnownFourComponents(GrColor color) {
        SkDEBUGCODE(this->validate());
        uint32_t a;
        if (GetAlphaAndCheckSingleChannel(color, &a)) {
            this->mulByKnownSingleComponent(a);
        } else {
            if (color != 0xffffffff) {
                fColor = GrColorPackRGBA(
                    SkMulDiv255Round(GrColorUnpackR(fColor), GrColorUnpackR(color)),
                    SkMulDiv255Round(GrColorUnpackG(fColor), GrColorUnpackG(color)),
                    SkMulDiv255Round(GrColorUnpackB(fColor), GrColorUnpackB(color)),
                    SkMulDiv255Round(GrColorUnpackA(fColor), a));
                if (kRGBA_GrColorComponentFlags == fValidFlags) {
                    fIsSingleComponent = GetAlphaAndCheckSingleChannel(fColor, &a);
                }
            }
        }
        SkDEBUGCODE(this->validate());
    }

    // Ignores the incoming color's RGB and muls its alpha by color.
    void mulAlphaByKnownFourComponents(GrColor color) {
        SkDEBUGCODE(this->validate());
        uint32_t a;
        if (GetAlphaAndCheckSingleChannel(color, &a)) {
            this->mulAlphaByKnownSingleComponent(a);
        } else if (fValidFlags & kA_GrColorComponentFlag) {
            GrColor preAlpha = GrColorUnpackA(fColor);
            if (0 == preAlpha) {
                this->internalSetToTransparentBlack();
            } else {
                // We know that color has different component values
                fIsSingleComponent = false;
                fColor = GrColorPackRGBA(
                    SkMulDiv255Round(preAlpha, GrColorUnpackR(color)),
                    SkMulDiv255Round(preAlpha, GrColorUnpackG(color)),
                    SkMulDiv255Round(preAlpha, GrColorUnpackB(color)),
                    SkMulDiv255Round(preAlpha, a));
                fValidFlags = kRGBA_GrColorComponentFlags;
            }
        } else {
            fIsSingleComponent = false;
            fValidFlags = kNone_GrColorComponentFlags;
        }
        SkDEBUGCODE(this->validate());
    }

    // Ignores the incoming color's RGB and muls its alpha by the alpha param and sets all channels
    // equal to that value.
    void mulAlphaByKnownSingleComponent(uint8_t alpha) {
        SkDEBUGCODE(this->validate());
        if (0 == alpha || this->hasZeroAlpha()) {
            this->internalSetToTransparentBlack();
        } else {
            if (fValidFlags & kA_GrColorComponentFlag) {
                GrColor a = GrColorUnpackA(fColor);
                a = SkMulDiv255Round(alpha, a);
                fColor = GrColorPackRGBA(a, a, a, a);
                fValidFlags = kRGBA_GrColorComponentFlags;
            } else {
                fValidFlags = kNone_GrColorComponentFlags;
            }
            fIsSingleComponent = true;
        }
        SkDEBUGCODE(this->validate());
    }

    void invalidateComponents(GrColorComponentFlags invalidateFlags, ReadInput readsInput) {
        SkDEBUGCODE(this->validate());
        fValidFlags = (fValidFlags & ~invalidateFlags);
        fIsSingleComponent = false;
        fNonMulStageFound = true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
        }
        SkDEBUGCODE(this->validate());
    }

    void setToOther(GrColorComponentFlags validFlags, GrColor color, ReadInput readsInput) {
        SkDEBUGCODE(this->validate());
        fValidFlags = validFlags;
        fColor = color;
        fIsSingleComponent = false;
        fNonMulStageFound = true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
        }
        if (kRGBA_GrColorComponentFlags == fValidFlags) {
            uint32_t a;
            if (GetAlphaAndCheckSingleChannel(color, &a)) {
                fIsSingleComponent = true;
            }
        }
        SkDEBUGCODE(this->validate());
    }

    void setToUnknown(ReadInput readsInput) {
        SkDEBUGCODE(this->validate());
        this->internalSetToUnknown();
        fNonMulStageFound= true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
        }
        SkDEBUGCODE(this->validate());
    }

    // Temporary setter to handle LCD text correctly until we improve texture pixel config queries
    // and thus can rely solely on number of coverage components for RGA vs single channel coverage.
    void setUsingLCDCoverage() {
        fIsLCDCoverage = true;
    }

    GrColor color() const { return fColor; }
    GrColorComponentFlags validFlags() const { return fValidFlags; }
    bool willUseInputColor() const { return fWillUseInputColor; }

    /**
     * If isSingleComponent is true, then the flag values for r, g, b, and a must all be the
     * same. If the flags are all set then all color components must be equal.
     */
    SkDEBUGCODE(void validate() const;)

private:
    friend class GrProcOptInfo;

    /** Extracts the alpha channel and returns true if r,g,b == a. */
    static bool GetAlphaAndCheckSingleChannel(GrColor color, uint32_t* alpha) {
        *alpha = GrColorUnpackA(color);
        return *alpha == GrColorUnpackR(color) && *alpha == GrColorUnpackG(color) &&
               *alpha == GrColorUnpackB(color);
    }

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
        fValidFlags = kNone_GrColorComponentFlags;
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
    GrColorComponentFlags fValidFlags;
    bool fIsSingleComponent;
    bool fNonMulStageFound;
    bool fWillUseInputColor;
    bool fIsLCDCoverage; // Temorary data member until texture pixel configs are updated

};

#endif

