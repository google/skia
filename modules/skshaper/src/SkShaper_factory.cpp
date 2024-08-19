/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkShaper_factory.h"

#include "include/core/SkFontMgr.h" // IWYU pragma: keep

namespace {
class PrimitiveFactory final : public SkShapers::Factory {
    std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) override {
        return SkShapers::Primitive::PrimitiveText();
    }
    std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(const char*,
                                                                size_t,
                                                                uint8_t) override {
        return std::make_unique<SkShaper::TrivialBiDiRunIterator>(0, 0);
    }
    std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(const char*,
                                                                 size_t,
                                                                 SkFourByteTag) override {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(0, 0);
    }

    SkUnicode* getUnicode() override {
        return nullptr;
    }
};
}

namespace SkShapers::Primitive {
sk_sp<SkShapers::Factory> Factory() {
    return sk_make_sp<PrimitiveFactory>();
}
}  // namespace SkShapers::Primitive
