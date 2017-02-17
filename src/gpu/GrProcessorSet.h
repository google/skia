/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorSet_DEFINED
#define GrProcessorSet_DEFINED

#include "GrFragmentProcessor.h"
#include "GrPaint.h"
#include "GrPipelineInput.h"
#include "SkTemplates.h"

class GrAppliedClip;
class GrPaint;
class GrXPFactory;

class GrProcessorSet : private SkNoncopyable {
public:
    GrProcessorSet(GrPaint&& paint);

    ~GrProcessorSet() {
        // We are deliberately not using sk_sp here because this will be updated to work with
        // "pending execution" refs.
        for (auto fp : fFragmentProcessors) {
            fp->unref();
        }
    }

    int numColorFragmentProcessors() const { return fColorFragmentProcessorCnt; }
    int numCoverageFragmentProcessors() const {
        return fFragmentProcessors.count() - fColorFragmentProcessorCnt;
    }
    int numFragmentProcessors() const { return fFragmentProcessors.count(); }

    const GrFragmentProcessor* colorFragmentProcessor(int idx) const {
        SkASSERT(idx < fColorFragmentProcessorCnt);
        return fFragmentProcessors[idx];
    }
    const GrFragmentProcessor* coverageFragmentProcessor(int idx) const {
        return fFragmentProcessors[idx + fColorFragmentProcessorCnt];
    }

    const GrXPFactory* xpFactory() const { return fXPFactory; }

    bool usesDistanceVectorField() const { return SkToBool(fFlags & kUseDistanceVectorField_Flag); }
    bool disableOutputConversionToSRGB() const {
        return SkToBool(fFlags & kDisableOutputConversionToSRGB_Flag);
    }
    bool allowSRGBInputs() const { return SkToBool(fFlags & kAllowSRGBInputs_Flag); }

    /** This is used to track pipeline analysis through the fragment processors. */
    class FPAnalysis {
    public:
        FPAnalysis() = default;

        void reset(const GrPipelineInput& colorInput, const GrPipelineInput coverageInput,
                   const GrProcessorSet& processors, bool usesPLSDstRead,
                   const GrAppliedClip& appliedClip);

        int initialColorProcessorsToEliminate(GrColor* newInputColor) const {
            if (fInitialColorProcessorsToEliminate > 0) {
                *newInputColor = fOverrideInputColor;
            }
            return fInitialColorProcessorsToEliminate;
        }

        bool usesPLSDstRead() const { return fUsesPLSDstRead; }
        bool isCompatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }
        bool isOutputColorOpaque() const {
            return ColorType::kOpaque == fColorType ||
                   ColorType::kOpaqueConstant == fColorType;
        }
        bool hasKnownOutputColor(GrColor* color = nullptr) const {
            bool constant = ColorType::kConstant == fColorType ||
                            ColorType::kOpaqueConstant == fColorType;
            if (constant && color) {
                *color = fKnownOutpputColor;
            }
            return constant;
        }
        bool hasCoverage() const {
            return CoverageType::kNone != fCoverageType;
        }
        bool hasLCDCoverage() const {
            return CoverageType::kLCD == fCoverageType;
        }

    private:
        /** Describes known properties of a draw's color input to the GrXferProcessor. */
        enum class ColorType { kUnknown, kOpaqueConstant, kConstant, kOpaque };

        /**
         * Indicates whether a draw's coverage input to the GrXferProcessor is solid, single channel
         * or LCD (four channel coverage).
         */
        enum class CoverageType { kNone, kSingleChannel, kLCD };

        bool fUsesPLSDstRead = false;
        bool fCompatibleWithCoverageAsAlpha = true;
        CoverageType fCoverageType = CoverageType::kNone;
        ColorType fColorType = ColorType::kUnknown;
        int fInitialColorProcessorsToEliminate = 0;
        GrColor fOverrideInputColor;
        GrColor fKnownOutpputColor;
    };

private:
    const GrXPFactory* fXPFactory = nullptr;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    int fColorFragmentProcessorCnt;
    enum Flags : uint32_t {
        kUseDistanceVectorField_Flag = 0x1,
        kDisableOutputConversionToSRGB_Flag = 0x2,
        kAllowSRGBInputs_Flag = 0x4
    };
    uint32_t fFlags;
};

#endif
