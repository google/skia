/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_harfbuzz_DEFINED
#define SkShaper_harfbuzz_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "modules/skshaper/include/SkShaper.h"

#include <cstddef>
#include <memory>

class SkFontMgr;
class SkUnicode;

namespace SkShapers::HB {
SKSHAPER_API std::unique_ptr<SkShaper> ShaperDrivenWrapper(sk_sp<SkUnicode> unicode,
                                                           sk_sp<SkFontMgr> fallback);
SKSHAPER_API std::unique_ptr<SkShaper> ShapeThenWrap(sk_sp<SkUnicode> unicode,
                                                     sk_sp<SkFontMgr> fallback);
SKSHAPER_API std::unique_ptr<SkShaper> ShapeDontWrapOrReorder(sk_sp<SkUnicode> unicode,
                                                              sk_sp<SkFontMgr> fallback);

SKSHAPER_API std::unique_ptr<SkShaper::ScriptRunIterator> ScriptRunIterator(const char* utf8,
                                                                            size_t utf8Bytes);
SKSHAPER_API std::unique_ptr<SkShaper::ScriptRunIterator> ScriptRunIterator(const char* utf8,
                                                                            size_t utf8Bytes,
                                                                            SkFourByteTag script);

SKSHAPER_API void PurgeCaches();
}  // namespace SkShapers::HB

#endif
