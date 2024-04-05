/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkShaperFactoryHelpers_DEFINED
#define SkShaperFactoryHelpers_DEFINED

#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/include/SkShaper_factory.h"

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"
#endif

#if defined(SK_SHAPER_CORETEXT_AVAILABLE)
#include "modules/skshaper/include/SkShaper_coretext.h"
#endif

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

namespace SkShapers {
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
class HarfbuzzFactory final : public Factory {
public:
    HarfbuzzFactory() {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
        fUnicode = SkUnicodes::ICU::Make();
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
        if (!fUnicode) {
            fUnicode = SkUnicodes::ICU4X::Make();
        }
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
        if (!fUnicode) {
            fUnicode = SkUnicodes::Libgrapheme::Make();
        }
#endif
    }
    std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr> fallback) override {
        return SkShapers::HB::ShaperDrivenWrapper(fUnicode, fallback);
    }

    std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(const char* utf8,
                                                                size_t utf8Bytes,
                                                                uint8_t bidiLevel) override {
        return SkShapers::unicode::BidiRunIterator(fUnicode, utf8, utf8Bytes, bidiLevel);
    }

    std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(const char* utf8,
                                                                 size_t utf8Bytes,
                                                                 SkFourByteTag script) override {
        return SkShapers::HB::ScriptRunIterator(utf8, utf8Bytes, script);
    }

    SkUnicode* getUnicode() override { return fUnicode.get(); }

private:
    sk_sp<SkUnicode> fUnicode;
};
#endif  // defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)

#if defined(SK_SHAPER_CORETEXT_AVAILABLE)
class CoreTextFactory final : public Factory {
    std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) override {
        return SkShapers::CT::CoreText();
    }
    std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(const char* utf8,
                                                                size_t utf8Bytes,
                                                                uint8_t bidiLevel) override {
        return std::make_unique<SkShaper::TrivialBiDiRunIterator>(0, 0);
    }
    std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(const char* utf8,
                                                                 size_t utf8Bytes,
                                                                 SkFourByteTag script) override {
        return std::make_unique<SkShaper::TrivialScriptRunIterator>(0, 0);
    }
    SkUnicode* getUnicode() override { return nullptr; }
};
#endif  // defined(SK_SHAPER_CORETEXT_AVAILABLE)

// This convenience function will return a set of callbacks that has the "best" text shaping
// depending on what parts of Skia the client has compiled in. For example, if the clients
// have compiled in SkShaper and a version of SkUnicode, callbacks which produce the
// appropriate types will be returned.
//
// This must be inline (and defined in this header) because the *client* has to compile this code
// with all defines set by *their* dependencies (which may include defines from SkShaper and
// SkUnicode modules).
inline sk_sp<Factory> BestAvailable() {
#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
    return sk_make_sp<SkShapers::HarfbuzzFactory>();
#elif defined(SK_SHAPER_CORETEXT_AVAILABLE)
    return sk_make_sp<SkShapers::CoreTextFactory>();
#else
    return SkShapers::Primitive::Factory();
#endif
}

};  // namespace SkShapers

#endif  // SkShaperFactoryHelpers_DEFINED
