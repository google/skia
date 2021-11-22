/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrXferProcessor_DEFINED
#define GrXferProcessor_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/gpu/GrBlend.h"
#include "src/gpu/GrNonAtomicRef.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrGLSLXPFragmentBuilder;
class GrGLSLProgramDataManager;
struct GrShaderCaps;

/**
 * Barriers for blending. When a shader reads the dst directly, an Xfer barrier is sometimes
 * required after a pixel has been written, before it can be safely read again.
 */
enum GrXferBarrierType {
    kNone_GrXferBarrierType = 0, //<! No barrier is required
    kTexture_GrXferBarrierType,  //<! Required when a shader reads and renders to the same texture.
    kBlend_GrXferBarrierType,    //<! Required by certain blend extensions.
};
/** Should be able to treat kNone as false in boolean expressions */
static_assert(SkToBool(kNone_GrXferBarrierType) == false);

// Flag version of the above enum.
enum class GrXferBarrierFlags {
    kNone    = 0,
    kTexture = 1 << 0,
    kBlend   = 1 << 1,
};

GR_MAKE_BITFIELD_CLASS_OPS(GrXferBarrierFlags)

/**
 * GrXferProcessor is responsible for implementing the xfer mode that blends the src color and dst
 * color, and for applying any coverage. It does this by emitting fragment shader code and
 * controlling the fixed-function blend state. When dual-source blending is available, it may also
 * write a seconday fragment shader output color. GrXferProcessor has two modes of operation:
 *
 * Dst read: When allowed by the backend API, or when supplied a texture of the destination, the
 * GrXferProcessor may read the destination color. While operating in this mode, the subclass only
 * provides shader code that blends the src and dst colors, and the base class applies coverage.
 *
 * No dst read: When not performing a dst read, the subclass is given full control of the fixed-
 * function blend state and/or secondary output, and is responsible to apply coverage on its own.
 *
 * A GrXferProcessor is never installed directly into our draw state, but instead is created from a
 * GrXPFactory once we have finalized the state of our draw.
 */
class GrXferProcessor : public GrProcessor, public GrNonAtomicRef<GrXferProcessor> {
public:
    /**
     * Every GrXferProcessor must be capable of creating a subclass of ProgramImpl. The ProgramImpl
     * emits the shader code combines determines the fragment shader output(s) from the color and
     * coverage FP outputs, is attached to the generated backend API pipeline/program, and used to
     * extract uniform data from GrXferProcessor instances.
     */
    class ProgramImpl;

    /**
     * Adds a key on the GrProcessorKeyBuilder calls onAddToKey(...) to get the specific subclass's
     * key.
     */
    void addToKey(const GrShaderCaps&,
                  GrProcessorKeyBuilder*,
                  const GrSurfaceOrigin* originIfDstTexture,
                  bool usesInputAttachmentForDstRead) const;

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrXferProcessor; caller is responsible for deleting
        the object. */
    virtual std::unique_ptr<ProgramImpl> makeProgramImpl() const = 0;

    /**
     * Returns the barrier type, if any, that this XP will require. Note that the possibility
     * that a kTexture type barrier is required is handled by the GrPipeline and need not be
     * considered by subclass overrides of this function.
     */
    virtual GrXferBarrierType xferBarrierType(const GrCaps& caps) const {
        return kNone_GrXferBarrierType;
    }

    struct BlendInfo {
        SkDEBUGCODE(SkString dump() const;)

        GrBlendEquation fEquation = kAdd_GrBlendEquation;
        GrBlendCoeff    fSrcBlend = kOne_GrBlendCoeff;
        GrBlendCoeff    fDstBlend = kZero_GrBlendCoeff;
        SkPMColor4f     fBlendConstant = SK_PMColor4fTRANSPARENT;
        bool            fWriteColor = true;
    };

    inline BlendInfo getBlendInfo() const {
        BlendInfo blendInfo;
        if (!this->willReadDstColor()) {
            this->onGetBlendInfo(&blendInfo);
        }
        return blendInfo;
    }

    bool willReadDstColor() const { return fWillReadDstColor; }

    /**
     * Returns whether or not this xferProcossor will set a secondary output to be used with dual
     * source blending.
     */
    bool hasSecondaryOutput() const;

    bool isLCD() const { return fIsLCD; }

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two processor are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the processor would
        generate the same shader code. To test for identical code generation use addToKey.
      */

    bool isEqual(const GrXferProcessor& that) const {
        if (this->classID() != that.classID()) {
            return false;
        }
        if (this->fWillReadDstColor != that.fWillReadDstColor) {
            return false;
        }
        if (fIsLCD != that.fIsLCD) {
            return false;
        }
        return this->onIsEqual(that);
    }

protected:
    GrXferProcessor(ClassID classID);
    GrXferProcessor(ClassID classID, bool willReadDstColor, GrProcessorAnalysisCoverage);

private:
    /**
     * Adds a key on the GrProcessorKeyBuilder that reflects any variety in the code that may be
     * emitted by the xfer processor subclass.
     */
    virtual void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const = 0;

    /**
     * If we are not performing a dst read, returns whether the subclass will set a secondary
     * output. When using dst reads, the base class controls the secondary output and this method
     * will not be called.
     */
    virtual bool onHasSecondaryOutput() const { return false; }

    /**
     * If we are not performing a dst read, retrieves the fixed-function blend state required by the
     * subclass. When using dst reads, the base class controls the fixed-function blend state and
     * this method will not be called. The BlendInfo struct comes initialized to "no blending".
     */
    virtual void onGetBlendInfo(BlendInfo*) const {}

    virtual bool onIsEqual(const GrXferProcessor&) const = 0;

    bool fWillReadDstColor;
    bool fIsLCD;

    using INHERITED = GrProcessor;
};

/**
 * We install a GrXPFactory (XPF) early on in the pipeline before all the final draw information is
 * known (e.g. whether there is fractional pixel coverage, will coverage be 1 or 4 channel, is the
 * draw opaque, etc.). Once the state of the draw is finalized, we use the XPF along with all the
 * draw information to create a GrXferProcessor (XP) which can implement the desired blending for
 * the draw.
 *
 * Before the XP is created, the XPF is able to answer queries about what functionality the XPs it
 * creates will have. For example, can it create an XP that supports RGB coverage or will the XP
 * blend with the destination color.
 *
 * GrXPFactories are intended to be static immutable objects. We pass them around as raw pointers
 * and expect the pointers to always be valid and for the factories to be reusable and thread safe.
 * Equality is tested for using pointer comparison. GrXPFactory destructors must be no-ops.
 */

// In order to construct GrXPFactory subclass instances as constexpr the subclass, and therefore
// GrXPFactory, must be a literal type. One requirement is having a trivial destructor. This is ok
// since these objects have no need for destructors. However, GCC and clang throw a warning when a
// class has virtual functions and a non-virtual destructor. We suppress that warning here and
// for the subclasses.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
class GrXPFactory {
public:
    enum class AnalysisProperties : unsigned {
        kNone = 0x0,
        /**
         * The fragment shader will require the destination color.
         */
        kReadsDstInShader = 0x1,
        /**
         * The op may apply coverage as alpha and still blend correctly.
         */
        kCompatibleWithCoverageAsAlpha = 0x2,
        /**
         * The color input to the GrXferProcessor will be ignored.
         */
        kIgnoresInputColor = 0x4,
        /**
         * The destination color will be provided to the fragment processor using a texture. This is
         * additional information about the implementation of kReadsDstInShader.
         */
        kRequiresDstTexture = 0x10,
        /**
         * If set, each pixel can only be touched once during a draw (e.g., because we have a dst
         * texture or because we need an xfer barrier).
         */
        kRequiresNonOverlappingDraws = 0x20,
        /**
         * If set the draw will use fixed function non coherent advanced blends.
         */
        kUsesNonCoherentHWBlending = 0x40,
        /**
         * If set, the existing dst value has no effect on the final output.
         */
        kUnaffectedByDstValue = 0x80,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(AnalysisProperties);

    static sk_sp<const GrXferProcessor> MakeXferProcessor(const GrXPFactory*,
                                                          const GrProcessorAnalysisColor&,
                                                          GrProcessorAnalysisCoverage,
                                                          const GrCaps& caps,
                                                          GrClampType);

    static AnalysisProperties GetAnalysisProperties(const GrXPFactory*,
                                                    const GrProcessorAnalysisColor&,
                                                    const GrProcessorAnalysisCoverage&,
                                                    const GrCaps&,
                                                    GrClampType);

protected:
    constexpr GrXPFactory() {}

private:
    virtual sk_sp<const GrXferProcessor> makeXferProcessor(const GrProcessorAnalysisColor&,
                                                           GrProcessorAnalysisCoverage,
                                                           const GrCaps&,
                                                           GrClampType) const = 0;

    /**
     * Subclass analysis implementation. This should not return kNeedsDstInTexture as that will be
     * inferred by the base class based on kReadsDstInShader and the caps.
     */
    virtual AnalysisProperties analysisProperties(const GrProcessorAnalysisColor&,
                                                  const GrProcessorAnalysisCoverage&,
                                                  const GrCaps&,
                                                  GrClampType) const = 0;
};
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

GR_MAKE_BITFIELD_CLASS_OPS(GrXPFactory::AnalysisProperties)

//////////////////////////////////////////////////////////////////////////////

class GrXferProcessor::ProgramImpl {
public:
    virtual ~ProgramImpl() = default;

    using SamplerHandle = GrGLSLUniformHandler::SamplerHandle;

    struct EmitArgs {
        EmitArgs(GrGLSLXPFragmentBuilder* fragBuilder,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrXferProcessor& xp,
                 const char* inputColor,
                 const char* inputCoverage,
                 const char* outputPrimary,
                 const char* outputSecondary,
                 const SamplerHandle dstTextureSamplerHandle,
                 GrSurfaceOrigin dstTextureOrigin,
                 const GrSwizzle& writeSwizzle)
                : fXPFragBuilder(fragBuilder)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fXP(xp)
                , fInputColor(inputColor ? inputColor : "half4(1.0)")
                , fInputCoverage(inputCoverage)
                , fOutputPrimary(outputPrimary)
                , fOutputSecondary(outputSecondary)
                , fDstTextureSamplerHandle(dstTextureSamplerHandle)
                , fDstTextureOrigin(dstTextureOrigin)
                , fWriteSwizzle(writeSwizzle) {}
        GrGLSLXPFragmentBuilder* fXPFragBuilder;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrXferProcessor& fXP;
        const char* fInputColor;
        const char* fInputCoverage;
        const char* fOutputPrimary;
        const char* fOutputSecondary;
        const SamplerHandle fDstTextureSamplerHandle;
        GrSurfaceOrigin fDstTextureOrigin;
        GrSwizzle fWriteSwizzle;
    };
    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    void emitCode(const EmitArgs&);

    /** A ProgramImpl instance can be reused with any GrXferProcessor that produces the same key.
        This function reads data from a GrXferProcessor and uploads any uniform variables required
        by the shaders created in emitCode(). The GrXferProcessor parameter is guaranteed to be of
        the same type that created this ProgramImpl and to have an identical processor key as the
        one that created this ProgramImpl. This function calls onSetData on the subclass of
        ProgramImpl.
     */
    void setData(const GrGLSLProgramDataManager& pdm, const GrXferProcessor& xp);

protected:
    ProgramImpl() = default;

    static void DefaultCoverageModulation(GrGLSLXPFragmentBuilder* fragBuilder,
                                          const char* srcCoverage,
                                          const char* dstColor,
                                          const char* outColor,
                                          const char* outColorSecondary,
                                          const GrXferProcessor& proc);

private:
    /**
     * Called by emitCode() when the XP will not be performing a dst read. This method is
     * responsible for both blending and coverage. A subclass only needs to implement this method if
     * it can construct a GrXferProcessor that will not read the dst color.
     */
    virtual void emitOutputsForBlendState(const EmitArgs&) {
        SK_ABORT("emitOutputsForBlendState not implemented.");
    }

    /**
     * Called by emitCode() when the XP will perform a dst read. This method only needs to supply
     * the blending logic. The base class applies coverage. A subclass only needs to implement this
     * method if it can construct a GrXferProcessor that reads the dst color.
     */
    virtual void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder*,
                                         GrGLSLUniformHandler*,
                                         const char* srcColor,
                                         const char* srcCoverage,
                                         const char* dstColor,
                                         const char* outColor,
                                         const char* outColorSecondary,
                                         const GrXferProcessor&) {
        SK_ABORT("emitBlendCodeForDstRead not implemented.");
    }

    virtual void emitWriteSwizzle(GrGLSLXPFragmentBuilder*,
                                  const GrSwizzle&,
                                  const char* outColor,
                                  const char* outColorSecondary) const;

    virtual void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) {}
};

#endif
