/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLType.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"  // IWYU pragma: keep
#include "src/sksl/ir/SkSLType.h"

#include <memory>
#include <string>

using namespace skia_private;

namespace SkSL {

struct Modifiers;

namespace dsl {

static const SkSL::Type* verify_type(const Context& context,
                                     const SkSL::Type* type,
                                     bool allowGenericTypes,
                                     Position pos) {
    if (!context.fConfig->fIsBuiltinCode && type) {
        if (!allowGenericTypes && (type->isGeneric() || type->isLiteral())) {
            context.fErrors->error(pos, "type '" + std::string(type->name()) + "' is generic");
            return context.fTypes.fPoison.get();
        }
        if (!type->isAllowedInES2(context)) {
            context.fErrors->error(pos, "type '" + std::string(type->name()) +"' is not supported");
            return context.fTypes.fPoison.get();
        }
    }
    return type;
}

static const SkSL::Type* find_type(const Context& context, std::string_view name, Position pos) {
    const Symbol* symbol = context.fSymbolTable->find(name);
    if (!symbol) {
        context.fErrors->error(pos, String::printf("no symbol named '%.*s'",
                                                   (int)name.length(), name.data()));
        return context.fTypes.fPoison.get();
    }
    if (!symbol->is<SkSL::Type>()) {
        context.fErrors->error(pos, String::printf("symbol '%.*s' is not a type",
                                                   (int)name.length(), name.data()));
        return context.fTypes.fPoison.get();
    }
    const SkSL::Type* type = &symbol->as<SkSL::Type>();
    return verify_type(context, type, /*allowGenericTypes=*/false, pos);
}

static const SkSL::Type* find_type(const Context& context,
                                   std::string_view name,
                                   Position overallPos,
                                   Modifiers* modifiers,
                                   Position modifiersPos) {
    const Type* type = find_type(context, name, overallPos);
    return type->applyQualifiers(context, modifiers, modifiersPos);
}

DSLType::DSLType(std::string_view name, Position pos)
        : fSkSLType(find_type(ThreadContext::Context(), name, pos)) {}

DSLType::DSLType(std::string_view name,
                 Position overallPos,
                 SkSL::Modifiers* modifiers,
                 Position modifiersPos)
        : fSkSLType(find_type(ThreadContext::Context(),
                              name,
                              overallPos,
                              modifiers,
                              modifiersPos)) {}

DSLType::DSLType(const SkSL::Type* type, Position pos)
        : fSkSLType(verify_type(ThreadContext::Context(), type, /*allowGenericTypes=*/true, pos)) {}

DSLType DSLType::Invalid() {
    return DSLType(ThreadContext::Context().fTypes.fInvalid.get(), Position());
}

DSLType DSLType::Poison() {
    return DSLType(ThreadContext::Context().fTypes.fPoison.get(), Position());
}

DSLType DSLType::Void() {
    return DSLType(ThreadContext::Context().fTypes.fVoid.get(), Position());
}

bool DSLType::isBoolean() const {
    return this->skslType().isBoolean();
}

bool DSLType::isNumber() const {
    return this->skslType().isNumber();
}

bool DSLType::isFloat() const {
    return this->skslType().isFloat();
}

bool DSLType::isSigned() const {
    return this->skslType().isSigned();
}

bool DSLType::isUnsigned() const {
    return this->skslType().isUnsigned();
}

bool DSLType::isInteger() const {
    return this->skslType().isInteger();
}

bool DSLType::isScalar() const {
    return this->skslType().isScalar();
}

bool DSLType::isVector() const {
    return this->skslType().isVector();
}

bool DSLType::isMatrix() const {
    return this->skslType().isMatrix();
}

bool DSLType::isArray() const {
    return this->skslType().isArray();
}

bool DSLType::isStruct() const {
    return this->skslType().isStruct();
}

bool DSLType::isInterfaceBlock() const {
    return this->skslType().isInterfaceBlock();
}

bool DSLType::isEffectChild() const {
    return this->skslType().isEffectChild();
}

DSLType Array(const DSLType& base, int count, Position pos) {
    SkSL::Context& context = ThreadContext::Context();
    count = base.skslType().convertArraySize(context, pos, pos, count);
    if (!count) {
        return DSLType::Poison();
    }
    return DSLType(context.fSymbolTable->addArrayDimension(&base.skslType(), count), pos);
}

DSLType UnsizedArray(const DSLType& base, Position pos) {
    SkSL::Context& context = ThreadContext::Context();
    if (!base.skslType().checkIfUsableInArray(context, pos)) {
        return DSLType::Poison();
    }
    return context.fSymbolTable->addArrayDimension(&base.skslType(), SkSL::Type::kUnsizedArray);
}

} // namespace dsl

} // namespace SkSL
