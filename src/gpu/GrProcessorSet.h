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

    ~GrProcessorSet();

    /**
     * If an op is recorded with this processor set then this must be called to ensure pending
     * reads and writes are propagated to resources referred to by the processors. Otherwise,
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

    bool operator==(const GrProcessorSet& that) const;
    bool operator!=(const GrProcessorSet& that) const { return !(*this == that); }

    /**
     * This is used to track analysis of color and coverage values through the fragment processors.
     */
    class FragmentProcessorAnalysis {
    public:
        /**
         * This constructor allows an op to record its initial color in a FragmentProcessorAnalysis
         * member and then run analysis later when the analysis inputs are available. If the
         * analysis produces color fragment processor elimination then the input color is replaced
         * by the expected input to the first non-eliminated processor. Otherwise, the original
         * input color is preserved. The only reason to use this is to save space on the op by not
         * separately storing the initial color.
         */
        explicit FragmentProcessorAnalysis(GrColor initialColor) : FragmentProcessorAnalysis() {
            fInputColor = initialColor;
            fValidInputColor = true;
        }

        FragmentProcessorAnalysis()
                : fIsInitializedWithProcessorSet(false)
                , fCompatibleWithCoverageAsAlpha(true)
                , fValidInputColor(false)
                , fOutputCoverageType(static_cast<unsigned>(CoverageType::kNone))
                , fOutputColorType(static_cast<unsigned>(ColorType::kUnknown))
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
         * a known color via constructor or init(). If color fragment processors are eliminated then
         * this returns the expected input to the first non-eliminated processors. Otherwise it is
         * the color passed to the constructor or init().
         */
        GrColor inputColor() const {
            SkASSERT(fValidInputColor);
            return fInputColor;
        }

        bool usesLocalCoords() const { return fUsesLocalCoords; }
        bool isCompatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }
        bool isOutputColorOpaque() const {
            return ColorType::kOpaque == this->outputColorType() ||
                   ColorType::kOpaqueConstant == this->outputColorType();
        }
        bool hasKnownOutputColor(GrColor* color = nullptr) const {
            bool constant = ColorType::kConstant == this->outputColorType() ||
                            ColorType::kOpaqueConstant == this->outputColorType();
            if (constant && color) {
                *color = fKnownOutputColor;
            }
            return constant;
        }
        bool hasCoverage() const { return CoverageType::kNone != this->outputCoverageType(); }
        bool hasLCDCoverage() const { return CoverageType::kLCD == this->outputCoverageType(); }

    private:
        enum class ColorType : unsigned { kUnknown, kOpaqueConstant, kConstant, kOpaque };
        enum class CoverageType : unsigned { kNone, kSingleChannel, kLCD };

        CoverageType outputCoverageType() const {
            return static_cast<CoverageType>(fOutputCoverageType);
        }
        ColorType outputColorType() const { return static_cast<ColorType>(fOutputColorType); }

        void internalInit(const GrPipelineInput& colorInput, const GrPipelineInput coverageInput,
                          const GrProcessorSet&, const GrFragmentProcessor* clipFP, const GrCaps&);

        // MSVS 2015 won't pack a bool with an unsigned.
        using PackedBool = unsigned;

        PackedBool fIsInitializedWithProcessorSet : 1;
        PackedBool fUsesLocalCoords : 1;
        PackedBool fCompatibleWithCoverageAsAlpha : 1;
        PackedBool fValidInputColor : 1;
        unsigned fOutputCoverageType : 2;
        unsigned fOutputColorType : 2;
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
        kPendingExecution_Flag = 0x8
    };

    const GrXPFactory* fXPFactory = nullptr;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    uint16_t fColorFragmentProcessorCnt;
    uint16_t fFlags;
};

#endif
