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
#include "GrDrawContextPriv.h"
#include "GrDrawingManager.h"
#include "GrInvariantOutput.h"
#include "GrPipeline.h"
#include "GrResourceProvider.h"
#include "GrTest.h"
#include "GrXferProcessor.h"
#include "SkChecksum.h"
#include "SkRandom.h"
#include "Test.h"

#include "batches/GrDrawBatch.h"

#include "effects/GrConfigConversionEffect.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

#include "gl/GrGLGpu.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"

/*
 * A dummy processor which just tries to insert a massive key and verify that it can retrieve the
 * whole thing correctly
 */
static const uint32_t kMaxKeySize = 1024;

class GLBigKeyProcessor : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        // pass through
        GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
        if (args.fInputColor) {
            fragBuilder->codeAppendf("%s = %s;\n", args.fOutputColor, args.fInputColor);
        } else {
            fragBuilder->codeAppendf("%s = vec4(1.0);\n", args.fOutputColor);
        }
    }

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
        for (uint32_t i = 0; i < kMaxKeySize; i++) {
            b->add32(i);
        }
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

class BigKeyProcessor : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create() {
        return new BigKeyProcessor;
    }

    const char* name() const override { return "Big Ole Key"; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLBigKeyProcessor;
    }

private:
    BigKeyProcessor() {
        this->initClassID<BigKeyProcessor>();
    }
    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GLBigKeyProcessor::GenKey(*this, caps, b);
    }
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }
    void onComputeInvariantOutput(GrInvariantOutput* inout) const override { }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(BigKeyProcessor);

const GrFragmentProcessor* BigKeyProcessor::TestCreate(GrProcessorTestData*) {
    return BigKeyProcessor::Create();
}

//////////////////////////////////////////////////////////////////////////////

class BlockInputFragmentProcessor : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(const GrFragmentProcessor* fp) {
        return new BlockInputFragmentProcessor(fp);
    }

    const char* name() const override { return "Block Input"; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLFP; }

private:
    class GLFP : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            this->emitChild(0, nullptr, args);
        }

    private:
        typedef GrGLSLFragmentProcessor INHERITED;
    };

    BlockInputFragmentProcessor(const GrFragmentProcessor* child) {
        this->initClassID<BlockInputFragmentProcessor>();
        this->registerChildProcessor(child);
    }

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {}

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToOther(kRGBA_GrColorComponentFlags, GrColor_WHITE,
                          GrInvariantOutput::kWillNot_ReadInput);
        this->childProcessor(0).computeInvariantOutput(inout);
    }

    typedef GrFragmentProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

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
        texture = textureProvider->createTexture(texDesc, SkBudgeted::kYes);
        if (texture) {
            textureProvider->assignUniqueKeyToTexture(key, texture);
        }
    }
    return texture ? texture->asRenderTarget() : nullptr;
}

static void set_random_xpf(GrPipelineBuilder* pipelineBuilder, GrProcessorTestData* d) {
    SkAutoTUnref<const GrXPFactory> xpf(GrProcessorTestFactory<GrXPFactory>::Create(d));
    SkASSERT(xpf);
    pipelineBuilder->setXPFactory(xpf.get());
}

static const GrFragmentProcessor* create_random_proc_tree(GrProcessorTestData* d,
                                                           int minLevels, int maxLevels) {
    SkASSERT(1 <= minLevels);
    SkASSERT(minLevels <= maxLevels);

    // Return a leaf node if maxLevels is 1 or if we randomly chose to terminate.
    // If returning a leaf node, make sure that it doesn't have children (e.g. another
    // GrComposeEffect)
    const float terminateProbability = 0.3f;
    if (1 == minLevels) {
        bool terminate = (1 == maxLevels) || (d->fRandom->nextF() < terminateProbability);
        if (terminate) {
            const GrFragmentProcessor* fp;
            while (true) {
                fp = GrProcessorTestFactory<GrFragmentProcessor>::Create(d);
                SkASSERT(fp);
                if (0 == fp->numChildProcessors()) {
                    break;
                }
                fp->unref();
            }
            return fp;
        }
    }
    // If we didn't terminate, choose either the left or right subtree to fulfill
    // the minLevels requirement of this tree; the other child can have as few levels as it wants.
    // Also choose a random xfer mode that's supported by CreateFrom2Procs().
    if (minLevels > 1) {
        --minLevels;
    }
    SkAutoTUnref<const GrFragmentProcessor> minLevelsChild(create_random_proc_tree(d, minLevels,
                                                                                   maxLevels - 1));
    SkAutoTUnref<const GrFragmentProcessor> otherChild(create_random_proc_tree(d, 1,
                                                                               maxLevels - 1));
    SkXfermode::Mode mode = static_cast<SkXfermode::Mode>(d->fRandom->nextRangeU(0,
                                                          SkXfermode::kLastCoeffMode));
    const GrFragmentProcessor* fp;
    if (d->fRandom->nextF() < 0.5f) {
        fp = GrXfermodeFragmentProcessor::CreateFromTwoProcessors(minLevelsChild, otherChild, mode);
        SkASSERT(fp);
    } else {
        fp = GrXfermodeFragmentProcessor::CreateFromTwoProcessors(otherChild, minLevelsChild, mode);
        SkASSERT(fp);
    }
    return fp;
}

static void set_random_color_coverage_stages(GrPipelineBuilder* pipelineBuilder,
                                             GrProcessorTestData* d, int maxStages) {
    // Randomly choose to either create a linear pipeline of procs or create one proc tree
    const float procTreeProbability = 0.5f;
    if (d->fRandom->nextF() < procTreeProbability) {
        // A full tree with 5 levels (31 nodes) may exceed the max allowed length of the gl
        // processor key; maxTreeLevels should be a number from 1 to 4 inclusive.
        const int maxTreeLevels = 4;
        SkAutoTUnref<const GrFragmentProcessor> fp(
                                        create_random_proc_tree(d, 2, maxTreeLevels));
        pipelineBuilder->addColorFragmentProcessor(fp);
    } else {
        int numProcs = d->fRandom->nextULessThan(maxStages + 1);
        int numColorProcs = d->fRandom->nextULessThan(numProcs + 1);

        for (int s = 0; s < numProcs;) {
            SkAutoTUnref<const GrFragmentProcessor> fp(
                GrProcessorTestFactory<GrFragmentProcessor>::Create(d));
            SkASSERT(fp);

            // finally add the stage to the correct pipeline in the drawstate
            if (s < numColorProcs) {
                pipelineBuilder->addColorFragmentProcessor(fp);
            } else {
                pipelineBuilder->addCoverageFragmentProcessor(fp);
            }
            ++s;
        }
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

bool GrDrawingManager::ProgramUnitTest(GrContext* context, int maxStages) {
    GrDrawingManager* drawingManager = context->drawingManager();

    // setup dummy textures
    GrSurfaceDesc dummyDesc;
    dummyDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    dummyDesc.fConfig = kSkia8888_GrPixelConfig;
    dummyDesc.fWidth = 34;
    dummyDesc.fHeight = 18;
    SkAutoTUnref<GrTexture> dummyTexture1(
        context->textureProvider()->createTexture(dummyDesc, SkBudgeted::kNo, nullptr, 0));
    dummyDesc.fFlags = kNone_GrSurfaceFlags;
    dummyDesc.fConfig = kAlpha_8_GrPixelConfig;
    dummyDesc.fWidth = 16;
    dummyDesc.fHeight = 22;
    SkAutoTUnref<GrTexture> dummyTexture2(
        context->textureProvider()->createTexture(dummyDesc, SkBudgeted::kNo, nullptr, 0));

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
    static const int NUM_TESTS = 1024;
    for (int t = 0; t < NUM_TESTS; t++) {
        // setup random render target(can fail)
        sk_sp<GrRenderTarget> rt(random_render_target(
            context->textureProvider(), &random, context->caps()));
        if (!rt.get()) {
            SkDebugf("Could not allocate render target");
            return false;
        }

        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setRenderTarget(rt.get());
        pipelineBuilder.setClip(clip);

        SkAutoTUnref<GrDrawBatch> batch(GrRandomDrawBatch(&random, context));
        SkASSERT(batch);

        GrProcessorTestData ptd(&random, context, context->caps(), rt.get(), dummyTextures);
        set_random_color_coverage_stages(&pipelineBuilder, &ptd, maxStages);
        set_random_xpf(&pipelineBuilder, &ptd);
        set_random_state(&pipelineBuilder, &random);
        set_random_stencil(&pipelineBuilder, &random);

        sk_sp<GrDrawContext> drawContext(context->drawContext(rt));
        if (!drawContext) {
            SkDebugf("Could not allocate drawContext");
            return false;
        }

        drawContext->drawContextPriv().testingOnly_drawBatch(pipelineBuilder, batch);
    }
    // Flush everything, test passes if flush is successful(ie, no asserts are hit, no crashes)
    drawingManager->flush();

    // Validate that GrFPs work correctly without an input.
    GrSurfaceDesc rtDesc;
    rtDesc.fWidth = kRenderTargetWidth;
    rtDesc.fHeight = kRenderTargetHeight;
    rtDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    rtDesc.fConfig = kRGBA_8888_GrPixelConfig;
    sk_sp<GrRenderTarget> rt(
        context->textureProvider()->createTexture(rtDesc, SkBudgeted::kNo)->asRenderTarget());
    int fpFactoryCnt = GrProcessorTestFactory<GrFragmentProcessor>::Count();
    for (int i = 0; i < fpFactoryCnt; ++i) {
        // Since FP factories internally randomize, call each 10 times.
        for (int j = 0; j < 10; ++j) {
            SkAutoTUnref<GrDrawBatch> batch(GrRandomDrawBatch(&random, context));
            SkASSERT(batch);
            GrProcessorTestData ptd(&random, context, context->caps(), rt.get(), dummyTextures);
            GrPipelineBuilder builder;
            builder.setXPFactory(GrPorterDuffXPFactory::Create(SkXfermode::kSrc_Mode))->unref();
            builder.setRenderTarget(rt.get());
            builder.setClip(clip);

            SkAutoTUnref<const GrFragmentProcessor> fp(
                GrProcessorTestFactory<GrFragmentProcessor>::CreateIdx(i, &ptd));
            SkAutoTUnref<const GrFragmentProcessor> blockFP(
                BlockInputFragmentProcessor::Create(fp));
            builder.addColorFragmentProcessor(blockFP);

            sk_sp<GrDrawContext> drawContext(context->drawContext(rt));
            if (!drawContext) {
                SkDebugf("Could not allocate a drawcontext");
                return false;
            }

            drawContext->drawContextPriv().testingOnly_drawBatch(builder, batch);
            drawingManager->flush();
        }
    }

    return true;
}

static int get_glprograms_max_stages(GrContext* context) {
    GrGLGpu* gpu = static_cast<GrGLGpu*>(context->getGpu());
    /*
     * For the time being, we only support the test with desktop GL or for android on
     * ARM platforms
     * TODO When we run ES 3.00 GLSL in more places, test again
     */
    if (kGL_GrGLStandard == gpu->glStandard() ||
        kARM_GrGLVendor == gpu->ctxInfo().vendor()) {
        return 6;
    } else if (kTegra3_GrGLRenderer == gpu->ctxInfo().renderer() ||
               kOther_GrGLRenderer == gpu->ctxInfo().renderer()) {
        return 1;
    }
    return 0;
}

static void test_glprograms_native(skiatest::Reporter* reporter,
                                   const sk_gpu_test::ContextInfo& ctxInfo) {
    int maxStages = get_glprograms_max_stages(ctxInfo.fGrContext);
    if (maxStages == 0) {
        return;
    }
    REPORTER_ASSERT(reporter, GrDrawingManager::ProgramUnitTest(ctxInfo.fGrContext, maxStages));
}

static void test_glprograms_other_contexts(
            skiatest::Reporter* reporter,
            const sk_gpu_test::ContextInfo& ctxInfo) {
    int maxStages = get_glprograms_max_stages(ctxInfo.fGrContext);
#ifdef SK_BUILD_FOR_WIN
    // Some long shaders run out of temporary registers in the D3D compiler on ANGLE and
    // command buffer.
    maxStages = SkTMin(maxStages, 2);
#endif
    if (maxStages == 0) {
        return;
    }
    REPORTER_ASSERT(reporter, GrDrawingManager::ProgramUnitTest(ctxInfo.fGrContext, maxStages));
}

static bool is_native_gl_context_type(sk_gpu_test::GrContextFactory::ContextType type) {
    return type == sk_gpu_test::GrContextFactory::kNativeGL_ContextType;
}

static bool is_other_rendering_gl_context_type(sk_gpu_test::GrContextFactory::ContextType type) {
    return !is_native_gl_context_type(type) &&
           kOpenGL_GrBackend == sk_gpu_test::GrContextFactory::ContextTypeBackend(type) &&
           sk_gpu_test::GrContextFactory::IsRenderingContext(type);
}

DEF_GPUTEST(GLPrograms, reporter, /*factory*/) {
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
    sk_gpu_test::GrContextFactory debugFactory(opts);
    skiatest::RunWithGPUTestContexts(test_glprograms_native, &is_native_gl_context_type,
                                     reporter, &debugFactory);
    skiatest::RunWithGPUTestContexts(test_glprograms_other_contexts,
                                     &is_other_rendering_gl_context_type, reporter, &debugFactory);
}

#endif
