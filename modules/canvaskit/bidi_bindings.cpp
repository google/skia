/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/private/base/SkOnce.h"
#include "modules/skunicode/include/SkUnicode.h"

#if defined(CK_ENABLE_BIDI)
#include "modules/skunicode/include/SkUnicode_bidi.h"
#else
#error "SkUnicode bidi component is required but missing"
#endif

#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>
#include "modules/canvaskit/WasmCommon.h"

using namespace emscripten;
using namespace skia_private;

void JSArrayFromBidiRegions(JSArray& array, std::vector<SkUnicode::BidiRegion>& regions) {
    for (auto region : regions) {
        array.call<void>("push", region.start);
        array.call<void>("push", region.end);
        array.call<void>("push", (int32_t)region.level);
    }
}

class BidiPlaceholder { };

class CodeUnitsPlaceholder { };

static sk_sp<SkUnicode> getClientUnicode() {
    static sk_sp<SkUnicode> unicode;
    static SkOnce once;
    once([] { unicode = SkUnicodes::Bidi::Make(); });
    return unicode;
}

EMSCRIPTEN_BINDINGS(Bidi) {
    class_<BidiPlaceholder>("Bidi")
        .class_function("_getBidiRegions",
                  optional_override([](JSString jtext, int dir) -> JSArray {
                      std::string textStorage = jtext.as<std::string>();
                      const char* text = textStorage.data();
                      size_t textCount = textStorage.size();
                      JSArray result = emscripten::val::array();
                      std::vector<SkUnicode::BidiRegion> regions;
                      SkUnicode::TextDirection textDirection =
                              dir == 0 ? SkUnicode::TextDirection::kLTR
                                       : SkUnicode::TextDirection::kRTL;
                      if (!getClientUnicode()->getBidiRegions(text, textCount, textDirection, &regions)) {
                          return result;
                      }
                      JSArrayFromBidiRegions(result, regions);
                      return result;
                  }),
                  allow_raw_pointers())

            .class_function("_reorderVisual",
                optional_override([](WASMPointerU8 runLevels,
                                   int levelsCount) -> JSArray {
                    // Convert WASMPointerU8 to std::vector<SkUnicode::BidiLevel>
                    SkUnicode::BidiLevel* data = reinterpret_cast<SkUnicode::BidiLevel*>(runLevels);

                    // The resulting vector
                    std::vector<int32_t> logicalFromVisual;
                    logicalFromVisual.resize(levelsCount);
                    getClientUnicode()->reorderVisual(data, levelsCount, logicalFromVisual.data());

                    // Convert std::vector<int32_t> to JSArray
                    JSArray result = emscripten::val::array();
                    for (auto logical : logicalFromVisual) {
                        result.call<void>("push", logical);
                    }
                    return result;
                }),
                allow_raw_pointers());

    class_<CodeUnitsPlaceholder>("CodeUnits")
        .class_function("_compute",
            optional_override([](JSString jtext) -> JSArray {
              std::u16string textStorage = jtext.as<std::u16string>();
              char16_t * text = textStorage.data();
              size_t textCount = textStorage.size();
              skia_private::TArray<SkUnicode::CodeUnitFlags, true> flags;
              flags.resize(textCount);
              JSArray result = emscripten::val::array();
              if (!getClientUnicode()->computeCodeUnitFlags(
                          text, textCount, /*replaceTabs=*/false, &flags)) {
                  return result;
              }
              for (auto flag : flags) {
                  result.call<void>("push", (uint16_t)flag);
              }
              return result;
            }),
            allow_raw_pointers());
}
