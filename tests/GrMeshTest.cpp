/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include <array>
#include <vector>
#include "include/core/SkBitmap.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrResourceKey.h"
#include "src/core/SkMakeUnique.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuCommandBuffer.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

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

    template<typename T> sk_sp<const GrBuffer> makeVertexBuffer(const SkTArray<T>& data) {
        return this->makeVertexBuffer(data.begin(), data.count());
    }
    template<typename T> sk_sp<const GrBuffer> makeVertexBuffer(const std::vector<T>& data) {
        return this->makeVertexBuffer(data.data(), data.size());
    }
    template<typename T> sk_sp<const GrBuffer> makeVertexBuffer(const T* data, int count);

    void drawMesh(const GrMesh& mesh);

private:
    GrOpFlushState* fState;
};

struct Box {
    float fX, fY;
    GrColor fColor;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This is a GPU-backend specific test. It tries to test all possible usecases of GrMesh. The test
 * works by drawing checkerboards of colored boxes, reading back the pixels, and comparing with
 * expected results. The boxes are drawn on integer boundaries and the (opaque) colors are chosen
 * from the set (r,g,b) = (0,255)^3, so the GPU renderings ought to produce exact matches.
 */

static void run_test(GrContext* context, const char* testName, skiatest::Reporter*,
                     const sk_sp<GrRenderTargetContext>&, const SkBitmap& gold,
                     std::function<void(DrawMeshHelper*)> testFn);

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrMeshTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    const GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    sk_sp<GrRenderTargetContext> rtc(context->priv().makeDeferredRenderTargetContext(
            format, SkBackingFit::kExact, kImageWidth, kImageHeight, kRGBA_8888_GrPixelConfig,
            nullptr));
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

    run_test(context, "setNonIndexedNonInstanced", reporter, rtc, gold,
             [&](DrawMeshHelper* helper) {
                 SkTArray<Box> expandedVertexData;
                 for (int i = 0; i < kBoxCount; ++i) {
                     for (int j = 0; j < 6; ++j) {
                         expandedVertexData.push_back(vertexData[i][kIndexPattern[j]]);
                     }
                 }

                 // Draw boxes one line at a time to exercise base vertex.
                 auto vbuff = helper->makeVertexBuffer(expandedVertexData);
                 VALIDATE(vbuff);
                 for (int y = 0; y < kBoxCountY; ++y) {
                     GrMesh mesh(GrPrimitiveType::kTriangles);
                     mesh.setNonIndexedNonInstanced(kBoxCountX * 6);
                     mesh.setVertexData(vbuff, y * kBoxCountX * 6);
                     helper->drawMesh(mesh);
                 }
             });

    run_test(context, "setIndexed", reporter, rtc, gold, [&](DrawMeshHelper* helper) {
        auto ibuff = helper->getIndexBuffer();
        VALIDATE(ibuff);
        auto vbuff = helper->makeVertexBuffer(vertexData);
        VALIDATE(vbuff);
        int baseRepetition = 0;
        int i = 0;

        // Start at various repetitions within the patterned index buffer to exercise base index.
        while (i < kBoxCount) {
            GR_STATIC_ASSERT(kIndexPatternRepeatCount >= 3);
            int repetitionCount = SkTMin(3 - baseRepetition, kBoxCount - i);

            GrMesh mesh(GrPrimitiveType::kTriangles);
            mesh.setIndexed(ibuff, repetitionCount * 6, baseRepetition * 6, baseRepetition * 4,
                            (baseRepetition + repetitionCount) * 4 - 1, GrPrimitiveRestart::kNo);
            mesh.setVertexData(vbuff, (i - baseRepetition) * 4);
            helper->drawMesh(mesh);

            baseRepetition = (baseRepetition + 1) % 3;
            i += repetitionCount;
        }
    });

    run_test(context, "setIndexedPatterned", reporter, rtc, gold, [&](DrawMeshHelper* helper) {
        auto ibuff = helper->getIndexBuffer();
        VALIDATE(ibuff);
        auto vbuff = helper->makeVertexBuffer(vertexData);
        VALIDATE(vbuff);

        // Draw boxes one line at a time to exercise base vertex. setIndexedPatterned does not
        // support a base index.
        for (int y = 0; y < kBoxCountY; ++y) {
            GrMesh mesh(GrPrimitiveType::kTriangles);
            mesh.setIndexedPatterned(ibuff, 6, 4, kBoxCountX, kIndexPatternRepeatCount);
            mesh.setVertexData(vbuff, y * kBoxCountX * 4);
            helper->drawMesh(mesh);
        }
    });

    for (bool indexed : {false, true}) {
        if (!context->priv().caps()->instanceAttribSupport()) {
            break;
        }

        run_test(context, indexed ? "setIndexedInstanced" : "setInstanced",
                 reporter, rtc, gold, [&](DrawMeshHelper* helper) {
            auto idxbuff = indexed ? helper->getIndexBuffer() : nullptr;
            auto instbuff = helper->makeVertexBuffer(boxes);
            VALIDATE(instbuff);
            auto vbuff = helper->makeVertexBuffer(std::vector<float>{0,0, 0,1, 1,0, 1,1});
            VALIDATE(vbuff);
            auto vbuff2 = helper->makeVertexBuffer( // for testing base vertex.
                              std::vector<float>{-1,-1, -1,-1, 0,0, 0,1, 1,0, 1,1});
            VALIDATE(vbuff2);

            // Draw boxes one line at a time to exercise base instance, base vertex, and null vertex
            // buffer. setIndexedInstanced intentionally does not support a base index.
            for (int y = 0; y < kBoxCountY; ++y) {
                GrMesh mesh(indexed ? GrPrimitiveType::kTriangles
                                    : GrPrimitiveType::kTriangleStrip);
                if (indexed) {
                    VALIDATE(idxbuff);
                    mesh.setIndexedInstanced(idxbuff, 6, instbuff, kBoxCountX, y * kBoxCountX,
                                             GrPrimitiveRestart::kNo);
                } else {
                    mesh.setInstanced(instbuff, kBoxCountX, y * kBoxCountX, 4);
                }
                switch (y % 3) {
                    case 0:
                        if (context->priv().caps()->shaderCaps()->vertexIDSupport()) {
                            if (y % 2) {
                                // We don't need this call because it's the initial state of GrMesh.
                                mesh.setVertexData(nullptr);
                            }
                            break;
                        }
                        // Fallthru.
                    case 1:
                        mesh.setVertexData(vbuff);
                        break;
                    case 2:
                        mesh.setVertexData(vbuff2, 2);
                        break;
                }
                helper->drawMesh(mesh);
            }
        });
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class GrMeshTestOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          std::function<void(DrawMeshHelper*)> testFn) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<GrMeshTestOp>(testFn);
    }

private:
    friend class GrOpMemoryPool; // for ctor

    GrMeshTestOp(std::function<void(DrawMeshHelper*)> testFn)
        : INHERITED(ClassID())
        , fTestFn(testFn) {
        this->setBounds(SkRect::MakeIWH(kImageWidth, kImageHeight),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "GrMeshTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state, const SkRect& chainBounds) override {
        DrawMeshHelper helper(state);
        fTestFn(&helper);
    }

    std::function<void(DrawMeshHelper*)> fTestFn;

    typedef GrDrawOp INHERITED;
};

class GrMeshTestProcessor : public GrGeometryProcessor {
public:
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

    const char* name() const override { return "GrMeshTest Processor"; }

    const Attribute& inColor() const {
        return fVertexColor.isInitialized() ? fVertexColor : fInstanceColor;
    }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32(fInstanceLocation.isInitialized());
        b->add32(fVertexPosition.isInitialized());
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    Attribute fVertexPosition;
    Attribute fVertexColor;

    Attribute fInstanceLocation;
    Attribute fInstanceColor;

    friend class GLSLMeshTestProcessor;
    typedef GrGeometryProcessor INHERITED;
};

class GLSLMeshTestProcessor : public GrGLSLGeometryProcessor {
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const GrMeshTestProcessor& mp = args.fGP.cast<GrMeshTestProcessor>();

        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        varyingHandler->emitAttributes(mp);
        varyingHandler->addPassThroughAttribute(mp.inColor(), args.fOutputColor);

        GrGLSLVertexBuilder* v = args.fVertBuilder;
        if (!mp.fInstanceLocation.isInitialized()) {
            v->codeAppendf("float2 vertex = %s;", mp.fVertexPosition.name());
        } else {
            if (mp.fVertexPosition.isInitialized()) {
                v->codeAppendf("float2 offset = %s;", mp.fVertexPosition.name());
            } else {
                v->codeAppend ("float2 offset = float2(sk_VertexID / 2, sk_VertexID % 2);");
            }
            v->codeAppendf("float2 vertex = %s + offset * %i;", mp.fInstanceLocation.name(),
                           kBoxSize);
        }
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");

        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;
        f->codeAppendf("%s = half4(1);", args.fOutputCoverage);
    }
};

GrGLSLPrimitiveProcessor* GrMeshTestProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLMeshTestProcessor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

void DrawMeshHelper::drawMesh(const GrMesh& mesh) {
    GrPipeline pipeline(GrScissorTest::kDisabled, SkBlendMode::kSrc, GrSwizzle::RGBA());
    GrMeshTestProcessor mtp(mesh.isInstanced(), mesh.hasVertexData());
    fState->rtCommandBuffer()->draw(mtp, pipeline, nullptr, nullptr, &mesh, 1,
                                    SkRect::MakeIWH(kImageWidth, kImageHeight));
}

static void run_test(GrContext* context, const char* testName, skiatest::Reporter* reporter,
                     const sk_sp<GrRenderTargetContext>& rtc, const SkBitmap& gold,
                     std::function<void(DrawMeshHelper*)> testFn) {
    const int w = gold.width(), h = gold.height(), rowBytes = gold.rowBytes();
    const uint32_t* goldPx = reinterpret_cast<const uint32_t*>(gold.getPixels());
    if (h != rtc->height() || w != rtc->width()) {
        ERRORF(reporter, "[%s] expectation and rtc not compatible (?).", testName);
        return;
    }
    if (sizeof(uint32_t) * kImageWidth != gold.rowBytes()) {
        ERRORF(reporter, "unexpected row bytes in gold image.", testName);
        return;
    }

    SkAutoSTMalloc<kImageHeight * kImageWidth, uint32_t> resultPx(h * rowBytes);
    rtc->clear(nullptr, SkPMColor4f::FromBytes_RGBA(0xbaaaaaad),
               GrRenderTargetContext::CanClearFullscreen::kYes);
    rtc->priv().testingOnly_addDrawOp(GrMeshTestOp::Make(context, testFn));
    rtc->readPixels(gold.info(), resultPx, rowBytes, 0, 0, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t expected = goldPx[y * kImageWidth + x];
            uint32_t actual = resultPx[y * kImageWidth + x];
            if (expected != actual) {
                ERRORF(reporter, "[%s] pixel (%i,%i): got 0x%x expected 0x%x",
                       testName, x, y, actual, expected);
                return;
            }
        }
    }
}
