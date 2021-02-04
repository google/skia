/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorSet_DEFINED
#define GrProcessorSet_DEFINED

#include "include/private/SkTemplates.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/GrXferProcessor.h"

struct GrUserStencilSettings;
class GrAppliedClip;
class GrXPFactory;

class GrProcessorSet {
private:
    // Arbitrary constructor arg for empty set and analysis
    enum class Empty { kEmpty };

public:
    GrProcessorSet(GrPaint&&);
    GrProcessorSet(SkBlendMode);
    GrProcessorSet(std::unique_ptr<GrFragmentProcessor> colorFP);
    GrProcessorSet(GrProcessorSet&&);
    GrProcessorSet(const GrProcessorSet&) = delete;
    GrProcessorSet& operator=(const GrProcessorSet&) = delete;

    ~GrProcessorSet();

    bool hasColorFragmentProcessor() const { return fColorFragmentProcessor != nullptr; }
    bool hasCoverageFragmentProcessor() const { return fCoverageFragmentProcessor != nullptr; }

    const GrFragmentProcessor* colorFragmentProcessor() const {
        return fColorFragmentProcessor.get();
    }
    const GrFragmentProcessor* coverageFragmentProcessor() const {
        return fCoverageFragmentProcessor.get();
    }

    bool usesVaryingCoords() const {
        return (fColorFragmentProcessor && fColorFragmentProcessor->usesVaryingCoords()) ||
               (fCoverageFragmentProcessor && fCoverageFragmentProcessor->usesVaryingCoords());
    }

    const GrXferProcessor* xferProcessor() const {
        SkASSERT(this->isFinalized());
        return fXP.fProcessor;
    }
    sk_sp<const GrXferProcessor> refXferProcessor() const {
        SkASSERT(this->isFinalized());
        return sk_ref_sp(fXP.fProcessor);
    }

    std::unique_ptr<GrFragmentProcessor> detachColorFragmentProcessor() {
        return std::move(fColorFragmentProcessor);
    }

    std::unique_ptr<GrFragmentProcessor> detachCoverageFragmentProcessor() {
        return std::move(fCoverageFragmentProcessor);
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
        bool requiresNonOverlappingDraws() const { return fRequiresNonOverlappingDraws; }
        bool isCompatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }
        // Indicates whether all color fragment processors were eliminated in the analysis.
        bool hasColorFragmentProcessor() const { return fHasColorFragmentProcessor; }

        bool inputColorIsIgnored() const { return fInputColorType == kIgnored_InputColorType; }
        bool inputColorIsOverridden() const {
            return fInputColorType == kOverridden_InputColorType;
        }
        bool usesNonCoherentHWBlending() const { return fUsesNonCoherentHWBlending; }
        bool unaffectedByDstValue() const { return fUnaffectedByDstValue; }

    private:
        constexpr Analysis(Empty)
                : fUsesLocalCoords(false)
                , fCompatibleWithCoverageAsAlpha(true)
                , fRequiresDstTexture(false)
                , fRequiresNonOverlappingDraws(false)
                , fHasColorFragmentProcessor(false)
                , fIsInitialized(true)
                , fUsesNonCoherentHWBlending(false)
                , fUnaffectedByDstValue(false)
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
        PackedBool fRequiresNonOverlappingDraws : 1;
        PackedBool fHasColorFragmentProcessor : 1;
        PackedBool fIsInitialized : 1;
        PackedBool fUsesNonCoherentHWBlending : 1;
        PackedBool fUnaffectedByDstValue : 1;
        PackedInputColorType fInputColorType : 2;

        friend class GrProcessorSet;
    };
    static_assert(sizeof(Analysis) <= sizeof(uint32_t));

    /**
     * This analyzes the processors given an op's input color and coverage as well as a clip. The
     * state of the processor set may change to an equivalent but more optimal set of processors.
     * This new state requires that the caller respect the returned 'inputColorOverride'. This is
     * indicated by the returned Analysis's inputColorIsOverridden(). 'inputColorOverride' will not
     * be written if the analysis does not override the input color.
     *
     * This must be called before the processor set is used to construct a GrPipeline and may only
     * be called once.
     *
     * This also puts the processors in "pending execution" state and must be called when an op
     * that owns a processor set is recorded to ensure pending and writes are propagated to
     * resources referred to by the processors. Otherwise, data hazards may occur.
     */
    Analysis finalize(
            const GrProcessorAnalysisColor&, const GrProcessorAnalysisCoverage,
            const GrAppliedClip*, const GrUserStencilSettings*, bool hasMixedSampledCoverage,
            const GrCaps&, GrClampType, SkPMColor4f* inputColorOverride);

    bool isFinalized() const { return SkToBool(kFinalized_Flag & fFlags); }

    /** These are valid only for non-LCD coverage. */
    static const GrProcessorSet& EmptySet();
    static GrProcessorSet MakeEmptySet();
    static constexpr Analysis EmptySetAnalysis() { return Analysis(Empty::kEmpty); }

#if GR_TEST_UTILS
    SkString dumpProcessors() const;
#endif

    void visitProxies(const GrOp::VisitProxyFunc& func) const;

private:
    GrProcessorSet(Empty) : fXP((const GrXferProcessor*)nullptr), fFlags(kFinalized_Flag) {}

    int numFragmentProcessors() const {
        return (fColorFragmentProcessor ? 1 : 0) + (fCoverageFragmentProcessor ? 1 : 0);
    }

    enum Flags : uint16_t { kFinalized_Flag = 0x1 };

    union XP {
        XP(const GrXPFactory* factory) : fFactory(factory) {}
        XP(const GrXferProcessor* processor) : fProcessor(processor) {}
        explicit XP(XP&& that) : fProcessor(that.fProcessor) {
            SkASSERT(fProcessor == that.fProcessor);
            that.fProcessor = nullptr;
        }
        const GrXPFactory* fFactory;
        const GrXferProcessor* fProcessor;
    };

    const GrXPFactory* xpFactory() const {
        SkASSERT(!this->isFinalized());
        return fXP.fFactory;
    }

    std::unique_ptr<GrFragmentProcessor> fColorFragmentProcessor;
    std::unique_ptr<GrFragmentProcessor> fCoverageFragmentProcessor;
    XP fXP;
    uint8_t fFlags = 0;
};

#endif
