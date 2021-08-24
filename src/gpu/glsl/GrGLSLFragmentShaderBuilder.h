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

    virtual void forceHighPrecision() = 0;

    /** Returns the variable name that holds the color of the destination pixel. This may be nullptr
     * if no effect advertised that it will read the destination. */
    virtual const char* dstColor() = 0;

private:
    // WARNING: LIke GrRenderTargetProxy, changes to this can cause issues in ASAN. This is caused
    // by GrGLSLProgramBuilder's SkTBlockLists requiring 16 byte alignment, but since
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

    void debugOnly_resetPerStageVerification() {
        fHasReadDstColorThisStage_DebugOnly = false;
    }
#endif

    static const char* DeclaredColorOutputName() { return "sk_FragColor"; }
    static const char* DeclaredSecondaryColorOutputName() { return "fsSecondaryColorOut"; }

    GrSurfaceOrigin getSurfaceOrigin() const;

    void onFinalize() override;

    static constexpr const char kDstColorName[] = "_dstColor";

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
