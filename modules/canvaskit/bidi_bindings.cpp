/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/canvaskit/WasmCommon.h"
#include "modules/canvaskit/bidi_c_api.h"
#include "modules/skunicode/include/SkUnicode.h"

#include <cstdint>
#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

class BidiPlaceholder {};

class CodeUnitsPlaceholder {};

EMSCRIPTEN_BINDINGS(Bidi) {
    class_<BidiPlaceholder>("Bidi")
            .class_function("_getBidiRegions",
                            optional_override([](JSString jtext, int dir) -> JSArray {
                                std::u16string textStorage = jtext.as<std::u16string>();
                                std::vector<int32_t> regions(textStorage.size() * 3);
                                int regionCount = skbidi::getBidiRegions(
                                        reinterpret_cast<const uint16_t*>(textStorage.data()),
                                        textStorage.size(),
                                        dir,
                                        regions.data());
                                JSArray result = emscripten::val::array();
                                for (int i = 0; i < regionCount * 3; ++i) {
                                    result.call<void>("push", regions[i]);
                                }
                                return result;
                            }),
                            allow_raw_pointers())

            .class_function(
                    "_reorderVisual",
                    optional_override([](WASMPointerU8 runLevels, int levelsCount) -> JSArray {
                        const auto* data = reinterpret_cast<const SkUnicode::BidiLevel*>(runLevels);
                        std::vector<int32_t> logicalFromVisual(levelsCount);
                        JSArray result = emscripten::val::array();
                        if (!skbidi::reorderVisual(data, levelsCount, logicalFromVisual.data())) {
                            return result;
                        }
                        for (int32_t logical : logicalFromVisual) {
                            result.call<void>("push", logical);
                        }
                        return result;
                    }),
                    allow_raw_pointers());

    class_<CodeUnitsPlaceholder>("CodeUnits")
            .class_function("_compute",
                            optional_override([](JSString jtext) -> JSArray {
                                std::u16string textStorage = jtext.as<std::u16string>();
                                std::vector<uint16_t> flags(textStorage.size());
                                JSArray result = emscripten::val::array();
                                if (!skbidi::computeCodeUnitFlags(
                                            reinterpret_cast<const uint16_t*>(textStorage.data()),
                                            textStorage.size(),
                                            flags.data())) {
                                    return result;
                                }
                                for (uint16_t flag : flags) {
                                    result.call<void>("push", flag);
                                }
                                return result;
                            }),
                            allow_raw_pointers());
}
