/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include <array>
#include <memory>
#include <vector>
#include "include/core/SkBitmap.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/GrResourceKey.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

#if 0
#include "tools/ToolUtils.h"
#define WRITE_PNG_CONTEXT_TYPE kANGLE_D3D11_ES3_ContextType
#endif

GR_DECLARE_STATIC_UNIQUE_KEY(gIndexBufferKey);

static constexpr int kBoxSize = 2;
static constexpr int kBoxCountY = 8;
static constexpr int kBoxCountX = 8;
static constexpr int kBoxCount = kBoxCountY * kBoxCountX;

static constexpr int kImageWidth = kBoxCountY * kBoxSize;
static constexpr int kImageHeight = kBoxCountX * kBoxSize;

static constexpr int kIndexPatternRepeatCount = 3;
constexpr uint16_t kIndexPattern[6] = {0, 1, 2, 1, 2, 3};


class DrawMeshHelper {
public:
    DrawMeshHelper(GrOpFlushState* state) : fState(state) {}

    sk_sp<const GrBuffer> getIndexBuffer();

    sk_sp<const GrBuffer> makeIndexBuffer(const uint16_t[], int count);

    template<typename T> sk_sp<const GrBuffer> makeVertexBuffer(const SkTArray<T>& data) {
        return this->makeVertexBuffer(data.begin(), data.count());
    }
    template<typename T> sk_sp<const GrBuffer> makeVertexBuffer(const std::vector<T>& data) {
        return this->makeVertexBuffer(data.data(), data.size());
    }
    template<typename T> sk_sp<const GrBuffer> makeVertexBuffer(const T* data, int count);

    GrMeshDrawOp::Target* target() { return fState; }

    sk_sp<const GrBuffer> fIndexBuffer;
    sk_sp<const GrBuffer> fIndexBuffer2;
    sk_sp<const GrBuffer> fInstBuffer;
    sk_sp<const GrBuffer> fVertBuffer;
    sk_sp<const GrBuffer> fVertBuffer2;
    sk_sp<const GrBuffer> fDrawIndirectBuffer;
    size_t fDrawIndirectBufferOffset;

    GrOpsRenderPass* bindPipeline(GrPrimitiveType, bool isInstanced, bool hasVertexBuffer);

private:
    GrOpFlushState* fState;
};

struct Box {
    float fX, fY;
    GrColor fColor;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This is a GPU-backend specific test. It tries to test all possible usecases of
 * GrOpsRenderPass::draw*. The test works by drawing checkerboards of colored boxes, reading back
 * the pixels, and comparing with expected results. The boxes are drawn on integer boundaries and
 * the (opaque) colors are chosen from the set (r,g,b) = (0,255)^3, so the GPU renderings ought to
 * produce exact matches.
 */

static void run_test(GrDirectContext*, const char* testName, skiatest::Reporter*,
                     const std::unique_ptr<GrSurfaceDrawContext>&, const SkBitmap& gold,
                     std::function<void(DrawMeshHelper*)> prepareFn,
                     std::function<void(DrawMeshHelper*)> executeFn);

#ifdef WRITE_PNG_CONTEXT_TYPE
static bool IsContextTypeForOutputPNGs(skiatest::GrContextFactoryContextType type) {
    return type == skiatest::GrContextFactoryContextType::WRITE_PNG_CONTEXT_TYPE;
}
DEF_GPUTEST_FOR_CONTEXTS(GrMeshTest, IsContextTypeForOutputPNGs, reporter, ctxInfo, nullptr) {
#else
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrMeshTest, reporter, ctxInfo) {
#endif
    auto dContext = ctxInfo.directContext();

    auto rtc = GrSurfaceDrawContext::Make(
            dContext, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact,
            {kImageWidth, kImageHeight});
    if (!rtc) {
        ERRORF(reporter, "could not create render target context.");
        return;
    }

    SkTArray<Box> boxes;
    SkTArray<std::array<Box, 4>> vertexData;
    SkBitmap gold;

    // ---- setup ----------

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    gold.allocN32Pixels(kImageWidth, kImageHeight);

    SkCanvas goldCanvas(gold);

    for (int y = 0; y < kBoxCountY; ++y) {
        for (int x = 0; x < kBoxCountX; ++x) {
            int c = y + x;
            int rgb[3] = {-(c & 1) & 0xff, -((c >> 1) & 1) & 0xff, -((c >> 2) & 1) & 0xff};

            const Box box = boxes.push_back() = {
                    float(x * kBoxSize),
                    float(y * kBoxSize),
                    GrColorPackRGBA(rgb[0], rgb[1], rgb[2], 255)
            };

            std::array<Box, 4>& boxVertices = vertexData.push_back();
            for (int i = 0; i < 4; ++i) {
                boxVertices[i] = {
                        box.fX + (i / 2) * kBoxSize,
                        box.fY + (i % 2) * kBoxSize,
                        box.fColor
                };
            }

            paint.setARGB(255, rgb[0], rgb[1], rgb[2]);
            goldCanvas.drawRect(SkRect::MakeXYWH(box.fX, box.fY, kBoxSize, kBoxSize), paint);
        }
    }

    // ---- tests ----------

#define VALIDATE(buff)                           \
    do {                                         \
        if (!buff) {                             \
            ERRORF(reporter, #buff " is null."); \
            return;                              \
        }                                        \
    } while (0)

    run_test(dContext, "draw", reporter, rtc, gold,
             [&](DrawMeshHelper* helper) {
                 SkTArray<Box> expandedVertexData;
                 for (int i = 0; i < kBoxCount; ++i) {
                     for (int j = 0; j < 6; ++j) {
                         expandedVertexData.push_back(vertexData[i][kIndexPattern[j]]);
                     }
                 }

                 // Draw boxes one line at a time to exercise base vertex.
                 helper->fVertBuffer = helper->makeVertexBuffer(expandedVertexData);
                 VALIDATE(helper->fVertBuffer);
             },
             [&](DrawMeshHelper* helper) {
                 for (int y = 0; y < kBoxCountY; ++y) {
                     auto pass = helper->bindPipeline(GrPrimitiveType::kTriangles, false, true);
                     pass->bindBuffers(nullptr, nullptr, helper->fVertBuffer);
                     pass->draw(kBoxCountX * 6, y * kBoxCountX * 6);
                 }
             });

    run_test(dContext, "drawIndexed", reporter, rtc, gold,
             [&](DrawMeshHelper* helper) {
                helper->fIndexBuffer = helper->getIndexBuffer();
                VALIDATE(helper->fIndexBuffer);
                helper->fVertBuffer = helper->makeVertexBuffer(vertexData);
                VALIDATE(helper->fVertBuffer);
             },
             [&](DrawMeshHelper* helper) {
                int baseRepetition = 0;
                int i = 0;
                // Start at various repetitions within the patterned index buffer to exercise base
                // index.
                while (i < kBoxCount) {
                    static_assert(kIndexPatternRepeatCount >= 3);
                    int repetitionCount = std::min(3 - baseRepetition, kBoxCount - i);

                    auto pass = helper->bindPipeline(GrPrimitiveType::kTriangles, false, true);
                    pass->bindBuffers(helper->fIndexBuffer, nullptr, helper->fVertBuffer);
                    pass->drawIndexed(repetitionCount * 6, baseRepetition * 6, baseRepetition * 4,
                                      (baseRepetition + repetitionCount) * 4 - 1,
                                      (i - baseRepetition) * 4);

                    baseRepetition = (baseRepetition + 1) % 3;
                    i += repetitionCount;
                }
            });

    run_test(dContext, "drawIndexPattern", reporter, rtc, gold,
             [&](DrawMeshHelper* helper) {
                 helper->fIndexBuffer = helper->getIndexBuffer();
                 VALIDATE(helper->fIndexBuffer);
                 helper->fVertBuffer = helper->makeVertexBuffer(vertexData);
                 VALIDATE(helper->fVertBuffer);
             },
             [&](DrawMeshHelper* helper) {
                // Draw boxes one line at a time to exercise base vertex. drawIndexPattern does
                // not support a base index.
                for (int y = 0; y < kBoxCountY; ++y) {
                    auto pass = helper->bindPipeline(GrPrimitiveType::kTriangles, false, true);
                    pass->bindBuffers(helper->fIndexBuffer, nullptr, helper->fVertBuffer);
                    pass->drawIndexPattern(6, kBoxCountX, kIndexPatternRepeatCount, 4,
                                           y * kBoxCountX * 4);

                }
             });

    for (bool indexed : {false, true}) {
        if (!dContext->priv().caps()->drawInstancedSupport()) {
            break;
        }

        run_test(dContext, indexed ? "drawIndexedInstanced" : "drawInstanced",
                 reporter, rtc, gold,
                 [&](DrawMeshHelper* helper) {
                     helper->fIndexBuffer = indexed ? helper->getIndexBuffer() : nullptr;
                     SkTArray<uint16_t> baseIndexData;
                     baseIndexData.push_back(kBoxCountX/2 * 6); // for testing base index.
                     for (int i = 0; i < 6; ++i) {
                         baseIndexData.push_back(kIndexPattern[i]);
                     }
                     helper->fIndexBuffer2 = helper->makeIndexBuffer(baseIndexData.begin(),
                                                                     baseIndexData.count());
                     helper->fInstBuffer = helper->makeVertexBuffer(boxes);
                     VALIDATE(helper->fInstBuffer);
                     helper->fVertBuffer =
                             helper->makeVertexBuffer(std::vector<float>{0,0, 0,1, 1,0, 1,1});
                     VALIDATE(helper->fVertBuffer);
                     helper->fVertBuffer2 = helper->makeVertexBuffer( // for testing base vertex.
                         std::vector<float>{-1,-1, -1,-1, 0,0, 0,1, 1,0, 1,1});
                     VALIDATE(helper->fVertBuffer2);
                 },
                 [&](DrawMeshHelper* helper) {
                     // Draw boxes one line at a time to exercise base instance, base vertex, and
                     // null vertex buffer.
                     for (int y = 0; y < kBoxCountY; ++y) {
                         sk_sp<const GrBuffer> vertexBuffer;
                         int baseVertex = 0;
                         switch (y % 3) {
                             case 0:
                                 if (dContext->priv().caps()->shaderCaps()->vertexIDSupport()) {
                                     break;
                                 }
                                 [[fallthrough]];
                             case 1:
                                 vertexBuffer = helper->fVertBuffer;
                                 break;
                             case 2:
                                 vertexBuffer = helper->fVertBuffer2;
                                 baseVertex = 2;
                                 break;
                         }

                         GrPrimitiveType primitiveType = indexed ? GrPrimitiveType::kTriangles
                                                                 : GrPrimitiveType::kTriangleStrip;
                         auto pass = helper->bindPipeline(primitiveType, true,
                                                          SkToBool(vertexBuffer));
                         if (indexed) {
                             sk_sp<const GrBuffer> indexBuffer = (y % 2) ?
                                     helper->fIndexBuffer2 : helper->fIndexBuffer;
                             VALIDATE(indexBuffer);
                             int baseIndex = (y % 2);
                             pass->bindBuffers(std::move(indexBuffer), helper->fInstBuffer,
                                               std::move(vertexBuffer));
                             pass->drawIndexedInstanced(6, baseIndex, kBoxCountX, y * kBoxCountX,
                                                        baseVertex);
                         } else {
                             pass->bindBuffers(nullptr, helper->fInstBuffer,
                                               std::move(vertexBuffer));
                             pass->drawInstanced(kBoxCountX, y * kBoxCountY, 4, baseVertex);
                         }
                     }
                 });
    }

    for (bool indexed : {false, true}) {
        if (!dContext->priv().caps()->drawInstancedSupport()) {
            break;
        }

        run_test(dContext, (indexed) ? "drawIndexedIndirect" : "drawIndirect",
                 reporter, rtc, gold,
                 [&](DrawMeshHelper* helper) {
                     SkTArray<uint16_t> baseIndexData;
                     baseIndexData.push_back(kBoxCountX/2 * 6); // for testing base index.
                     for (int j = 0; j < kBoxCountY; ++j) {
                         for (int i = 0; i < 6; ++i) {
                             baseIndexData.push_back(kIndexPattern[i]);
                         }
                     }
                     helper->fIndexBuffer2 = helper->makeIndexBuffer(baseIndexData.begin(),
                                                                     baseIndexData.count());
                     VALIDATE(helper->fIndexBuffer2);
                     helper->fInstBuffer = helper->makeVertexBuffer(boxes);
                     VALIDATE(helper->fInstBuffer);
                     helper->fVertBuffer = helper->makeVertexBuffer(std::vector<float>{
                             -1,-1, 0,0, 0,1, 1,0, 1,1, -1,-1, 0,0, 1,0, 0,1, 1,1});
                     VALIDATE(helper->fVertBuffer);

                     GrDrawIndirectWriter indirectWriter;
                     GrDrawIndexedIndirectWriter indexedIndirectWriter;
                     if (indexed) {
                         // Make helper->fDrawIndirectBufferOffset nonzero.
                         sk_sp<const GrBuffer> dummyBuff;
                         size_t dummyOffset;
                         // Make a superfluous call to makeDrawIndirectSpace in order to test
                         // "offsetInBytes!=0" for the actual call to makeDrawIndexedIndirectSpace.
                         helper->target()->makeDrawIndirectSpace(29, &dummyBuff, &dummyOffset);
                         indexedIndirectWriter = helper->target()->makeDrawIndexedIndirectSpace(
                                 kBoxCountY, &helper->fDrawIndirectBuffer,
                                 &helper->fDrawIndirectBufferOffset);
                     } else {
                         // Make helper->fDrawIndirectBufferOffset nonzero.
                         sk_sp<const GrBuffer> dummyBuff;
                         size_t dummyOffset;
                         // Make a superfluous call to makeDrawIndexedIndirectSpace in order to test
                         // "offsetInBytes!=0" for the actual call to makeDrawIndirectSpace.
                         helper->target()->makeDrawIndexedIndirectSpace(7, &dummyBuff,
                                                                        &dummyOffset);
                         indirectWriter = helper->target()->makeDrawIndirectSpace(
                                 kBoxCountY, &helper->fDrawIndirectBuffer,
                                 &helper->fDrawIndirectBufferOffset);
                     }

                     // Draw boxes one line at a time to exercise multiple draws.
                     for (int y = 0; y < kBoxCountY; ++y) {
                         int baseVertex = (y % 2) ? 1 : 6;
                         if (indexed) {
                             int baseIndex = 1 + y * 6;
                             indexedIndirectWriter.writeIndexed(6, baseIndex, kBoxCountX,
                                                                y * kBoxCountX, baseVertex);
                         } else {
                             indirectWriter.write(kBoxCountX, y * kBoxCountX, 4, baseVertex);
                         }
                     }
                 },
                 [&](DrawMeshHelper* helper) {
                     GrOpsRenderPass* pass;
                     if (indexed) {
                         pass = helper->bindPipeline(GrPrimitiveType::kTriangles, true, true);
                         pass->bindBuffers(helper->fIndexBuffer2, helper->fInstBuffer,
                                           helper->fVertBuffer);
                         for (int i = 0; i < 3; ++i) {
                             int start = kBoxCountY * i / 3;
                             int end = kBoxCountY * (i + 1) / 3;
                             size_t offset = helper->fDrawIndirectBufferOffset + start *
                                             sizeof(GrDrawIndexedIndirectCommand);
                             pass->drawIndexedIndirect(helper->fDrawIndirectBuffer.get(), offset,
                                                       end - start);
                         }
                     } else {
                         pass = helper->bindPipeline(GrPrimitiveType::kTriangleStrip, true, true);
                         pass->bindBuffers(nullptr, helper->fInstBuffer, helper->fVertBuffer);
                         for (int i = 0; i < 2; ++i) {
                             int start = kBoxCountY * i / 2;
                             int end = kBoxCountY * (i + 1) / 2;
                             size_t offset = helper->fDrawIndirectBufferOffset + start *
                                             sizeof(GrDrawIndirectCommand);
                             pass->drawIndirect(helper->fDrawIndirectBuffer.get(), offset,
                                                end - start);
                         }
                     }
                 });
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class GrMeshTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* rContext,
                            std::function<void(DrawMeshHelper*)> prepareFn,
                            std::function<void(DrawMeshHelper*)> executeFn) {
        return GrOp::Make<GrMeshTestOp>(rContext, prepareFn, executeFn);
    }

private:
    friend class GrOp; // for ctor

    GrMeshTestOp(std::function<void(DrawMeshHelper*)> prepareFn,
                 std::function<void(DrawMeshHelper*)> executeFn)
        : INHERITED(ClassID())
        , fPrepareFn(prepareFn)
        , fExecuteFn(executeFn){
        this->setBounds(SkRect::MakeIWH(kImageWidth, kImageHeight),
                        HasAABloat::kNo, IsHairline::kNo);
    }

    const char* name() const override { return "GrMeshTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) override {}
    void onPrepare(GrOpFlushState* state) override {
        fHelper = std::make_unique<DrawMeshHelper>(state);
        fPrepareFn(fHelper.get());
    }
    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override {
        fExecuteFn(fHelper.get());
    }

    std::unique_ptr<DrawMeshHelper> fHelper;
    std::function<void(DrawMeshHelper*)> fPrepareFn;
    std::function<void(DrawMeshHelper*)> fExecuteFn;

    using INHERITED = GrDrawOp;
};

class GrMeshTestProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, bool instanced, bool hasVertexBuffer) {
        return arena->make([&](void* ptr) {
            return new (ptr) GrMeshTestProcessor(instanced, hasVertexBuffer);
        });
    }

    const char* name() const override { return "GrMeshTestProcessor"; }

    const Attribute& inColor() const {
        return fVertexColor.isInitialized() ? fVertexColor : fInstanceColor;
    }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32(fInstanceLocation.isInitialized());
        b->add32(fVertexPosition.isInitialized());
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    friend class GLSLMeshTestProcessor;

    GrMeshTestProcessor(bool instanced, bool hasVertexBuffer)
            : INHERITED(kGrMeshTestProcessor_ClassID) {
        if (instanced) {
            fInstanceLocation = {"location", kFloat2_GrVertexAttribType, kHalf2_GrSLType};
            fInstanceColor = {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
            this->setInstanceAttributes(&fInstanceLocation, 2);
            if (hasVertexBuffer) {
                fVertexPosition = {"vertex", kFloat2_GrVertexAttribType, kHalf2_GrSLType};
                this->setVertexAttributes(&fVertexPosition, 1);
            }
        } else {
            fVertexPosition = {"vertex", kFloat2_GrVertexAttribType, kHalf2_GrSLType};
            fVertexColor = {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
            this->setVertexAttributes(&fVertexPosition, 2);
        }
    }

    Attribute fVertexPosition;
    Attribute fVertexColor;

    Attribute fInstanceLocation;
    Attribute fInstanceColor;

    using INHERITED = GrGeometryProcessor;
};

class GLSLMeshTestProcessor : public GrGLSLGeometryProcessor {
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&) final {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const GrMeshTestProcessor& mp = args.fGP.cast<GrMeshTestProcessor>();
        GrGLSLVertexBuilder* v = args.fVertBuilder;
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        varyingHandler->emitAttributes(mp);
        f->codeAppendf("half4 %s;", args.fOutputColor);
        varyingHandler->addPassThroughAttribute(mp.inColor(), args.fOutputColor);

        if (!mp.fInstanceLocation.isInitialized()) {
            v->codeAppendf("float2 vertex = %s;", mp.fVertexPosition.name());
        } else {
            if (mp.fVertexPosition.isInitialized()) {
                v->codeAppendf("float2 offset = %s;", mp.fVertexPosition.name());
            } else {
                v->codeAppend("float2 offset = float2(sk_VertexID / 2, sk_VertexID % 2);");
            }
            v->codeAppendf("float2 vertex = %s + offset * %i;", mp.fInstanceLocation.name(),
                           kBoxSize);
        }
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");

        f->codeAppendf("const half4 %s = half4(1);", args.fOutputCoverage);
    }
};

GrGLSLPrimitiveProcessor* GrMeshTestProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLMeshTestProcessor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<const GrBuffer> DrawMeshHelper::makeIndexBuffer(const uint16_t indices[], int count) {
    return sk_sp<const GrBuffer>(fState->resourceProvider()->createBuffer(
            count * sizeof(uint16_t), GrGpuBufferType::kIndex, kDynamic_GrAccessPattern, indices));
}

template<typename T>
sk_sp<const GrBuffer> DrawMeshHelper::makeVertexBuffer(const T* data, int count) {
    return sk_sp<const GrBuffer>(fState->resourceProvider()->createBuffer(
            count * sizeof(T), GrGpuBufferType::kVertex, kDynamic_GrAccessPattern, data));
}

sk_sp<const GrBuffer> DrawMeshHelper::getIndexBuffer() {
    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);
    return fState->resourceProvider()->findOrCreatePatternedIndexBuffer(
            kIndexPattern, 6, kIndexPatternRepeatCount, 4, gIndexBufferKey);
}

GrOpsRenderPass* DrawMeshHelper::bindPipeline(GrPrimitiveType primitiveType, bool isInstanced,
                                              bool hasVertexBuffer) {
    GrProcessorSet processorSet(SkBlendMode::kSrc);

    // TODO: add a GrProcessorSet testing helper to make this easier
    SkPMColor4f overrideColor;
    processorSet.finalize(GrProcessorAnalysisColor(),
                          GrProcessorAnalysisCoverage::kNone,
                          fState->appliedClip(),
                          nullptr,
                          false,
                          fState->caps(),
                          GrClampType::kAuto,
                          &overrideColor);

    auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(fState,
                                                             std::move(processorSet),
                                                             GrPipeline::InputFlags::kNone);

    GrGeometryProcessor* mtp = GrMeshTestProcessor::Make(fState->allocator(), isInstanced,
                                                         hasVertexBuffer);

    GrProgramInfo programInfo(fState->writeView(), pipeline, &GrUserStencilSettings::kUnused,
                              mtp, primitiveType, 0, fState->renderPassBarriers(),
                              fState->colorLoadOp());

    fState->opsRenderPass()->bindPipeline(programInfo, SkRect::MakeIWH(kImageWidth, kImageHeight));
    return fState->opsRenderPass();
}

static void run_test(GrDirectContext* dContext, const char* testName,
                     skiatest::Reporter* reporter,
                     const std::unique_ptr<GrSurfaceDrawContext>& rtc, const SkBitmap& gold,
                     std::function<void(DrawMeshHelper*)> prepareFn,
                     std::function<void(DrawMeshHelper*)> executeFn) {
    const int w = gold.width(), h = gold.height();
    const uint32_t* goldPx = reinterpret_cast<const uint32_t*>(gold.getPixels());
    if (h != rtc->height() || w != rtc->width()) {
        ERRORF(reporter, "[%s] expectation and rtc not compatible (?).", testName);
        return;
    }
    if (sizeof(uint32_t) * kImageWidth != gold.rowBytes()) {
        ERRORF(reporter, "[%s] unexpected row bytes in gold image", testName);
        return;
    }

    GrPixmap resultPM = GrPixmap::Allocate(gold.info());
    rtc->clear(SkPMColor4f::FromBytes_RGBA(0xbaaaaaad));
    rtc->addDrawOp(GrMeshTestOp::Make(dContext, prepareFn, executeFn));

    rtc->readPixels(dContext, resultPM, {0, 0});

#ifdef WRITE_PNG_CONTEXT_TYPE
#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)
    SkString filename;
    filename.printf("GrMeshTest_%s_%s.png", TOSTRING(WRITE_PNG_CONTEXT_TYPE), testName);
    SkDebugf("writing %s...\n", filename.c_str());
    ToolUtils::EncodeImageToFile(filename.c_str(), resultPM, SkEncodedImageFormat::kPNG, 100);
#endif

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t expected = goldPx[y * kImageWidth + x];
            uint32_t actual = static_cast<uint32_t*>(resultPM.addr())[y * kImageWidth + x];
            if (expected != actual) {
                ERRORF(reporter, "[%s] pixel (%i,%i): got 0x%x expected 0x%x",
                       testName, x, y, actual, expected);
                return;
            }
        }
    }
}
