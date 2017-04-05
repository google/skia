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
#include "GrProcessorAnalysis.h"
#include "SkTemplates.h"

class GrAppliedClip;
class GrXPFactory;

class GrProcessorSet : private SkNoncopyable {
public:
    GrProcessorSet(GrPaint&& paint);

    ~GrProcessorSet();

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
     * This is used to report results of processor analysis when a processor set is finalized (see
     * below).
     */
    class Analysis {
    public:
        /**
         * This constructor allows an op to record its initial color in an Analysis member and then
         * then run analysis later when inputs to finalize() are available. If the analysis produces
         * color fragment processor elimination then the input color is replaced by the expected
         * input to the first non-eliminated processor. Otherwise, the original input color is
         * preserved. The only reason to use this is to save space on the op by not separately
         * storing the initial color.
         */
        explicit Analysis(GrColor initialColor) : Analysis() {
            fInputColor = initialColor;
            fValidInputColor = true;
        }

        Analysis()
                : fIsInitializedWithProcessorSet(false)
                , fCompatibleWithCoverageAsAlpha(true)
                , fValidInputColor(false)
                , fRequiresDstTexture(false)
                , fCanCombineOverlappedStencilAndCover(true)
                , fIgnoresInputColor(false)
                , fRequiresBarrierBetweenOverlappingDraws(false)
                , fOutputCoverageType(static_cast<unsigned>(GrProcessorAnalysisCoverage::kNone))
                , fOutputColorType(static_cast<unsigned>(ColorType::kUnknown)) {}

        bool isInitializedWithProcessorSet() const { return fIsInitializedWithProcessorSet; }

        /**
         * This returns the color that should be input to a pipeline created from the processor set.
         * This will be valid if the analysis was initialized with a known color or if the analysis
         * determined that the input color should be overridden.
         */
        int getInputColorIfValid(GrColor* newInputColor) const {
            if (fValidInputColor) {
                *newInputColor = fInputColor;
                return true;
            }
            return false;
        }

        /**
         * A unconditional version of the above that should only be called when the caller knows
         * that the analysis was initialized with a valid color.
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
        bool requiresBarrierBetweenOverlappingDraws() const {
            return fRequiresBarrierBetweenOverlappingDraws;
        }
        bool isCompatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }
        bool isInputColorIgnored() const { return fIgnoresInputColor; }
        GrProcessorAnalysisCoverage outputCoverage() const {
            return static_cast<GrProcessorAnalysisCoverage>(fOutputCoverageType);
        }
        GrProcessorAnalysisColor outputColor() const {
            switch (this->outputColorType()) {
                case ColorType::kConstant:
                    return fKnownOutputColor;
                case ColorType::kOpaque:
                    return GrProcessorAnalysisColor::Opaque::kYes;
                case ColorType::kUnknown:
                    return GrProcessorAnalysisColor::Opaque::kNo;
            }
            SkFAIL("Unexpected color type");
            return GrProcessorAnalysisColor::Opaque::kNo;
        }

    private:
        enum class ColorType : unsigned { kUnknown, kConstant, kOpaque };

        ColorType outputColorType() const { return static_cast<ColorType>(fOutputColorType); }

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
        PackedBool fRequiresBarrierBetweenOverlappingDraws : 1;
        unsigned fOutputCoverageType : 2;
        unsigned fOutputColorType : 2;

        GrColor fInputColor;
        // This could be removed if we created the XP from the XPFactory when doing analysis.
        GrColor fKnownOutputColor;

        friend class GrProcessorSet;
    };
    GR_STATIC_ASSERT(sizeof(Analysis) == 2 * sizeof(GrColor) + sizeof(uint32_t));

    /**
     * This analyzes the processors given an op's input color and coverage as well as a clip. The
     * state of the processor set may change to an equivalent but more optimal set of processors.
     * This new state requires that the caller respect the returned analysis's
     * getInputColorIfValid().
     *
     * This must be called before the processor set is used to construct a GrPipeline and may only
     * be called once.
     *
     * This also puts the processors in "pending execution" state and must be called when an op
     * that owns a processor set is recorded to ensure pending and writes are propagated to
     * resources referred to by the processors. Otherwise, data hazards may occur.
     */
    Analysis finalize(const GrProcessorAnalysisColor& colorInput,
                      const GrProcessorAnalysisCoverage coverageInput, const GrAppliedClip*,
                      const GrCaps&);

private:
    bool isFinalized() const { return SkToBool(kFinalized_Flag & fFlags); }

    // This absurdly large limit allows Analysis and this to pack fields together.
    static constexpr int kMaxColorProcessors = UINT8_MAX;

    enum Flags : uint16_t {
        kUseDistanceVectorField_Flag = 0x1,
        kDisableOutputConversionToSRGB_Flag = 0x2,
        kAllowSRGBInputs_Flag = 0x4,
        kFinalized_Flag = 0x8
    };

    const GrXPFactory* fXPFactory = nullptr;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    uint8_t fColorFragmentProcessorCnt;
    uint8_t fFragmentProcessorOffset = 0;
    uint8_t fFlags;
};

#endif
