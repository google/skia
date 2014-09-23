/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLFragmentShaderBuilder_DEFINED
#define GrGLFragmentShaderBuilder_DEFINED
#include "GrGLShaderBuilder.h"

class GrGLProgramBuilder;

/*
 * This base class encapsulates the functionality which all GrProcessors are allowed to use in their
 * fragment shader
 */
class GrGLProcessorFragmentShaderBuilder : public GrGLShaderBuilder {
public:
    GrGLProcessorFragmentShaderBuilder(GrGLProgramBuilder* program) : INHERITED(program) {}
    virtual ~GrGLProcessorFragmentShaderBuilder() {}
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
    typedef GrGLShaderBuilder INHERITED;
};

/*
 * Fragment processor's, in addition to all of the above, may need to use dst color so they use
 * this builder to create their shader
 */
class GrGLFragmentProcessorShaderBuilder : public GrGLProcessorFragmentShaderBuilder {
public:
    GrGLFragmentProcessorShaderBuilder(GrGLProgramBuilder* program) : INHERITED(program) {}
    /** Returns the variable name that holds the color of the destination pixel. This may be NULL if
        no effect advertised that it will read the destination. */
    virtual const char* dstColor() = 0;

private:
    typedef GrGLProcessorFragmentShaderBuilder INHERITED;
};

class GrGLFragmentShaderBuilder : public GrGLFragmentProcessorShaderBuilder {
public:
    typedef uint8_t DstReadKey;
    typedef uint8_t FragPosKey;

    /**  Returns a key for adding code to read the copy-of-dst color in service of effects that
        require reading the dst. It must not return 0 because 0 indicates that there is no dst
        copy read at all (in which case this function should not be called). */
    static DstReadKey KeyForDstRead(const GrTexture* dstCopy, const GrGLCaps&);

    /** Returns a key for reading the fragment location. This should only be called if there is an
       effect that will requires the fragment position. If the fragment position is not required,
       the key is 0. */
    static FragPosKey KeyForFragmentPosition(const GrRenderTarget* dst, const GrGLCaps&);

    GrGLFragmentShaderBuilder(GrGLProgramBuilder* program, const GrGLProgramDesc& desc);

    virtual const char* dstColor() SK_OVERRIDE;

    virtual bool enableFeature(GLSLFeature) SK_OVERRIDE;

    virtual SkString ensureFSCoords2D(const GrGLProcessor::TransformedCoordsArray& coords,
                                      int index) SK_OVERRIDE;

    virtual const char* fragmentPosition() SK_OVERRIDE;

private:
    /*
     * An internal call for GrGLFullProgramBuilder to use to add varyings to the vertex shader
     */
    void addVarying(GrSLType type,
                   const char* name,
                   const char** fsInName,
                   GrGLShaderVar::Precision fsPrecision = GrGLShaderVar::kDefault_Precision);

    /*
     * Private functions used by GrGLProgramBuilder for compilation
    */
    void bindProgramLocations(GrGLuint programId);
    bool compileAndAttachShaders(GrGLuint programId, SkTDArray<GrGLuint>* shaderIds) const;
    void emitCodeBeforeEffects();
    void emitCodeAfterEffects(const GrGLSLExpr4& inputColor, const GrGLSLExpr4& inputCoverage);

    /** Enables using the secondary color output and returns the name of the var in which it is
        to be stored */
    const char* enableSecondaryOutput();

    /** Gets the name of the primary color output. */
    const char* getColorOutputName() const;

    /**
     * Features that should only be enabled by GrGLFragmentShaderBuilder itself.
     */
    enum GLSLPrivateFeature {
        kFragCoordConventions_GLSLPrivateFeature = kLastGLSLFeature + 1,
        kLastGLSLPrivateFeature = kFragCoordConventions_GLSLPrivateFeature
    };

    // Interpretation of DstReadKey when generating code
    enum {
        kNoDstRead_DstReadKey           = 0,
        kYesDstRead_DstReadKeyBit       = 0x1, // Set if we do a dst-copy-read.
        kUseAlphaConfig_DstReadKeyBit   = 0x2, // Set if dst-copy config is alpha only.
        kTopLeftOrigin_DstReadKeyBit    = 0x4, // Set if dst-copy origin is top-left.
    };

    enum {
        kNoFragPosRead_FragPosKey           = 0,  // The fragment positition will not be needed.
        kTopLeftFragPosRead_FragPosKey      = 0x1,// Read frag pos relative to top-left.
        kBottomLeftFragPosRead_FragPosKey   = 0x2,// Read frag pos relative to bottom-left.
    };

    bool fHasCustomColorOutput;
    bool fHasSecondaryOutput;
    bool fSetupFragPosition;
    bool fTopLeftFragPosRead;

    friend class GrGLProgramBuilder;
    friend class GrGLFullProgramBuilder;

    typedef GrGLFragmentProcessorShaderBuilder INHERITED;
};

#endif
