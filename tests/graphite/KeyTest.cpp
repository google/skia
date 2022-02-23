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

    SkASSERT(size <= 1024);
    static const uint8_t kDummyData[1024] = { 0 };

    SkDEBUGCODE(builder->checkReset());

    builder->beginBlock(dummySnippetID);

    builder->addBytes(size, kDummyData);

    builder->endBlock();

    return builder->lockAsKey();
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();

    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    // invalid code snippet ID
    {
        SkPaintParamsKey key = create_key(&builder, kBuiltInCodeSnippetIDCount, 32);
        // key creation fails
        REPORTER_ASSERT(reporter, key.isErrorKey());
    }

    // _Just_ on the edge of being too big
    {
        static const int kMaxBlockDataSize = SkPaintParamsKey::kMaxBlockSize -
                                             SkPaintParamsKey::kBlockHeaderSizeInBytes;

        static constexpr int kNumFields1 = 1;
        static constexpr SkPaintParamsKey::DataPayloadField kDataFields1[kNumFields1] = {
            { "data", SkPaintParamsKey::DataPayloadType::kByte, kMaxBlockDataSize },
        };

        int dummySnippetID1 = dict->addUserDefinedSnippet("keyAlmostTooBig",
                                                          SkMakeSpan(kDataFields1, kNumFields1));

        SkPaintParamsKey key = create_key(&builder, dummySnippetID1, kMaxBlockDataSize);
        REPORTER_ASSERT(reporter, key.sizeInBytes() == SkPaintParamsKey::kMaxBlockSize);
    }

    // Too big
    {
        static constexpr int kNumFields2 = 1;
        static constexpr SkPaintParamsKey::DataPayloadField kDataFields2[kNumFields2] = {
                { "data", SkPaintParamsKey::DataPayloadType::kByte, 1024 },
        };

        int dummySnippetID2 = dict->addUserDefinedSnippet("keyTooBig",
                                                          SkMakeSpan(kDataFields2, kNumFields2));

        SkPaintParamsKey key = create_key(&builder, dummySnippetID2, 1024);
        // key creation fails
        REPORTER_ASSERT(reporter, key.isErrorKey());
    }
}
