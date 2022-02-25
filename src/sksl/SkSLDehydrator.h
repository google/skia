/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEHYDRATOR
#define SKSL_DEHYDRATOR

#include "include/core/SkSpan.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTHash.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLStringStream.h"

#include <unordered_map>
#include <vector>

namespace SkSL {

class AnyConstructor;
class Expression;
struct Program;
class ProgramElement;
class Statement;
class Symbol;
class SymbolTable;

/**
 * Converts SkSL objects into a binary file. See binary_format.md for a description of the file
 * format.
 */
class Dehydrator {
public:
    Dehydrator() {
        fSymbolMap.emplace_back();
    }

    ~Dehydrator() {
        SkASSERT(fSymbolMap.size() == 1);
    }

    void write(const Program& program);

    void write(const SymbolTable& symbols);

    void write(const std::vector<std::unique_ptr<ProgramElement>>& elements);

    void finish(OutputStream& out);

    // Inserts line breaks at meaningful offsets.
    const char* prefixAtOffset(size_t byte);

private:
    void writeS8(int32_t i) {
        SkASSERT(SkTFitsIn<int8_t>(i));
        fBody.write8(i);
    }

    void writeCommand(int32_t c) {
        fCommandBreaks.add(fBody.bytesWritten());
        fBody.write8(c);
    }

    void writeU8(int32_t i) {
        SkASSERT(SkTFitsIn<uint8_t>(i));
        fBody.write8(i);
    }

    void writeS16(int32_t i) {
        SkASSERT(SkTFitsIn<int16_t>(i));
        fBody.write16(i);
    }

    void writeU16(int32_t i) {
        SkASSERT(SkTFitsIn<uint16_t>(i));
        fBody.write16(i);
    }

    void writeS32(int64_t i) {
        SkASSERT(SkTFitsIn<int32_t>(i));
        fBody.write32(i);
    }

    void writeU32(int64_t i) {
        SkASSERT(SkTFitsIn<uint32_t>(i));
        fBody.write32(i);
    }

    void allocSymbolId(const Symbol* s) {
        SkASSERT(!symbolId(s));
        fSymbolMap.back()[s] = fNextId++;
    }

    void writeId(const Symbol* s);

    uint16_t symbolId(const Symbol* s) {
        for (const auto& symbols : fSymbolMap) {
            auto found = symbols.find(s);
            if (found != symbols.end()) {
                return found->second;
            }
        }
        return 0;
    }

    void write(Layout l);

    void write(Modifiers m);

    void write(std::string_view s);

    void write(std::string s);

    void write(const ProgramElement& e);

    void write(const Expression* e);

    void write(const Statement* s);

    void write(const Symbol& s);

    void writeExpressionSpan(const SkSpan<const std::unique_ptr<Expression>>& span);

    uint16_t fNextId = 1;

    StringStream fStringBuffer;

    StringStream fBody;

    std::unordered_map<std::string, int> fStrings;

    std::vector<std::unordered_map<const Symbol*, int>> fSymbolMap;
    SkTHashSet<size_t> fStringBreaks;
    SkTHashSet<size_t> fCommandBreaks;
    size_t fStringBufferStart;
    size_t fCommandStart;

    friend class AutoDehydratorSymbolTable;
};

} // namespace SkSL

#endif
