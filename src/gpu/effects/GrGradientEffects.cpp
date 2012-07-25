/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGradientEffects.h"
#include "gl/GrGLProgramStage.h"
#include "GrProgramStageFactory.h"
#include "SkGr.h"
#include "../core/SkShader.h"

// Base class for GL gradient custom stages
class GrGLGradientStage : public GrGLProgramStage {
public:

    GrGLGradientStage(const GrProgramStageFactory& factory);
    virtual ~GrGLGradientStage();

    // emit code that gets a fragment's color from an expression for t; for now
    // this always uses the texture, but for simpler cases we'll be able to lerp
    void emitColorLookup(GrGLShaderBuilder* builder, const char* t, 
                         const char* outputColor, const char* samplerName);

private:

    typedef GrGLProgramStage INHERITED;
};

GrGLGradientStage::GrGLGradientStage(const GrProgramStageFactory& factory)
                                     : INHERITED(factory) { }

GrGLGradientStage::~GrGLGradientStage() { }

void GrGLGradientStage::emitColorLookup(GrGLShaderBuilder* builder, 
                                        const char* tName, 
                                        const char* outputColor,
                                        const char* samplerName) {
    // Texture is effectively 1D so the y coordinate is 0.5, if we pack multiple
    // gradients into a texture, we could instead pick the appropriate row here
    builder->fSampleCoords.printf("vec2(%s, 0.5)", tName);
    builder->fComplexCoord = true;
    builder->emitDefaultFetch(outputColor, samplerName);
}

/////////////////////////////////////////////////////////////////////

GrGradientEffect::GrGradientEffect(GrTexture* texture) 
                                   : fTexture (texture)
                                   , fUseTexture(true) {
    SkSafeRef(fTexture);
}

GrGradientEffect::GrGradientEffect(GrContext* ctx, 
                                   const SkShader& shader,
                                   GrSamplerState* sampler)
                                   : fTexture (NULL)
                                   , fUseTexture (false) {
    // TODO: check for simple cases where we don't need a texture:
    //GradientInfo info;
    //shader.asAGradient(&info);
    //if (info.fColorCount == 2) { ...

    SkBitmap bitmap;
    shader.asABitmap(&bitmap, NULL, NULL);

    GrContext::TextureCacheEntry entry = GrLockCachedBitmapTexture(ctx, bitmap,
                                                                   sampler);
    fTexture = entry.texture();
    SkSafeRef(fTexture);
    fUseTexture = true;

    // Unlock immediately, this is not great, but we don't have a way of
    // knowing when else to unlock it currently, so it may get purged from
    // the cache, but it'll still be ref'd until it's no longer being used.
    GrUnlockCachedBitmapTexture(ctx, entry);
}

GrGradientEffect::~GrGradientEffect() {
    SkSafeUnref(fTexture);
}

unsigned int GrGradientEffect::numTextures() const {
    return fUseTexture ? 1 : 0;
}

GrTexture* GrGradientEffect::texture(unsigned int index) 
                             const {
    GrAssert(fUseTexture && 0 == index);
    return fTexture;
}

/////////////////////////////////////////////////////////////////////

class GrGLLinearGradient : public GrGLGradientStage {
public:

    GrGLLinearGradient(const GrProgramStageFactory& factory,
                       const GrCustomStage&)
                       : INHERITED (factory) { }

    virtual ~GrGLLinearGradient() { }

    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;
    static StageKey GenKey(const GrCustomStage& s) { return 0; }

private:

    typedef GrGLGradientStage INHERITED;
};

void GrGLLinearGradient::emitFS(GrGLShaderBuilder* builder,
                                const char* outputColor,
                                const char* inputColor,
                                const char* samplerName) {
    SkString t;
    t.printf("%s.x", builder->fSampleCoords.c_str());
    this->emitColorLookup(builder, t.c_str(), outputColor, samplerName);
}

/////////////////////////////////////////////////////////////////////

GrLinearGradient::GrLinearGradient(GrTexture* texture)
                  : INHERITED(texture) { 
}

GrLinearGradient::GrLinearGradient(GrContext* ctx, 
                                   const SkShader& shader,
                                   GrSamplerState* sampler)
                                   : INHERITED(ctx, shader, sampler) {
}

GrLinearGradient::~GrLinearGradient() {

}

const GrProgramStageFactory& GrLinearGradient::getFactory() const {
    return GrTProgramStageFactory<GrLinearGradient>::getInstance();
}

/////////////////////////////////////////////////////////////////////

class GrGLRadialGradient : public GrGLGradientStage {

public:

    GrGLRadialGradient(const GrProgramStageFactory& factory,
                       const GrCustomStage&) : INHERITED (factory) { }
    virtual ~GrGLRadialGradient() { }

    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s) { return 0; }

private:

    typedef GrGLGradientStage INHERITED;

};

void GrGLRadialGradient::emitFS(GrGLShaderBuilder* builder,
                                const char* outputColor,
                                const char* inputColor,
                                const char* samplerName) {
    SkString t;
    t.printf("length(%s.xy)", builder->fSampleCoords.c_str());
    this->emitColorLookup(builder, t.c_str(), outputColor, samplerName);
}


/////////////////////////////////////////////////////////////////////


GrRadialGradient::GrRadialGradient(GrTexture* texture)
    : INHERITED(texture) {

}

GrRadialGradient::GrRadialGradient(GrContext* ctx, const SkShader& shader,
                                   GrSamplerState* sampler)
                                   : INHERITED(ctx, shader, sampler) {
}

GrRadialGradient::~GrRadialGradient() {

}

const GrProgramStageFactory& GrRadialGradient::getFactory() const {
    return GrTProgramStageFactory<GrRadialGradient>::getInstance();
}

/////////////////////////////////////////////////////////////////////

// For brevity
typedef GrGLUniformManager::UniformHandle UniformHandle;
static const UniformHandle kInvalidUniformHandle = GrGLUniformManager::kInvalidUniformHandle;

class GrGLRadial2Gradient : public GrGLGradientStage {

public:

    GrGLRadial2Gradient(const GrProgramStageFactory& factory,
                        const GrCustomStage&);
    virtual ~GrGLRadial2Gradient() { }

    virtual void setupVariables(GrGLShaderBuilder* builder,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;
    virtual void setData(const GrGLUniformManager&,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s) {
        return (static_cast<const GrRadial2Gradient&>(s).isDegenerate());
    }

protected:

    UniformHandle   fVSParamUni;
    UniformHandle   fFSParamUni;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    bool fIsDegenerate;

    // @{
    /// Values last uploaded as uniforms

    GrScalar fCachedCenter;
    GrScalar fCachedRadius;
    bool     fCachedPosRoot;

    // @}

private:

    typedef GrGLGradientStage INHERITED;

};

GrGLRadial2Gradient::GrGLRadial2Gradient(
        const GrProgramStageFactory& factory,
        const GrCustomStage& baseData)
    : INHERITED(factory)
    , fVSParamUni(kInvalidUniformHandle)
    , fFSParamUni(kInvalidUniformHandle)
    , fVSVaryingName(NULL)
    , fFSVaryingName(NULL)
    , fCachedCenter(GR_ScalarMax)
    , fCachedRadius(-GR_ScalarMax)
    , fCachedPosRoot(0) {

    const GrRadial2Gradient& data =
        static_cast<const GrRadial2Gradient&>(baseData);
    fIsDegenerate = data.isDegenerate();
}

void GrGLRadial2Gradient::setupVariables(GrGLShaderBuilder* builder, int stage) {
    // 2 copies of uniform array, 1 for each of vertex & fragment shader,
    // to work around Xoom bug. Doesn't seem to cause performance decrease
    // in test apps, but need to keep an eye on it.
    fVSParamUni = builder->addUniform(GrGLShaderBuilder::kVertex_ShaderType,
                                      kFloat_GrSLType, "uRadial2VSParams", stage, 6);
    fFSParamUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                       kFloat_GrSLType, "uRadial2FSParams", stage, 6);

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (builder->fVaryingDims == builder->fCoordDims) {
        builder->addVarying(kFloat_GrSLType, "Radial2BCoeff", stage,
                          &fVSVaryingName, &fFSVaryingName);
    }
}

void GrGLRadial2Gradient::emitVS(GrGLShaderBuilder* builder,
                                 const char* vertexCoords) {
    SkString* code = &builder->fVSCode;
    SkString p2;
    SkString p3;
    builder->getUniformVariable(fVSParamUni).appendArrayAccess(2, &p2);
    builder->getUniformVariable(fVSParamUni).appendArrayAccess(3, &p3);

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (builder->fVaryingDims == builder->fCoordDims) {
        // r2Var = 2 * (r2Parm[2] * varCoord.x - r2Param[3])
        code->appendf("\t%s = 2.0 *(%s * %s.x - %s);\n",
                      fVSVaryingName, p2.c_str(),
                      vertexCoords, p3.c_str());
    }
}

void GrGLRadial2Gradient::emitFS(GrGLShaderBuilder* builder,
                                 const char* outputColor,
                                 const char* inputColor,
                                 const char* samplerName) {
    SkString* code = &builder->fFSCode;
    SkString cName("c");
    SkString ac4Name("ac4");
    SkString rootName("root");
    SkString t;
    SkString p0;
    SkString p1;
    SkString p2;
    SkString p3;
    SkString p4;
    SkString p5;
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(0, &p0);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(1, &p1);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(2, &p2);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(3, &p3);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(4, &p4);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(5, &p5);

    // If we we're able to interpolate the linear component,
    // bVar is the varying; otherwise compute it
    SkString bVar;
    if (builder->fCoordDims == builder->fVaryingDims) {
        bVar = fFSVaryingName;
        GrAssert(2 == builder->fVaryingDims);
    } else {
        GrAssert(3 == builder->fVaryingDims);
        bVar = "b";
        //bVar.appendS32(stageNum);
        code->appendf("\tfloat %s = 2.0 * (%s * %s.x - %s);\n",
                      bVar.c_str(), p2.c_str(),
                      builder->fSampleCoords.c_str(), p3.c_str());
    }

    // c = (x^2)+(y^2) - params[4]
    code->appendf("\tfloat %s = dot(%s, %s) - %s;\n",
                  cName.c_str(), builder->fSampleCoords.c_str(),
                  builder->fSampleCoords.c_str(),
                  p4.c_str());

    // If we aren't degenerate, emit some extra code, and accept a slightly
    // more complex coord.
    if (!fIsDegenerate) {

        // ac4 = 4.0 * params[0] * c
        code->appendf("\tfloat %s = %s * 4.0 * %s;\n",
                      ac4Name.c_str(), p0.c_str(),
                      cName.c_str());

        // root = sqrt(b^2-4ac)
        // (abs to avoid exception due to fp precision)
        code->appendf("\tfloat %s = sqrt(abs(%s*%s - %s));\n",
                      rootName.c_str(), bVar.c_str(), bVar.c_str(),
                      ac4Name.c_str());

        // t is: (-b + params[5] * sqrt(b^2-4ac)) * params[1]
        t.printf("(-%s + %s * %s) * %s", bVar.c_str(), p5.c_str(),
                 rootName.c_str(), p1.c_str());
    } else {
        // t is: -c/b
        t.printf("-%s / %s", cName.c_str(), bVar.c_str());
    }

    this->emitColorLookup(builder, t.c_str(), outputColor, samplerName);
}

void GrGLRadial2Gradient::setData(const GrGLUniformManager& uman,
                                  const GrCustomStage& baseData,
                                  const GrRenderTarget*,
                                  int stageNum) {
    const GrRadial2Gradient& data =
        static_cast<const GrRadial2Gradient&>(baseData);
    GrAssert(data.isDegenerate() == fIsDegenerate);
    GrScalar centerX1 = data.center();
    GrScalar radius0 = data.radius();
    if (fCachedCenter != centerX1 ||
        fCachedRadius != radius0 ||
        fCachedPosRoot != data.isPosRoot()) {

        GrScalar a = GrMul(centerX1, centerX1) - GR_Scalar1;

        // When we're in the degenerate (linear) case, the second
        // value will be INF but the program doesn't read it. (We
        // use the same 6 uniforms even though we don't need them
        // all in the linear case just to keep the code complexity
        // down).
        float values[6] = {
            GrScalarToFloat(a),
            1 / (2.f * GrScalarToFloat(a)),
            GrScalarToFloat(centerX1),
            GrScalarToFloat(radius0),
            GrScalarToFloat(GrMul(radius0, radius0)),
            data.isPosRoot() ? 1.f : -1.f
        };

        uman.set1fv(fVSParamUni, 0, 6, values);
        uman.set1fv(fFSParamUni, 0, 6, values);
        fCachedCenter = centerX1;
        fCachedRadius = radius0;
        fCachedPosRoot = data.isPosRoot();
    }
}


/////////////////////////////////////////////////////////////////////

GrRadial2Gradient::GrRadial2Gradient(GrTexture* texture,
                                     GrScalar center,
                                     GrScalar radius,
                                     bool posRoot)
    : INHERITED(texture)
    , fCenterX1 (center)
    , fRadius0 (radius)
    , fPosRoot (posRoot) {

}

GrRadial2Gradient::GrRadial2Gradient(GrContext* ctx, 
                                     const SkShader& shader, 
                                     GrSamplerState* sampler,
                                     SkScalar center,
                                     SkScalar startRadius,
                                     SkScalar diffRadius)
                                     : INHERITED(ctx, shader, sampler)
                                     , fCenterX1(center)
                                     , fRadius0(startRadius) 
                                     , fPosRoot(diffRadius < 0) {
}

GrRadial2Gradient::~GrRadial2Gradient() {

}


const GrProgramStageFactory& GrRadial2Gradient::getFactory() const {
    return GrTProgramStageFactory<GrRadial2Gradient>::getInstance();
}

bool GrRadial2Gradient::isEqual(const GrCustomStage& sBase) const {
    const GrRadial2Gradient& s = static_cast<const GrRadial2Gradient&>(sBase);
    return (INHERITED::isEqual(sBase) &&
            this->fCenterX1 == s.fCenterX1 &&
            this->fRadius0 == s.fRadius0 &&
            this->fPosRoot == s.fPosRoot);
}

/////////////////////////////////////////////////////////////////////

class GrGLConical2Gradient : public GrGLGradientStage {

public:

    GrGLConical2Gradient(const GrProgramStageFactory& factory,
                         const GrCustomStage&);
    virtual ~GrGLConical2Gradient() { }

    virtual void setupVariables(GrGLShaderBuilder* builder,
                                int stage) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE;
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;
    virtual void setData(const GrGLUniformManager&,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s) {
        return (static_cast<const GrConical2Gradient&>(s).isDegenerate());
    }

protected:

    UniformHandle           fVSParamUni;
    GrGLint                 fVSParamLocation;
    UniformHandle           fFSParamUni;
    GrGLint                 fFSParamLocation;

    const char* fVSVaryingName;
    const char* fFSVaryingName;

    bool fIsDegenerate;

    // @{
    /// Values last uploaded as uniforms

    GrScalar fCachedCenter;
    GrScalar fCachedRadius;
    GrScalar fCachedDiffRadius;

    // @}

private:

    typedef GrGLGradientStage INHERITED;

};

GrGLConical2Gradient::GrGLConical2Gradient(
        const GrProgramStageFactory& factory,
        const GrCustomStage& baseData)
    : INHERITED(factory)
    , fVSParamUni(kInvalidUniformHandle)
    , fFSParamUni(kInvalidUniformHandle)
    , fVSVaryingName(NULL)
    , fFSVaryingName(NULL)
    , fCachedCenter(GR_ScalarMax)
    , fCachedRadius(-GR_ScalarMax)
    , fCachedDiffRadius(-GR_ScalarMax) {

    const GrConical2Gradient& data =
        static_cast<const GrConical2Gradient&>(baseData);
    fIsDegenerate = data.isDegenerate();
}

void GrGLConical2Gradient::setupVariables(GrGLShaderBuilder* builder, int stage) {
    // 2 copies of uniform array, 1 for each of vertex & fragment shader,
    // to work around Xoom bug. Doesn't seem to cause performance decrease
    // in test apps, but need to keep an eye on it.
    fVSParamUni = builder->addUniform(GrGLShaderBuilder::kVertex_ShaderType,
                                      kFloat_GrSLType, "uConical2VSParams", stage, 6);
    fFSParamUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                      kFloat_GrSLType, "uConical2FSParams", stage, 6);

    fVSParamLocation = GrGLProgramStage::kUseUniform;
    fFSParamLocation = GrGLProgramStage::kUseUniform;

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (builder->fVaryingDims == builder->fCoordDims) {
        builder->addVarying(kFloat_GrSLType, "Conical2BCoeff", stage,
                            &fVSVaryingName, &fFSVaryingName);
    }
}

void GrGLConical2Gradient::emitVS(GrGLShaderBuilder* builder,
                                  const char* vertexCoords) {
    SkString* code = &builder->fVSCode;
    SkString p2; // distance between centers
    SkString p3; // start radius
    SkString p5; // difference in radii (r1 - r0)
    builder->getUniformVariable(fVSParamUni).appendArrayAccess(2, &p2);
    builder->getUniformVariable(fVSParamUni).appendArrayAccess(3, &p3);
    builder->getUniformVariable(fVSParamUni).appendArrayAccess(5, &p5);

    // For radial gradients without perspective we can pass the linear
    // part of the quadratic as a varying.
    if (builder->fVaryingDims == builder->fCoordDims) {
        // r2Var = -2 * (r2Parm[2] * varCoord.x - r2Param[3] * r2Param[5])
        code->appendf("\t%s = -2.0 * (%s * %s.x + %s * %s);\n",
                      fVSVaryingName, p2.c_str(),
                      vertexCoords, p3.c_str(), p5.c_str());
    }
}

void GrGLConical2Gradient::emitFS(GrGLShaderBuilder* builder,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const char* samplerName) {
    SkString* code = &builder->fFSCode;

    SkString cName("c");
    SkString ac4Name("ac4");
    SkString dName("d");
    SkString qName("q");
    SkString r0Name("r0");
    SkString r1Name("r1");
    SkString tName("t");
    SkString p0; // 4a
    SkString p1; // 1/a
    SkString p2; // distance between centers
    SkString p3; // start radius
    SkString p4; // start radius squared
    SkString p5; // difference in radii (r1 - r0)

    builder->getUniformVariable(fFSParamUni).appendArrayAccess(0, &p0);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(1, &p1);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(2, &p2);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(3, &p3);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(4, &p4);
    builder->getUniformVariable(fFSParamUni).appendArrayAccess(5, &p5);

    // If we we're able to interpolate the linear component,
    // bVar is the varying; otherwise compute it
    SkString bVar;
    if (builder->fCoordDims == builder->fVaryingDims) {
        bVar = fFSVaryingName;
        GrAssert(2 == builder->fVaryingDims);
    } else {
        GrAssert(3 == builder->fVaryingDims);
        bVar = "b";
        code->appendf("\tfloat %s = -2.0 * (%s * %s.x + %s * %s);\n", 
                      bVar.c_str(), p2.c_str(), builder->fSampleCoords.c_str(), 
                      p3.c_str(), p5.c_str());
    }

    // output will default to transparent black (we simply won't write anything
    // else to it if invalid, instead of discarding or returning prematurely)
    code->appendf("\t%s = vec4(0.0,0.0,0.0,0.0);\n", outputColor);

    // c = (x^2)+(y^2) - params[4]
    code->appendf("\tfloat %s = dot(%s, %s) - %s;\n", cName.c_str(), 
                  builder->fSampleCoords.c_str(), builder->fSampleCoords.c_str(),
                  p4.c_str());

    // Non-degenerate case (quadratic)
    if (!fIsDegenerate) {

        // ac4 = params[0] * c
        code->appendf("\tfloat %s = %s * %s;\n", ac4Name.c_str(), p0.c_str(),
                      cName.c_str());
        
        // d = b^2 - ac4
        code->appendf("\tfloat %s = %s * %s - %s;\n", dName.c_str(), 
                      bVar.c_str(), bVar.c_str(), ac4Name.c_str());

        // only proceed if discriminant is >= 0
        code->appendf("\tif (%s >= 0.0) {\n", dName.c_str());

        // intermediate value we'll use to compute the roots
        // q = -0.5 * (b +/- sqrt(d))
        code->appendf("\t\tfloat %s = -0.5 * (%s + (%s < 0.0 ? -1.0 : 1.0)"
                      " * sqrt(%s));\n", qName.c_str(), bVar.c_str(), 
                      bVar.c_str(), dName.c_str());

        // compute both roots
        // r0 = q * params[1]
        code->appendf("\t\tfloat %s = %s * %s;\n", r0Name.c_str(), 
                      qName.c_str(), p1.c_str());
        // r1 = c / q
        code->appendf("\t\tfloat %s = %s / %s;\n", r1Name.c_str(), 
                      cName.c_str(), qName.c_str());

        // Note: If there are two roots that both generate radius(t) > 0, the 
        // Canvas spec says to choose the larger t.

        // so we'll look at the larger one first:
        code->appendf("\t\tfloat %s = max(%s, %s);\n", tName.c_str(), 
                      r0Name.c_str(), r1Name.c_str());

        // if r(t) > 0, then we're done; t will be our x coordinate
        code->appendf("\t\tif (%s * %s + %s > 0.0) {\n", tName.c_str(), 
                      p5.c_str(), p3.c_str());

        code->appendf("\t\t");
        this->emitColorLookup(builder, tName.c_str(), outputColor, samplerName);

        // otherwise, if r(t) for the larger root was <= 0, try the other root
        code->appendf("\t\t} else {\n");
        code->appendf("\t\t\t%s = min(%s, %s);\n", tName.c_str(), 
                      r0Name.c_str(), r1Name.c_str());

        // if r(t) > 0 for the smaller root, then t will be our x coordinate
        code->appendf("\t\t\tif (%s * %s + %s > 0.0) {\n",
                      tName.c_str(), p5.c_str(), p3.c_str());

        code->appendf("\t\t\t");
        this->emitColorLookup(builder, tName.c_str(), outputColor, samplerName);

        // end if (r(t) > 0) for smaller root 
        code->appendf("\t\t\t}\n");
        // end if (r(t) > 0), else, for larger root
        code->appendf("\t\t}\n");
        // end if (discriminant >= 0)
        code->appendf("\t}\n");
    } else {

        // linear case: t = -c/b
        code->appendf("\tfloat %s = -(%s / %s);\n", tName.c_str(), 
                      cName.c_str(), bVar.c_str());

        // if r(t) > 0, then t will be the x coordinate
        code->appendf("\tif (%s * %s + %s > 0.0) {\n", tName.c_str(),
                      p5.c_str(), p3.c_str());
        code->appendf("\t");
        this->emitColorLookup(builder, tName.c_str(), outputColor, samplerName);
        code->appendf("\t}\n");
    }
}

void GrGLConical2Gradient::setData(const GrGLUniformManager& uman,
                                   const GrCustomStage& baseData,
                                   const GrRenderTarget*,
                                   int stageNum) {
    const GrConical2Gradient& data =
        static_cast<const GrConical2Gradient&>(baseData);
    GrAssert(data.isDegenerate() == fIsDegenerate);
    GrScalar centerX1 = data.center();
    GrScalar radius0 = data.radius();
    GrScalar diffRadius = data.diffRadius();

    if (fCachedCenter != centerX1 ||
        fCachedRadius != radius0 ||
        fCachedDiffRadius != diffRadius) {

        GrScalar a = GrMul(centerX1, centerX1) - diffRadius * diffRadius;

        // When we're in the degenerate (linear) case, the second
        // value will be INF but the program doesn't read it. (We
        // use the same 6 uniforms even though we don't need them
        // all in the linear case just to keep the code complexity
        // down).
        float values[6] = {
            GrScalarToFloat(a * 4),
            1.f / (GrScalarToFloat(a)),
            GrScalarToFloat(centerX1),
            GrScalarToFloat(radius0),
            GrScalarToFloat(SkScalarMul(radius0, radius0)),
            GrScalarToFloat(diffRadius)
        };

        uman.set1fv(fVSParamUni, 0, 6, values);
        uman.set1fv(fFSParamUni, 0, 6, values);
        fCachedCenter = centerX1;
        fCachedRadius = radius0;
        fCachedDiffRadius = diffRadius;
    }
}


/////////////////////////////////////////////////////////////////////

GrConical2Gradient::GrConical2Gradient(GrTexture* texture,
                                       GrScalar center,
                                       GrScalar radius,
                                       GrScalar diffRadius)
    : INHERITED (texture)
    , fCenterX1 (center)
    , fRadius0 (radius)
    , fDiffRadius (diffRadius) {

}

GrConical2Gradient::GrConical2Gradient(GrContext* ctx, 
                                       const SkShader& shader,
                                       GrSamplerState* sampler,
                                       SkScalar center,
                                       SkScalar startRadius,
                                       SkScalar diffRadius)
                                       : INHERITED(ctx, shader, sampler) 
                                       , fCenterX1(center)
                                       , fRadius0(startRadius)
                                       , fDiffRadius(diffRadius) {
}

GrConical2Gradient::~GrConical2Gradient() {

}


const GrProgramStageFactory& GrConical2Gradient::getFactory() const {
    return GrTProgramStageFactory<GrConical2Gradient>::getInstance();
}

bool GrConical2Gradient::isEqual(const GrCustomStage& sBase) const {
    const GrConical2Gradient& s = static_cast<const GrConical2Gradient&>(sBase);
    return (INHERITED::isEqual(sBase) &&
            this->fCenterX1 == s.fCenterX1 &&
            this->fRadius0 == s.fRadius0 &&
            this->fDiffRadius == s.fDiffRadius);
}

/////////////////////////////////////////////////////////////////////


class GrGLSweepGradient : public GrGLGradientStage {

public:

    GrGLSweepGradient(const GrProgramStageFactory& factory,
                      const GrCustomStage&) : INHERITED (factory) { }
    virtual ~GrGLSweepGradient() { }

    virtual void emitVS(GrGLShaderBuilder* builder,
                        const char* vertexCoords) SK_OVERRIDE { }
    virtual void emitFS(GrGLShaderBuilder* builder,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    static StageKey GenKey(const GrCustomStage& s) { return 0; }

private:

    typedef GrGLGradientStage INHERITED;

};

void GrGLSweepGradient::emitFS(GrGLShaderBuilder* builder,
                              const char* outputColor,
                              const char* inputColor,
                              const char* samplerName) {
    SkString t;
    t.printf("atan(- %s.y, - %s.x) * 0.1591549430918 + 0.5",
        builder->fSampleCoords.c_str(), builder->fSampleCoords.c_str());
    this->emitColorLookup(builder, t.c_str(), outputColor, samplerName);
}

/////////////////////////////////////////////////////////////////////

GrSweepGradient::GrSweepGradient(GrTexture* texture)
    : INHERITED(texture) {

}

GrSweepGradient::GrSweepGradient(GrContext* ctx, const SkShader& shader,
                                 GrSamplerState* sampler) 
                                 : INHERITED(ctx, shader, sampler) {
}

GrSweepGradient::~GrSweepGradient() {

}

const GrProgramStageFactory& GrSweepGradient::getFactory() const {
    return GrTProgramStageFactory<GrSweepGradient>::getInstance();
}

