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
        if (this->isPendingExecution()) {
            for (auto fp : fFragmentProcessors) {
                fp->completedExecution();
            }
        } else {
            for (auto fp : fFragmentProcessors) {
                fp->unref();
            }
        }
    }

    /**
     * If an op is recorded with this processor set then this must be called to ensure pending
     * reads and writes are propogated to resources referred to by the processors. Otherwise,
     * data hazards may occur.
     */
    void makePendingExecution();
    bool isPendingExecution() const { return SkToBool(kPendingExecution_Flag & fFlags); }

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

    bool operator==(const GrProcessorSet& that) const {
        if (fFlags != that.fFlags ||
            fFragmentProcessors.count() != that.fFragmentProcessors.count() ||
            fColorFragmentProcessorCnt != that.fColorFragmentProcessorCnt) {
            return false;
        }
        for (int i = 0; i < fFragmentProcessors.count(); ++i) {
            if (!fFragmentProcessors[i]->isEqual(*that.fFragmentProcessors[i])) {
                return false;
            }
        }
        if (fXPFactory != that.fXPFactory) {
            return false;
        }
        return true;
    }
    bool operator!=(const GrProcessorSet& that) const { return !(*this == that); }

    /**
     * This is used to track analysis of color and coverage values through the fragment processors.
     */
    class FragmentProcessorAnalysis {
    public:
        FragmentProcessorAnalysis(GrColor initialColor) : FragmentProcessorAnalysis() {
            fInputColor = initialColor;
            fValidInputColor = true;
        }

        FragmentProcessorAnalysis()
                : fIsInitializedWithProcessorSet(false)
                , fCompatibleWithCoverageAsAlpha(true)
                , fValidInputColor(false)
                , fOutputCoverageType(CoverageType::kNone)
                , fOutputColorType(ColorType::kUnknown)
                , fInitialColorProcessorsToEliminate(0) {}

        // This version is used by a unit test that assumes no clip, no processors, and no PLS.
        FragmentProcessorAnalysis(const GrPipelineInput& colorInput,
                                  const GrPipelineInput coverageInput, const GrCaps&);

        void init(const GrPipelineInput& colorInput, const GrPipelineInput coverageInput,
                  const GrProcessorSet&, const GrAppliedClip*, const GrCaps&);

        bool isInitializedWithProcessorSet() const { return fIsInitializedWithProcessorSet; }

        int initialColorProcessorsToEliminate(GrColor* newInputColor) const {
            if (fInitialColorProcessorsToEliminate > 0) {
                SkASSERT(fValidInputColor);
                *newInputColor = fInputColor;
            }
            return fInitialColorProcessorsToEliminate;
        }

        /**
         * Valid if initialProcessorsToEliminate returns true or this analysis was initialized with
         * a known color.
         */
        GrColor inputColor() const {
            SkASSERT(fValidInputColor);
            return fInputColor;
        }

        bool usesLocalCoords() const { return fUsesLocalCoords; }
        bool isCompatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }
        bool isOutputColorOpaque() const {
            return ColorType::kOpaque == fOutputColorType ||
                   ColorType::kOpaqueConstant == fOutputColorType;
        }
        bool hasKnownOutputColor(GrColor* color = nullptr) const {
            bool constant = ColorType::kConstant == fOutputColorType ||
                            ColorType::kOpaqueConstant == fOutputColorType;
            if (constant && color) {
                *color = fKnownOutputColor;
            }
            return constant;
        }
        bool hasCoverage() const { return CoverageType::kNone != fOutputCoverageType; }
        bool hasLCDCoverage() const { return CoverageType::kLCD == fOutputCoverageType; }

    private:
        void internalInit(const GrPipelineInput& colorInput, const GrPipelineInput coverageInput,
                          const GrProcessorSet&, const GrFragmentProcessor* clipFP, const GrCaps&);

        enum class ColorType : uint32_t { kUnknown, kOpaqueConstant, kConstant, kOpaque };
        enum class CoverageType : uint32_t { kNone, kSingleChannel, kLCD };

        bool fIsInitializedWithProcessorSet : 1;
        bool fUsesLocalCoords : 1;
        bool fCompatibleWithCoverageAsAlpha : 1;
        bool fValidInputColor : 1;
        CoverageType fOutputCoverageType : 2;
        ColorType fOutputColorType : 2;
        unsigned fInitialColorProcessorsToEliminate : 32 - 8;

        GrColor fInputColor;
        GrColor fKnownOutputColor;
    };
    GR_STATIC_ASSERT(sizeof(FragmentProcessorAnalysis) == 2 * sizeof(GrColor) + sizeof(uint32_t));

private:
    // This absurdly large limit allows FragmentProcessorAnalysis and this to pack fields together.
    static constexpr int kMaxColorProcessors = SK_MaxU16;

    enum Flags : uint16_t {
        kUseDistanceVectorField_Flag = 0x1,
        kDisableOutputConversionToSRGB_Flag = 0x2,
        kAllowSRGBInputs_Flag = 0x4,
        kPendingExecution_Flag  = 0x8
    };

    const GrXPFactory* fXPFactory = nullptr;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    uint16_t fColorFragmentProcessorCnt;
    uint16_t fFlags;
};

#endif
