
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
#include "GrOptDrawState.h"
#include "GrTBackendProcessorFactory.h"
#include "GrTest.h"
#include "SkChecksum.h"
#include "SkRandom.h"
#include "Test.h"
#include "effects/GrConfigConversionEffect.h"
#include "gl/GrGLPathRendering.h"
#include "gl/GrGpuGL.h"
#include "gl/builders/GrGLProgramBuilder.h"

/*
 * A dummy processor which just tries to insert a massive key and verify that it can retrieve the
 * whole thing correctly
 */
static const uint32_t kMaxKeySize = 1024;

class GLBigKeyProcessor;

class BigKeyProcessor : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create() {
        GR_CREATE_STATIC_PROCESSOR(gBigKeyProcessor, BigKeyProcessor, ())
        return SkRef(gBigKeyProcessor);
    }

    static const char* Name() { return "Big ol' Key"; }

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendFragmentProcessorFactory<BigKeyProcessor>::getInstance();
    }

    typedef GLBigKeyProcessor GLProcessor;

private:
    BigKeyProcessor() { }
    virtual bool onIsEqual(const GrFragmentProcessor&) const SK_OVERRIDE { return true; }
    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE { }

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

class GLBigKeyProcessor : public GrGLFragmentProcessor {
public:
    GLBigKeyProcessor(const GrBackendProcessorFactory& factory, const GrProcessor&)
        : INHERITED(factory) {}

    virtual void emitCode(GrGLFPBuilder* builder,
                          const GrFragmentProcessor& fp,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) {
        for (uint32_t i = 0; i < kMaxKeySize; i++) {
            SkASSERT(key.get32(i) == i);
        }
    }

    static void GenKey(const GrProcessor& processor, const GrGLCaps&, GrProcessorKeyBuilder* b) {
        for (uint32_t i = 0; i < kMaxKeySize; i++) {
            b->add32(i);
        }
    }

private:
    typedef GrGLFragmentProcessor INHERITED;
};

/*
 * Begin test code
 */
static const int kRenderTargetHeight = 1;
static const int kRenderTargetWidth = 1;

static GrRenderTarget* random_render_target(GrContext* context,
                                            const GrCacheID& cacheId,
                                            SkRandom* random) {
    // setup render target
    GrTextureParams params;
    GrSurfaceDesc texDesc;
    texDesc.fWidth = kRenderTargetWidth;
    texDesc.fHeight = kRenderTargetHeight;
    texDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    texDesc.fConfig = kRGBA_8888_GrPixelConfig;
    texDesc.fOrigin = random->nextBool() == true ? kTopLeft_GrSurfaceOrigin :
                                                   kBottomLeft_GrSurfaceOrigin;

    SkAutoTUnref<GrTexture> texture(context->findAndRefTexture(texDesc, cacheId, &params));
    if (!texture) {
        texture.reset(context->createTexture(&params, texDesc, cacheId, 0, 0));
        if (!texture) {
            return NULL;
        }
    }
    return SkRef(texture->asRenderTarget());
}

// TODO clean this up, we have to do this to test geometry processors but there has got to be
// a better way.  In the mean time, we actually fill out these generic vertex attribs below with
// the correct vertex attribs from the GP.  We have to ensure, however, we don't try to add more
// than two attributes.  In addition, we 'pad' the below array with GPs up to 6 entries, 4 fixed
// function vertex attributes and 2 GP custom attributes.
GrVertexAttrib kGenericVertexAttribs[] = {
    { kVec2f_GrVertexAttribType, 0,   kPosition_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, 0,   kGeometryProcessor_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, 0,   kGeometryProcessor_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, 0,   kGeometryProcessor_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, 0,   kGeometryProcessor_GrVertexAttribBinding },
    { kVec2f_GrVertexAttribType, 0,   kGeometryProcessor_GrVertexAttribBinding }
};

/*
 * convert sl type to vertexattrib type, not a complete implementation, only use for debugging
 */
static GrVertexAttribType convert_sltype_to_attribtype(GrSLType type) {
    switch (type) {
        case kFloat_GrSLType:
            return kFloat_GrVertexAttribType;
        case kVec2f_GrSLType:
            return kVec2f_GrVertexAttribType;
        case kVec3f_GrSLType:
            return kVec3f_GrVertexAttribType;
        case kVec4f_GrSLType:
            return kVec4f_GrVertexAttribType;
        default:
            SkFAIL("Type isn't convertible");
            return kFloat_GrVertexAttribType;
    }
}
// end test hack

static void setup_random_ff_attribute(GrVertexAttribBinding binding, GrVertexAttribType type,
                                      SkRandom* random, int* attribIndex, int* runningStride) {
    if (random->nextBool()) {
        kGenericVertexAttribs[*attribIndex].fType = type;
        kGenericVertexAttribs[*attribIndex].fOffset = *runningStride;
        kGenericVertexAttribs[*attribIndex].fBinding = binding;
        *runningStride += GrVertexAttribTypeSize(kGenericVertexAttribs[(*attribIndex)++].fType);
    }
}

static void set_random_gp(GrContext* context,
                          const GrDrawTargetCaps& caps,
                          GrDrawState* ds,
                          SkRandom* random,
                          GrTexture* dummyTextures[]) {
    SkAutoTUnref<const GrGeometryProcessor> gp(
            GrProcessorTestFactory<GrGeometryProcessor>::CreateStage(random,
                                                                     context,
                                                                     caps,
                                                                     dummyTextures));
    SkASSERT(gp);

    // we have to set dummy vertex attributes, first we setup the fixed function attributes
    // always leave the position attribute untouched in the array
    int attribIndex = 1;
    int runningStride = GrVertexAttribTypeSize(kGenericVertexAttribs[0].fType);

    // local coords
    setup_random_ff_attribute(kLocalCoord_GrVertexAttribBinding, kVec2f_GrVertexAttribType,
                              random, &attribIndex, &runningStride);

    // color
    setup_random_ff_attribute(kColor_GrVertexAttribBinding, kVec4f_GrVertexAttribType,
                              random, &attribIndex, &runningStride);

    // coverage
    setup_random_ff_attribute(kCoverage_GrVertexAttribBinding, kUByte_GrVertexAttribType,
                              random, &attribIndex, &runningStride);

    // Update the geometry processor attributes
    const GrGeometryProcessor::VertexAttribArray& v = gp->getVertexAttribs();
    int numGPAttribs = v.count();
    SkASSERT(numGPAttribs <= GrGeometryProcessor::kMaxVertexAttribs &&
             GrGeometryProcessor::kMaxVertexAttribs == 2);

    // we actually can't overflow if kMaxVertexAttribs == 2, but GCC 4.8 wants more proof
    int maxIndex = SK_ARRAY_COUNT(kGenericVertexAttribs);
    for (int i = 0; i < numGPAttribs && i + attribIndex < maxIndex; i++) {
        kGenericVertexAttribs[i + attribIndex].fType =
                convert_sltype_to_attribtype(v[i].getType());
        kGenericVertexAttribs[i + attribIndex].fOffset = runningStride;
        kGenericVertexAttribs[i + attribIndex].fBinding = kGeometryProcessor_GrVertexAttribBinding;
        runningStride += GrVertexAttribTypeSize(kGenericVertexAttribs[i + attribIndex].fType);
    }

    // update the vertex attributes with the ds
    ds->setVertexAttribs<kGenericVertexAttribs>(attribIndex + numGPAttribs, runningStride);
    ds->setGeometryProcessor(gp);
}

static void set_random_color_coverage_stages(GrGpuGL* gpu,
                                             GrDrawState* ds,
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

        // don't add dst color reads to coverage stage
        if (s >= numColorProcs && fp->willReadDstColor()) {
            continue;
        }

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
            ds->addColorProcessor(fp);
        } else {
            ds->addCoverageProcessor(fp);
        }
        ++s;
    }
}

// There are only a few cases of random colors which interest us
enum ColorMode {
    kAllOnes_ColorMode,
    kAllZeros_ColorMode,
    kAlphaOne_ColorMode,
    kRandom_ColorMode,
    kLast_ColorMode = kRandom_ColorMode
};

static void set_random_color(GrDrawState* ds, SkRandom* random) {
    ColorMode colorMode = ColorMode(random->nextULessThan(kLast_ColorMode + 1));
    GrColor color;
    switch (colorMode) {
        case kAllOnes_ColorMode:
            color = GrColorPackRGBA(0xFF, 0xFF, 0xFF, 0xFF);
            break;
        case kAllZeros_ColorMode:
            color = GrColorPackRGBA(0, 0, 0, 0);
            break;
        case kAlphaOne_ColorMode:
            color = GrColorPackRGBA(random->nextULessThan(256),
                                    random->nextULessThan(256),
                                    random->nextULessThan(256),
                                    0xFF);
            break;
        case kRandom_ColorMode:
            uint8_t alpha = random->nextULessThan(256);
            color = GrColorPackRGBA(random->nextRangeU(0, alpha),
                                    random->nextRangeU(0, alpha),
                                    random->nextRangeU(0, alpha),
                                    alpha);
            break;
    }
    GrColorIsPMAssert(color);
    ds->setColor(color);
}

// There are only a few cases of random coverages which interest us
enum CoverageMode {
    kZero_CoverageMode,
    kFF_CoverageMode,
    kRandom_CoverageMode,
    kLast_CoverageMode = kRandom_CoverageMode
};

static void set_random_coverage(GrDrawState* ds, SkRandom* random) {
    CoverageMode coverageMode = CoverageMode(random->nextULessThan(kLast_CoverageMode + 1));
    uint8_t coverage;
    switch (coverageMode) {
        case kZero_CoverageMode:
            coverage = 0;
            break;
        case kFF_CoverageMode:
            coverage = 0xFF;
            break;
        case kRandom_CoverageMode:
            coverage = uint8_t(random->nextU());
            break;
    }
    ds->setCoverage(coverage);
}

static void set_random_hints(GrDrawState* ds, SkRandom* random) {
    for (int i = 1; i <= GrDrawState::kLast_Hint; i <<= 1) {
        ds->setHint(GrDrawState::Hints(i), random->nextBool());
    }
}

static void set_random_state(GrDrawState* ds, SkRandom* random) {
    int state = 0;
    for (int i = 1; i <= GrDrawState::kLast_StateBit; i <<= 1) {
        state |= random->nextBool() * i;
    }
    ds->enableState(state);
}

// this function will randomly pick non-self referencing blend modes
static void set_random_blend_func(GrDrawState* ds, SkRandom* random) {
    GrBlendCoeff src;
    do {
        src = GrBlendCoeff(random->nextRangeU(kFirstPublicGrBlendCoeff, kLastPublicGrBlendCoeff));
    } while (GrBlendCoeffRefsSrc(src));

    GrBlendCoeff dst;
    do {
        dst = GrBlendCoeff(random->nextRangeU(kFirstPublicGrBlendCoeff, kLastPublicGrBlendCoeff));
    } while (GrBlendCoeffRefsDst(dst));

    ds->setBlendFunc(src, dst);
}

// right now, the only thing we seem to care about in drawState's stencil is 'doesWrite()'
static void set_random_stencil(GrDrawState* ds, SkRandom* random) {
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
        ds->setStencil(kDoesWriteStencil);
    } else {
        ds->setStencil(kDoesNotWriteStencil);
    }
}

bool GrDrawTarget::programUnitTest(int maxStages) {
    GrGpuGL* gpu = static_cast<GrGpuGL*>(fContext->getGpu());
    // setup dummy textures
    GrSurfaceDesc dummyDesc;
    dummyDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    dummyDesc.fConfig = kSkia8888_GrPixelConfig;
    dummyDesc.fWidth = 34;
    dummyDesc.fHeight = 18;
    SkAutoTUnref<GrTexture> dummyTexture1(gpu->createTexture(dummyDesc, NULL, 0));
    dummyDesc.fFlags = kNone_GrSurfaceFlags;
    dummyDesc.fConfig = kAlpha_8_GrPixelConfig;
    dummyDesc.fWidth = 16;
    dummyDesc.fHeight = 22;
    SkAutoTUnref<GrTexture> dummyTexture2(gpu->createTexture(dummyDesc, NULL, 0));

    if (!dummyTexture1 || ! dummyTexture2) {
        SkDebugf("Could not allocate dummy textures");
        return false;
    }

    GrTexture* dummyTextures[] = {dummyTexture1.get(), dummyTexture2.get()};

    // Setup texture cache id key
    const GrCacheID::Domain glProgramsDomain = GrCacheID::GenerateDomain();
    GrCacheID::Key key;
    memset(&key, 0, sizeof(key));
    key.fData32[0] = kRenderTargetWidth;
    key.fData32[1] = kRenderTargetHeight;
    GrCacheID glProgramsCacheID(glProgramsDomain, key);

    // setup clip
    SkRect screen = SkRect::MakeWH(SkIntToScalar(kRenderTargetWidth),
                                   SkIntToScalar(kRenderTargetHeight));

    SkClipStack stack;
    stack.clipDevRect(screen, SkRegion::kReplace_Op, false);

    // wrap the SkClipStack in a GrClipData
    GrClipData clipData;
    clipData.fClipStack = &stack;
    this->setClip(&clipData);

    SkRandom random;
    static const int NUM_TESTS = 512;
    for (int t = 0; t < NUM_TESTS;) {
        // setup random render target(can fail)
        SkAutoTUnref<GrRenderTarget> rt(random_render_target(fContext, glProgramsCacheID, &random));
        if (!rt.get()) {
            SkDebugf("Could not allocate render target");
            return false;
        }

        GrDrawState ds;
        ds.setRenderTarget(rt.get());

        // if path rendering we have to setup a couple of things like the draw type
        bool usePathRendering = gpu->glCaps().pathRenderingSupport() && random.nextBool();

        GrGpu::DrawType drawType = usePathRendering ? GrGpu::kDrawPath_DrawType :
                                                      GrGpu::kDrawPoints_DrawType;

        // twiddle drawstate knobs randomly
        bool hasGeometryProcessor = !usePathRendering;
        if (hasGeometryProcessor) {
            set_random_gp(fContext, gpu->glCaps(), &ds, &random, dummyTextures);
        }
        set_random_color_coverage_stages(gpu,
                                         &ds,
                                         maxStages - hasGeometryProcessor,
                                         usePathRendering,
                                         &random,
                                         dummyTextures);
        set_random_color(&ds, &random);
        set_random_coverage(&ds, &random);
        set_random_hints(&ds, &random);
        set_random_state(&ds, &random);
        set_random_blend_func(&ds, &random);
        set_random_stencil(&ds, &random);

        GrDeviceCoordTexture dstCopy;

        if (!this->setupDstReadIfNecessary(&ds, &dstCopy, NULL)) {
            SkDebugf("Couldn't setup dst read texture");
            return false;
        }

        // create optimized draw state, setup readDst texture if required, and build a descriptor
        // and program.  ODS creation can fail, so we have to check
        SkAutoTUnref<GrOptDrawState> ods(GrOptDrawState::Create(ds,
                                                                gpu,
                                                                &dstCopy,
                                                                drawType));
        if (!ods.get()) {
            continue;
        }
        SkAutoTUnref<GrGLProgram> program(GrGLProgramBuilder::CreateProgram(*ods, drawType, gpu));
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
            GrGpuGL* gpu = static_cast<GrGpuGL*>(context->getGpu());

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
                maxStages = 3;
            }
#endif
            GrTestTarget target;
            context->getTestTarget(&target);
            REPORTER_ASSERT(reporter, target.target()->programUnitTest(maxStages));
        }
    }
}

#endif
