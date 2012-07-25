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

    virtual void setupVariables(GrGLShaderBuilder* state,
                                int stage) SK_OVERRIDE {}
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE {}
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    virtual void setData(const GrGLUniformManager&,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE {}

    static inline StageKey GenKey(const GrCustomStage&);

private:
    typedef GrGLProgramStage INHERITED;
};

GrGLColorTableEffect::GrGLColorTableEffect(
    const GrProgramStageFactory& factory, const GrCustomStage& stage)
    : INHERITED(factory) {
 }

void GrGLColorTableEffect::emitFS(GrGLShaderBuilder* state,
                                  const char* outputColor,
                                  const char* inputColor,
                                  const char* samplerName) {
    static const float kColorScaleFactor = 255.0f / 256.0f;
    static const float kColorOffsetFactor = 1.0f / 512.0f;
    SkString* code = &state->fFSCode; 
    code->appendf("\t\tvec4 coord = vec4(%s.rgb / %s.a, %s.a);\n",
                  inputColor, inputColor, inputColor);
    code->appendf("\t\tcoord = coord * %f + vec4(%f, %f, %f, %f);\n",
                  kColorScaleFactor,
                  kColorOffsetFactor, kColorOffsetFactor, kColorOffsetFactor, kColorOffsetFactor);
    code->appendf("\t\t%s.a = texture2D(%s, vec2(coord.a, 0.125)).a;\n",
                  outputColor, samplerName);
    code->appendf("\t\t%s.r = texture2D(%s, vec2(coord.r, 0.375)).a;\n",
                  outputColor, samplerName);
    code->appendf("\t\t%s.g = texture2D(%s, vec2(coord.g, 0.625)).a;\n",
                  outputColor, samplerName);
    code->appendf("\t\t%s.b = texture2D(%s, vec2(coord.b, 0.875)).a;\n",
                  outputColor, samplerName);
    code->appendf("\t\t%s.rgb *= %s.a;\n", outputColor, outputColor);
}

GrGLProgramStage::StageKey GrGLColorTableEffect::GenKey(
    const GrCustomStage& s) {
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

GrColorTableEffect::GrColorTableEffect(GrTexture* texture)
    : INHERITED(texture) {
}

GrColorTableEffect::~GrColorTableEffect() {
}

const GrProgramStageFactory&  GrColorTableEffect::getFactory() const {
    return GrTProgramStageFactory<GrColorTableEffect>::getInstance();
}

bool GrColorTableEffect::isEqual(const GrCustomStage& sBase) const {
    return INHERITED::isEqual(sBase);
}
