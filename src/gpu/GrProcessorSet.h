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

    /**
     * This is used to track analysis of color and coverage values through the fragment processors.
     */
    class FragmentProcessorAnalysis {
    public:
        FragmentProcessorAnalysis() = default;
        // This version is used by a unit test that assumes no clip, no processors, and no PLS.
        FragmentProcessorAnalysis(const GrPipelineInput& colorInput,
                                  const GrPipelineInput coverageInput, const GrCaps&);

        void reset(const GrPipelineInput& colorInput, const GrPipelineInput coverageInput,
                   const GrProcessorSet&, bool usesPLSDstRead, const GrAppliedClip&, const GrCaps&);

        int initialColorProcessorsToEliminate(GrColor* newInputColor) const {
            if (fInitialColorProcessorsToEliminate > 0) {
                *newInputColor = fOverrideInputColor;
            }
            return fInitialColorProcessorsToEliminate;
        }

        bool usesPLSDstRead() const { return fUsesPLSDstRead; }
        bool usesLocalCoords() const { return fUsesLocalCoords; }
        bool isCompatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }
        bool isOutputColorOpaque() const {
            return ColorType::kOpaque == fColorType || ColorType::kOpaqueConstant == fColorType;
        }
        bool hasKnownOutputColor(GrColor* color = nullptr) const {
            bool constant =
                    ColorType::kConstant == fColorType || ColorType::kOpaqueConstant == fColorType;
            if (constant && color) {
                *color = fKnownOutputColor;
            }
            return constant;
        }
        bool hasCoverage() const { return CoverageType::kNone != fCoverageType; }
        bool hasLCDCoverage() const { return CoverageType::kLCD == fCoverageType; }

    private:
        void internalReset(const GrPipelineInput& colorInput, const GrPipelineInput coverageInput,
                           const GrProcessorSet&, bool usesPLSDstRead,
                           const GrFragmentProcessor* clipFP, const GrCaps&);

        enum class ColorType { kUnknown, kOpaqueConstant, kConstant, kOpaque };
        enum class CoverageType { kNone, kSingleChannel, kLCD };

        bool fUsesPLSDstRead = false;
        bool fCompatibleWithCoverageAsAlpha = true;
        bool fUsesLocalCoords = false;
        CoverageType fCoverageType = CoverageType::kNone;
        ColorType fColorType = ColorType::kUnknown;
        int fInitialColorProcessorsToEliminate = 0;
        GrColor fOverrideInputColor;
        GrColor fKnownOutputColor;
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
