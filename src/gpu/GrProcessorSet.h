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
#include "GrXferProcessor.h"

class GrAppliedClip;
class GrXPFactory;

class GrProcessorSet : private SkNoncopyable {
private:
    // Arbitrary constructor arg for empty set and analysis
    enum class Empty { kEmpty };

public:
    GrProcessorSet(GrPaint&& paint);
    GrProcessorSet(SkBlendMode mode);
    GrProcessorSet(sk_sp<GrFragmentProcessor> colorFP);

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

    const GrXferProcessor* xferProcessor() const {
        SkASSERT(this->isFinalized());
        return fXP.fProcessor;
    }
    sk_sp<const GrXferProcessor> refXferProcessor() const {
        SkASSERT(this->isFinalized());
        return sk_ref_sp(fXP.fProcessor);
    }

    /** Comparisons are only legal on finalized processor sets. */
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

        bool inputColorIsIgnored() const { return fInputColorType == kIgnored_InputColorType; }
        bool inputColorIsOverridden() const {
            return fInputColorType == kOverridden_InputColorType;
        }

    private:
        constexpr Analysis(Empty)
                : fUsesLocalCoords(false)
                , fCompatibleWithCoverageAsAlpha(true)
                , fRequiresDstTexture(false)
                , fCanCombineOverlappedStencilAndCover(true)
                , fRequiresBarrierBetweenOverlappingDraws(false)
                , fIsInitialized(true)
                , fInputColorType(kOriginal_InputColorType) {}
        enum InputColorType : uint32_t {
            kOriginal_InputColorType,
            kOverridden_InputColorType,
            kIgnored_InputColorType
        };

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
    GR_STATIC_ASSERT(sizeof(Analysis) <= sizeof(uint32_t));

    /**
     * This analyzes the processors given an op's input color and coverage as well as a clip. The
     * state of the processor set may change to an equivalent but more optimal set of processors.
     * This new state requires that the caller respect the returned 'inputColorOverride'. This is
     * indicated by the returned Analysis's inputColorIsOverriden(). 'inputColorOverride' will not
     * be written if the analysis does not override the input color.
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

    /** These are valid only for non-LCD coverage. */
    static const GrProcessorSet& EmptySet();
    static constexpr const Analysis EmptySetAnalysis() { return Analysis(Empty::kEmpty); }

    SkString dumpProcessors() const;

private:
    GrProcessorSet(Empty) : fXP((const GrXferProcessor*)nullptr), fFlags(kFinalized_Flag) {}

    // This absurdly large limit allows Analysis and this to pack fields together.
    static constexpr int kMaxColorProcessors = UINT8_MAX;

    enum Flags : uint16_t { kFinalized_Flag = 0x1 };

    union XP {
        XP(const GrXPFactory* factory) : fFactory(factory) {}
        XP(const GrXferProcessor* processor) : fProcessor(processor) {}
        const GrXPFactory* fFactory;
        const GrXferProcessor* fProcessor;
    };

    const GrXPFactory* xpFactory() const {
        SkASSERT(!this->isFinalized());
        return fXP.fFactory;
    }

    SkAutoSTArray<4, const GrFragmentProcessor*> fFragmentProcessors;
    XP fXP;
    uint8_t fColorFragmentProcessorCnt = 0;
    uint8_t fFragmentProcessorOffset = 0;
    uint8_t fFlags;
};

#endif
