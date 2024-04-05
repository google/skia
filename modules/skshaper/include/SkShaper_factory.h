/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkShaperFactory_DEFINED
#define SkShaperFactory_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "modules/skshaper/include/SkShaper.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkFontMgr;
class SkUnicode;

namespace SkShapers {

class SKSHAPER_API Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr> fallback) = 0;
    virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(
            const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) = 0;
    virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(
            const char* utf8, size_t utf8Bytes, SkFourByteTag script) = 0;

    virtual SkUnicode* getUnicode() = 0;
};

namespace Primitive {
SKSHAPER_API sk_sp<Factory> Factory();
}

}  // namespace SkShapers

#endif  // SkShaperFactory_DEFINED
