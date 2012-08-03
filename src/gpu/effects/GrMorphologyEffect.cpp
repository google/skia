/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMorphologyEffect.h"
#include "gl/GrGLProgramStage.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLTexture.h"
#include "GrProgramStageFactory.h"

///////////////////////////////////////////////////////////////////////////////

class GrGLMorphologyEffect  : public GrGLProgramStage {
public:
    GrGLMorphologyEffect (const GrProgramStageFactory& factory,
                          const GrCustomStage& stage);

    virtual void setupVariables(GrGLShaderBuilder* builder) SK_OVERRIDE;
    virtual void emitVS(GrGLShaderBuilder* state,
                        const char* vertexCoords) SK_OVERRIDE {};
    virtual void emitFS(GrGLShaderBuilder* state,
                        const char* outputColor,
                        const char* inputColor,
                        const char* samplerName) SK_OVERRIDE;

    static inline StageKey GenKey(const GrCustomStage& s, const GrGLCaps& caps);

    virtual void setData(const GrGLUniformManager&,
                         const GrCustomStage&,
                         const GrRenderTarget*,
                         int stageNum) SK_OVERRIDE;

private:
    int width() const { return GrMorphologyEffect::WidthFromRadius(fRadius); }

    int                                 fRadius;
    GrMorphologyEffect::MorphologyType  fType;
    GrGLUniformManager::UniformHandle   fImageIncrementUni;

    typedef GrGLProgramStage INHERITED;
};

GrGLMorphologyEffect ::GrGLMorphologyEffect(const GrProgramStageFactory& factory,
                                            const GrCustomStage& stage)
    : GrGLProgramStage(factory)
    , fImageIncrementUni(GrGLUniformManager::kInvalidUniformHandle) {
    const GrMorphologyEffect& m = static_cast<const GrMorphologyEffect&>(stage);
    fRadius = m.radius();
    fType = m.type();
}

void GrGLMorphologyEffect::setupVariables(GrGLShaderBuilder* builder) {
    fImageIncrementUni = builder->addUniform(GrGLShaderBuilder::kFragment_ShaderType,
                                             kVec2f_GrSLType, "ImageIncrement");
}

void GrGLMorphologyEffect ::emitFS(GrGLShaderBuilder* builder,
                                   const char* outputColor,
                                   const char* inputColor,
                                   const char* samplerName) {
    SkString* code = &builder->fFSCode;

    const char* func;
    switch (fType) {
        case GrContext::kErode_MorphologyType:
            code->appendf("\t\tvec4 value = vec4(1, 1, 1, 1);\n");
            func = "min";
            break;
        case GrContext::kDilate_MorphologyType:
            code->appendf("\t\tvec4 value = vec4(0, 0, 0, 0);\n");
            func = "max";
            break;
        default:
            GrCrash("Unexpected type");
            func = ""; // suppress warning
            break;
    }
    const char* imgInc = builder->getUniformCStr(fImageIncrementUni);

    code->appendf("\t\tvec2 coord = %s - %d.0 * %s;\n",
                   builder->fSampleCoords.c_str(), fRadius, imgInc);
    code->appendf("\t\tfor (int i = 0; i < %d; i++) {\n", this->width());
    code->appendf("\t\t\tvalue = %s(value, ", func);
    builder->emitTextureLookup(samplerName, "coord");
    code->appendf(");\n");
    code->appendf("\t\t\tcoord += %s;\n", imgInc);
    code->appendf("\t\t}\n");
    code->appendf("\t\t%s = value%s;\n", outputColor, builder->fModulate.c_str());
}

GrGLProgramStage::StageKey GrGLMorphologyEffect::GenKey(const GrCustomStage& s,
                                                        const GrGLCaps& caps) {
    const GrMorphologyEffect& m = static_cast<const GrMorphologyEffect&>(s);
    StageKey key = static_cast<StageKey>(m.radius());
    key |= (m.type() << 8);
    return key;
}

void GrGLMorphologyEffect ::setData(const GrGLUniformManager& uman,
                                    const GrCustomStage& data,
                                    const GrRenderTarget*,
                                    int stageNum) {
    const Gr1DKernelEffect& kern =
        static_cast<const Gr1DKernelEffect&>(data);
    GrGLTexture& texture =
        *static_cast<GrGLTexture*>(data.texture(0));
    // the code we generated was for a specific kernel radius
    GrAssert(kern.radius() == fRadius);
    float imageIncrement[2] = { 0 };
    switch (kern.direction()) {
        case Gr1DKernelEffect::kX_Direction:
            imageIncrement[0] = 1.0f / texture.width();
            break;
        case Gr1DKernelEffect::kY_Direction:
            imageIncrement[1] = 1.0f / texture.height();
            break;
        default:
            GrCrash("Unknown filter direction.");
    }
    uman.set2fv(fImageIncrementUni, 0, 1, imageIncrement);
}

///////////////////////////////////////////////////////////////////////////////

GrMorphologyEffect::GrMorphologyEffect(GrTexture* texture,
                                       Direction direction,
                                       int radius,
                                       MorphologyType type)
    : Gr1DKernelEffect(texture, direction, radius)
    , fType(type) {
}

GrMorphologyEffect::~GrMorphologyEffect() {
}

const GrProgramStageFactory& GrMorphologyEffect::getFactory() const {
    return GrTProgramStageFactory<GrMorphologyEffect>::getInstance();
}

bool GrMorphologyEffect::isEqual(const GrCustomStage& sBase) const {
    const GrMorphologyEffect& s =
        static_cast<const GrMorphologyEffect&>(sBase);
    return (INHERITED::isEqual(sBase) &&
            this->radius() == s.radius() &&
            this->direction() == s.direction() &&
            this->type() == s.type());
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_CUSTOM_STAGE_TEST(GrMorphologyEffect);

GrCustomStage* GrMorphologyEffect::TestCreate(SkRandom* random,
                                              GrContext* context,
                                              GrTexture* textures[]) {
    int texIdx = random->nextBool() ? GrCustomStageUnitTest::kSkiaPMTextureIdx :
                                      GrCustomStageUnitTest::kAlphaTextureIdx;
    Direction dir = random->nextBool() ? kX_Direction : kY_Direction;
    static const int kMaxRadius = 10;
    int radius = random->nextRangeU(1, kMaxRadius);
    MorphologyType type = random->nextBool() ? GrContext::kErode_MorphologyType :
                                               GrContext::kDilate_MorphologyType;

    return SkNEW_ARGS(GrMorphologyEffect, (textures[texIdx], dir, radius, type));
}
