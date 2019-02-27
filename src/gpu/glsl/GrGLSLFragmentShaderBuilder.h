/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLFragmentShaderBuilder_DEFINED
#define GrGLSLFragmentShaderBuilder_DEFINED

#include "GrBlend.h"
#include "GrGLSLShaderBuilder.h"
#include "GrProcessor.h"

class GrRenderTarget;
class GrGLSLVarying;

/*
 * This base class encapsulates the common functionality which all processors use to build fragment
 * shaders.
 */
class GrGLSLFragmentBuilder : public GrGLSLShaderBuilder {
public:
    GrGLSLFragmentBuilder(GrGLSLProgramBuilder* program) : INHERITED(program) {}
    virtual ~GrGLSLFragmentBuilder() {}

    /**
     * This returns a variable name to access the 2D, perspective correct version of the coords in
     * the fragment shader. The passed in coordinates must either be of type kHalf2 or kHalf3. If
     * the coordinates are 3-dimensional, it a perspective divide into is emitted into the
     * fragment shader (xy / z) to convert them to 2D.
     */
    virtual SkString ensureCoords2D(const GrShaderVar&) = 0;

    // TODO: remove this method.
    void declAppendf(const char* fmt, ...);

private:
    typedef GrGLSLShaderBuilder INHERITED;
};

/*
 * This class is used by fragment processors to build their fragment code.
 */
class GrGLSLFPFragmentBuilder : virtual public GrGLSLFragmentBuilder {
public:
    /** Appease the compiler; the derived class initializes GrGLSLFragmentBuilder. */
    GrGLSLFPFragmentBuilder() : GrGLSLFragmentBuilder(nullptr) {}

    enum class Scope : bool {
        kTopLevel,
        kInsideLoopOrBranch
    };

    /**
     * Subtracts multisample coverage by AND-ing the sample mask with the provided "mask".
     * Sample N corresponds to bit "1 << N".
     *
     * If the given scope is "kTopLevel" and the sample mask has not yet been modified, this method
     * assigns the sample mask in place rather than pre-initializing it to ~0 then AND-ing it.
     *
     * Requires MSAA and GLSL support for sample variables.
     */
    virtual void maskOffMultisampleCoverage(const char* mask, Scope) = 0;

    /**
     * Fragment procs with child procs should call these functions before/after calling emitCode
     * on a child proc.
     */
    virtual void onBeforeChildProcEmitCode() = 0;
    virtual void onAfterChildProcEmitCode() = 0;

    virtual const SkString& getMangleString() const = 0;

    virtual void forceHighPrecision() = 0;
};

/*
 * This class is used by Xfer processors to build their fragment code.
 */
class GrGLSLXPFragmentBuilder : virtual public GrGLSLFragmentBuilder {
public:
    /** Appease the compiler; the derived class initializes GrGLSLFragmentBuilder. */
    GrGLSLXPFragmentBuilder() : GrGLSLFragmentBuilder(nullptr) {}

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
   /** Returns a nonzero key for a surface's origin. This should only be called if a processor will
       use the fragment position and/or sample locations. */
    static uint8_t KeyForSurfaceOrigin(GrSurfaceOrigin);

    GrGLSLFragmentShaderBuilder(GrGLSLProgramBuilder* program);

    // Shared GrGLSLFragmentBuilder interface.
    virtual SkString ensureCoords2D(const GrShaderVar&) override;

    // GrGLSLFPFragmentBuilder interface.
    void maskOffMultisampleCoverage(const char* mask, Scope) override;
    const SkString& getMangleString() const override { return fMangleString; }
    void onBeforeChildProcEmitCode() override;
    void onAfterChildProcEmitCode() override;
    void forceHighPrecision() override { fForceHighPrecision = true; }

    // GrGLSLXPFragmentBuilder interface.
    bool hasCustomColorOutput() const override { return fHasCustomColorOutput; }
    bool hasSecondaryOutput() const override { return fHasSecondaryOutput; }
    const char* dstColor() override;
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
    bool hasReadDstColor() const { return fHasReadDstColor; }
    void resetVerification() {
        fHasReadDstColor = false;
    }
#endif

    static const char* DeclaredColorOutputName() { return "sk_FragColor"; }
    static const char* DeclaredSecondaryColorOutputName() { return "fsSecondaryColorOut"; }

    GrSurfaceOrigin getSurfaceOrigin() const;

    void onFinalize() override;

    static const char* kDstColorName;

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

    bool fSetupFragPosition;
    bool fHasCustomColorOutput;
    int fCustomColorOutputIndex;
    bool fHasSecondaryOutput;
    bool fHasInitializedSampleMask;
    bool fForceHighPrecision;

#ifdef SK_DEBUG
    // some state to verify shaders and effects are consistent, this is reset between effects by
    // the program creator
    bool fHasReadDstColor;
#endif

    friend class GrGLSLProgramBuilder;
    friend class GrGLProgramBuilder;
};

#endif
