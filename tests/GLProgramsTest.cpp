
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "GrAutoLocaleSetter.h"
#include "GrBatchTest.h"
#include "GrContextFactory.h"
#include "GrInvariantOutput.h"
#include "GrPipeline.h"
#include "GrResourceProvider.h"
#include "GrTest.h"
#include "GrXferProcessor.h"
#include "SkChecksum.h"
#include "SkRandom.h"
#include "Test.h"

#include "batches/GrBatch.h"

#include "effects/GrConfigConversionEffect.h"
#include "effects/GrPorterDuffXferProcessor.h"

#include "gl/GrGLGpu.h"
#include "gl/GrGLPathRendering.h"
#include "gl/builders/GrGLProgramBuilder.h"

/*
 * A dummy processor which just tries to insert a massive key and verify that it can retrieve the
 * whole thing correctly
 */
static const uint32_t kMaxKeySize = 1024;

class GLBigKeyProcessor : public GrGLFragmentProcessor {
public:
    GLBigKeyProcessor(const GrProcessor&) {}

    virtual void emitCode(EmitArgs& args) override {
        // pass through
        GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
        fsBuilder->codeAppendf("%s = %s;\n", args.fOutputColor, args.fInputColor);
    }

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
        for (uint32_t i = 0; i < kMaxKeySize; i++) {
            b->add32(i);
        }
    }

private:
    typedef GrGLFragmentProcessor INHERITED;
};

class BigKeyProcessor : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create() {
        GR_CREATE_STATIC_PROCESSOR(gBigKeyProcessor, BigKeyProcessor, ())
        return SkRef(gBigKeyProcessor);
    }

    const char* name() const override { return "Big Ole Key"; }

    GrGLFragmentProcessor* createGLInstance() const override {
        return SkNEW_ARGS(GLBigKeyProcessor, (*this));
    }

private:
    BigKeyProcessor() {
        this->initClassID<BigKeyProcessor>();
    }
    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override {
        GLBigKeyProcessor::GenKey(*this, caps, b);
    }
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }
    void onComputeInvariantOutput(GrInvariantOutput* inout) const override { }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(BigKeyProcessor);

GrFragmentProcessor* BigKeyProcessor::TestCreate(GrProcessorTestData*) {
    return BigKeyProcessor::Create();
}

/*
 * Begin test code
 */
static const int kRenderTargetHeight = 1;
static const int kRenderTargetWidth = 1;

static GrRenderTarget* random_render_target(GrTextureProvider* textureProvider, SkRandom* random,
                                            const GrCaps* caps) {
    // setup render target
    GrTextureParams params;
    GrSurfaceDesc texDesc;
    texDesc.fWidth = kRenderTargetWidth;
    texDesc.fHeight = kRenderTargetHeight;
    texDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    texDesc.fConfig = kRGBA_8888_GrPixelConfig;
    texDesc.fOrigin = random->nextBool() == true ? kTopLeft_GrSurfaceOrigin :
                                                   kBottomLeft_GrSurfaceOrigin;
    texDesc.fSampleCnt = random->nextBool() == true ? SkTMin(4, caps->maxSampleCount()) : 0;

    GrUniqueKey key;
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(&key, kDomain, 2);
    builder[0] = texDesc.fOrigin;
    builder[1] = texDesc.fSampleCnt;
    builder.finish();

    GrTexture* texture = textureProvider->findAndRefTextureByUniqueKey(key);
    if (!texture) {
        texture = textureProvider->createTexture(texDesc, true);
        if (texture) {
            textureProvider->assignUniqueKeyToTexture(key, texture);
        }
    }
    return texture ? texture->asRenderTarget() : NULL;
}

static void set_random_xpf(GrPipelineBuilder* pipelineBuilder, GrProcessorTestData* d) {
    SkAutoTUnref<const GrXPFactory> xpf(GrProcessorTestFactory<GrXPFactory>::CreateStage(d));
    SkASSERT(xpf);
    pipelineBuilder->setXPFactory(xpf.get());
}

static void set_random_color_coverage_stages(GrPipelineBuilder* pipelineBuilder,
                                             GrProcessorTestData* d, int maxStages) {
    int numProcs = d->fRandom->nextULessThan(maxStages + 1);
    int numColorProcs = d->fRandom->nextULessThan(numProcs + 1);

    for (int s = 0; s < numProcs;) {
        SkAutoTUnref<const GrFragmentProcessor> fp(
                GrProcessorTestFactory<GrFragmentProcessor>::CreateStage(d));
        SkASSERT(fp);

        // finally add the stage to the correct pipeline in the drawstate
        if (s < numColorProcs) {
            pipelineBuilder->addColorProcessor(fp);
        } else {
            pipelineBuilder->addCoverageProcessor(fp);
        }
        ++s;
    }
}

static void set_random_state(GrPipelineBuilder* pipelineBuilder, SkRandom* random) {
    int state = 0;
    for (int i = 1; i <= GrPipelineBuilder::kLast_Flag; i <<= 1) {
        state |= random->nextBool() * i;
    }

    // If we don't have an MSAA rendertarget then we have to disable useHWAA
    if ((state | GrPipelineBuilder::kHWAntialias_Flag) &&
        !pipelineBuilder->getRenderTarget()->isUnifiedMultisampled()) {
        state &= ~GrPipelineBuilder::kHWAntialias_Flag;
    }
    pipelineBuilder->enableState(state);
}

// right now, the only thing we seem to care about in drawState's stencil is 'doesWrite()'
static void set_random_stencil(GrPipelineBuilder* pipelineBuilder, SkRandom* random) {
    GR_STATIC_CONST_SAME_STENCIL(kDoesWriteStencil,
                                 kReplace_StencilOp,
                                 kReplace_StencilOp,
                                 kAlways_StencilFunc,
                                 0xffff,
                                 0xffff,
                                 0xffff);
    GR_STATIC_CONST_SAME_STENCIL(kDoesNotWriteStencil,
                                 kKeep_StencilOp,
                                 kKeep_StencilOp,
                                 kNever_StencilFunc,
                                 0xffff,
                                 0xffff,
                                 0xffff);

    if (random->nextBool()) {
        pipelineBuilder->setStencil(kDoesWriteStencil);
    } else {
        pipelineBuilder->setStencil(kDoesNotWriteStencil);
    }
}

bool GrDrawTarget::programUnitTest(GrContext* context, int maxStages) {
    // setup dummy textures
    GrSurfaceDesc dummyDesc;
    dummyDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    dummyDesc.fConfig = kSkia8888_GrPixelConfig;
    dummyDesc.fWidth = 34;
    dummyDesc.fHeight = 18;
    SkAutoTUnref<GrTexture> dummyTexture1(
        context->textureProvider()->createTexture(dummyDesc, false, NULL, 0));
    dummyDesc.fFlags = kNone_GrSurfaceFlags;
    dummyDesc.fConfig = kAlpha_8_GrPixelConfig;
    dummyDesc.fWidth = 16;
    dummyDesc.fHeight = 22;
    SkAutoTUnref<GrTexture> dummyTexture2(
        context->textureProvider()->createTexture(dummyDesc, false, NULL, 0));

    if (!dummyTexture1 || ! dummyTexture2) {
        SkDebugf("Could not allocate dummy textures");
        return false;
    }

    GrTexture* dummyTextures[] = {dummyTexture1.get(), dummyTexture2.get()};

    // dummy scissor state
    GrScissorState scissor;

    // wide open clip
    GrClip clip;

    SkRandom random;
    static const int NUM_TESTS = 2048;
    for (int t = 0; t < NUM_TESTS; t++) {
        // setup random render target(can fail)
        SkAutoTUnref<GrRenderTarget> rt(random_render_target(
            context->textureProvider(), &random, this->caps()));
        if (!rt.get()) {
            SkDebugf("Could not allocate render target");
            return false;
        }

        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setRenderTarget(rt.get());
        pipelineBuilder.setClip(clip);

        SkAutoTUnref<GrBatch> batch(GrRandomBatch(&random, context));
        SkASSERT(batch);

        GrProcessorDataManager procDataManager;
        GrProcessorTestData ptd(&random, context, &procDataManager, fGpu->caps(), dummyTextures);
        set_random_color_coverage_stages(&pipelineBuilder, &ptd, maxStages);
        set_random_xpf(&pipelineBuilder, &ptd);
        set_random_state(&pipelineBuilder, &random);
        set_random_stencil(&pipelineBuilder, &random);

        this->drawBatch(pipelineBuilder, batch);
    }

    // Flush everything, test passes if flush is successful(ie, no asserts are hit, no crashes)
    this->flush();
    return true;
}

DEF_GPUTEST(GLPrograms, reporter, factory) {
    // Set a locale that would cause shader compilation to fail because of , as decimal separator.
    // skbug 3330
#ifdef SK_BUILD_FOR_WIN
    GrAutoLocaleSetter als("sv-SE");
#else
    GrAutoLocaleSetter als("sv_SE.UTF-8");
#endif

    // We suppress prints to avoid spew
    GrContextOptions opts;
    opts.fSuppressPrints = true;
    GrContextFactory debugFactory(opts);
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContext* context = debugFactory.get(static_cast<GrContextFactory::GLContextType>(type));
        if (context) {
            GrGLGpu* gpu = static_cast<GrGLGpu*>(context->getGpu());

            /*
             * For the time being, we only support the test with desktop GL or for android on
             * ARM platforms
             * TODO When we run ES 3.00 GLSL in more places, test again
             */
            int maxStages;
            if (kGL_GrGLStandard == gpu->glStandard() ||
                kARM_GrGLVendor == gpu->ctxInfo().vendor()) {
                maxStages = 6;
            } else if (kTegra3_GrGLRenderer == gpu->ctxInfo().renderer() ||
                       kOther_GrGLRenderer == gpu->ctxInfo().renderer()) {
                maxStages = 1;
            } else {
                return;
            }
#if SK_ANGLE
            // Some long shaders run out of temporary registers in the D3D compiler on ANGLE.
            if (type == GrContextFactory::kANGLE_GLContextType) {
                maxStages = 2;
            }
#endif
            GrTestTarget target;
            context->getTestTarget(&target);
            REPORTER_ASSERT(reporter, target.target()->programUnitTest(context, maxStages));
        }
    }
}

#endif
