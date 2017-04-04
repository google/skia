/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrSimpleTextureEffect.h"
#include "GrProxyMove.h"
#include "GrTexture.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

GrSimpleTextureEffect::GrSimpleTextureEffect(GrResourceProvider* resourceProvider,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& matrix,
                                             GrSamplerParams::FilterMode filterMode)
        : INHERITED{resourceProvider,
                    ModulationFlags(proxy->config()),
                    GR_PROXY_MOVE(proxy),
                    std::move(colorSpaceXform),
                    matrix,
                    filterMode} {
    this->initClassID<GrSimpleTextureEffect>();
}

GrSimpleTextureEffect::GrSimpleTextureEffect(GrResourceProvider* resourceProvider,
                                             sk_sp<GrTextureProxy> proxy,
                                             sk_sp<GrColorSpaceXform> colorSpaceXform,
                                             const SkMatrix& matrix,
                                             const GrSamplerParams& params)
        : INHERITED{resourceProvider,
                    ModulationFlags(proxy->config()),
                    GR_PROXY_MOVE(proxy),
                    std::move(colorSpaceXform),
                    matrix,
                    params} {
    this->initClassID<GrSimpleTextureEffect>();
}

class GrGLSimpleTextureEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrSimpleTextureEffect& textureEffect = args.fFp.cast<GrSimpleTextureEffect>();
        fColorSpaceHelper.emitCode(args.fUniformHandler, textureEffect.colorSpaceXform());

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        fragBuilder->codeAppendf("%s = ", args.fOutputColor);
        fragBuilder->appendTextureLookupAndModulate(args.fInputColor,
                                                    args.fTexSamplers[0],
                                                    args.fTransformedCoords[0].c_str(),
                                                    args.fTransformedCoords[0].getType(),
                                                    &fColorSpaceHelper);
        fragBuilder->codeAppend(";");
    }

    static inline void GenKey(const GrProcessor& effect, const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrSimpleTextureEffect& textureEffect = effect.cast<GrSimpleTextureEffect>();
        b->add32(GrColorSpaceXform::XformKey(textureEffect.colorSpaceXform()));
    }

protected:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& processor) override {
        const GrSimpleTextureEffect& textureEffect = processor.cast<GrSimpleTextureEffect>();
        if (SkToBool(textureEffect.colorSpaceXform())) {
            fColorSpaceHelper.setData(pdman, textureEffect.colorSpaceXform());
        }
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;

    GrGLSLColorSpaceXformHelper fColorSpaceHelper;
};

///////////////////////////////////////////////////////////////////////////////

void GrSimpleTextureEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    GrGLSimpleTextureEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrSimpleTextureEffect::onCreateGLSLInstance() const  {
    return new GrGLSimpleTextureEffect;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSimpleTextureEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrSimpleTextureEffect::TestCreate(GrProcessorTestData* d) {
    int texIdx = d->fRandom->nextBool() ? GrProcessorUnitTest::kSkiaPMTextureIdx
                                        : GrProcessorUnitTest::kAlphaTextureIdx;
    static const SkShader::TileMode kTileModes[] = {
        SkShader::kClamp_TileMode,
        SkShader::kRepeat_TileMode,
        SkShader::kMirror_TileMode,
    };
    SkShader::TileMode tileModes[] = {
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
        kTileModes[d->fRandom->nextULessThan(SK_ARRAY_COUNT(kTileModes))],
    };
    GrSamplerParams params(tileModes, d->fRandom->nextBool() ? GrSamplerParams::kBilerp_FilterMode
                                                             : GrSamplerParams::kNone_FilterMode);

    const SkMatrix& matrix = GrTest::TestMatrix(d->fRandom);
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrTest::TestColorXform(d->fRandom);
    return GrSimpleTextureEffect::Make(d->resourceProvider(), d->textureProxy(texIdx),
                                       std::move(colorSpaceXform), matrix);
}
#endif
