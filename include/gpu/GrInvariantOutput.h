/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInvariantOutput_DEFINED
#define GrInvariantOutput_DEFINED

#include "GrColor.h"

/**
 * This describes the color or coverage input that will be seen by the first color or coverage stage
 * of a GrPipeline. This is also the GrPrimitiveProcessor color or coverage *output*.
 */
struct GrPipelineInput {
    GrPipelineInput()
            : fValidFlags(kNone_GrColorComponentFlags)
            , fColor(0)
            , fIsLCDCoverage(false) {}

    void setColor(GrColor color) {
        fColor = color;
        fValidFlags = kRGBA_GrColorComponentFlags;
    }

    void setUnknown() { fValidFlags = kNone_GrColorComponentFlags; }

    void setOpaque() {
        fColor = 0xffU << GrColor_SHIFT_A;
        fValidFlags = kA_GrColorComponentFlag;
    }

    void setSingleChannel(uint8_t channel) {
        fColor = GrColorPackRGBA(channel, channel, channel, channel);
        fValidFlags = kRGBA_GrColorComponentFlags;
    }

    void setUsingLCDCoverage() { fIsLCDCoverage = true; }

    GrColorComponentFlags fValidFlags;
    GrColor fColor;
    bool fIsLCDCoverage;
};

/** This describes the output of a GrFragmentProcessor in a GrPipeline. */
class GrInvariantOutput {
public:
    GrInvariantOutput(GrColor color, GrColorComponentFlags flags)
            : fColor(color)
            , fValidFlags(flags)
            , fNonMulStageFound(false)
            , fWillUseInputColor(true) {}

    GrInvariantOutput(const GrPipelineInput& input)
            : fColor(input.fColor)
            , fValidFlags(input.fValidFlags)
            , fNonMulStageFound(false)
            , fWillUseInputColor(false) {}

    virtual ~GrInvariantOutput() {}

    enum ReadInput {
        kWill_ReadInput,
        kWillNot_ReadInput,
    };

    void mulByOpaque() {
        SkDEBUGCODE(this->validate());
        if (this->isOpaque()) {
            fValidFlags = kA_GrColorComponentFlag;
        } else {
            // Since the current state is not opaque we no longer care if the color being
            // multiplied is opaque.
            this->mulByUnknown();
        }
        SkDEBUGCODE(this->validate());
    }

    void mulByUnknown() {
        SkDEBUGCODE(this->validate());
        if (this->hasZeroAlpha()) {
            this->internalSetToTransparentBlack();
        } else {
            this->internalSetToUnknown();
        }
        SkDEBUGCODE(this->validate());
    }

    void mulByAlpha(uint8_t alpha) {
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

    void mulByColor(GrColor color) {
        SkDEBUGCODE(this->validate());
        uint32_t a;
        if (GetAlphaAndCheckSingleChannel(color, &a)) {
            this->mulByAlpha(a);
        } else {
            if (color != 0xffffffff) {
                fColor = GrColorPackRGBA(
                    SkMulDiv255Round(GrColorUnpackR(fColor), GrColorUnpackR(color)),
                    SkMulDiv255Round(GrColorUnpackG(fColor), GrColorUnpackG(color)),
                    SkMulDiv255Round(GrColorUnpackB(fColor), GrColorUnpackB(color)),
                    SkMulDiv255Round(GrColorUnpackA(fColor), a));
            }
        }
        SkDEBUGCODE(this->validate());
    }

    // Ignores the incoming color's RGB and muls its alpha by color.
    void mulAlphaByColor(GrColor color) {
        SkDEBUGCODE(this->validate());
        uint32_t a;
        if (GetAlphaAndCheckSingleChannel(color, &a)) {
            this->mulAlphaByAlpha(a);
        } else if (fValidFlags & kA_GrColorComponentFlag) {
            GrColor preAlpha = GrColorUnpackA(fColor);
            if (0 == preAlpha) {
                this->internalSetToTransparentBlack();
            } else {
                // We know that color has different component values
                fColor = GrColorPackRGBA(
                    SkMulDiv255Round(preAlpha, GrColorUnpackR(color)),
                    SkMulDiv255Round(preAlpha, GrColorUnpackG(color)),
                    SkMulDiv255Round(preAlpha, GrColorUnpackB(color)),
                    SkMulDiv255Round(preAlpha, a));
                fValidFlags = kRGBA_GrColorComponentFlags;
            }
        } else {
            fValidFlags = kNone_GrColorComponentFlags;
        }
        SkDEBUGCODE(this->validate());
    }

    // Ignores the incoming color's RGB and muls its alpha by the alpha param and sets all channels
    // equal to that value.
    void mulAlphaByAlpha(uint8_t alpha) {
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
        }
        SkDEBUGCODE(this->validate());
    }

    void premul() {
        SkDEBUGCODE(this->validate());
        fNonMulStageFound = true;
        if (!(fValidFlags & kA_GrColorComponentFlag)) {
            fValidFlags = kNone_GrColorComponentFlags;
        } else {
            fColor = GrPremulColor(fColor);
        }
        SkDEBUGCODE(this->validate());
    }

    void invalidateComponents(GrColorComponentFlags invalidateFlags, ReadInput readsInput) {
        SkDEBUGCODE(this->validate());
        fValidFlags = (fValidFlags & ~invalidateFlags);
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
        fNonMulStageFound = true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
        }
        if (kRGBA_GrColorComponentFlags == fValidFlags) {
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

    GrColor color() const { return fColor; }
    GrColorComponentFlags validFlags() const { return fValidFlags; }
    bool willUseInputColor() const { return fWillUseInputColor; }

#ifdef SK_DEBUG
    void validate() const {
        // If we claim that we are not using the input color we must not be modulating the input.
        SkASSERT(fNonMulStageFound || fWillUseInputColor);
    }
#endif

private:
    friend class GrProcOptInfo;

    /** Extracts the alpha channel and returns true if r,g,b == a. */
    static bool GetAlphaAndCheckSingleChannel(GrColor color, uint32_t* alpha) {
        *alpha = GrColorUnpackA(color);
        return *alpha == GrColorUnpackR(color) && *alpha == GrColorUnpackG(color) &&
               *alpha == GrColorUnpackB(color);
    }

    void reset(GrColor color, GrColorComponentFlags flags) {
        fColor = color;
        fValidFlags = flags;
        fNonMulStageFound = false;
        fWillUseInputColor = true;
    }

    void reset(const GrPipelineInput& input) {
        fColor = input.fColor;
        fValidFlags = input.fValidFlags;
        fNonMulStageFound = false;
        fWillUseInputColor = true;
    }

    void internalSetToTransparentBlack() {
        fValidFlags = kRGBA_GrColorComponentFlags;
        fColor = 0;
    }

    void internalSetToUnknown() {
        fValidFlags = kNone_GrColorComponentFlags;
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

    void resetWillUseInputColor() { fWillUseInputColor = true; }

    bool allStagesMulInput() const { return !fNonMulStageFound; }
    void resetNonMulStageFound() { fNonMulStageFound = false; }

    /**
     * If alpha is valid, check that any valid R,G,B values are <= A
     */
    SkDEBUGCODE(bool validPreMulColor() const;)

    GrColor fColor;
    GrColorComponentFlags fValidFlags;
    bool fNonMulStageFound;
    bool fWillUseInputColor;
};

#endif

