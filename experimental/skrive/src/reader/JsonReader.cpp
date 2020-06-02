/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>

#include "experimental/skrive/src/reader/StreamReader.h"
#include "src/utils/SkJSON.h"

namespace skrive::internal {
namespace {

class JsonReader final : public StreamReader {
public:
    explicit JsonReader(std::unique_ptr<skjson::DOM> dom)
        : fDom(std::move(dom)) {

    }

private:
    BlockType openBlock() override {
        return BlockType::kUnknown;
    }

    void closeBlock() override {

    }

    const std::unique_ptr<skjson::DOM> fDom;
    const skjson::Value*               fNode;
};

} // namespace

std::unique_ptr<StreamReader> MakeJsonStreamReader(const char json[], size_t len) {
    auto dom = std::make_unique<skjson::DOM>(json, len);

    return dom->root().is<skjson::ObjectValue>() ? std::make_unique<JsonReader>(std::move(dom))
                                                 : nullptr;
}

} // namespace skrive::internal
