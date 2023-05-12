/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLType.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSLModifiers.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

#include <memory>
#include <string>
#include <vector>

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

static const SkSL::Type* find_type(const Context& context,
                                   Position pos,
                                   std::string_view name) {
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
                                   Position overallPos,
                                   std::string_view name,
                                   Position modifiersPos,
                                   Modifiers* modifiers) {
    const Type* type = find_type(context, overallPos, name);
    return type->applyQualifiers(context, modifiers, modifiersPos);
}

DSLType::DSLType(std::string_view name, Position pos)
        : fSkSLType(find_type(ThreadContext::Context(), pos, name)) {}

DSLType::DSLType(std::string_view name, DSLModifiers* modifiers, Position pos)
        : fSkSLType(find_type(ThreadContext::Context(),
                              pos,
                              name,
                              modifiers->fPosition,
                              &modifiers->fModifiers)) {}

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

DSLType StructType(std::string_view name,
                   TArray<Field> fields,
                   bool interfaceBlock,
                   Position pos) {
    SkSL::Context& context = ThreadContext::Context();
    std::unique_ptr<Type> newType = Type::MakeStructType(context, pos, name, std::move(fields),
                                                         interfaceBlock);
    return DSLType(context.fSymbolTable->add(std::move(newType)), pos);
}

DSLType Struct(std::string_view name, TArray<Field> fields, Position pos) {
    DSLType result = StructType(name, std::move(fields), /*interfaceBlock=*/false, pos);
    ThreadContext::ProgramElements().push_back(
            std::make_unique<SkSL::StructDefinition>(pos, result.skslType()));
    return result;
}

} // namespace dsl

} // namespace SkSL
