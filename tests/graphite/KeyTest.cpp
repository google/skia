/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkShaderCodeDictionary.h"

#include "experimental/graphite/src/ContextPriv.h"

namespace {

SkPaintParamsKey create_key(SkPaintParamsKeyBuilder* builder, int dummySnippetID, int size) {

    SkDEBUGCODE(builder->checkReset());

    builder->beginBlock(dummySnippetID);

    for (int i = 0; i < size; ++i) {
        builder->addByte(i % 256);
    }

    builder->endBlock();

    return builder->lockAsKey();
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();

    SkPaintParamsKeyBuilder builder(dict);

    static const int kMaxBlockDataSize = SkPaintParamsKey::kMaxBlockSize -
                                         SkPaintParamsKey::kBlockHeaderSizeInBytes;

    // invalid code snippet ID
    {
        SkPaintParamsKey key = create_key(&builder, kBuiltInCodeSnippetIDCount, kMaxBlockDataSize);
        REPORTER_ASSERT(reporter, key.isErrorKey());
    }

    int dummySnippetID = dict->addUserDefinedSnippet();

    // _Just_ on the edge of being too big
    {
        SkPaintParamsKey key = create_key(&builder, dummySnippetID, kMaxBlockDataSize);
        REPORTER_ASSERT(reporter, key.sizeInBytes() == SkPaintParamsKey::kMaxBlockSize);
    }

    // Too big
    {
        SkPaintParamsKey key = create_key(&builder, dummySnippetID, 1024);
        REPORTER_ASSERT(reporter, key.isErrorKey());
    }
}
