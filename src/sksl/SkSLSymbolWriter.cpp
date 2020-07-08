/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSymbolWriter.h"

#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"

#ifdef SKSL_STANDALONE

namespace SkSL {

String SymbolWriter::runInFunction(const FunctionDeclaration& f,
                                   const String& code) {
    return String::printf("((void) symbols.reset(symbol_table_for(\"%s\", symbols)), "
                          "pop_symbols(&symbols, %s))", f.declaration().c_str(), code.c_str());
}

String SymbolWriter::runInSymbolTable(const SymbolTable& symbols,
                                      const String& code) {
    if (symbols.fOwnedSymbols.size()) {
        return String::printf("((void) symbols.reset(%s), pop_symbols(&symbols, %s))",
                              symbols.constructionCode().c_str(), code.c_str());
    } else {
        return code;
    }
}

String SymbolWriter::symbolCode(const Symbol& symbol) {
    const char* typeName;
    switch (symbol.fKind) {
        case Symbol::kFunctionDeclaration_Kind:
            typeName = "FunctionDeclaration";
            break;
        case Symbol::kUnresolvedFunction_Kind:
            SkASSERT(false);
            typeName = "UnresolvedFunction";
            break;
        case Symbol::kType_Kind:
            typeName = "Type";
            break;
        case Symbol::kVariable_Kind:
            typeName = "Variable";
            break;
        case Symbol::kField_Kind:
            typeName = "Field";
            break;
        case Symbol::kExternal_Kind:
            typeName = "External";
            break;
    }
    switch (symbol.fKind) {
        case Symbol::kFunctionDeclaration_Kind: {
            const FunctionDeclaration& f = (const FunctionDeclaration&) symbol;
            return String::printf("get_function(*symbols, \"%s\")", f.declaration().c_str());
        }
        case Symbol::kType_Kind: {
            const Type& t = (const Type&) symbol;
            switch (t.kind()) {
                case Type::kArray_Kind:
                case Type::kNullable_Kind:
                case Type::kStruct_Kind:
                    return t.constructionCode();
                default:
                    break;
            }
            [[fallthrough]];
        }
        default:
            return String::printf("(%s*) (*symbols)[\"%s\"]", typeName,
                                  String(symbol.fName).c_str());
    }
}

} // namespace

#endif
