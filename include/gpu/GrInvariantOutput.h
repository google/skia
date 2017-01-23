/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInvariantOutput_DEFINED
#define GrInvariantOutput_DEFINED

#include "GrColor.h"

class GrKnownColorComponents {
public:
    GrKnownColorComponents() = default;

    GrKnownColorComponents(GrColor color, GrColorComponentFlags knownComponents)
            : fColor(color), fKnownFlags(knownComponents) {}

    GrColorComponentFlags knownFlags() const { return fKnownFlags; }

    void setUnknown() { fKnownFlags = kNone_GrColorComponentFlags; }

    void reset(GrColor color, GrColorComponentFlags knownComponents) {
        fColor = color;
        fKnownFlags = knownComponents;
    }

    GrColor color() const { return fColor; }

    void setColor(GrColor color) {
        fColor = color;
        fKnownFlags = kRGBA_GrColorComponentFlags;
    }

    void setOpaque() {
        fColor = 0xffU << GrColor_SHIFT_A;
        fKnownFlags = kA_GrColorComponentFlag;
    }

    void setSingleChannel(uint8_t channel) {
        fColor = GrColorPackRGBA(channel, channel, channel, channel);
        fKnownFlags = kRGBA_GrColorComponentFlags;
    }

    bool isConstant() const { return fKnownFlags == kRGBA_GrColorComponentFlags; }
    bool hasKnownAlpha() const { return fKnownFlags & kA_GrColorComponentFlag; }

    bool hasZeroAlpha() const {
        return ((fKnownFlags & kA_GrColorComponentFlag) && 0 == GrColorUnpackA(fColor));
    }

    bool isOpaque() const {
        return ((fKnownFlags & kA_GrColorComponentFlag) && 0xFF == GrColorUnpackA(fColor));
    }

    bool isSolidWhite() const {
        return (fKnownFlags == kRGBA_GrColorComponentFlags && 0xFFFFFFFF == fColor);
    }

    void invalidateComponents(GrColorComponentFlags invalidComponents) {
        fKnownFlags = (fKnownFlags & ~invalidComponents);
    }

private:
    GrColor fColor;
    GrColorComponentFlags fKnownFlags = kNone_GrColorComponentFlags;
};

/**
 * This describes the color or coverage input that will be seen by the first color or coverage stage
 * of a GrPipeline. This is also the GrPrimitiveProcessor color or coverage *output*.
 */
class GrPipelineInput : public GrKnownColorComponents {
public:
    void setUsingLCDCoverage() { fIsLCDCoverage = true; }
    bool isLCDCoverage() const { return fIsLCDCoverage; }

private:
    bool fIsLCDCoverage = false;
};

/** This describes the output of a GrFragmentProcessor in a GrPipeline. */
class GrInvariantOutput {
public:
    GrInvariantOutput(GrColor color, GrColorComponentFlags flags)
            : fKnownComponents(color, flags), fNonMulStageFound(false), fWillUseInputColor(true) {}

    GrInvariantOutput(const GrPipelineInput& input)
            : fKnownComponents(input), fNonMulStageFound(false), fWillUseInputColor(false) {}

    virtual ~GrInvariantOutput() {}

    enum ReadInput {
        kWill_ReadInput,
        kWillNot_ReadInput,
    };

    void mulByOpaque() {
        SkDEBUGCODE(this->validate());
        if (!fKnownComponents.isOpaque()) {
            // Since the current state is not opaque we no longer care if the color being
            // multiplied is opaque.
            this->mulByUnknown();
        }
        SkDEBUGCODE(this->validate());
    }

    void mulByUnknown() {
        SkDEBUGCODE(this->validate());
        if (fKnownComponents.hasZeroAlpha()) {
            this->internalSetToTransparentBlack();
        } else {
            this->internalSetToUnknown();
        }
        SkDEBUGCODE(this->validate());
    }

    void mulByAlpha(uint8_t alpha) {
        SkDEBUGCODE(this->validate());
        if (fKnownComponents.hasZeroAlpha() || 0 == alpha) {
            this->internalSetToTransparentBlack();
        } else {
            if (alpha != 255) {
                GrColor color = fKnownComponents.color();
                // Multiply color by alpha
                color = GrColorPackRGBA(SkMulDiv255Round(GrColorUnpackR(color), alpha),
                                        SkMulDiv255Round(GrColorUnpackG(color), alpha),
                                        SkMulDiv255Round(GrColorUnpackB(color), alpha),
                                        SkMulDiv255Round(GrColorUnpackA(color), alpha));
                fKnownComponents.reset(color, fKnownComponents.knownFlags());
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
                GrColor color = fKnownComponents.color();
                color = GrColorPackRGBA(
                        SkMulDiv255Round(GrColorUnpackR(color), GrColorUnpackR(color)),
                        SkMulDiv255Round(GrColorUnpackG(color), GrColorUnpackG(color)),
                        SkMulDiv255Round(GrColorUnpackB(color), GrColorUnpackB(color)),
                        SkMulDiv255Round(GrColorUnpackA(color), a));
                fKnownComponents.reset(color, fKnownComponents.knownFlags());
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
        } else if (fKnownComponents.hasKnownAlpha()) {
            GrColor preAlpha = GrColorUnpackA(fKnownComponents.color());
            if (0 == preAlpha) {
                this->internalSetToTransparentBlack();
            } else {
                // We know that color has different component values
                GrColor color = fKnownComponents.color();
                color = GrColorPackRGBA(SkMulDiv255Round(preAlpha, GrColorUnpackR(color)),
                                        SkMulDiv255Round(preAlpha, GrColorUnpackG(color)),
                                        SkMulDiv255Round(preAlpha, GrColorUnpackB(color)),
                                        SkMulDiv255Round(preAlpha, a));
                fKnownComponents.reset(color, kRGBA_GrColorComponentFlags);
            }
        } else {
            fKnownComponents.setUnknown();
        }
        SkDEBUGCODE(this->validate());
    }

    // Ignores the incoming color's RGB and muls its alpha by the alpha param and sets all channels
    // equal to that value.
    void mulAlphaByAlpha(uint8_t alpha) {
        SkDEBUGCODE(this->validate());
        if (0 == alpha || fKnownComponents.hasZeroAlpha()) {
            this->internalSetToTransparentBlack();
        } else {
            if (fKnownComponents.hasKnownAlpha()) {
                GrColor a = GrColorUnpackA(fKnownComponents.color());
                a = SkMulDiv255Round(alpha, a);
                fKnownComponents.reset(GrColorPackRGBA(a, a, a, a), kRGBA_GrColorComponentFlags);
            } else {
                fKnownComponents.setUnknown();
            }
        }
        SkDEBUGCODE(this->validate());
    }

    void premul() {
        SkDEBUGCODE(this->validate());
        fNonMulStageFound = true;
        if (fKnownComponents.hasKnownAlpha()) {
            fKnownComponents.reset(GrPremulColor(fKnownComponents.color()),
                                   fKnownComponents.knownFlags());
        } else {
            fKnownComponents.setUnknown();
        }
        SkDEBUGCODE(this->validate());
    }

    void invalidateComponents(GrColorComponentFlags invalidateFlags, ReadInput readsInput) {
        SkDEBUGCODE(this->validate());
        fKnownComponents.invalidateComponents(invalidateFlags);
        fNonMulStageFound = true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
        }
        SkDEBUGCODE(this->validate());
    }

    void setToColor(GrColor color, ReadInput readsInput) {
        this->setToOther(GrKnownColorComponents(color, kRGBA_GrColorComponentFlags), readsInput);
    }

    void setToUnknownOpaque(ReadInput readsInput) {
        this->setToOther(GrKnownColorComponents(0xFF << GrColor_SHIFT_A, kA_GrColorComponentFlag),
                         readsInput);
    }

    void setToOther(const GrKnownColorComponents& knownColorComponents, ReadInput readsInput) {
        SkDEBUGCODE(this->validate());
        fKnownComponents = knownColorComponents;
        fNonMulStageFound = true;
        if (kWillNot_ReadInput == readsInput) {
            fWillUseInputColor = false;
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

    GrKnownColorComponents knownComponents() const { return fKnownComponents; }
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

    void reset(GrKnownColorComponents knownColorComponents) {
        fKnownComponents = knownColorComponents;
        fNonMulStageFound = false;
        fWillUseInputColor = true;
    }

    void internalSetToTransparentBlack() { fKnownComponents.setColor(0); }

    void internalSetToUnknown() { fKnownComponents.setUnknown(); }

    void resetWillUseInputColor() { fWillUseInputColor = true; }

    bool allStagesMulInput() const { return !fNonMulStageFound; }
    void resetNonMulStageFound() { fNonMulStageFound = false; }

    GrKnownColorComponents fKnownComponents;
    bool fNonMulStageFound;
    bool fWillUseInputColor;
};

#endif

