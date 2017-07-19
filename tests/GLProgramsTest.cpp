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
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrDrawOpTest.h"
#include "GrDrawingManager.h"
#include "GrPipeline.h"
#include "GrRenderTargetContextPriv.h"
#include "GrTest.h"
#include "GrXferProcessor.h"
#include "SkChecksum.h"
#include "SkRandom.h"
#include "Test.h"

#include "ops/GrDrawOp.h"

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

    static void GenKey(const GrProcessor&, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
        for (uint32_t i = 0; i < kMaxKeySize; i++) {
            b->add32(i);
        }
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

class BigKeyProcessor : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make() {
        return sk_sp<GrFragmentProcessor>(new BigKeyProcessor);
    }

    const char* name() const override { return "Big Ole Key"; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLBigKeyProcessor;
    }

private:
    BigKeyProcessor() : INHERITED(kNone_OptimizationFlags) { this->initClassID<BigKeyProcessor>(); }
    virtual void onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GLBigKeyProcessor::GenKey(*this, caps, b);
    }
    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(BigKeyProcessor);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> BigKeyProcessor::TestCreate(GrProcessorTestData*) {
    return BigKeyProcessor::Make();
}
#endif

//////////////////////////////////////////////////////////////////////////////

class BlockInputFragmentProcessor : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrFragmentProcessor> fp) {
        return sk_sp<GrFragmentProcessor>(new BlockInputFragmentProcessor(fp));
    }

    const char* name() const override { return "Block Input"; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override { return new GLFP; }

private:
    class GLFP : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            this->emitChild(0, args);
        }

    private:
        typedef GrGLSLFragmentProcessor INHERITED;
    };

    BlockInputFragmentProcessor(sk_sp<GrFragmentProcessor> child)
            : INHERITED(kNone_OptimizationFlags) {
        this->initClassID<BlockInputFragmentProcessor>();
        this->registerChildProcessor(std::move(child));
    }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {}

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    typedef GrFragmentProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

/*
 * Begin test code
 */
static const int kRenderTargetHeight = 1;
static const int kRenderTargetWidth = 1;

static sk_sp<GrRenderTargetContext> random_render_target_context(GrContext* context,
                                                                 SkRandom* random,
                                                                 const GrCaps* caps) {
    GrSurfaceOrigin origin = random->nextBool() ? kTopLeft_GrSurfaceOrigin
                                                : kBottomLeft_GrSurfaceOrigin;
    int sampleCnt = random->nextBool() ? caps->getSampleCount(4, kRGBA_8888_GrPixelConfig) : 0;

    sk_sp<GrRenderTargetContext> renderTargetContext(context->makeDeferredRenderTargetContext(
                                                                           SkBackingFit::kExact,
                                                                           kRenderTargetWidth,
                                                                           kRenderTargetHeight,
                                                                           kRGBA_8888_GrPixelConfig,
                                                                           nullptr,
                                                                           sampleCnt,
                                                                           origin));
    return renderTargetContext;
}

#if GR_TEST_UTILS
static void set_random_xpf(GrPaint* paint, GrProcessorTestData* d) {
    paint->setXPFactory(GrXPFactoryTestFactory::Get(d));
}

static sk_sp<GrFragmentProcessor> create_random_proc_tree(GrProcessorTestData* d,
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
            sk_sp<GrFragmentProcessor> fp;
            while (true) {
                fp = GrProcessorTestFactory<GrFragmentProcessor>::Make(d);
                SkASSERT(fp);
                if (0 == fp->numChildProcessors()) {
                    break;
                }
            }
            return fp;
        }
    }
    // If we didn't terminate, choose either the left or right subtree to fulfill
    // the minLevels requirement of this tree; the other child can have as few levels as it wants.
    // Also choose a random xfer mode.
    if (minLevels > 1) {
        --minLevels;
    }
    sk_sp<GrFragmentProcessor> minLevelsChild(create_random_proc_tree(d, minLevels, maxLevels - 1));
    sk_sp<GrFragmentProcessor> otherChild(create_random_proc_tree(d, 1, maxLevels - 1));
    SkBlendMode mode = static_cast<SkBlendMode>(d->fRandom->nextRangeU(0,
                                                               (int)SkBlendMode::kLastMode));
    sk_sp<GrFragmentProcessor> fp;
    if (d->fRandom->nextF() < 0.5f) {
        fp = GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(minLevelsChild),
                                                                std::move(otherChild), mode);
        SkASSERT(fp);
    } else {
        fp = GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(otherChild),
                                                                std::move(minLevelsChild), mode);
        SkASSERT(fp);
    }
    return fp;
}

static void set_random_color_coverage_stages(GrPaint* paint,
                                             GrProcessorTestData* d,
                                             int maxStages,
                                             int maxTreeLevels) {
    // Randomly choose to either create a linear pipeline of procs or create one proc tree
    const float procTreeProbability = 0.5f;
    if (d->fRandom->nextF() < procTreeProbability) {
        sk_sp<GrFragmentProcessor> fp(create_random_proc_tree(d, 2, maxTreeLevels));
        if (fp) {
            paint->addColorFragmentProcessor(std::move(fp));
        }
    } else {
        int numProcs = d->fRandom->nextULessThan(maxStages + 1);
        int numColorProcs = d->fRandom->nextULessThan(numProcs + 1);

        for (int s = 0; s < numProcs;) {
            sk_sp<GrFragmentProcessor> fp(GrProcessorTestFactory<GrFragmentProcessor>::Make(d));
            SkASSERT(fp);

            // finally add the stage to the correct pipeline in the drawstate
            if (s < numColorProcs) {
                paint->addColorFragmentProcessor(std::move(fp));
            } else {
                paint->addCoverageFragmentProcessor(std::move(fp));
            }
            ++s;
        }
    }
}

static void set_random_state(GrPaint* paint, SkRandom* random) {
    if (random->nextBool()) {
        paint->setDisableOutputConversionToSRGB(true);
    }
    if (random->nextBool()) {
        paint->setAllowSRGBInputs(true);
    }
}

#endif

#if !GR_TEST_UTILS
bool GrDrawingManager::ProgramUnitTest(GrContext*, int) { return true; }
#else
bool GrDrawingManager::ProgramUnitTest(GrContext* context, int maxStages, int maxLevels) {
    GrDrawingManager* drawingManager = context->contextPriv().drawingManager();

    sk_sp<GrTextureProxy> proxies[2];

    // setup dummy textures
    GrSurfaceDesc dummyDesc;
    dummyDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    dummyDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    dummyDesc.fConfig = kRGBA_8888_GrPixelConfig;
    dummyDesc.fWidth = 34;
    dummyDesc.fHeight = 18;
    proxies[0] = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                              dummyDesc, SkBudgeted::kNo, nullptr, 0);
    dummyDesc.fFlags = kNone_GrSurfaceFlags;
    dummyDesc.fOrigin = kTopLeft_GrSurfaceOrigin;
    dummyDesc.fConfig = kAlpha_8_GrPixelConfig;
    dummyDesc.fWidth = 16;
    dummyDesc.fHeight = 22;
    proxies[1] = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                              dummyDesc, SkBudgeted::kNo, nullptr, 0);

    if (!proxies[0] || !proxies[1]) {
        SkDebugf("Could not allocate dummy textures");
        return false;
    }

    // dummy scissor state
    GrScissorState scissor;

    SkRandom random;
    static const int NUM_TESTS = 1024;
    for (int t = 0; t < NUM_TESTS; t++) {
        // setup random render target(can fail)
        sk_sp<GrRenderTargetContext> renderTargetContext(random_render_target_context(
            context, &random, context->caps()));
        if (!renderTargetContext) {
            SkDebugf("Could not allocate renderTargetContext");
            return false;
        }

        GrPaint paint;
        GrProcessorTestData ptd(&random, context, renderTargetContext.get(), proxies);
        set_random_color_coverage_stages(&paint, &ptd, maxStages, maxLevels);
        set_random_xpf(&paint, &ptd);
        set_random_state(&paint, &random);
        GrDrawRandomOp(&random, renderTargetContext.get(), std::move(paint));
    }
    // Flush everything, test passes if flush is successful(ie, no asserts are hit, no crashes)
    drawingManager->flush(nullptr);

    // Validate that GrFPs work correctly without an input.
    sk_sp<GrRenderTargetContext> renderTargetContext(context->makeDeferredRenderTargetContext(
                                                                           SkBackingFit::kExact,
                                                                           kRenderTargetWidth,
                                                                           kRenderTargetHeight,
                                                                           kRGBA_8888_GrPixelConfig,
                                                                           nullptr));
    if (!renderTargetContext) {
        SkDebugf("Could not allocate a renderTargetContext");
        return false;
    }

    int fpFactoryCnt = GrProcessorTestFactory<GrFragmentProcessor>::Count();
    for (int i = 0; i < fpFactoryCnt; ++i) {
        // Since FP factories internally randomize, call each 10 times.
        for (int j = 0; j < 10; ++j) {
            GrProcessorTestData ptd(&random, context, renderTargetContext.get(), proxies);

            GrPaint paint;
            paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
            sk_sp<GrFragmentProcessor> fp(
                GrProcessorTestFactory<GrFragmentProcessor>::MakeIdx(i, &ptd));
            sk_sp<GrFragmentProcessor> blockFP(
                BlockInputFragmentProcessor::Make(std::move(fp)));
            paint.addColorFragmentProcessor(std::move(blockFP));
            GrDrawRandomOp(&random, renderTargetContext.get(), std::move(paint));
            drawingManager->flush(nullptr);
        }
    }

    return true;
}
#endif

static int get_glprograms_max_stages(const sk_gpu_test::ContextInfo& ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGLGpu* gpu = static_cast<GrGLGpu*>(context->getGpu());
    int maxStages = 6;
    if (kGLES_GrGLStandard == gpu->glStandard()) {
    // We've had issues with driver crashes and HW limits being exceeded with many effects on
    // Android devices. We have passes on ARM devices with the default number of stages.
    // TODO When we run ES 3.00 GLSL in more places, test again
#ifdef SK_BUILD_FOR_ANDROID
        if (kARM_GrGLVendor != gpu->ctxInfo().vendor()) {
            maxStages = 1;
        }
#endif
    // On iOS we can exceed the maximum number of varyings. http://skbug.com/6627.
#ifdef SK_BUILD_FOR_IOS
        maxStages = 3;
#endif
    }
    if (ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_D3D9_ES2_ContextType ||
        ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_D3D11_ES2_ContextType) {
        // On Angle D3D we will hit a limit of out variables if we use too many stages.
        maxStages = 3;
    }
    return maxStages;
}

static int get_glprograms_max_levels(const sk_gpu_test::ContextInfo& ctxInfo) {
    // A full tree with 5 levels (31 nodes) may cause a program that exceeds shader limits
    // (e.g. uniform or varying limits); maxTreeLevels should be a number from 1 to 4 inclusive.
    int maxTreeLevels = 4;
    // On iOS we can exceed the maximum number of varyings. http://skbug.com/6627.
#ifdef SK_BUILD_FOR_IOS
    maxTreeLevels = 2;
#endif
    if (ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_D3D9_ES2_ContextType ||
        ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_D3D11_ES2_ContextType) {
        // On Angle D3D we will hit a limit of out variables if we use too many stages.
        maxTreeLevels = 2;
    }
    return maxTreeLevels;
}

static void test_glprograms(skiatest::Reporter* reporter, const sk_gpu_test::ContextInfo& ctxInfo) {
    int maxStages = get_glprograms_max_stages(ctxInfo);
    if (maxStages == 0) {
        return;
    }
    int maxLevels = get_glprograms_max_levels(ctxInfo);
    if (maxLevels == 0) {
        return;
    }

    // Disable this test on ANGLE D3D9 configurations. We keep hitting a D3D compiler bug.
    // See skbug.com/6842 and anglebug.com/2098
    if (sk_gpu_test::GrContextFactory::kANGLE_D3D9_ES2_ContextType == ctxInfo.type()) {
        return;
    }

    REPORTER_ASSERT(reporter, GrDrawingManager::ProgramUnitTest(ctxInfo.grContext(), maxStages,
                                                                maxLevels));
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
    skiatest::RunWithGPUTestContexts(test_glprograms, &skiatest::IsRenderingGLContextType, reporter,
                                     &debugFactory);
}

#endif
