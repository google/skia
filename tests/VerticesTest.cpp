/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVertices.h"
#include "Test.h"

static bool equal(const SkVertices* v0, const SkVertices* v1) {
    if (v0->mode() != v1->mode()) {
        return false;
    }
    if (v0->vertexCount() != v1->vertexCount()) {
        return false;
    }
    if (v0->indexCount() != v1->indexCount()) {
        return false;
    }

    if (!!v0->texCoords() != !!v1->texCoords()) {
        return false;
    }
    if (!!v0->colors() != !!v1->colors()) {
        return false;
    }

    for (int i = 0; i < v0->vertexCount(); ++i) {
        if (v0->positions()[i] != v1->positions()[i]) {
            return false;
        }
        if (v0->texCoords()) {
            if (v0->texCoords()[i] != v1->texCoords()[i]) {
                return false;
            }
        }
        if (v0->colors()) {
            if (v0->colors()[i] != v1->colors()[i]) {
                return false;
            }
        }
    }
    for (int i = 0; i < v0->indexCount(); ++i) {
        if (v0->indices()[i] != v1->indices()[i]) {
            return false;
        }
    }
    return true;
}

DEF_TEST(Vertices, reporter) {
    int vCount = 5;
    int iCount = 9; // odd value exercises padding logic in encode()

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
            for (int i = 0; i < builder.indexCount(); ++i) {
                builder.indices()[i] = i % vCount;
            }

            sk_sp<SkVertices> v0 = builder.detach();
            sk_sp<SkData> data = v0->encode();
            sk_sp<SkVertices> v1 = SkVertices::Decode(data->data(), data->size());

            REPORTER_ASSERT(reporter, v0->uniqueID() != 0);
            REPORTER_ASSERT(reporter, v1->uniqueID() != 0);
            REPORTER_ASSERT(reporter, v0->uniqueID() != v1->uniqueID());
            REPORTER_ASSERT(reporter, equal(v0.get(), v1.get()));
        }
    }
}
