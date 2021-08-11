/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLFragmentShaderBuilder_DEFINED
#define GrGLSLFragmentShaderBuilder_DEFINED

#include "src/gpu/GrBlend.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/glsl/GrGLSLShaderBuilder.h"

class GrRenderTarget;
class GrGLSLVarying;

/*
 * This class is used by fragment processors to build their fragment code.
 */
class GrGLSLFPFragmentBuilder : virtual public GrGLSLShaderBuilder {
public:
    /** Appease the compiler; the derived class initializes GrGLSLShaderBuilder. */
    GrGLSLFPFragmentBuilder() : GrGLSLShaderBuilder(nullptr) {
        // Suppress unused warning error
        (void) fPadding;
    }

    enum class ScopeFlags {
        // Every fragment will always execute this code, and will do it exactly once.
        kTopLevel = 0,
        // Either all fragments in a given primitive, or none, will execute this code.
        kInsidePerPrimitiveBranch = (1 << 0),
        // Any given fragment may or may not execute this code.
        kInsidePerPixelBranch = (1 << 1),
        // This code will be executed more than once.
        kInsideLoop = (1 << 2)
    };

    void writeProcessorFunction(GrFragmentProcessor::ProgramImpl*,
                                GrFragmentProcessor::ProgramImpl::EmitArgs&);

    virtual void forceHighPrecision() = 0;

    /** Returns the variable name that holds the color of the destination pixel. This may be nullptr
     * if no effect advertised that it will read the destination. */
    virtual const char* dstColor() = 0;

private:
    /**
     * These are called before/after calling emitCode on a child proc to update mangling.
     */
    virtual void onBeforeChildProcEmitCode() = 0;
    virtual void onAfterChildProcEmitCode() = 0;

    virtual const SkString& getMangleString() const = 0;

    // WARNING: LIke GrRenderTargetProxy, changes to this can cause issues in ASAN. This is caused
    // by GrGLSLProgramBuilder's GrTBlockLists requiring 16 byte alignment, but since
    // GrGLSLFragmentShaderBuilder has a virtual diamond hierarchy, ASAN requires all this pointers
    // to start aligned, even though clang is already correctly offsetting the individual fields
    // that require the larger alignment. In the current world, this extra padding is sufficient to
    // correctly initialize GrGLSLXPFragmentBuilder second.
    char fPadding[4] = {};
};

GR_MAKE_BITFIELD_CLASS_OPS(GrGLSLFPFragmentBuilder::ScopeFlags)

/*
 * This class is used by Xfer processors to build their fragment code.
 */
class GrGLSLXPFragmentBuilder : virtual public GrGLSLShaderBuilder {
public:
    /** Appease the compiler; the derived class initializes GrGLSLShaderBuilder. */
    GrGLSLXPFragmentBuilder() : GrGLSLShaderBuilder(nullptr) {}

    virtual bool hasCustomColorOutput() const = 0;
    virtual bool hasSecondaryOutput() const = 0;

    /** Returns the variable name that holds the color of the destination pixel. This may be nullptr
     * if no effect advertised that it will read the destination. */
    virtual const char* dstColor() = 0;

    /** Adds any necessary layout qualifiers in order to legalize the supplied blend equation with
        this shader. It is only legal to call this method with an advanced blend equation, and only
        if these equations are supported. */
    virtual void enableAdvancedBlendEquationIfNeeded(GrBlendEquation) = 0;
};

/*
 * This class implements the various fragment builder interfaces.
 */
class GrGLSLFragmentShaderBuilder : public GrGLSLFPFragmentBuilder, public GrGLSLXPFragmentBuilder {
public:
    GrGLSLFragmentShaderBuilder(GrGLSLProgramBuilder* program);

    // Shared FP/XP interface.
    const char* dstColor() override;

    // GrGLSLFPFragmentBuilder interface.
    void forceHighPrecision() override { fForceHighPrecision = true; }

    // GrGLSLXPFragmentBuilder interface.
    bool hasCustomColorOutput() const override { return SkToBool(fCustomColorOutput); }
    bool hasSecondaryOutput() const override { return fHasSecondaryOutput; }
    void enableAdvancedBlendEquationIfNeeded(GrBlendEquation) override;

private:
    using CustomFeatures = GrProcessor::CustomFeatures;

    // GrGLSLFPFragmentBuilder private interface.
    void onBeforeChildProcEmitCode() override;
    void onAfterChildProcEmitCode() override;
    const SkString& getMangleString() const override { return fMangleString; }

    // Private public interface, used by GrGLProgramBuilder to build a fragment shader
    void enableCustomOutput();
    void enableSecondaryOutput();
    const char* getPrimaryColorOutputName() const;
    const char* getSecondaryColorOutputName() const;
    bool primaryColorOutputIsInOut() const;

#ifdef SK_DEBUG
    // As GLSLProcessors emit code, there are some conditions we need to verify.  We use the below
    // state to track this.  The reset call is called per processor emitted.
    bool fHasReadDstColorThisStage_DebugOnly = false;
    CustomFeatures fUsedProcessorFeaturesThisStage_DebugOnly = CustomFeatures::kNone;
    CustomFeatures fUsedProcessorFeaturesAllStages_DebugOnly = CustomFeatures::kNone;

    void debugOnly_resetPerStageVerification() {
        fHasReadDstColorThisStage_DebugOnly = false;
        fUsedProcessorFeaturesThisStage_DebugOnly = CustomFeatures::kNone;
    }
#endif

    static const char* DeclaredColorOutputName() { return "sk_FragColor"; }
    static const char* DeclaredSecondaryColorOutputName() { return "fsSecondaryColorOut"; }

    GrSurfaceOrigin getSurfaceOrigin() const;

    void onFinalize() override;

    static constexpr const char kDstColorName[] = "_dstColor";

    /*
     * State that tracks which child proc in the proc tree is currently emitting code.  This is
     * used to update the fMangleString, which is used to mangle the names of uniforms and functions
     * emitted by the proc.  fSubstageIndices is a stack: its count indicates how many levels deep
     * we are in the tree, and its second-to-last value is the index of the child proc at that
     * level which is currently emitting code. For example, if fSubstageIndices = [3, 1, 2, 0], that
     * means we're currently emitting code for the base proc's 3rd child's 1st child's 2nd child.
     */
    SkTArray<int> fSubstageIndices;

    /*
     * The mangle string is used to mangle the names of uniforms/functions emitted by the child
     * procs so no duplicate uniforms/functions appear in the generated shader program. The mangle
     * string is simply based on fSubstageIndices. For example, if fSubstageIndices = [3, 1, 2, 0],
     * then the manglestring will be "_c3_c1_c2", and any uniform/function emitted by that proc will
     * have "_c3_c1_c2" appended to its name, which can be interpreted as "base proc's 3rd child's
     * 1st child's 2nd child".
     */
    SkString fMangleString;

    GrShaderVar* fCustomColorOutput = nullptr;

    bool fSetupFragPosition = false;
    bool fHasSecondaryOutput = false;
    bool fHasModifiedSampleMask = false;
    bool fForceHighPrecision = false;

    friend class GrGLSLProgramBuilder;
    friend class GrGLProgramBuilder;
    friend class GrVkPipelineStateBuilder;
};

#endif
