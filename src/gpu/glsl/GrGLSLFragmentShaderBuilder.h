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
 * This base class encapsulates the functionality which the GP uses to build fragment shaders
 */
class GrGLSLFragmentBuilder : public GrGLSLShaderBuilder {
public:
    GrGLSLFragmentBuilder(GrGLSLProgramBuilder* program)
        : INHERITED(program)
        , fHasCustomColorOutput(false)
        , fHasSecondaryOutput(false) {
        fSubstageIndices.push_back(0);
    }
    virtual ~GrGLSLFragmentBuilder() {}
    /**
     * Use of these features may require a GLSL extension to be enabled. Shaders may not compile
     * if code is added that uses one of these features without calling enableFeature()
     */
    enum GLSLFeature {
        kStandardDerivatives_GLSLFeature = 0,
        kLastGLSLFeature = kStandardDerivatives_GLSLFeature
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

    /**
     * Fragment procs with child procs should call these functions before/after calling emitCode
     * on a child proc.
     */
    void onBeforeChildProcEmitCode();
    void onAfterChildProcEmitCode();

    const SkString& getMangleString() const { return fMangleString; }

    bool hasCustomColorOutput() const { return fHasCustomColorOutput; }
    bool hasSecondaryOutput() const { return fHasSecondaryOutput; }

protected:
    bool fHasCustomColorOutput;
    bool fHasSecondaryOutput;

private:
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

    friend class GrGLPathProcessor;

    typedef GrGLSLShaderBuilder INHERITED;
};

/*
 * Fragment processor's, in addition to all of the above, may need to use dst color so they use
 * this builder to create their shader.  Because this is the only shader builder the FP sees, we
 * just call it FPShaderBuilder
 */
class GrGLSLXPFragmentBuilder : public GrGLSLFragmentBuilder {
public:
    GrGLSLXPFragmentBuilder(GrGLSLProgramBuilder* program) : INHERITED(program) {}

    /** Returns the variable name that holds the color of the destination pixel. This may be nullptr if
        no effect advertised that it will read the destination. */
    virtual const char* dstColor() = 0;

    /** Adds any necessary layout qualifiers in order to legalize the supplied blend equation with
        this shader. It is only legal to call this method with an advanced blend equation, and only
        if these equations are supported. */
    virtual void enableAdvancedBlendEquationIfNeeded(GrBlendEquation) = 0;

private:
    typedef GrGLSLFragmentBuilder INHERITED;
};

// TODO rename to Fragment Builder
class GrGLSLFragmentShaderBuilder : public GrGLSLXPFragmentBuilder {
public:
    typedef uint8_t FragPosKey;

    /** Returns a key for reading the fragment location. This should only be called if there is an
       effect that will requires the fragment position. If the fragment position is not required,
       the key is 0. */
    static FragPosKey KeyForFragmentPosition(const GrRenderTarget* dst);

    GrGLSLFragmentShaderBuilder(GrGLSLProgramBuilder* program, uint8_t fragPosKey);

    // true public interface, defined explicitly in the abstract interfaces above
    bool enableFeature(GLSLFeature) override;
    virtual SkString ensureFSCoords2D(const GrGLSLTransformedCoordsArray& coords,
                                      int index) override;
    const char* fragmentPosition() override;
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

    /**
     * Features that should only be enabled by GrGLSLFragmentShaderBuilder itself.
     */
    enum GLSLPrivateFeature {
        kFragCoordConventions_GLSLPrivateFeature = kLastGLSLFeature + 1,
        kBlendEquationAdvanced_GLSLPrivateFeature,
        kBlendFuncExtended_GLSLPrivateFeature,
        kExternalTexture_GLSLPrivateFeature,
        kLastGLSLPrivateFeature = kBlendFuncExtended_GLSLPrivateFeature
    };

    // Interpretation of FragPosKey when generating code
    enum {
        kNoFragPosRead_FragPosKey           = 0,  // The fragment positition will not be needed.
        kTopLeftFragPosRead_FragPosKey      = 0x1,// Read frag pos relative to top-left.
        kBottomLeftFragPosRead_FragPosKey   = 0x2,// Read frag pos relative to bottom-left.
    };

    static const char* kDstTextureColorName;

    bool fSetupFragPosition;
    bool fTopLeftFragPosRead;
    int  fCustomColorOutputIndex;

    // some state to verify shaders and effects are consistent, this is reset between effects by
    // the program creator
    bool fHasReadDstColor;
    bool fHasReadFragmentPosition;

    friend class GrGLSLProgramBuilder;
    friend class GrGLProgramBuilder;

    typedef GrGLSLXPFragmentBuilder INHERITED;
};

#endif
