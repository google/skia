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
#include "GrPipelineAnalysis.h"
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
        return this->numFragmentProcessors() - fColorFragmentProcessorCnt;
    }
    int numFragmentProcessors() const {
        return fFragmentProcessors.count() - fFragmentProcessorOffset;
    }

    const GrFragmentProcessor* colorFragmentProcessor(int idx) const {
        SkASSERT(idx < fColorFragmentProcessorCnt);
        return fFragmentProcessors[idx + fFragmentProcessorOffset];
    }
    const GrFragmentProcessor* coverageFragmentProcessor(int idx) const {
        return fFragmentProcessors[idx + fColorFragmentProcessorCnt + fFragmentProcessorOffset];
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
                , fRequiresDstTexture(false)
                , fCanCombineOverlappedStencilAndCover(true)
                , fIgnoresInputColor(false)
                , fOutputCoverageType(static_cast<unsigned>(GrPipelineAnalysisCoverage::kNone))
                , fOutputColorType(static_cast<unsigned>(ColorType::kUnknown))
                , fInitialColorProcessorsToEliminate(0) {}

        // This version is used by a unit test that assumes no clip and no fragment processors.
        FragmentProcessorAnalysis(const GrPipelineAnalysisColor&, GrPipelineAnalysisCoverage,
                                  const GrXPFactory*, const GrCaps&);

        void init(const GrPipelineAnalysisColor&, GrPipelineAnalysisCoverage, const GrProcessorSet&,
                  const GrAppliedClip*, const GrCaps&);

        bool isInitializedWithProcessorSet() const { return fIsInitializedWithProcessorSet; }

        /**
         * If the return is greater than or equal to zero then 'newInputColor' should be used as the
         * input color to the GrPipeline derived from this processor set, replacing the GrDrawOp's
         * initial color. If the return is less than zero then newInputColor has not been
         * modified and no modification need be made to the pipeline's input color by the op.
         */
        int getInputColorOverrideAndColorProcessorEliminationCount(GrColor* newInputColor) const {
            if (fValidInputColor) {
                *newInputColor = fInputColor;
                return fInitialColorProcessorsToEliminate;
            }
            SkASSERT(!fInitialColorProcessorsToEliminate);
            return -1;
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
        bool requiresDstTexture() const { return fRequiresDstTexture; }
        bool canCombineOverlappedStencilAndCover() const {
            return fCanCombineOverlappedStencilAndCover;
        }
        bool isCompatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }
        bool isInputColorIgnored() const { return fIgnoresInputColor; }
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
        GrPipelineAnalysisCoverage outputCoverageType() const {
            return static_cast<GrPipelineAnalysisCoverage>(fOutputCoverageType);
        }
        bool hasCoverage() const {
            return this->outputCoverageType() != GrPipelineAnalysisCoverage::kNone;
        }

    private:
        enum class ColorType : unsigned { kUnknown, kOpaqueConstant, kConstant, kOpaque };

        ColorType outputColorType() const { return static_cast<ColorType>(fOutputColorType); }

        void internalInit(const GrPipelineAnalysisColor&, const GrPipelineAnalysisCoverage,
                          const GrProcessorSet&, const GrFragmentProcessor* clipFP, const GrCaps&);

        // MSVS 2015 won't pack a bool with an unsigned.
        using PackedBool = unsigned;

        PackedBool fIsInitializedWithProcessorSet : 1;
        PackedBool fUsesLocalCoords : 1;
        PackedBool fCompatibleWithCoverageAsAlpha : 1;
        PackedBool fValidInputColor : 1;
        PackedBool fRequiresDstTexture : 1;
        PackedBool fCanCombineOverlappedStencilAndCover : 1;
        // These could be removed if we created the XP from the XPFactory when doing analysis.
        PackedBool fIgnoresInputColor : 1;
        unsigned fOutputCoverageType : 2;
        unsigned fOutputColorType : 2;

        unsigned fInitialColorProcessorsToEliminate : 32 - 11;

        GrColor fInputColor;
        // This could be removed if we created the XP from the XPFactory when doing analysis.
        GrColor fKnownOutputColor;

        friend class GrProcessorSet;
    };
    GR_STATIC_ASSERT(sizeof(FragmentProcessorAnalysis) == 2 * sizeof(GrColor) + sizeof(uint32_t));

    void analyzeAndEliminateFragmentProcessors(FragmentProcessorAnalysis*,
                                               const GrPipelineAnalysisColor& colorInput,
                                               const GrPipelineAnalysisCoverage coverageInput,
                                               const GrAppliedClip*, const GrCaps&);

private:
    // This absurdly large limit allows FragmentProcessorAnalysis and this to pack fields together.
    static constexpr int kMaxColorProcessors = UINT8_MAX;

    enum Flags : uint16_t {
        kUseDistanceVectorField_Flag = 0x1,
        kDisableOutputConversionToSRGB_Flag = 0x2,
        kAllowSRGBInputs_Flag = 0x4,
        kPendingExecution_Flag = 0x8
    };

    const GrXPFactory* fXPFactory = nullptr;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    uint8_t fColorFragmentProcessorCnt;
    uint8_t fFragmentProcessorOffset = 0;
    uint8_t fFlags;
};

#endif
