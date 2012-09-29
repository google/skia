/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "GrColorTableEffect.h"
 #include "gl/GrGLProgramStage.h"
 #include "GrProgramStageFactory.h"
 #include "SkString.h"

///////////////////////////////////////////////////////////////////////////////

class GrGLColorTableEffect : public GrGLProgramStage {
public:
    GrGLColorTableEffect(const GrProgramStageFactory& factory,
                         const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* state) SK_OVERRIDE {}
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE {}
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const TextureSamplerArray&) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE {}

    static StageKey GenKey(const GrCustomStage&, const GrGLCaps&);

private:

    typedef GrGLProgramStage INHERITED;
};

GrGLColorTableEffect::GrGLColorTableEffect(
    const GrProgramStageFactory& factory, const GrCustomStage& stage)
    : INHERITED(factory) {
 }

void GrGLColorTableEffect::emitFS(GrGLShaderBuilder* builder,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const TextureSamplerArray& samplers) {
    static const float kColorScaleFactor = 255.0f / 256.0f;
    static const float kColorOffsetFactor = 1.0f / 512.0f;
    SkString* code = &builder->fFSCode;
    if (NULL == inputColor) {
        // the input color is solid white (all ones).
        static const float kMaxValue = kColorScaleFactor + kColorOffsetFactor;
        code->appendf("\t\tvec4 coord = vec4(%f, %f, %f, %f);\n",
                      kMaxValue, kMaxValue, kMaxValue, kMaxValue);

    } else {
        code->appendf("\t\tvec4 coord = vec4(%s.rgb / %s.a, %s.a);\n",
                      inputColor, inputColor, inputColor);
        code->appendf("\t\tcoord = coord * %f + vec4(%f, %f, %f, %f);\n",
                      kColorScaleFactor,
                      kColorOffsetFactor, kColorOffsetFactor,
                      kColorOffsetFactor, kColorOffsetFactor);
    }

    code->appendf("\t\t%s.a = ", outputColor);
    builder->appendTextureLookup(code, samplers[0], "vec2(coord.a, 0.125)");
    code->append(";\n");

    code->appendf("\t\t%s.r = ", outputColor);
    builder->appendTextureLookup(code, samplers[0], "vec2(coord.r, 0.375)");
    code->append(";\n");

    code->appendf("\t\t%s.g = ", outputColor);
    builder->appendTextureLookup(code, samplers[0], "vec2(coord.g, 0.625)");
    code->append(";\n");

    code->appendf("\t\t%s.b = ", outputColor);
    builder->appendTextureLookup(code, samplers[0], "vec2(coord.b, 0.875)");
    code->append(";\n");

    code->appendf("\t\t%s.rgb *= %s.a;\n", outputColor, outputColor);
}

GrGLProgramStage::StageKey GrGLColorTableEffect::GenKey(const GrCustomStage& s,
                                                        const GrGLCaps& caps) {
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

GrColorTableEffect::GrColorTableEffect(GrTexture* texture)
    : fTextureAccess(texture, "a") {
}

GrColorTableEffect::~GrColorTableEffect() {
}

const GrProgramStageFactory&  GrColorTableEffect::getFactory() const {
    return GrTProgramStageFactory<GrColorTableEffect>::getInstance();
}

bool GrColorTableEffect::isEqual(const GrCustomStage& sBase) const {
    return INHERITED::isEqual(sBase);
}

const GrTextureAccess& GrColorTableEffect::textureAccess(int index) const {
    GrAssert(0 == index);
    return fTextureAccess;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_CUSTOM_STAGE_TEST(GrColorTableEffect);

GrCustomStage* GrColorTableEffect::TestCreate(SkRandom* random,
                                              GrContext* context,
                                              GrTexture* textures[]) {
    return SkNEW_ARGS(GrColorTableEffect, (textures[GrCustomStageUnitTest::kAlphaTextureIdx]));
}
