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
