/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLFragmentShaderBuilder_DEFINED
#define GrGLSLFragmentShaderBuilder_DEFINED

#include "GrGLSLShaderBuilder.h"

#include "glsl/GrGLSLProcessorTypes.h"

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
     * Use of these features may require a GLSL extension to be enabled. Shaders may not compile
     * if code is added that uses one of these features without calling enableFeature()
     */
    enum GLSLFeature {
        kStandardDerivatives_GLSLFeature = kLastGLSLPrivateFeature + 1,
        kPixelLocalStorage_GLSLFeature
    };

    /**
     * If the feature is supported then true is returned and any necessary #extension declarations
     * are added to the shaders. If the feature is not supported then false will be returned.
     */
    virtual bool enableFeature(GLSLFeature) = 0;

    /**
     * This returns a variable name to access the 2D, perspective correct version of the coords in
     * the fragment shader. If the coordinates at index are 3-dimensional, it immediately emits a
     * perspective divide into the fragment shader (xy / z) to convert them to 2D.
     */
    virtual SkString ensureFSCoords2D(const GrGLSLTransformedCoordsArray& coords, int index) = 0;


    /** Returns a variable name that represents the position of the fragment in the FS. The position
        is in device space (e.g. 0,0 is the top left and pixel centers are at half-integers). */
    virtual const char* fragmentPosition() = 0;

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

    /**
     * Subtracts sample coverage from the fragment. Any sample whose corresponding bit is not found
     * in the mask will not be written out to the framebuffer.
     *
     * @param mask      int that contains the sample mask. Bit N corresponds to the Nth sample.
     * @param invert    perform a bit-wise NOT on the provided mask before applying it?
     *
     * Requires GLSL support for sample variables.
     */
    virtual void maskSampleCoverage(const char* mask, bool invert = false) = 0;

    /**
     * Fragment procs with child procs should call these functions before/after calling emitCode
     * on a child proc.
     */
    virtual void onBeforeChildProcEmitCode() = 0;
    virtual void onAfterChildProcEmitCode() = 0;

    virtual const SkString& getMangleString() const = 0;
};

/*
 * This class is used by primitive processors to build their fragment code.
 */
class GrGLSLPPFragmentBuilder : public GrGLSLFPFragmentBuilder {
public:
    /** Appease the compiler; the derived class initializes GrGLSLFragmentBuilder. */
    GrGLSLPPFragmentBuilder() : GrGLSLFragmentBuilder(nullptr) {}

    /**
     * Overrides the fragment's sample coverage. The provided mask determines which samples will now
     * be written out to the framebuffer. Note that this mask can be reduced by a future call to
     * maskSampleCoverage.
     *
     * If a primitive processor uses this method, it must guarantee that every codepath through the
     * shader overrides the sample mask at some point.
     *
     * @param mask    int that contains the new coverage mask. Bit N corresponds to the Nth sample.
     *
     * Requires NV_sample_mask_override_coverage.
     */
    virtual void overrideSampleCoverage(const char* mask) = 0;
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
class GrGLSLFragmentShaderBuilder : public GrGLSLPPFragmentBuilder, public GrGLSLXPFragmentBuilder {
public:
    typedef uint8_t FragPosKey;

    /** Returns a key for reading the fragment location. This should only be called if there is an
       effect that will requires the fragment position. If the fragment position is not required,
       the key is 0. */
    static FragPosKey KeyForFragmentPosition(const GrRenderTarget* dst);

    GrGLSLFragmentShaderBuilder(GrGLSLProgramBuilder* program, uint8_t fragPosKey);

    // Shared GrGLSLFragmentBuilder interface.
    bool enableFeature(GLSLFeature) override;
    virtual SkString ensureFSCoords2D(const GrGLSLTransformedCoordsArray& coords,
                                      int index) override;
    const char* fragmentPosition() override;

    // GrGLSLFPFragmentBuilder interface.
    void maskSampleCoverage(const char* mask, bool invert = false) override;
    void overrideSampleCoverage(const char* mask) override;
    const SkString& getMangleString() const override { return fMangleString; }
    void onBeforeChildProcEmitCode() override;
    void onAfterChildProcEmitCode() override;

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

    // As GLSLProcessors emit code, there are some conditions we need to verify.  We use the below
    // state to track this.  The reset call is called per processor emitted.
    bool hasReadDstColor() const { return fHasReadDstColor; }
    bool hasReadFragmentPosition() const { return fHasReadFragmentPosition; }
    void reset() {
        fHasReadDstColor = false;
        fHasReadFragmentPosition = false;
    }

    static const char* DeclaredColorOutputName() { return "fsColorOut"; }
    static const char* DeclaredSecondaryColorOutputName() { return "fsSecondaryColorOut"; }

    /*
     * An internal call for GrGLProgramBuilder to use to add varyings to the vertex shader
     */
    void addVarying(GrGLSLVarying*, GrSLPrecision);

    void onFinalize() override;

    // Interpretation of FragPosKey when generating code
    enum {
        kNoFragPosRead_FragPosKey           = 0,  // The fragment positition will not be needed.
        kTopLeftFragPosRead_FragPosKey      = 0x1,// Read frag pos relative to top-left.
        kBottomLeftFragPosRead_FragPosKey   = 0x2,// Read frag pos relative to bottom-left.
    };

    static const char* kDstTextureColorName;

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
    bool fTopLeftFragPosRead;
    bool fHasCustomColorOutput;
    int  fCustomColorOutputIndex;
    bool fHasSecondaryOutput;
    bool fHasInitializedSampleMask;

    // some state to verify shaders and effects are consistent, this is reset between effects by
    // the program creator
    bool fHasReadDstColor;
    bool fHasReadFragmentPosition;

    friend class GrGLSLProgramBuilder;
    friend class GrGLProgramBuilder;
};

#endif
