/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkShaderCodeDictionary.h"

#include "src/gpu/graphite/ContextPriv.h"

namespace {

SkPaintParamsKey create_key_with_data(SkPaintParamsKeyBuilder* builder,
                                      int snippetID,
                                      SkSpan<const uint8_t> span) {
    SkDEBUGCODE(builder->checkReset());

    builder->beginBlock(snippetID);

    builder->addBytes(span.size(), span.data());

    builder->endBlock();

    return builder->lockAsKey();
}

SkPaintParamsKey create_key(SkPaintParamsKeyBuilder* builder, int snippetID, int size) {
    SkASSERT(size <= 1024);
    static constexpr uint8_t kDummyData[1024] = {};
    return create_key_with_data(builder, snippetID, SkMakeSpan(kDummyData, size));
}

} // anonymous namespace

// These are intended to be unit tests of the SkPaintParamsKeyBuilder and SkPaintParamsKey.
DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyWithInvalidCodeSnippetIDTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    // Invalid code snippet ID, key creation fails.
    SkPaintParamsKey key = create_key(&builder, kBuiltInCodeSnippetIDCount, /*size=*/32);
    REPORTER_ASSERT(reporter, key.isErrorKey());
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyValidBlockSizeTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    // _Just_ on the edge of being too big
    static const int kMaxBlockDataSize = SkPaintParamsKey::kMaxBlockSize -
                                         SkPaintParamsKey::kBlockHeaderSizeInBytes;
    static constexpr SkPaintParamsKey::DataPayloadField kDataFields[] = {
            {"data", SkPaintParamsKey::DataPayloadType::kByte, kMaxBlockDataSize},
    };

    int dummySnippetID = dict->addUserDefinedSnippet("keyAlmostTooBig", SkMakeSpan(kDataFields));
    SkPaintParamsKey key = create_key(&builder, dummySnippetID, kMaxBlockDataSize);

    // Key is created successfully.
    REPORTER_ASSERT(reporter, key.sizeInBytes() == SkPaintParamsKey::kMaxBlockSize);
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyTooLargeBlockSizeTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    // Too big by one byte
    static const int kBlockDataSize = SkPaintParamsKey::kMaxBlockSize -
                                      SkPaintParamsKey::kBlockHeaderSizeInBytes + 1;
    static constexpr SkPaintParamsKey::DataPayloadField kDataFields[] = {
            {"data", SkPaintParamsKey::DataPayloadType::kByte, kBlockDataSize},
    };

    int dummySnippetID = dict->addUserDefinedSnippet("keyTooBig", SkMakeSpan(kDataFields));
    SkPaintParamsKey key = create_key(&builder, dummySnippetID, kBlockDataSize);

    // Key creation fails.
    REPORTER_ASSERT(reporter, key.isErrorKey());
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyEqualityChecksSnippetID, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();
    static const int kBlockDataSize = 4;
    static constexpr SkPaintParamsKey::DataPayloadField kDataFields[] = {
            {"data", SkPaintParamsKey::DataPayloadType::kByte, kBlockDataSize},
    };

    int dummySnippetID1 = dict->addUserDefinedSnippet("key1", SkMakeSpan(kDataFields));
    int dummySnippetID2 = dict->addUserDefinedSnippet("key2", SkMakeSpan(kDataFields));

    SkPaintParamsKeyBuilder builderA(dict, SkBackend::kGraphite);
    SkPaintParamsKeyBuilder builderB(dict, SkBackend::kGraphite);
    SkPaintParamsKeyBuilder builderC(dict, SkBackend::kGraphite);
    SkPaintParamsKey keyA = create_key(&builderA, dummySnippetID1, kBlockDataSize);
    SkPaintParamsKey keyB = create_key(&builderB, dummySnippetID1, kBlockDataSize);
    SkPaintParamsKey keyC = create_key(&builderC, dummySnippetID2, kBlockDataSize);

    // Verify that keyA matches keyB, and that it does not match keyC.
    REPORTER_ASSERT(reporter, keyA == keyB);
    REPORTER_ASSERT(reporter, keyA != keyC);
    REPORTER_ASSERT(reporter, !(keyA == keyC));
    REPORTER_ASSERT(reporter, !(keyA != keyB));
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyEqualityChecksData, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();
    static const int kBlockDataSize = 4;
    static constexpr SkPaintParamsKey::DataPayloadField kDataFields[] = {
            {"data", SkPaintParamsKey::DataPayloadType::kByte, kBlockDataSize},
    };

    int dummySnippetID = dict->addUserDefinedSnippet("key", SkMakeSpan(kDataFields));

    static constexpr uint8_t kData [4] = {1, 2, 3, 4};
    static constexpr uint8_t kData2[4] = {1, 2, 3, 99};

    SkPaintParamsKeyBuilder builderA(dict, SkBackend::kGraphite);
    SkPaintParamsKeyBuilder builderB(dict, SkBackend::kGraphite);
    SkPaintParamsKeyBuilder builderC(dict, SkBackend::kGraphite);
    SkPaintParamsKey keyA = create_key_with_data(&builderA, dummySnippetID, SkMakeSpan(kData));
    SkPaintParamsKey keyB = create_key_with_data(&builderB, dummySnippetID, SkMakeSpan(kData));
    SkPaintParamsKey keyC = create_key_with_data(&builderC, dummySnippetID, SkMakeSpan(kData2));

    // Verify that keyA matches keyB, and that it does not match keyC.
    REPORTER_ASSERT(reporter, keyA == keyB);
    REPORTER_ASSERT(reporter, keyA != keyC);
    REPORTER_ASSERT(reporter, !(keyA == keyC));
    REPORTER_ASSERT(reporter, !(keyA != keyB));
}
