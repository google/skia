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

    void setKnownFourComponents(GrColor color) {
        fColor = color;
        fValidFlags = kRGBA_GrColorComponentFlags;
    }

    void setUnknownFourComponents() {
        fValidFlags = kNone_GrColorComponentFlags;
    }

    void setUnknownOpaqueFourComponents() {
        fColor = 0xffU << GrColor_SHIFT_A;
        fValidFlags = kA_GrColorComponentFlag;
    }

    void setKnownSingleComponent(uint8_t alpha) {
        fColor = GrColorPackRGBA(alpha, alpha, alpha, alpha);
        fValidFlags = kRGBA_GrColorComponentFlags;
    }

    void setUnknownSingleComponent() {
        fValidFlags = kNone_GrColorComponentFlags;
    }

    void setUsingLCDCoverage() { fIsLCDCoverage = true; }

    GrColorComponentFlags   fValidFlags;
    GrColor                 fColor;
    bool                    fIsLCDCoverage; // Temorary data member until texture pixel configs are
                                            // updated
};

/** This describes the output of a GrFragmentProcessor in a GrPipeline. */
class GrInvariantOutput {
public:
    GrInvariantOutput(GrColor color, GrColorComponentFlags flags)
            : fColor(color), fValidFlags(flags), fNonMulStageFound(false) {}

    GrInvariantOutput(const GrPipelineInput& input)
            : fColor(input.fColor), fValidFlags(input.fValidFlags), fNonMulStageFound(false) {}

    virtual ~GrInvariantOutput() {}

    void mulByUnknownOpaqueFourComponents() {
        if (this->isOpaque()) {
            fValidFlags = kA_GrColorComponentFlag;
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
            fValidFlags = kNone_GrColorComponentFlags;
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
                // We don't need to change fIsSingleComponent in this case
            }
        }
    }

    void mulByKnownFourComponents(GrColor color) {
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
            }
        }
    }

    // Ignores the incoming color's RGB and muls its alpha by color.
    void mulAlphaByKnownFourComponents(GrColor color) {
        uint32_t a;
        if (GetAlphaAndCheckSingleChannel(color, &a)) {
            this->mulAlphaByKnownSingleComponent(a);
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
    }

    // Ignores the incoming color's RGB and muls its alpha by the alpha param and sets all channels
    // equal to that value.
    void mulAlphaByKnownSingleComponent(uint8_t alpha) {
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
    }

    void premulFourChannelColor() {
        fNonMulStageFound = true;
        if (!(fValidFlags & kA_GrColorComponentFlag)) {
            fValidFlags = kNone_GrColorComponentFlags;
        } else {
            fColor = GrPremulColor(fColor);
        }
    }

    void invalidateComponents(GrColorComponentFlags invalidateFlags) {
        fValidFlags = (fValidFlags & ~invalidateFlags);
        fNonMulStageFound = true;
    }

    void setToOther(GrColorComponentFlags validFlags, GrColor color) {
        fValidFlags = validFlags;
        fColor = color;
        fNonMulStageFound = true;
    }

    void setToUnknown() {
        this->internalSetToUnknown();
        fNonMulStageFound = true;
    }

    GrColor color() const { return fColor; }
    GrColorComponentFlags validFlags() const { return fValidFlags; }

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
    }

    void reset(const GrPipelineInput& input) {
        fColor = input.fColor;
        fValidFlags = input.fValidFlags;
        fNonMulStageFound = false;
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

    bool allStagesMulInput() const { return !fNonMulStageFound; }
    void resetNonMulStageFound() { fNonMulStageFound = false; }

    /**
     * If alpha is valid, check that any valid R,G,B values are <= A
     */
    SkDEBUGCODE(bool validPreMulColor() const;)

    GrColor fColor;
    GrColorComponentFlags fValidFlags;
    bool fNonMulStageFound;
};

#endif

