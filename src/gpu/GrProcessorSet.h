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
class GrXferProcessor;
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

    // TODO: need this?
    const GrXPFactory* xpFactory() const { return fXP.factory(); }
    const GrXferProcessor* xferProcessor() const { return fXP.processor(); }

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
        Analysis(const Analysis&) = default;
        Analysis() { *reinterpret_cast<uint32_t*>(this) = 0; }

        bool isInitialized() const { return fIsInitialized; }
        bool usesLocalCoords() const { return fUsesLocalCoords; }
        bool requiresDstTexture() const { return fRequiresDstTexture; }
        bool canCombineOverlappedStencilAndCover() const {
            return fCanCombineOverlappedStencilAndCover;
        }
        bool requiresBarrierBetweenOverlappingDraws() const {
            return fRequiresBarrierBetweenOverlappingDraws;
        }
        bool isCompatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }

        enum class InputColorType : unsigned {
            /** The op's primitive processor should output the primitive's color as usual. */
            kOriginal,
            /**
             * The op's primitive processor must output the override color returned by
             * GrProcessorSet::finalize().
             */
            kOverriden,
            /** The primitive processor's output color will be ignored. */
            kIgnored
        };

        InputColorType inputColorType() const {
            return static_cast<InputColorType>(fInputColorType);
        }

    private:
        // MSVS 2015 won't pack different underlying types
        using PackedBool = uint32_t;
        using PackedInputColorType = uint32_t;

        PackedBool fUsesLocalCoords : 1;
        PackedBool fCompatibleWithCoverageAsAlpha : 1;
        PackedBool fRequiresDstTexture : 1;
        PackedBool fCanCombineOverlappedStencilAndCover : 1;
        PackedBool fRequiresBarrierBetweenOverlappingDraws : 1;
        PackedBool fIsInitialized : 1;
        PackedInputColorType fInputColorType : 2;

        friend class GrProcessorSet;
    };
    GR_STATIC_ASSERT(sizeof(Analysis) == sizeof(uint32_t));

    /**
     * This analyzes the processors given an op's input color and coverage as well as a clip. The
     * state of the processor set may change to an equivalent but more optimal set of processors.
     * This new state requires that the caller respect the returned 'inputColorOverride'. This is
     * indicated by the returned Analysis's inputColorType(). 'inputColorOverride' will not be
     * written if the analysis does not override the input color.
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
                      bool isMixedSamples, const GrCaps&, GrColor* inputColorOverride);

    bool isFinalized() const { return SkToBool(kFinalized_Flag & fFlags); }

private:

    // This absurdly large limit allows Analysis and this to pack fields together.
    static constexpr int kMaxColorProcessors = UINT8_MAX;

    enum Flags : uint16_t {
        kUseDistanceVectorField_Flag = 0x1,
        kDisableOutputConversionToSRGB_Flag = 0x2,
        kAllowSRGBInputs_Flag = 0x4,
        kFinalized_Flag = 0x8
    };

    class XferFactoryOrProcessor {
    public:
        XferFactoryOrProcessor(const GrXPFactory* xpFactory = nullptr) : fFactory(xpFactory) {
            SkDEBUGCODE(fMode= Mode::kFactory);
        }
        const GrXPFactory* factory() const {
            SkASSERT(Mode::kFactory == fMode);
            return fFactory;
        }
        const GrXferProcessor* processor() const {
            SkASSERT(Mode::kProcessor == fMode);
            return fProcessor;
        }
        void convertToProcessor(const GrCaps& caps, const GrProcessorAnalysisColor&,
                                GrProcessorAnalysisCoverage, bool hasMixedSamples);
    private:
        union {
            const GrXPFactory* fFactory;
            const GrXferProcessor* fProcessor;
        };
        SkDEBUGCODE(enum class Mode { kFactory, kProcessor } fMode;)
    };

    XferFactoryOrProcessor fXP;
    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    uint8_t fColorFragmentProcessorCnt;
    uint8_t fFragmentProcessorOffset = 0;
    uint8_t fFlags;
};

#endif
