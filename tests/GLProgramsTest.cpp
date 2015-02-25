
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

#include "GrContextFactory.h"
#include "GrInvariantOutput.h"
#include "GrPipeline.h"
#include "GrTest.h"
#include "GrXferProcessor.h"
#include "SkChecksum.h"
#include "SkRandom.h"
#include "Test.h"
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

    virtual void emitCode(GrGLFPBuilder* builder,
                          const GrFragmentProcessor& fp,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) {}

    static void GenKey(const GrProcessor& processor, const GrGLCaps&, GrProcessorKeyBuilder* b) {
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

    const char* name() const SK_OVERRIDE { return "Big Ole Key"; }

    virtual void getGLProcessorKey(const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE {
        GLBigKeyProcessor::GenKey(*this, caps, b);
    }

    GrGLFragmentProcessor* createGLInstance() const SK_OVERRIDE {
        return SkNEW_ARGS(GLBigKeyProcessor, (*this));
    }

private:
    BigKeyProcessor() {
        this->initClassID<BigKeyProcessor>();
    }
    bool onIsEqual(const GrFragmentProcessor&) const SK_OVERRIDE { return true; }
    void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE { }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(BigKeyProcessor);

GrFragmentProcessor* BigKeyProcessor::TestCreate(SkRandom*,
                                                 GrContext*,
                                                 const GrDrawTargetCaps&,
                                                 GrTexture*[]) {
    return BigKeyProcessor::Create();
}

/*
 * Begin test code
 */
static const int kRenderTargetHeight = 1;
static const int kRenderTargetWidth = 1;

static GrRenderTarget* random_render_target(GrContext* context, SkRandom* random) {
    // setup render target
    GrTextureParams params;
    GrSurfaceDesc texDesc;
    texDesc.fWidth = kRenderTargetWidth;
    texDesc.fHeight = kRenderTargetHeight;
    texDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    texDesc.fConfig = kRGBA_8888_GrPixelConfig;
    texDesc.fOrigin = random->nextBool() == true ? kTopLeft_GrSurfaceOrigin :
                                                   kBottomLeft_GrSurfaceOrigin;
    GrUniqueKey key;
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(&key, kDomain, 1);
    builder[0] = texDesc.fOrigin;
    builder.finish();

    GrTexture* texture = context->findAndRefCachedTexture(key);
    if (!texture) {
        texture = context->createTexture(texDesc, true);
        if (texture) {
            context->addResourceToCache(key, texture);
        }
    }
    return texture ? texture->asRenderTarget() : NULL;
}

static void set_random_xpf(GrContext* context, const GrDrawTargetCaps& caps,
                           GrPipelineBuilder* pipelineBuilder, SkRandom* random,
                           GrTexture* dummyTextures[]) {
    SkAutoTUnref<const GrXPFactory> xpf(
        GrProcessorTestFactory<GrXPFactory>::CreateStage(random, context, caps, dummyTextures));
    SkASSERT(xpf);
    pipelineBuilder->setXPFactory(xpf.get());
}

static const GrGeometryProcessor* get_random_gp(GrContext* context,
                                                const GrDrawTargetCaps& caps,
                                                SkRandom* random,
                                                GrTexture* dummyTextures[]) {
    return GrProcessorTestFactory<GrGeometryProcessor>::CreateStage(random,
                                                                    context,
                                                                    caps,
                                                                    dummyTextures);
}

static void set_random_color_coverage_stages(GrGLGpu* gpu,
                                             GrPipelineBuilder* pipelineBuilder,
                                             int maxStages,
                                             bool usePathRendering,
                                             SkRandom* random,
                                             GrTexture* dummyTextures[]) {
    int numProcs = random->nextULessThan(maxStages + 1);
    int numColorProcs = random->nextULessThan(numProcs + 1);

    int currTextureCoordSet = 0;
    for (int s = 0; s < numProcs;) {
        SkAutoTUnref<const GrFragmentProcessor> fp(
                GrProcessorTestFactory<GrFragmentProcessor>::CreateStage(random,
                                                                         gpu->getContext(),
                                                                         *gpu->caps(),
                                                                         dummyTextures));
        SkASSERT(fp);

        // If adding this effect would exceed the max texture coord set count then generate a
        // new random effect.
        if (usePathRendering && gpu->glPathRendering()->texturingMode() ==
                                GrGLPathRendering::FixedFunction_TexturingMode) {;
            int numTransforms = fp->numTransforms();
            if (currTextureCoordSet + numTransforms >
                gpu->glCaps().maxFixedFunctionTextureCoords()) {
                continue;
            }
            currTextureCoordSet += numTransforms;
        }

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
    for (int i = 1; i <= GrPipelineBuilder::kLast_StateBit; i <<= 1) {
        state |= random->nextBool() * i;
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

bool GrDrawTarget::programUnitTest(int maxStages) {
    GrGLGpu* gpu = static_cast<GrGLGpu*>(fContext->getGpu());
    // setup dummy textures
    GrSurfaceDesc dummyDesc;
    dummyDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    dummyDesc.fConfig = kSkia8888_GrPixelConfig;
    dummyDesc.fWidth = 34;
    dummyDesc.fHeight = 18;
    SkAutoTUnref<GrTexture> dummyTexture1(gpu->createTexture(dummyDesc, false, NULL, 0));
    dummyDesc.fFlags = kNone_GrSurfaceFlags;
    dummyDesc.fConfig = kAlpha_8_GrPixelConfig;
    dummyDesc.fWidth = 16;
    dummyDesc.fHeight = 22;
    SkAutoTUnref<GrTexture> dummyTexture2(gpu->createTexture(dummyDesc, false, NULL, 0));

    if (!dummyTexture1 || ! dummyTexture2) {
        SkDebugf("Could not allocate dummy textures");
        return false;
    }

    GrTexture* dummyTextures[] = {dummyTexture1.get(), dummyTexture2.get()};

    // dummy scissor state
    GrScissorState scissor;

    // setup clip
    SkRect screen = SkRect::MakeWH(SkIntToScalar(kRenderTargetWidth),
                                   SkIntToScalar(kRenderTargetHeight));

    SkClipStack stack;
    stack.clipDevRect(screen, SkRegion::kReplace_Op, false);

    // wrap the SkClipStack in a GrClip
    GrClip clip;
    clip.setClipStack(&stack);

    SkRandom random;
    static const int NUM_TESTS = 512;
    for (int t = 0; t < NUM_TESTS;) {
        // setup random render target(can fail)
        SkAutoTUnref<GrRenderTarget> rt(random_render_target(fContext, &random));
        if (!rt.get()) {
            SkDebugf("Could not allocate render target");
            return false;
        }

        GrPipelineBuilder pipelineBuilder;
        pipelineBuilder.setRenderTarget(rt.get());
        pipelineBuilder.setClip(clip);

        // if path rendering we have to setup a couple of things like the draw type
        bool usePathRendering = gpu->glCaps().pathRenderingSupport() && random.nextBool();

        // twiddle drawstate knobs randomly
        bool hasGeometryProcessor = !usePathRendering;
        SkAutoTUnref<const GrGeometryProcessor> gp;
        SkAutoTUnref<const GrPathProcessor> pathProc;
        if (hasGeometryProcessor) {
            gp.reset(get_random_gp(fContext, gpu->glCaps(), &random, dummyTextures));
        } else {
            pathProc.reset(GrPathProcessor::Create(GrColor_WHITE));
        }
        set_random_color_coverage_stages(gpu,
                                         &pipelineBuilder,
                                         maxStages - hasGeometryProcessor,
                                         usePathRendering,
                                         &random,
                                         dummyTextures);

        // creates a random xfer processor factory on the draw state 
        set_random_xpf(fContext, gpu->glCaps(), &pipelineBuilder, &random, dummyTextures);

        set_random_state(&pipelineBuilder, &random);
        set_random_stencil(&pipelineBuilder, &random);

        GrDeviceCoordTexture dstCopy;

        const GrPrimitiveProcessor* primProc;
        if (hasGeometryProcessor) {
            primProc = gp.get();
        } else {
            primProc = pathProc.get();
        }

        const GrProcOptInfo& colorPOI = pipelineBuilder.colorProcInfo(primProc);
        const GrProcOptInfo& coveragePOI = pipelineBuilder.coverageProcInfo(primProc);

        if (!this->setupDstReadIfNecessary(pipelineBuilder, colorPOI, coveragePOI, &dstCopy,
                                           NULL)) {
            SkDebugf("Couldn't setup dst read texture");
            return false;
        }

        // create optimized draw state, setup readDst texture if required, and build a descriptor
        // and program.  ODS creation can fail, so we have to check
        GrPipeline pipeline(pipelineBuilder, colorPOI, coveragePOI,
                            *gpu->caps(), scissor, &dstCopy);
        if (pipeline.mustSkip()) {
            continue;
        }
        GrBatchTracker bt;
        primProc->initBatchTracker(&bt, pipeline.getInitBatchTracker());

        GrProgramDesc desc;
        gpu->buildProgramDesc(&desc, *primProc, pipeline, bt);

        GrGpu::DrawArgs args(primProc, &pipeline, &desc, &bt);
        SkAutoTUnref<GrGLProgram> program(GrGLProgramBuilder::CreateProgram(args, gpu));

        if (NULL == program.get()) {
            SkDebugf("Failed to create program!");
            return false;
        }

        // because occasionally optimized drawstate creation will fail for valid reasons, we only
        // want to increment on success
        ++t;
    }
    return true;
}

DEF_GPUTEST(GLPrograms, reporter, factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(type));
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
            REPORTER_ASSERT(reporter, target.target()->programUnitTest(maxStages));
        }
    }
}

#endif
