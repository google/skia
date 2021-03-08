/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEHYDRATOR
#define SKSL_DEHYDRATOR

#ifdef SKSL_STANDALONE

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/SkTHash.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLStringStream.h"

#include <set>
#include <unordered_map>
#include <vector>

namespace SkSL {

class Expression;
class ProgramElement;
class Statement;
class Symbol;
class SymbolTable;

// The file has the structure:
//
// uint16 total string length
// string data
// symboltable
// elements
class Dehydrator {
public:
    Dehydrator() {
        fSymbolMap.emplace_back();
    }

    ~Dehydrator() {
        SkASSERT(fSymbolMap.size() == 1);
    }

    void write(const SymbolTable& symbols);

    void write(const std::vector<std::unique_ptr<ProgramElement>>& elements);

    void finish(OutputStream& out);

    // Inserts line breaks at meaningful offsets.
    const char* prefixAtOffset(size_t byte);

private:
    void writeS8(int32_t i) {
        SkASSERT(i >= -128 && i <= 127);
        fBody.write8(i);
    }

    void writeCommand(int32_t c) {
        fCommandBreaks.add(fBody.bytesWritten());
        fBody.write8(c);
    }

    void writeU8(int32_t i) {
        SkASSERT(i >= 0 && i <= 255);
        fBody.write8(i);
    }

    void writeS16(int32_t i) {
        SkASSERT(i >= -32768 && i <= 32767);
        fBody.write16(i);
    }

    void writeU16(int32_t i) {
        SkASSERT(i >= 0 && i <= 65535);
        fBody.write16(i);
    }

    void writeS32(int64_t i) {
        SkASSERT(i >= -2147483648 && i <= 2147483647);
        fBody.write32(i);
    }

    void writeId(const Symbol* s) {
        if (!symbolId(s, false)) {
            fSymbolMap.back()[s] = fNextId++;
        }
        this->writeU16(symbolId(s));
    }

    uint16_t symbolId(const Symbol* s, bool required = true) {
        for (const auto& symbols : fSymbolMap) {
            auto found = symbols.find(s);
            if (found != symbols.end()) {
                return found->second;
            }
        }
        SkASSERT(!required);
        return 0;
    }

    void write(Layout l);

    void write(Modifiers m);

    void write(StringFragment s);

    void write(String s);

    void write(const ProgramElement& e);

    void write(const Expression* e);

    void write(const Statement* s);

    void write(const Symbol& s);

    uint16_t fNextId = 1;

    StringStream fStringBuffer;

    StringStream fBody;

    std::unordered_map<String, int> fStrings;

    std::vector<std::unordered_map<const Symbol*, int>> fSymbolMap;
    SkTHashSet<size_t> fStringBreaks;
    SkTHashSet<size_t> fCommandBreaks;
    size_t fStringBufferStart;
    size_t fCommandStart;

    friend class AutoDehydratorSymbolTable;
};

} // namespace

#endif

#endif
