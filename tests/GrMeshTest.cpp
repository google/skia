/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrGeometryProcessor.h"
#include "GrGpuCommandBuffer.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "GrResourceKey.h"
#include "SkMakeUnique.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"
#include <array>
#include <vector>


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

static void run_test(const char* testName, skiatest::Reporter*, const sk_sp<GrRenderTargetContext>&,
                     const SkBitmap& gold, std::function<void(DrawMeshHelper*)> testFn);

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrMeshTest, reporter, ctxInfo) {
    GrContext* const context = ctxInfo.grContext();

    sk_sp<GrRenderTargetContext> rtc(
        context->makeDeferredRenderTargetContext(SkBackingFit::kExact, kImageWidth, kImageHeight,
                                                 kRGBA_8888_GrPixelConfig, nullptr));
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
                    box.fX + (i/2) * kBoxSize,
                    box.fY + (i%2) * kBoxSize,
                    box.fColor
                };
            }

            paint.setARGB(255, rgb[0], rgb[1], rgb[2]);
            goldCanvas.drawRect(SkRect::MakeXYWH(box.fX, box.fY, kBoxSize, kBoxSize), paint);
        }
    }

    goldCanvas.flush();

    // ---- tests ----------

#define VALIDATE(buff) \
    if (!buff) { \
        ERRORF(reporter, #buff " is null."); \
        return; \
    }

    run_test("setNonIndexedNonInstanced", reporter, rtc, gold, [&](DrawMeshHelper* helper) {
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
            mesh.setVertexData(vbuff.get(), y * kBoxCountX * 6);
            helper->drawMesh(mesh);
        }
    });

    run_test("setIndexed", reporter, rtc, gold, [&](DrawMeshHelper* helper) {
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
            mesh.setIndexed(ibuff.get(), repetitionCount * 6, baseRepetition * 6,
                            baseRepetition * 4, (baseRepetition + repetitionCount) * 4 - 1);
            mesh.setVertexData(vbuff.get(), (i - baseRepetition) * 4);
            helper->drawMesh(mesh);

            baseRepetition = (baseRepetition + 1) % 3;
            i += repetitionCount;
        }
    });

    run_test("setIndexedPatterned", reporter, rtc, gold, [&](DrawMeshHelper* helper) {
        auto ibuff = helper->getIndexBuffer();
        VALIDATE(ibuff);
        auto vbuff = helper->makeVertexBuffer(vertexData);
        VALIDATE(vbuff);

        // Draw boxes one line at a time to exercise base vertex. setIndexedPatterned does not
        // support a base index.
        for (int y = 0; y < kBoxCountY; ++y) {
            GrMesh mesh(GrPrimitiveType::kTriangles);
            mesh.setIndexedPatterned(ibuff.get(), 6, 4, kBoxCountX, kIndexPatternRepeatCount);
            mesh.setVertexData(vbuff.get(), y * kBoxCountX * 4);
            helper->drawMesh(mesh);
        }
    });

    for (bool indexed : {false, true}) {
        if (!context->caps()->instanceAttribSupport()) {
            break;
        }

        run_test(indexed ? "setIndexedInstanced" : "setInstanced",
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
                    mesh.setIndexedInstanced(idxbuff.get(), 6,
                                             instbuff.get(), kBoxCountX, y * kBoxCountX);
                } else {
                    mesh.setInstanced(instbuff.get(), kBoxCountX, y * kBoxCountX, 4);
                }
                switch (y % 3) {
                    case 0:
                        if (context->caps()->shaderCaps()->vertexIDSupport()) {
                            if (y % 2) {
                                // We don't need this call because it's the initial state of GrMesh.
                                mesh.setVertexData(nullptr);
                            }
                            break;
                        }
                        // Fallthru.
                    case 1:
                        mesh.setVertexData(vbuff.get());
                        break;
                    case 2:
                        mesh.setVertexData(vbuff2.get(), 2);
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

    GrMeshTestOp(std::function<void(DrawMeshHelper*)> testFn)
        : INHERITED(ClassID())
        , fTestFn(testFn) {
        this->setBounds(SkRect::MakeIWH(kImageWidth, kImageHeight),
                        HasAABloat::kNo, IsZeroArea::kNo);
    }

private:
    const char* name() const override { return "GrMeshTestOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* state) override {
        DrawMeshHelper helper(state);
        fTestFn(&helper);
    }

    std::function<void(DrawMeshHelper*)> fTestFn;

    typedef GrDrawOp INHERITED;
};

class GrMeshTestProcessor : public GrGeometryProcessor {
public:
    GrMeshTestProcessor(bool instanced, bool hasVertexBuffer)
        : fInstanceLocation(nullptr)
        , fVertex(nullptr)
        , fColor(nullptr) {
        if (instanced) {
            fInstanceLocation = &this->addInstanceAttrib("location", kVec2f_GrVertexAttribType);
            if (hasVertexBuffer) {
                fVertex = &this->addVertexAttrib("vertex", kVec2f_GrVertexAttribType);
            }
            fColor = &this->addInstanceAttrib("color", kVec4ub_GrVertexAttribType);
        } else {
            fVertex = &this->addVertexAttrib("vertex", kVec2f_GrVertexAttribType);
            fColor = &this->addVertexAttrib("color", kVec4ub_GrVertexAttribType);
        }
        this->initClassID<GrMeshTestProcessor>();
    }

    const char* name() const override { return "GrMeshTest Processor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {
        b->add32(SkToBool(fInstanceLocation));
        b->add32(SkToBool(fVertex));
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

protected:
    const Attribute* fInstanceLocation;
    const Attribute* fVertex;
    const Attribute* fColor;

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
        varyingHandler->addPassThroughAttribute(mp.fColor, args.fOutputColor);

        GrGLSLVertexBuilder* v = args.fVertBuilder;
        if (!mp.fInstanceLocation) {
            v->codeAppendf("vec2 vertex = %s;", mp.fVertex->fName);
        } else {
            if (mp.fVertex) {
                v->codeAppendf("vec2 offset = %s;", mp.fVertex->fName);
            } else {
                v->codeAppend ("vec2 offset = vec2(sk_VertexID / 2, sk_VertexID % 2);");
            }
            v->codeAppendf("vec2 vertex = %s + offset * %i;",
                           mp.fInstanceLocation->fName, kBoxSize);
        }
        gpArgs->fPositionVar.set(kVec2f_GrSLType, "vertex");

        GrGLSLPPFragmentBuilder* f = args.fFragBuilder;
        f->codeAppendf("%s = vec4(1);", args.fOutputCoverage);
    }
};

GrGLSLPrimitiveProcessor* GrMeshTestProcessor::createGLSLInstance(const GrShaderCaps&) const {
    return new GLSLMeshTestProcessor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
sk_sp<const GrBuffer> DrawMeshHelper::makeVertexBuffer(const T* data, int count) {
    return sk_sp<const GrBuffer>(
        fState->resourceProvider()->createBuffer(
            count * sizeof(T), kVertex_GrBufferType, kDynamic_GrAccessPattern,
            GrResourceProvider::kNoPendingIO_Flag |
            GrResourceProvider::kRequireGpuMemory_Flag, data));
}

sk_sp<const GrBuffer> DrawMeshHelper::getIndexBuffer() {
    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);
    return sk_sp<const GrBuffer>(
        fState->resourceProvider()->findOrCreatePatternedIndexBuffer(
            kIndexPattern, 6, kIndexPatternRepeatCount, 4, gIndexBufferKey));
}

void DrawMeshHelper::drawMesh(const GrMesh& mesh) {
    GrRenderTarget* rt = fState->drawOpArgs().fRenderTarget;
    GrPipeline pipeline(rt, GrPipeline::ScissorState::kDisabled, SkBlendMode::kSrc);
    GrMeshTestProcessor mtp(mesh.isInstanced(), mesh.hasVertexData());
    fState->commandBuffer()->draw(pipeline, mtp, &mesh, nullptr, 1,
                                  SkRect::MakeIWH(kImageWidth, kImageHeight));
}

static void run_test(const char* testName, skiatest::Reporter* reporter,
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
    rtc->clear(nullptr, 0xbaaaaaad, true);
    rtc->priv().testingOnly_addDrawOp(skstd::make_unique<GrMeshTestOp>(testFn));
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

#endif
