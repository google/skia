/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkVertices.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVerticesPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

static bool equal(const SkVertices* vert0, const SkVertices* vert1) {
    SkVerticesPriv v0(vert0->priv()), v1(vert1->priv());

    if (v0.mode() != v1.mode()) {
        return false;
    }
    if (v0.vertexCount() != v1.vertexCount()) {
        return false;
    }
    if (v0.indexCount() != v1.indexCount()) {
        return false;
    }
    if (v0.attributeCount() != v1.attributeCount()) {
        return false;
    }
    for (int i = 0; i < v0.attributeCount(); ++i) {
        if (v0.attributes()[i] != v1.attributes()[i]) {
            return false;
        }
    }

    if (!!v0.customData() != !!v1.customData()) {
        return false;
    }
    if (!!v0.texCoords() != !!v1.texCoords()) {
        return false;
    }
    if (!!v0.colors() != !!v1.colors()) {
        return false;
    }

    for (int i = 0; i < v0.vertexCount(); ++i) {
        if (v0.positions()[i] != v1.positions()[i]) {
            return false;
        }
        if (v0.texCoords()) {
            if (v0.texCoords()[i] != v1.texCoords()[i]) {
                return false;
            }
        }
        if (v0.colors()) {
            if (v0.colors()[i] != v1.colors()[i]) {
                return false;
            }
        }
    }
    size_t totalCustomDataSize = v0.vertexCount() * v0.customDataSize();
    if (totalCustomDataSize) {
        if (0 != memcmp(v0.customData(), v1.customData(), totalCustomDataSize)) {
            return false;
        }
    }
    for (int i = 0; i < v0.indexCount(); ++i) {
        if (v0.indices()[i] != v1.indices()[i]) {
            return false;
        }
    }
    return true;
}

static void self_test(sk_sp<SkVertices> v0, skiatest::Reporter* reporter) {
    SkBinaryWriteBuffer writer;
    v0->priv().encode(writer);

    SkAutoMalloc buf(writer.bytesWritten());
    writer.writeToMemory(buf.get());
    SkReadBuffer reader(buf.get(), writer.bytesWritten());

    sk_sp<SkVertices> v1 = SkVerticesPriv::Decode(reader);

    REPORTER_ASSERT(reporter, v1 != nullptr);
    REPORTER_ASSERT(reporter, v0->uniqueID() != 0);
    REPORTER_ASSERT(reporter, v1->uniqueID() != 0);
    REPORTER_ASSERT(reporter, v0->uniqueID() != v1->uniqueID());
    REPORTER_ASSERT(reporter, equal(v0.get(), v1.get()));
}

DEF_TEST(Vertices, reporter) {
    int vCount = 5;
    int iCount = 9; // odd value exercises padding logic in encode()

    // color-tex tests
    const uint32_t texFlags[] = { 0, SkVertices::kHasTexCoords_BuilderFlag };
    const uint32_t colFlags[] = { 0, SkVertices::kHasColors_BuilderFlag };
    for (auto texF : texFlags) {
        for (auto colF : colFlags) {
            uint32_t flags = texF | colF;

            SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, vCount, iCount, flags);

            for (int i = 0; i < vCount; ++i) {
                float x = (float)i;
                builder.positions()[i].set(x, 1);
                if (builder.texCoords()) {
                    builder.texCoords()[i].set(x, 2);
                }
                if (builder.colors()) {
                    builder.colors()[i] = SkColorSetARGB(0xFF, i, 0x80, 0);
                }
            }
            for (int i = 0; i < iCount; ++i) {
                builder.indices()[i] = i % vCount;
            }
            self_test(builder.detach(), reporter);
        }
    }

    // custom data tests
    using AttrType = SkVertices::Attribute::Type;
    struct {
        int count;
        size_t expected_size;
        SkVertices::Attribute attrs[4];
    } attrTests[] = {
        { 1,  4, { AttrType::kFloat } },
        { 1,  8, { AttrType::kFloat2 } },
        { 1, 12, { AttrType::kFloat3 } },
        { 1, 16, { AttrType::kFloat4 } },
        { 1,  4, { AttrType::kByte4_unorm } },
        { 4, 16, { AttrType::kFloat, AttrType::kFloat, AttrType::kFloat, AttrType::kFloat } },
        { 2, 12, { AttrType::kFloat2, AttrType::kByte4_unorm } },
        { 2, 12, { AttrType::kByte4_unorm, AttrType::kFloat2 } },
    };

    for (const auto& test : attrTests) {
        SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, vCount, iCount,
                                    test.attrs, test.count);

        float* customData = (float*)builder.customData();
        int customDataCount = test.expected_size / sizeof(float);
        for (int i = 0; i < vCount; ++i) {
            builder.positions()[i].set((float)i, 1);
            for (int j = 0; j < customDataCount; ++j) {
                customData[i * customDataCount + j] = (float)j;
            }
        }
        for (int i = 0; i < iCount; ++i) {
            builder.indices()[i] = i % vCount;
        }
        self_test(builder.detach(), reporter);
    }

    {
        // This has the maximum number of vertices to be rewritten as indexed triangles without
        // overflowing a 16bit index.
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, UINT16_MAX + 1, 0,
                                    SkVertices::kHasColors_BuilderFlag);
        REPORTER_ASSERT(reporter, builder.isValid());
    }
    {
        // This has too many to be rewritten.
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, UINT16_MAX + 2, 0,
                                    SkVertices::kHasColors_BuilderFlag);
        REPORTER_ASSERT(reporter, !builder.isValid());
    }
    {
        // Only two vertices - can't be rewritten.
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, 2, 0,
                                    SkVertices::kHasColors_BuilderFlag);
        REPORTER_ASSERT(reporter, !builder.isValid());
    }
    {
        // Minimum number of indices to be rewritten.
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, 10, 3,
                                    SkVertices::kHasColors_BuilderFlag);
        REPORTER_ASSERT(reporter, builder.isValid());
    }
    {
        // Too few indices to be rewritten.
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, 10, 2,
                                    SkVertices::kHasColors_BuilderFlag);
        REPORTER_ASSERT(reporter, !builder.isValid());
    }

    // validity tests for per-vertex-data

    // Check that invalid counts fail to initialize the builder
    for (int attrCount : {-1, 0, SkVertices::kMaxCustomAttributes + 1}) {
        SkVertices::Attribute attrs[] = { AttrType::kFloat };
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, 10, 0, attrs, attrCount);
        REPORTER_ASSERT(reporter, !builder.isValid());
    }
    {   // nullptr is definitely bad
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, 10, 0, nullptr, 4);
        REPORTER_ASSERT(reporter, !builder.isValid());
    }
    {   // "normal" number of per-vertex-data (all floats)
        SkVertices::Attribute attrs[] = {AttrType::kFloat2, AttrType::kFloat2};
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, 10, 0, attrs, 2);
        REPORTER_ASSERT(reporter, builder.isValid());
        REPORTER_ASSERT(reporter, builder.customData() != nullptr);
    }
    {   // "normal" number of per-vertex-data (with packed bytes)
        SkVertices::Attribute attrs[] = {AttrType::kFloat2, AttrType::kByte4_unorm};
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, 10, 0, attrs, 2);
        REPORTER_ASSERT(reporter, builder.isValid());
        REPORTER_ASSERT(reporter, builder.customData() != nullptr);
    }
}

static void fill_triangle(SkCanvas* canvas, const SkPoint pts[], SkColor c) {
    SkColor colors[] = { c, c, c };
    auto verts = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, 3, pts, nullptr, colors);
    canvas->drawVertices(verts, SkBlendMode::kSrc, SkPaint());
}

DEF_TEST(Vertices_clipping, reporter) {
    // A very large triangle has to be geometrically clipped (since its "fast" clipping is
    // normally done in after building SkFixed coordinates). Check that we handle this.
    // (and don't assert).
    auto surf = SkSurface::MakeRasterN32Premul(3, 3);

    SkPoint pts[] = { { -10, 1 }, { -10, 2 }, { 1e9f, 1.5f } };
    fill_triangle(surf->getCanvas(), pts, SK_ColorBLACK);

    ToolUtils::PixelIter iter(surf.get());
    SkIPoint loc;
    while (void* addr = iter.next(&loc)) {
        SkPMColor c = *(SkPMColor*)addr;
        if (loc.fY == 1) {
            REPORTER_ASSERT(reporter, c == 0xFF000000);
        } else {
            REPORTER_ASSERT(reporter, c == 0);
        }
    }
}
