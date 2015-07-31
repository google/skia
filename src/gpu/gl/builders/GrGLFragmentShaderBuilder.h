/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFragmentShaderBuilder_DEFINED
#define GrGLFragmentShaderBuilder_DEFINED

#include "GrGLShaderBuilder.h"

class GrGLVarying;

/*
 * This base class encapsulates the functionality which the GP uses to build fragment shaders
 */
class GrGLFragmentBuilder : public GrGLShaderBuilder {
public:
    GrGLFragmentBuilder(GrGLProgramBuilder* program) : INHERITED(program) {}
    virtual ~GrGLFragmentBuilder() {}
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
    virtual SkString ensureFSCoords2D(const GrGLProcessor::TransformedCoordsArray& coords,
                                      int index) = 0;


    /** Returns a variable name that represents the position of the fragment in the FS. The position
        is in device space (e.g. 0,0 is the top left and pixel centers are at half-integers). */
    virtual const char* fragmentPosition() = 0;

private:
    friend class GrGLPathProcessor;

    typedef GrGLShaderBuilder INHERITED;
};

/*
 * Fragment processor's, in addition to all of the above, may need to use dst color so they use
 * this builder to create their shader.  Because this is the only shader builder the FP sees, we
 * just call it FPShaderBuilder
 */
class GrGLXPFragmentBuilder : public GrGLFragmentBuilder {
public:
    GrGLXPFragmentBuilder(GrGLProgramBuilder* program) : INHERITED(program) {}

    /** Returns the variable name that holds the color of the destination pixel. This may be NULL if
        no effect advertised that it will read the destination. */
    virtual const char* dstColor() = 0;

    /** Adds any necessary layout qualifiers in order to legalize the supplied blend equation with
        this shader. It is only legal to call this method with an advanced blend equation, and only
        if these equations are supported. */
    virtual void enableAdvancedBlendEquationIfNeeded(GrBlendEquation) = 0;

private:
    typedef GrGLFragmentBuilder INHERITED;
};

// TODO rename to Fragment Builder
class GrGLFragmentShaderBuilder : public GrGLXPFragmentBuilder {
public:
    typedef uint8_t DstReadKey;
    typedef uint8_t FragPosKey;

    /** Returns a key for adding code to read the dst texture color in service of effects that
        require reading the dst. It must not return 0 because 0 indicates that there is no dst
        texture at all (in which case this function should not be called). */
    static DstReadKey KeyForDstRead(const GrTexture* dsttexture, const GrGLCaps&);

    /** Returns a key for reading the fragment location. This should only be called if there is an
       effect that will requires the fragment position. If the fragment position is not required,
       the key is 0. */
    static FragPosKey KeyForFragmentPosition(const GrRenderTarget* dst, const GrGLCaps&);

    GrGLFragmentShaderBuilder(GrGLProgramBuilder* program, uint8_t fragPosKey);

    // true public interface, defined explicitly in the abstract interfaces above
    bool enableFeature(GLSLFeature) override;
    virtual SkString ensureFSCoords2D(const GrGLProcessor::TransformedCoordsArray& coords,
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
    bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds);
    void bindFragmentShaderLocations(GrGLuint programID);

    // As GLProcessors emit code, there are some conditions we need to verify.  We use the below
    // state to track this.  The reset call is called per processor emitted.
    bool hasReadDstColor() const { return fHasReadDstColor; }
    bool hasReadFragmentPosition() const { return fHasReadFragmentPosition; }
    void reset() {
        fHasReadDstColor = false;
        fHasReadFragmentPosition = false;
    }

    /*
     * An internal call for GrGLProgramBuilder to use to add varyings to the vertex shader
     */
    void addVarying(GrGLVarying*, GrSLPrecision);

    /**
     * Features that should only be enabled by GrGLFragmentShaderBuilder itself.
     */
    enum GLSLPrivateFeature {
        kFragCoordConventions_GLSLPrivateFeature = kLastGLSLFeature + 1,
        kBlendEquationAdvanced_GLSLPrivateFeature,
        kBlendFuncExtended_GLSLPrivateFeature,
        kLastGLSLPrivateFeature = kBlendFuncExtended_GLSLPrivateFeature
    };

    // Interpretation of DstReadKey when generating code
    enum {
        kNoDstRead_DstReadKey           = 0,
        kYesDstRead_DstReadKeyBit       = 0x1, // Set if we do a dst-copy-read.
        kUseAlphaConfig_DstReadKeyBit   = 0x2, // Set if dst-copy config is alpha only.
        kTopLeftOrigin_DstReadKeyBit    = 0x4, // Set if dst-copy origin is top-left.
    };

    // Interpretation of FragPosKey when generating code
    enum {
        kNoFragPosRead_FragPosKey           = 0,  // The fragment positition will not be needed.
        kTopLeftFragPosRead_FragPosKey      = 0x1,// Read frag pos relative to top-left.
        kBottomLeftFragPosRead_FragPosKey   = 0x2,// Read frag pos relative to bottom-left.
    };

    static const char* kDstTextureColorName;

    bool fHasCustomColorOutput;
    bool fHasSecondaryOutput;
    bool fSetupFragPosition;
    bool fTopLeftFragPosRead;
    int  fCustomColorOutputIndex;

    // some state to verify shaders and effects are consistent, this is reset between effects by
    // the program creator
    bool fHasReadDstColor;
    bool fHasReadFragmentPosition;

    friend class GrGLProgramBuilder;

    typedef GrGLXPFragmentBuilder INHERITED;
};

#endif
