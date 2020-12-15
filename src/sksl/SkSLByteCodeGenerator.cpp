/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLByteCodeGenerator.h"

#include <algorithm>

namespace SkSL {

static TypeCategory type_category(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kVector:
        case Type::TypeKind::kMatrix:
            return type_category(type.componentType());
        default:
            if (type.isBoolean()) {
                return TypeCategory::kBool;
            }
            const StringFragment& name = type.name();
            if (name == "int" ||
                name == "short" ||
                name == "$intLiteral") {
                return TypeCategory::kSigned;
            }
            if (name == "uint" ||
                name == "ushort") {
                return TypeCategory::kUnsigned;
            }
            SkASSERT(name == "float" ||
                     name == "half" ||
                     name == "$floatLiteral");
            return TypeCategory::kFloat;
    }
}


ByteCodeGenerator::ByteCodeGenerator(const Context* context, const Program* program,
                                     ErrorReporter* errors, ByteCode* output)
    : INHERITED(program, errors, nullptr)
    , fContext(*context)
    , fOutput(output)
    , fSynthetics(errors, /*builtin=*/true)
    // If you're adding new intrinsics here, ensure that they're declared in sksl_interp.sksl or
    // sksl_public.sksl, so they're available to "generic" interpreter programs (eg particles).
    // You can probably copy the declarations from sksl_gpu.sksl.
    , fIntrinsics {
        { "abs",         ByteCodeInstruction::kAbs },
        { "acos",        ByteCodeInstruction::kACos },
        { "asin",        ByteCodeInstruction::kASin },
        { "atan",        SpecialIntrinsic::kATan },
        { "ceil",        ByteCodeInstruction::kCeil },
        { "clamp",       SpecialIntrinsic::kClamp },
        { "cos",         ByteCodeInstruction::kCos },
        { "distance",    SpecialIntrinsic::kDistance },
        { "dot",         SpecialIntrinsic::kDot },
        { "exp",         ByteCodeInstruction::kExp },
        { "exp2",        ByteCodeInstruction::kExp2 },
        { "floor",       ByteCodeInstruction::kFloor },
        { "fract",       ByteCodeInstruction::kFract },
        { "inverse",     ByteCodeInstruction::kInverse2x2 },
        { "inversesqrt", ByteCodeInstruction::kInvSqrt },
        { "length",      SpecialIntrinsic::kLength },
        { "log",         ByteCodeInstruction::kLog },
        { "log2",        ByteCodeInstruction::kLog2 },
        { "max",         SpecialIntrinsic::kMax },
        { "min",         SpecialIntrinsic::kMin },
        { "mix",         SpecialIntrinsic::kMix },
        { "mod",         SpecialIntrinsic::kMod },
        { "normalize",   SpecialIntrinsic::kNormalize },
        { "pow",         ByteCodeInstruction::kPow },
        { "sample",      SpecialIntrinsic::kSample },
        { "saturate",    SpecialIntrinsic::kSaturate },
        { "sign",        ByteCodeInstruction::kSign },
        { "sin",         ByteCodeInstruction::kSin },
        { "smoothstep",  SpecialIntrinsic::kSmoothstep },
        { "step",        SpecialIntrinsic::kStep },
        { "sqrt",        ByteCodeInstruction::kSqrt },
        { "tan",         ByteCodeInstruction::kTan },

        { "lessThan",         { ByteCodeInstruction::kCompareFLT,
                                ByteCodeInstruction::kCompareSLT,
                                ByteCodeInstruction::kCompareULT } },
        { "lessThanEqual",    { ByteCodeInstruction::kCompareFLTEQ,
                                ByteCodeInstruction::kCompareSLTEQ,
                                ByteCodeInstruction::kCompareULTEQ } },
        { "greaterThan",      { ByteCodeInstruction::kCompareFGT,
                                ByteCodeInstruction::kCompareSGT,
                                ByteCodeInstruction::kCompareUGT } },
        { "greaterThanEqual", { ByteCodeInstruction::kCompareFGTEQ,
                                ByteCodeInstruction::kCompareSGTEQ,
                                ByteCodeInstruction::kCompareUGTEQ } },
        { "equal",            { ByteCodeInstruction::kCompareFEQ,
                                ByteCodeInstruction::kCompareIEQ,
                                ByteCodeInstruction::kCompareIEQ } },
        { "notEqual",         { ByteCodeInstruction::kCompareFNEQ,
                                ByteCodeInstruction::kCompareINEQ,
                                ByteCodeInstruction::kCompareINEQ } },

        { "any", SpecialIntrinsic::kAny },
        { "all", SpecialIntrinsic::kAll },
        { "not", ByteCodeInstruction::kNotB },
    } {}


int ByteCodeGenerator::SlotCount(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kOther:
            return 0;
        case Type::TypeKind::kStruct: {
            int slots = 0;
            for (const auto& f : type.fields()) {
                slots += SlotCount(*f.fType);
            }
            SkASSERT(slots <= 255);
            return slots;
        }
        case Type::TypeKind::kArray: {
            int columns = type.columns();
            SkASSERT(columns >= 0);
            int slots = columns * SlotCount(type.componentType());
            SkASSERT(slots <= 255);
            return slots;
        }
        default:
            return type.columns() * type.rows();
    }
}

static inline bool is_uniform(const SkSL::Variable& var) {
    return var.modifiers().fFlags & Modifiers::kUniform_Flag;
}

static inline bool is_in(const SkSL::Variable& var) {
    return var.modifiers().fFlags & Modifiers::kIn_Flag;
}

void ByteCodeGenerator::gatherUniforms(const Type& type, const String& name) {
    switch (type.typeKind()) {
        case Type::TypeKind::kOther:
            break;
        case Type::TypeKind::kStruct:
            for (const auto& f : type.fields()) {
                this->gatherUniforms(*f.fType, name + "." + f.fName);
            }
            break;
        case Type::TypeKind::kArray:
            for (int i = 0; i < type.columns(); ++i) {
                this->gatherUniforms(type.componentType(), String::printf("%s[%d]", name.c_str(),
                                     i));
            }
            break;
        default:
            fOutput->fUniforms.push_back({ name, type_category(type), type.rows(), type.columns(),
                                           fOutput->fUniformSlotCount });
            fOutput->fUniformSlotCount += type.columns() * type.rows();
    }
}

bool ByteCodeGenerator::generateCode() {
    for (const ProgramElement* e : fProgram.elements()) {
        switch (e->kind()) {
            case ProgramElement::Kind::kFunction: {
                std::unique_ptr<ByteCodeFunction> f =
                        this->writeFunction(e->as<FunctionDefinition>());
                if (!f) {
                    return false;
                }
                fOutput->fFunctions.push_back(std::move(f));
                fFunctions.push_back(&e->as<FunctionDefinition>());
                break;
            }
            case ProgramElement::Kind::kGlobalVar: {
                const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
                const Variable& declVar = decl.declaration()->as<VarDeclaration>().var();
                if (declVar.type() == *fContext.fFragmentProcessor_Type) {
                    fOutput->fChildFPCount++;
                }
                if (declVar.modifiers().fLayout.fBuiltin >= 0 || is_in(declVar)) {
                    continue;
                }
                if (is_uniform(declVar)) {
                    this->gatherUniforms(declVar.type(), declVar.name());
                } else {
                    fOutput->fGlobalSlotCount += SlotCount(declVar.type());
                }
                break;
            }
            default:
                ; // ignore
        }
    }
    return 0 == fErrors.errorCount();
}

std::unique_ptr<ByteCodeFunction> ByteCodeGenerator::writeFunction(const FunctionDefinition& f) {
    fFunction = &f;
    std::unique_ptr<ByteCodeFunction> result(new ByteCodeFunction(&f.declaration()));
    fParameterCount = result->fParameterCount;
    fLoopCount = fMaxLoopCount = 0;
    fConditionCount = fMaxConditionCount = 0;
    fStackCount = fMaxStackCount = 0;
    fCode = &result->fCode;

    this->writeStatement(*f.body());
    if (0 == fErrors.errorCount()) {
        SkASSERT(fLoopCount == 0);
        SkASSERT(fConditionCount == 0);
        SkASSERT(fStackCount == 0);
    }
    this->write(ByteCodeInstruction::kReturn, 0);

    result->fLocalCount     = fLocals.size();
    result->fConditionCount = fMaxConditionCount;
    result->fLoopCount      = fMaxLoopCount;
    result->fStackCount     = fMaxStackCount;

    const Type& returnType = f.declaration().returnType();
    if (returnType != *fContext.fVoid_Type) {
        result->fReturnCount = SlotCount(returnType);
    }
    fLocals.clear();
    fFunction = nullptr;
    return result;
}

// If the expression is a reference to a builtin global variable, return the builtin ID.
// Otherwise, return -1.
static int expression_as_builtin(const Expression& e) {
    if (e.is<VariableReference>()) {
        const Variable& var(*e.as<VariableReference>().variable());
        if (var.storage() == Variable::Storage::kGlobal) {
            return var.modifiers().fLayout.fBuiltin;
        }
    }
    return -1;
}

// A "simple" Swizzle is based on a variable (or a compound variable like a struct or array), and
// that references consecutive values, such that it can be implemented using normal load/store ops
// with an offset. Note that all single-component swizzles (of suitable base types) are simple.
static bool swizzle_is_simple(const Swizzle& s) {
    // Builtin variables use dedicated instructions that don't allow subset loads
    if (expression_as_builtin(*s.base()) >= 0) {
        return false;
    }

    switch (s.base()->kind()) {
        case Expression::Kind::kFieldAccess:
        case Expression::Kind::kIndex:
        case Expression::Kind::kVariableReference:
            break;
        default:
            return false;
    }

    for (size_t i = 1; i < s.components().size(); ++i) {
        if (s.components()[i] != s.components()[i - 1] + 1) {
            return false;
        }
    }
    return true;
}

int ByteCodeGenerator::StackUsage(ByteCodeInstruction inst, int count_) {
    // Ensures that we use count iff we're passed a non-default value. Most instructions have an
    // implicit count, so the caller shouldn't need to worry about it (or count makes no sense).
    // The asserts avoids callers thinking they're supplying useful information in that scenario,
    // or failing to supply necessary information for the ops that need a count.
    struct CountValue {
        operator int() {
            SkASSERT(val != ByteCodeGenerator::kUnusedStackCount);
            SkDEBUGCODE(used = true);
            return val;
        }
        ~CountValue() {
            SkASSERT(used || val == ByteCodeGenerator::kUnusedStackCount);
        }
        int val;
        SkDEBUGCODE(bool used = false;)
    } count = { count_ };

    switch (inst) {
        // Unary functions/operators that don't change stack depth at all:

#define VEC_UNARY(inst) case ByteCodeInstruction::inst: return count - count;

        VEC_UNARY(kConvertFtoI)
        VEC_UNARY(kConvertStoF)
        VEC_UNARY(kConvertUtoF)

        VEC_UNARY(kAbs)
        VEC_UNARY(kACos)
        VEC_UNARY(kASin)
        VEC_UNARY(kATan)
        VEC_UNARY(kCeil)
        VEC_UNARY(kCos)
        VEC_UNARY(kExp)
        VEC_UNARY(kExp2)
        VEC_UNARY(kFloor)
        VEC_UNARY(kFract)
        VEC_UNARY(kInvSqrt)
        VEC_UNARY(kLog)
        VEC_UNARY(kLog2)
        VEC_UNARY(kSign)
        VEC_UNARY(kSin)
        VEC_UNARY(kSqrt)
        VEC_UNARY(kTan)

        VEC_UNARY(kNegateF)
        VEC_UNARY(kNegateI)
        VEC_UNARY(kNotB)

#undef VEC_UNARY

        case ByteCodeInstruction::kInverse2x2:
        case ByteCodeInstruction::kInverse3x3:
        case ByteCodeInstruction::kInverse4x4: return 0;

        case ByteCodeInstruction::kClampIndex:  return 0;
        case ByteCodeInstruction::kShiftLeft:   return 0;
        case ByteCodeInstruction::kShiftRightS: return 0;
        case ByteCodeInstruction::kShiftRightU: return 0;

        // Binary functions/operators that do a 2 -> 1 reduction, N times
        case ByteCodeInstruction::kAndB: return -count;
        case ByteCodeInstruction::kOrB:  return -count;
        case ByteCodeInstruction::kXorB: return -count;

        case ByteCodeInstruction::kAddI: return -count;
        case ByteCodeInstruction::kAddF: return -count;

        case ByteCodeInstruction::kATan2: return -count;
        case ByteCodeInstruction::kMod:   return -count;
        case ByteCodeInstruction::kStep:  return -count;

        case ByteCodeInstruction::kCompareIEQ:   return -count;
        case ByteCodeInstruction::kCompareFEQ:   return -count;
        case ByteCodeInstruction::kCompareINEQ:  return -count;
        case ByteCodeInstruction::kCompareFNEQ:  return -count;
        case ByteCodeInstruction::kCompareSGT:   return -count;
        case ByteCodeInstruction::kCompareUGT:   return -count;
        case ByteCodeInstruction::kCompareFGT:   return -count;
        case ByteCodeInstruction::kCompareSGTEQ: return -count;
        case ByteCodeInstruction::kCompareUGTEQ: return -count;
        case ByteCodeInstruction::kCompareFGTEQ: return -count;
        case ByteCodeInstruction::kCompareSLT:   return -count;
        case ByteCodeInstruction::kCompareULT:   return -count;
        case ByteCodeInstruction::kCompareFLT:   return -count;
        case ByteCodeInstruction::kCompareSLTEQ: return -count;
        case ByteCodeInstruction::kCompareULTEQ: return -count;
        case ByteCodeInstruction::kCompareFLTEQ: return -count;

        case ByteCodeInstruction::kDivideS:    return -count;
        case ByteCodeInstruction::kDivideU:    return -count;
        case ByteCodeInstruction::kDivideF:    return -count;
        case ByteCodeInstruction::kMaxF:       return -count;
        case ByteCodeInstruction::kMaxS:       return -count;
        case ByteCodeInstruction::kMinF:       return -count;
        case ByteCodeInstruction::kMinS:       return -count;
        case ByteCodeInstruction::kMultiplyI:  return -count;
        case ByteCodeInstruction::kMultiplyF:  return -count;
        case ByteCodeInstruction::kPow:        return -count;
        case ByteCodeInstruction::kRemainderF: return -count;
        case ByteCodeInstruction::kRemainderS: return -count;
        case ByteCodeInstruction::kRemainderU: return -count;
        case ByteCodeInstruction::kSubtractI:  return -count;
        case ByteCodeInstruction::kSubtractF:  return -count;

        // Ops that push or load data to grow the stack:
        case ByteCodeInstruction::kPushImmediate:
            return 1;
        case ByteCodeInstruction::kLoadFragCoord:
            return 4;

        case ByteCodeInstruction::kDup:
        case ByteCodeInstruction::kLoad:
        case ByteCodeInstruction::kLoadGlobal:
        case ByteCodeInstruction::kLoadUniform:
        case ByteCodeInstruction::kReadExternal:
        case ByteCodeInstruction::kReserve:
            return count;

        // Pushes 'count' values, minus one for the 'address' that's consumed first
        case ByteCodeInstruction::kLoadExtended:
        case ByteCodeInstruction::kLoadExtendedGlobal:
        case ByteCodeInstruction::kLoadExtendedUniform:
            return count - 1;

        // Ops that pop or store data to shrink the stack:
        case ByteCodeInstruction::kPop:
        case ByteCodeInstruction::kReturn:
        case ByteCodeInstruction::kStore:
        case ByteCodeInstruction::kStoreGlobal:
        case ByteCodeInstruction::kWriteExternal:
            return -count;

        // Consumes 'count' values, plus one for the 'address'
        case ByteCodeInstruction::kStoreExtended:
        case ByteCodeInstruction::kStoreExtendedGlobal:
            return -count - 1;

        // Strange ops where the caller computes the delta for us:
        case ByteCodeInstruction::kCallExternal:
        case ByteCodeInstruction::kMatrixToMatrix:
        case ByteCodeInstruction::kMatrixMultiply:
        case ByteCodeInstruction::kScalarToMatrix:
        case ByteCodeInstruction::kSwizzle:
            return count;

        // Miscellaneous

        // () -> (R, G, B, A)
        case ByteCodeInstruction::kSample: return 4;
        // (X, Y) -> (R, G, B, A)
        case ByteCodeInstruction::kSampleExplicit: return 4 - 2;
        // (float3x3) -> (R, G, B, A)
        case ByteCodeInstruction::kSampleMatrix: return 4 - 9;

        // kMix does a 3 -> 1 reduction (A, B, M -> A -or- B) for each component
        case ByteCodeInstruction::kMix:  return -(2 * count);

        // kLerp works the same way (producing lerp(A, B, T) for each component)
        case ByteCodeInstruction::kLerp:  return -(2 * count);

        // kCall is net-zero. Max stack depth is adjusted in writeFunctionCall.
        case ByteCodeInstruction::kCall:             return 0;
        case ByteCodeInstruction::kBranch:           return 0;
        case ByteCodeInstruction::kBranchIfAllFalse: return 0;

        case ByteCodeInstruction::kMaskPush:         return -1;
        case ByteCodeInstruction::kMaskPop:          return 0;
        case ByteCodeInstruction::kMaskNegate:       return 0;
        case ByteCodeInstruction::kMaskBlend:        return -count;

        case ByteCodeInstruction::kLoopBegin:        return 0;
        case ByteCodeInstruction::kLoopNext:         return 0;
        case ByteCodeInstruction::kLoopMask:         return -1;
        case ByteCodeInstruction::kLoopEnd:          return 0;
        case ByteCodeInstruction::kLoopBreak:        return 0;
        case ByteCodeInstruction::kLoopContinue:     return 0;
    }

    SkUNREACHABLE;
}

ByteCodeGenerator::Location ByteCodeGenerator::getLocation(const Variable& var) {
    // given that we seldom have more than a couple of variables, linear search is probably the most
    // efficient way to handle lookups
    switch (var.storage()) {
        case Variable::Storage::kLocal: {
            for (int i = fLocals.size() - 1; i >= 0; --i) {
                if (fLocals[i] == &var) {
                    SkASSERT(fParameterCount + i <= 255);
                    return { fParameterCount + i, Storage::kLocal };
                }
            }
            int result = fParameterCount + fLocals.size();
            fLocals.push_back(&var);
            for (int i = 0; i < SlotCount(var.type()) - 1; ++i) {
                fLocals.push_back(nullptr);
            }
            SkASSERT(result <= 255);
            return { result, Storage::kLocal };
        }
        case Variable::Storage::kParameter: {
            int offset = 0;
            for (const auto& p : fFunction->declaration().parameters()) {
                if (p == &var) {
                    SkASSERT(offset <= 255);
                    return { offset, Storage::kLocal };
                }
                offset += SlotCount(p->type());
            }
            SkASSERT(false);
            return Location::MakeInvalid();
        }
        case Variable::Storage::kGlobal: {
            if (var.type() == *fContext.fFragmentProcessor_Type) {
                int offset = 0;
                for (const ProgramElement* e : fProgram.elements()) {
                    if (e->is<GlobalVarDeclaration>()) {
                        const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
                        const Variable& declVar = decl.declaration()->as<VarDeclaration>().var();
                        if (declVar.type() != *fContext.fFragmentProcessor_Type) {
                            continue;
                        }
                        if (&declVar == &var) {
                            SkASSERT(offset <= 255);
                            return { offset, Storage::kChildFP };
                        }
                        offset++;
                    }
                }
                SkASSERT(false);
                return Location::MakeInvalid();
            }
            if (is_in(var)) {
                // If you see this error, it means the program is using raw 'in' variables. You
                // should either specialize the program (Compiler::specialize) to bake in the final
                // values of the 'in' variables, or not use 'in' variables (maybe you meant to use
                // 'uniform' instead?).
                fErrors.error(var.fOffset,
                              "'in' variable is not specialized or has unsupported type");
                return Location::MakeInvalid();
            }
            int offset = 0;
            bool isUniform = is_uniform(var);
            for (const ProgramElement* e : fProgram.elements()) {
                if (e->is<GlobalVarDeclaration>()) {
                    const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
                    const Variable& declVar = decl.declaration()->as<VarDeclaration>().var();
                    if (declVar.modifiers().fLayout.fBuiltin >= 0 || is_in(declVar)) {
                        continue;
                    }
                    if (isUniform != is_uniform(declVar)) {
                        continue;
                    }
                    if (&declVar == &var) {
                        SkASSERT(offset <= 255);
                        return  { offset, isUniform ? Storage::kUniform : Storage::kGlobal };
                    }
                    offset += SlotCount(declVar.type());
                }
            }
            SkASSERT(false);
            return Location::MakeInvalid();
        }
        default:
            SkASSERT(false);
            return Location::MakeInvalid();
    }
}

ByteCodeGenerator::Location ByteCodeGenerator::getLocation(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& f = expr.as<FieldAccess>();
            Location baseLoc = this->getLocation(*f.base());
            int offset = 0;
            for (int i = 0; i < f.fieldIndex(); ++i) {
                offset += SlotCount(*f.base()->type().fields()[i].fType);
            }
            if (baseLoc.isOnStack()) {
                if (offset != 0) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(offset);
                    this->write(ByteCodeInstruction::kAddI, 1);
                }
                return baseLoc;
            } else {
                return baseLoc + offset;
            }
        }
        case Expression::Kind::kIndex: {
            const IndexExpression& i = expr.as<IndexExpression>();
            int stride = SlotCount(i.type());
            int length = i.base()->type().columns();
            SkASSERT(length <= 255);
            int offset = -1;
            const Expression& base = *i.base();
            const Expression& index = *i.index();
            if (index.isCompileTimeConstant()) {
                int64_t indexValue = index.getConstantInt();
                if (indexValue < 0 || indexValue >= length) {
                    fErrors.error(index.fOffset, "Array index out of bounds.");
                    return Location::MakeInvalid();
                }
                offset = indexValue * stride;
            } else {
                if (index.hasSideEffects()) {
                    // Having a side-effect in an indexer is technically safe for an rvalue,
                    // but with lvalues we have to evaluate the indexer twice, so make it an error.
                    fErrors.error(index.fOffset,
                            "Index expressions with side-effects not supported in byte code.");
                    return Location::MakeInvalid();
                }
                this->writeExpression(index);
                this->write(ByteCodeInstruction::kClampIndex);
                this->write8(length);
                if (stride != 1) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(stride);
                    this->write(ByteCodeInstruction::kMultiplyI, 1);
                }
            }
            Location baseLoc = this->getLocation(base);

            // Are both components known statically?
            if (!baseLoc.isOnStack() && offset >= 0) {
                return baseLoc + offset;
            }

            // At least one component is dynamic (and on the stack).

            // If the other component is zero, we're done
            if (baseLoc.fSlot == 0 || offset == 0) {
                return baseLoc.makeOnStack();
            }

            // Push the non-dynamic component (if any) to the stack, then add the two
            if (!baseLoc.isOnStack()) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(baseLoc.fSlot);
            }
            if (offset >= 0) {
                this->write(ByteCodeInstruction::kPushImmediate);
                this->write32(offset);
            }
            this->write(ByteCodeInstruction::kAddI, 1);
            return baseLoc.makeOnStack();
        }
        case Expression::Kind::kSwizzle: {
            const Swizzle& s = expr.as<Swizzle>();
            SkASSERT(swizzle_is_simple(s));
            Location baseLoc = this->getLocation(*s.base());
            int offset = s.components()[0];
            if (baseLoc.isOnStack()) {
                if (offset != 0) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(offset);
                    this->write(ByteCodeInstruction::kAddI, 1);
                }
                return baseLoc;
            } else {
                return baseLoc + offset;
            }
        }
        case Expression::Kind::kVariableReference: {
            const Variable& var = *expr.as<VariableReference>().variable();
            return this->getLocation(var);
        }
        default:
            SkASSERT(false);
            return Location::MakeInvalid();
    }
}

void ByteCodeGenerator::write8(uint8_t b) {
    fCode->push_back(b);
}

void ByteCodeGenerator::write16(uint16_t i) {
    size_t n = fCode->size();
    fCode->resize(n+2);
    memcpy(fCode->data() + n, &i, 2);
}

void ByteCodeGenerator::write32(uint32_t i) {
    size_t n = fCode->size();
    fCode->resize(n+4);
    memcpy(fCode->data() + n, &i, 4);
}

void ByteCodeGenerator::write(ByteCodeInstruction i, int count) {
    switch (i) {
        case ByteCodeInstruction::kLoopBegin: this->enterLoop();      break;
        case ByteCodeInstruction::kLoopEnd:   this->exitLoop();       break;

        case ByteCodeInstruction::kMaskPush:  this->enterCondition(); break;
        case ByteCodeInstruction::kMaskPop:
        case ByteCodeInstruction::kMaskBlend: this->exitCondition();  break;
        default: /* Do nothing */ break;
    }
    this->write8((uint8_t)i);
    fStackCount += StackUsage(i, count);
    fMaxStackCount = std::max(fMaxStackCount, fStackCount);

    // Most ops have an explicit count byte after them (passed here as 'count')
    // Ops that don't have a count byte pass the default (kUnusedStackCount)
    // There are a handful of strange ops that pass in a computed stack delta as count, but where
    // that value should *not* be written as a count byte (it may even be negative!)
    if (count != kUnusedStackCount) {
        switch (i) {
            // Odd instructions that have a non-default count, but we shouldn't write it
            case ByteCodeInstruction::kCallExternal:
            case ByteCodeInstruction::kMatrixToMatrix:
            case ByteCodeInstruction::kMatrixMultiply:
            case ByteCodeInstruction::kScalarToMatrix:
            case ByteCodeInstruction::kSwizzle:
                break;
            default:
                this->write8(count);
                break;
        }
    }
}

void ByteCodeGenerator::writeTypedInstruction(const Type& type,
                                              ByteCodeInstruction s,
                                              ByteCodeInstruction u,
                                              ByteCodeInstruction f,
                                              int count) {
    switch (type_category(type)) {
        case TypeCategory::kBool:
        case TypeCategory::kSigned:   this->write(s, count); break;
        case TypeCategory::kUnsigned: this->write(u, count); break;
        case TypeCategory::kFloat:    this->write(f, count); break;
        default:
            SkASSERT(false);
    }
}

bool ByteCodeGenerator::writeBinaryExpression(const BinaryExpression& b, bool discard) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Token::Kind op = b.getOperator();
    if (op == Token::Kind::TK_EQ) {
        std::unique_ptr<LValue> lvalue = this->getLValue(left);
        this->writeExpression(right);
        lvalue->store(discard);
        discard = false;
        return discard;
    }
    const Type& lType = left.type();
    const Type& rType = right.type();
    bool lVecOrMtx = (lType.isVector() || lType.isMatrix());
    bool rVecOrMtx = (rType.isVector() || rType.isMatrix());
    std::unique_ptr<LValue> lvalue;
    if (Compiler::IsAssignment(op)) {
        lvalue = this->getLValue(left);
        lvalue->load();
        op = Compiler::RemoveAssignment(op);
    } else {
        this->writeExpression(left);
        if (!lVecOrMtx && rVecOrMtx) {
            for (int i = SlotCount(rType); i > 1; --i) {
                this->write(ByteCodeInstruction::kDup, 1);
            }
        }
    }
    int count = std::max(SlotCount(lType), SlotCount(rType));
    SkDEBUGCODE(TypeCategory tc = type_category(lType));
    switch (op) {
        case Token::Kind::TK_LOGICALAND: {
            SkASSERT(tc == SkSL::TypeCategory::kBool && count == 1);
            this->write(ByteCodeInstruction::kDup, 1);
            this->write(ByteCodeInstruction::kMaskPush);
            this->write(ByteCodeInstruction::kBranchIfAllFalse);
            DeferredLocation falseLocation(this);
            this->writeExpression(right);
            this->write(ByteCodeInstruction::kAndB, 1);
            falseLocation.set();
            this->write(ByteCodeInstruction::kMaskPop);
            return false;
        }
        case Token::Kind::TK_LOGICALOR: {
            SkASSERT(tc == SkSL::TypeCategory::kBool && count == 1);
            this->write(ByteCodeInstruction::kDup, 1);
            this->write(ByteCodeInstruction::kNotB, 1);
            this->write(ByteCodeInstruction::kMaskPush);
            this->write(ByteCodeInstruction::kBranchIfAllFalse);
            DeferredLocation falseLocation(this);
            this->writeExpression(right);
            this->write(ByteCodeInstruction::kOrB, 1);
            falseLocation.set();
            this->write(ByteCodeInstruction::kMaskPop);
            return false;
        }
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR: {
            SkASSERT(count == 1 && (tc == SkSL::TypeCategory::kSigned ||
                                    tc == SkSL::TypeCategory::kUnsigned));
            if (!right.isCompileTimeConstant()) {
                fErrors.error(right.fOffset, "Shift amounts must be constant");
                return false;
            }
            int64_t shift = right.getConstantInt();
            if (shift < 0 || shift > 31) {
                fErrors.error(right.fOffset, "Shift amount out of range");
                return false;
            }

            if (op == Token::Kind::TK_SHL) {
                this->write(ByteCodeInstruction::kShiftLeft);
            } else {
                this->write(type_category(lType) == TypeCategory::kSigned
                                ? ByteCodeInstruction::kShiftRightS
                                : ByteCodeInstruction::kShiftRightU);
            }
            this->write8(shift);
            return false;
        }

        default:
            break;
    }
    this->writeExpression(right);
    if (lVecOrMtx && !rVecOrMtx) {
        for (int i = SlotCount(lType); i > 1; --i) {
            this->write(ByteCodeInstruction::kDup, 1);
        }
    }
    // Special case for M*V, V*M, M*M (but not V*V!)
    if (op == Token::Kind::TK_STAR && lVecOrMtx && rVecOrMtx &&
        !(lType.isVector() && rType.isVector())) {
        this->write(ByteCodeInstruction::kMatrixMultiply,
                    SlotCount(b.type()) - (SlotCount(lType) + SlotCount(rType)));
        int rCols = rType.columns(),
            rRows = rType.rows(),
            lCols = lType.columns(),
            lRows = lType.rows();
        // M*V treats the vector as a column
        if (rType.isVector()) {
            std::swap(rCols, rRows);
        }
        SkASSERT(lCols == rRows);
        SkASSERT(SlotCount(b.type()) == lRows * rCols);
        this->write8(lCols);
        this->write8(lRows);
        this->write8(rCols);
    } else {
        switch (op) {
            case Token::Kind::TK_EQEQ:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareIEQ,
                                            ByteCodeInstruction::kCompareIEQ,
                                            ByteCodeInstruction::kCompareFEQ,
                                            count);
                // Collapse to a single bool
                for (int i = count; i > 1; --i) {
                    this->write(ByteCodeInstruction::kAndB, 1);
                }
                break;
            case Token::Kind::TK_GT:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareSGT,
                                            ByteCodeInstruction::kCompareUGT,
                                            ByteCodeInstruction::kCompareFGT,
                                            count);
                break;
            case Token::Kind::TK_GTEQ:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareSGTEQ,
                                            ByteCodeInstruction::kCompareUGTEQ,
                                            ByteCodeInstruction::kCompareFGTEQ,
                                            count);
                break;
            case Token::Kind::TK_LT:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareSLT,
                                            ByteCodeInstruction::kCompareULT,
                                            ByteCodeInstruction::kCompareFLT,
                                            count);
                break;
            case Token::Kind::TK_LTEQ:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareSLTEQ,
                                            ByteCodeInstruction::kCompareULTEQ,
                                            ByteCodeInstruction::kCompareFLTEQ,
                                            count);
                break;
            case Token::Kind::TK_MINUS:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractF,
                                            count);
                break;
            case Token::Kind::TK_NEQ:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kCompareINEQ,
                                            ByteCodeInstruction::kCompareINEQ,
                                            ByteCodeInstruction::kCompareFNEQ,
                                            count);
                // Collapse to a single bool
                for (int i = count; i > 1; --i) {
                    this->write(ByteCodeInstruction::kOrB, 1);
                }
                break;
            case Token::Kind::TK_PERCENT:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kRemainderS,
                                            ByteCodeInstruction::kRemainderU,
                                            ByteCodeInstruction::kRemainderF,
                                            count);
                break;
            case Token::Kind::TK_PLUS:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            count);
                break;
            case Token::Kind::TK_SLASH:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kDivideS,
                                            ByteCodeInstruction::kDivideU,
                                            ByteCodeInstruction::kDivideF,
                                            count);
                break;
            case Token::Kind::TK_STAR:
                this->writeTypedInstruction(lType, ByteCodeInstruction::kMultiplyI,
                                            ByteCodeInstruction::kMultiplyI,
                                            ByteCodeInstruction::kMultiplyF,
                                            count);
                break;

            case Token::Kind::TK_LOGICALXOR:
                SkASSERT(tc == SkSL::TypeCategory::kBool);
                this->write(ByteCodeInstruction::kXorB, count);
                break;

            case Token::Kind::TK_BITWISEAND:
                SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
                this->write(ByteCodeInstruction::kAndB, count);
                break;
            case Token::Kind::TK_BITWISEOR:
                SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
                this->write(ByteCodeInstruction::kOrB, count);
                break;
            case Token::Kind::TK_BITWISEXOR:
                SkASSERT(tc == SkSL::TypeCategory::kSigned || tc == SkSL::TypeCategory::kUnsigned);
                this->write(ByteCodeInstruction::kXorB, count);
                break;

            default:
                fErrors.error(b.fOffset, SkSL::String::printf("Unsupported binary operator '%s'",
                                                              Compiler::OperatorName(op)));
                break;
        }
    }
    if (lvalue) {
        lvalue->store(discard);
        discard = false;
    }
    return discard;
}

void ByteCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(b.value() ? ~0 : 0);
}

void ByteCodeGenerator::writeConstructor(const Constructor& c) {
    for (const auto& arg : c.arguments()) {
        this->writeExpression(*arg);
    }
    if (c.arguments().size() == 1) {
        const Type& inType = c.arguments()[0]->type();
        const Type& outType = c.type();
        TypeCategory inCategory = type_category(inType);
        TypeCategory outCategory = type_category(outType);
        int inCount = SlotCount(inType);
        int outCount = SlotCount(outType);
        if (inCategory != outCategory) {
            SkASSERT(inCount == outCount);
            if (inCategory == TypeCategory::kFloat) {
                SkASSERT(outCategory == TypeCategory::kSigned ||
                         outCategory == TypeCategory::kUnsigned);
                this->write(ByteCodeInstruction::kConvertFtoI, outCount);
            } else if (outCategory == TypeCategory::kFloat) {
                if (inCategory == TypeCategory::kSigned) {
                    this->write(ByteCodeInstruction::kConvertStoF, outCount);
                } else {
                    SkASSERT(inCategory == TypeCategory::kUnsigned);
                    this->write(ByteCodeInstruction::kConvertUtoF, outCount);
                }
            } else {
                SkASSERT(false);
            }
        }
        if (inType.isMatrix() && outType.isMatrix()) {
            this->write(ByteCodeInstruction::kMatrixToMatrix,
                        SlotCount(outType) - SlotCount(inType));
            this->write8(inType.columns());
            this->write8(inType.rows());
            this->write8(outType.columns());
            this->write8(outType.rows());
        } else if (inCount != outCount) {
            SkASSERT(inCount == 1);
            if (outType.isMatrix()) {
                this->write(ByteCodeInstruction::kScalarToMatrix, SlotCount(outType) - 1);
                this->write8(outType.columns());
                this->write8(outType.rows());
            } else {
                SkASSERT(outType.isVector());
                for (; inCount != outCount; ++inCount) {
                    this->write(ByteCodeInstruction::kDup, 1);
                }
            }
        }
    }
}

void ByteCodeGenerator::writeExternalFunctionCall(const ExternalFunctionCall& f) {
    int argumentCount = 0;
    for (const auto& arg : f.arguments()) {
        this->writeExpression(*arg);
        argumentCount += SlotCount(arg->type());
    }
    this->write(ByteCodeInstruction::kCallExternal, SlotCount(f.type()) - argumentCount);
    SkASSERT(argumentCount <= 255);
    this->write8(argumentCount);
    this->write8(SlotCount(f.type()));
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(&f.function());
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeExternalValue(const ExternalValueReference& e) {
    int count = SlotCount(e.value().type());
    this->write(ByteCodeInstruction::kReadExternal, count);
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(&e.value());
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeVariableExpression(const Expression& expr) {
    if (int builtin = expression_as_builtin(expr); builtin >= 0) {
        switch (builtin) {
            case SK_FRAGCOORD_BUILTIN:
                this->write(ByteCodeInstruction::kLoadFragCoord);
                fOutput->fUsesFragCoord = true;
                break;
            default:
                fErrors.error(expr.fOffset, "Unsupported builtin");
                break;
        }
        return;
    }

    Location location = this->getLocation(expr);
    int count = SlotCount(expr.type());
    if (count == 0) {
        return;
    }
    if (location.isOnStack()) {
        this->write(location.selectLoad(ByteCodeInstruction::kLoadExtended,
                                        ByteCodeInstruction::kLoadExtendedGlobal,
                                        ByteCodeInstruction::kLoadExtendedUniform),
                    count);
    } else {
        this->write(location.selectLoad(ByteCodeInstruction::kLoad,
                                        ByteCodeInstruction::kLoadGlobal,
                                        ByteCodeInstruction::kLoadUniform),
                    count);
        this->write8(location.fSlot);
    }
}

static inline uint32_t float_to_bits(float x) {
    uint32_t u;
    memcpy(&u, &x, sizeof(uint32_t));
    return u;
}

void ByteCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(float_to_bits(f.value()));
}

void ByteCodeGenerator::writeSmoothstep(const ExpressionArray& args) {
    // genType smoothstep(genType edge0, genType edge1, genType x) {
    //   genType t = saturate((x - edge0) / (edge1 - edge0));
    //   return t * t * (3 - 2 * t);
    // }

    // There are variants where the first two arguments are scalar
    SkASSERT(args.size() == 3);
    int edgeCount = SlotCount(args[0]->type()),
        xCount    = SlotCount(args[2]->type());
    SkASSERT(edgeCount == 1 || edgeCount == xCount);
    SkASSERT(edgeCount == SlotCount(args[1]->type()));

    // Expand a (possibly scalar) value to be as wide as 'x'
    auto dupToX = [xCount, this](int from) {
        for (int i = from; i < xCount; ++i) {
            this->write(ByteCodeInstruction::kDup, 1);
        }
    };

    // Push xCount copies of 'f'
    auto scalarToX = [&dupToX, this](float f) {
        this->write(ByteCodeInstruction::kPushImmediate);
        this->write32(float_to_bits(f));
        dupToX(1);
    };

    // To avoid possible double-eval, we store edge0 in a local
    const Variable* edge0Var = fSynthetics.takeOwnershipOfSymbol(
            std::make_unique<Variable>(/*offset=*/-1,
                                       fProgram.fModifiers->addToPool(Modifiers()),
                                       "sksl_smoothstep_edge0",
                                       &args[0]->type(),
                                       /*builtin=*/true,
                                       Variable::Storage::kLocal));
    Location edge0Loc = this->getLocation(*edge0Var);
    this->writeExpression(*args[0]);  // 'edge0'
    this->write(ByteCodeInstruction::kStore, edgeCount);
    this->write8(edge0Loc.fSlot);

    // (x - edge0)
    this->writeExpression(*args[2]);                     // 'x'
    this->write(ByteCodeInstruction::kLoad, edgeCount);  // 'edge0'
    this->write8(edge0Loc.fSlot);
    dupToX(edgeCount);
    this->write(ByteCodeInstruction::kSubtractF, xCount);

    // (edge1 - edge0)
    this->writeExpression(*args[1]);                     // 'edge1'
    this->write(ByteCodeInstruction::kLoad, edgeCount);  // 'edge0'
    this->write8(edge0Loc.fSlot);
    this->write(ByteCodeInstruction::kSubtractF, edgeCount);
    dupToX(edgeCount);

    // saturate((x - edge0) / (edge1 - edge0))
    this->write(ByteCodeInstruction::kDivideF, xCount);
    scalarToX(0.0f);
    this->write(ByteCodeInstruction::kMaxF, xCount);
    scalarToX(1.0f);
    this->write(ByteCodeInstruction::kMinF, xCount);

    // Now, 't' is on the stack, we need three copies
    this->write(ByteCodeInstruction::kDup, xCount);
    this->write(ByteCodeInstruction::kDup, xCount);

    // (3 - 2 * t) ... as (-2t + 3)
    scalarToX(-2.0f);
    this->write(ByteCodeInstruction::kMultiplyF, xCount);
    scalarToX(3.0f);
    this->write(ByteCodeInstruction::kAddF, xCount);

    // ... * t * t
    this->write(ByteCodeInstruction::kMultiplyF, xCount);
    this->write(ByteCodeInstruction::kMultiplyF, xCount);
}

static bool is_generic_type(const Type* type, const Type* generic) {
    const std::vector<const Type*>& concrete(generic->coercibleTypes());
    return std::find(concrete.begin(), concrete.end(), type) != concrete.end();
}

void ByteCodeGenerator::writeIntrinsicCall(const FunctionCall& c) {
    auto found = fIntrinsics.find(c.function().name());
    if (found == fIntrinsics.end()) {
        fErrors.error(c.fOffset, String::printf("Unsupported intrinsic: '%s'",
                                                String(c.function().name()).c_str()));
        return;
    }
    Intrinsic intrin = found->second;

    const auto& args = c.arguments();
    const size_t nargs = args.size();
    SkASSERT(nargs >= 1);

    int count = SlotCount(args[0]->type());

    // Several intrinsics have variants where one argument is either scalar, or the same size as
    // the first argument. Call dupSmallerType(SlotCount(argType)) to ensure equal component count.
    auto dupSmallerType = [count, this](int smallCount) {
        SkASSERT(smallCount == 1 || smallCount == count);
        for (int i = smallCount; i < count; ++i) {
            this->write(ByteCodeInstruction::kDup, 1);
        }
    };

    if (intrin.is_special && intrin.special == SpecialIntrinsic::kSample) {
        // Sample is very special, the first argument is an FP, which can't be pushed to the stack.
        if (nargs > 2 || args[0]->type() != *fContext.fFragmentProcessor_Type ||
            (nargs == 2 && (args[1]->type() != *fContext.fFloat2_Type &&
                            args[1]->type() != *fContext.fFloat3x3_Type))) {
            fErrors.error(c.fOffset, "Unsupported form of sample");
            return;
        }

        if (nargs == 2) {
            // Write our coords or matrix
            this->writeExpression(*args[1]);
            this->write(args[1]->type() == *fContext.fFloat3x3_Type
                                ? ByteCodeInstruction::kSampleMatrix
                                : ByteCodeInstruction::kSampleExplicit);
        } else {
            this->write(ByteCodeInstruction::kSample);
        }

        Location childLoc = this->getLocation(*args[0]);
        SkASSERT(childLoc.fStorage == Storage::kChildFP);
        this->write8(childLoc.fSlot);
        return;
    }

    if (intrin.is_special && intrin.special == SpecialIntrinsic::kSmoothstep) {
        this->writeSmoothstep(args);
        return;
    }

    if (intrin.is_special && intrin.special == SpecialIntrinsic::kStep) {
        // There are variants where the *first* argument is scalar
        SkASSERT(nargs == 2);
        int xCount = SlotCount(args[1]->type());
        SkASSERT(count == 1 || count == xCount);

        this->writeExpression(*args[0]);  // 'edge'

        // Not 'dupSmallerType', because we're duping the first to match the second
        for (int i = count; i < xCount; ++i) {
            this->write(ByteCodeInstruction::kDup, 1);
        }

        this->writeExpression(*args[1]);  // 'x'
        this->write(ByteCodeInstruction::kStep, xCount);
        return;
    }

    if (intrin.is_special && (intrin.special == SpecialIntrinsic::kClamp ||
                              intrin.special == SpecialIntrinsic::kSaturate)) {
        // These intrinsics are extra-special, we need instructions interleaved with arguments
        bool saturate = (intrin.special == SpecialIntrinsic::kSaturate);
        SkASSERT(nargs == (saturate ? 1 : 3));
        int limitCount = saturate ? 1 : SlotCount(args[1]->type());

        // 'x'
        this->writeExpression(*args[0]);

        // 'minVal'
        if (saturate) {
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(float_to_bits(0.0f));
        } else {
            this->writeExpression(*args[1]);
        }
        dupSmallerType(limitCount);
        this->writeTypedInstruction(args[0]->type(),
                                    ByteCodeInstruction::kMaxS,
                                    ByteCodeInstruction::kMaxS,
                                    ByteCodeInstruction::kMaxF,
                                    count);

        // 'maxVal'
        if (saturate) {
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(float_to_bits(1.0f));
        } else {
            SkASSERT(limitCount == SlotCount(args[2]->type()));
            this->writeExpression(*args[2]);
        }
        dupSmallerType(limitCount);
        this->writeTypedInstruction(args[0]->type(),
                                    ByteCodeInstruction::kMinS,
                                    ByteCodeInstruction::kMinS,
                                    ByteCodeInstruction::kMinF,
                                    count);
        return;
    }

    // All other intrinsics can handle their arguments being on the stack in order
    for (const auto& arg : args) {
        this->writeExpression(*arg);
    }

    if (intrin.is_special) {
        auto doDotProduct = [count, this] {
            this->write(ByteCodeInstruction::kMultiplyF, count);
            for (int i = count - 1; i-- > 0;) {
                this->write(ByteCodeInstruction::kAddF, 1);
            }
        };

        auto doLength = [count, this, &doDotProduct] {
            this->write(ByteCodeInstruction::kDup, count);
            doDotProduct();
            this->write(ByteCodeInstruction::kSqrt, 1);
        };

        switch (intrin.special) {
            case SpecialIntrinsic::kAll: {
                for (int i = count-1; i --> 0;) {
                    this->write(ByteCodeInstruction::kAndB, 1);
                }
            } break;

            case SpecialIntrinsic::kAny: {
                for (int i = count-1; i --> 0;) {
                    this->write(ByteCodeInstruction::kOrB, 1);
                }
            } break;

            case SpecialIntrinsic::kATan: {
                // GLSL uses "atan" for both 'atan' and 'atan2'
                SkASSERT(nargs == 1 || (nargs == 2 && count == SlotCount(args[1]->type())));
                this->write(nargs == 1 ? ByteCodeInstruction::kATan : ByteCodeInstruction::kATan2,
                            count);
            } break;

            case SpecialIntrinsic::kDistance: {
                SkASSERT(nargs == 2 && count == SlotCount(args[1]->type()));
                this->write(ByteCodeInstruction::kSubtractF, count);
                doLength();
            } break;

            case SpecialIntrinsic::kDot: {
                SkASSERT(nargs == 2 && count == SlotCount(args[1]->type()));
                doDotProduct();
            } break;

            case SpecialIntrinsic::kLength: {
                SkASSERT(nargs == 1);
                doLength();
            } break;

            case SpecialIntrinsic::kMax:
            case SpecialIntrinsic::kMin: {
                SkASSERT(nargs == 2);
                // There are variants where the second argument is scalar
                dupSmallerType(SlotCount(args[1]->type()));
                if (intrin.special == SpecialIntrinsic::kMax) {
                    this->writeTypedInstruction(args[0]->type(),
                                                ByteCodeInstruction::kMaxS,
                                                ByteCodeInstruction::kMaxS,
                                                ByteCodeInstruction::kMaxF,
                                                count);
                } else {
                    this->writeTypedInstruction(args[0]->type(),
                                                ByteCodeInstruction::kMinS,
                                                ByteCodeInstruction::kMinS,
                                                ByteCodeInstruction::kMinF,
                                                count);
                }
            } break;

            case SpecialIntrinsic::kMix: {
                // Two main variants of mix to handle
                SkASSERT(nargs == 3);
                SkASSERT(count == SlotCount(args[1]->type()));
                int selectorCount = SlotCount(args[2]->type());

                if (is_generic_type(&args[2]->type(), fContext.fGenBType_Type.get())) {
                    // mix(genType, genType, genBoolType)
                    SkASSERT(selectorCount == count);
                    this->write(ByteCodeInstruction::kMix, count);
                } else {
                    // mix(genType, genType, genType) or mix(genType, genType, float)
                    dupSmallerType(selectorCount);
                    this->write(ByteCodeInstruction::kLerp, count);
                }
            } break;

            case SpecialIntrinsic::kMod: {
                SkASSERT(nargs == 2);
                // There are variants where the second argument is scalar
                dupSmallerType(SlotCount(args[1]->type()));
                this->write(ByteCodeInstruction::kMod, count);
            } break;

            case SpecialIntrinsic::kNormalize: {
                SkASSERT(nargs == 1);
                this->write(ByteCodeInstruction::kDup, count);
                doLength();
                dupSmallerType(1);
                this->write(ByteCodeInstruction::kDivideF, count);
            } break;

            default:
                SkASSERT(false);
        }
    } else {
        switch (intrin.inst_f) {
            case ByteCodeInstruction::kInverse2x2: {
                auto op = ByteCodeInstruction::kInverse2x2;
                switch (count) {
                    case 4: break;  // float2x2
                    case 9:  op = ByteCodeInstruction::kInverse3x3; break;
                    case 16: op = ByteCodeInstruction::kInverse4x4; break;
                    default: SkASSERT(false);
                }
                this->write(op);
                break;
            }

            default:
                this->writeTypedInstruction(args[0]->type(),
                                            intrin.inst_s,
                                            intrin.inst_u,
                                            intrin.inst_f,
                                            count);
                break;
        }
    }
}

void ByteCodeGenerator::writeFunctionCall(const FunctionCall& f) {
    // Find the index of the function we're calling. We explicitly do not allow calls to functions
    // before they're defined. This is an easy-to-understand rule that prevents recursion.
    int idx = -1;
    for (size_t i = 0; i < fFunctions.size(); ++i) {
        if (f.function().matches(fFunctions[i]->declaration())) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        this->writeIntrinsicCall(f);
        return;
    }


    if (idx > 255) {
        fErrors.error(f.fOffset, "Function count limit exceeded");
        return;
    } else if (idx >= (int) fFunctions.size()) {
        fErrors.error(f.fOffset, "Call to undefined function");
        return;
    }

    // We may need to deal with out parameters, so the sequence is tricky
    if (int returnCount = SlotCount(f.type())) {
        this->write(ByteCodeInstruction::kReserve, returnCount);
    }

    int argCount = f.arguments().size();
    std::vector<std::unique_ptr<LValue>> lvalues;
    for (int i = 0; i < argCount; ++i) {
        const auto& param = f.function().parameters()[i];
        const auto& arg = f.arguments()[i];
        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            lvalues.emplace_back(this->getLValue(*arg));
            lvalues.back()->load();
        } else {
            this->writeExpression(*arg);
        }
    }

    // The space used by the call is based on the callee, but it also unwinds all of that before
    // we continue execution. We adjust our max stack depths below.
    this->write(ByteCodeInstruction::kCall);
    this->write8(idx);

    const ByteCodeFunction* callee = fOutput->fFunctions[idx].get();
    fMaxLoopCount      = std::max(fMaxLoopCount,      fLoopCount      + callee->fLoopCount);
    fMaxConditionCount = std::max(fMaxConditionCount, fConditionCount + callee->fConditionCount);
    fMaxStackCount     = std::max(fMaxStackCount,     fStackCount     + callee->fLocalCount
                                                                      + callee->fStackCount);

    // After the called function returns, the stack will still contain our arguments. We have to
    // pop them (storing any out parameters back to their lvalues as we go). We glob together slot
    // counts for all parameters that aren't out-params, so we can pop them in one big chunk.
    int popCount = 0;
    auto pop = [&]() {
        if (popCount > 0) {
            this->write(ByteCodeInstruction::kPop, popCount);
        }
        popCount = 0;
    };

    for (int i = argCount - 1; i >= 0; --i) {
        const auto& param = f.function().parameters()[i];
        const auto& arg = f.arguments()[i];
        if (param->modifiers().fFlags & Modifiers::kOut_Flag) {
            pop();
            lvalues.back()->store(true);
            lvalues.pop_back();
        } else {
            popCount += SlotCount(arg->type());
        }
    }
    pop();
}

void ByteCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(i.value());
}

void ByteCodeGenerator::writeNullLiteral(const NullLiteral& n) {
    // not yet implemented
    abort();
}

bool ByteCodeGenerator::writePrefixExpression(const PrefixExpression& p, bool discard) {
    switch (p.getOperator()) {
        case Token::Kind::TK_PLUSPLUS: // fall through
        case Token::Kind::TK_MINUSMINUS: {
            SkASSERT(SlotCount(p.operand()->type()) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.operand());
            lvalue->load();
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.type()) == TypeCategory::kFloat ? float_to_bits(1.0f)
                                                                          : 1);
            if (p.getOperator() == Token::Kind::TK_PLUSPLUS) {
                this->writeTypedInstruction(p.type(),
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            1);
            } else {
                this->writeTypedInstruction(p.type(),
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractF,
                                            1);
            }
            lvalue->store(discard);
            discard = false;
            break;
        }
        case Token::Kind::TK_MINUS: {
            this->writeExpression(*p.operand());
            this->writeTypedInstruction(p.type(),
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateF,
                                        SlotCount(p.operand()->type()));
            break;
        }
        case Token::Kind::TK_LOGICALNOT:
        case Token::Kind::TK_BITWISENOT: {
            SkASSERT(SlotCount(p.operand()->type()) == 1);
            SkDEBUGCODE(TypeCategory tc = type_category(p.operand()->type()));
            SkASSERT((p.getOperator() == Token::Kind::TK_LOGICALNOT &&
                      tc == TypeCategory::kBool) ||
                     (p.getOperator() == Token::Kind::TK_BITWISENOT &&
                      (tc == TypeCategory::kSigned || tc == TypeCategory::kUnsigned)));
            this->writeExpression(*p.operand());
            this->write(ByteCodeInstruction::kNotB, 1);
            break;
        }
        default:
            SkASSERT(false);
    }
    return discard;
}

bool ByteCodeGenerator::writePostfixExpression(const PostfixExpression& p, bool discard) {
    switch (p.getOperator()) {
        case Token::Kind::TK_PLUSPLUS: // fall through
        case Token::Kind::TK_MINUSMINUS: {
            SkASSERT(SlotCount(p.operand()->type()) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.operand());
            lvalue->load();
            // If we're not supposed to discard the result, then make a copy *before* the +/-
            if (!discard) {
                this->write(ByteCodeInstruction::kDup, 1);
            }
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.type()) == TypeCategory::kFloat ? float_to_bits(1.0f)
                                                                          : 1);
            if (p.getOperator() == Token::Kind::TK_PLUSPLUS) {
                this->writeTypedInstruction(p.type(),
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            1);
            } else {
                this->writeTypedInstruction(p.type(),
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractI,
                                            ByteCodeInstruction::kSubtractF,
                                            1);
            }
            // Always consume the result as part of the store
            lvalue->store(true);
            discard = false;
            break;
        }
        default:
            SkASSERT(false);
    }
    return discard;
}

void ByteCodeGenerator::writeSwizzle(const Swizzle& s) {
    if (swizzle_is_simple(s)) {
        this->writeVariableExpression(s);
        return;
    }

    this->writeExpression(*s.base());
    this->write(ByteCodeInstruction::kSwizzle, s.components().size() - s.base()->type().columns());
    this->write8(s.base()->type().columns());
    this->write8(s.components().size());
    for (int c : s.components()) {
        this->write8(c);
    }
}

void ByteCodeGenerator::writeTernaryExpression(const TernaryExpression& t) {
    int count = SlotCount(t.type());
    SkASSERT(count == SlotCount(t.ifTrue()->type()));
    SkASSERT(count == SlotCount(t.ifFalse()->type()));

    this->writeExpression(*t.test());
    this->write(ByteCodeInstruction::kMaskPush);
    this->writeExpression(*t.ifTrue());
    this->write(ByteCodeInstruction::kMaskNegate);
    this->writeExpression(*t.ifFalse());
    this->write(ByteCodeInstruction::kMaskBlend, count);
}

void ByteCodeGenerator::writeExpression(const Expression& e, bool discard) {
    switch (e.kind()) {
        case Expression::Kind::kBinary:
            discard = this->writeBinaryExpression(e.as<BinaryExpression>(), discard);
            break;
        case Expression::Kind::kBoolLiteral:
            this->writeBoolLiteral(e.as<BoolLiteral>());
            break;
        case Expression::Kind::kConstructor:
            this->writeConstructor(e.as<Constructor>());
            break;
        case Expression::Kind::kExternalFunctionCall:
            this->writeExternalFunctionCall(e.as<ExternalFunctionCall>());
            break;
        case Expression::Kind::kExternalValue:
            this->writeExternalValue(e.as<ExternalValueReference>());
            break;
        case Expression::Kind::kFieldAccess:
        case Expression::Kind::kIndex:
        case Expression::Kind::kVariableReference:
            this->writeVariableExpression(e);
            break;
        case Expression::Kind::kFloatLiteral:
            this->writeFloatLiteral(e.as<FloatLiteral>());
            break;
        case Expression::Kind::kFunctionCall:
            this->writeFunctionCall(e.as<FunctionCall>());
            break;
        case Expression::Kind::kIntLiteral:
            this->writeIntLiteral(e.as<IntLiteral>());
            break;
        case Expression::Kind::kNullLiteral:
            this->writeNullLiteral(e.as<NullLiteral>());
            break;
        case Expression::Kind::kPrefix:
            discard = this->writePrefixExpression(e.as<PrefixExpression>(), discard);
            break;
        case Expression::Kind::kPostfix:
            discard = this->writePostfixExpression(e.as<PostfixExpression>(), discard);
            break;
        case Expression::Kind::kSwizzle:
            this->writeSwizzle(e.as<Swizzle>());
            break;
        case Expression::Kind::kTernary:
            this->writeTernaryExpression(e.as<TernaryExpression>());
            break;
        default:
#ifdef SK_DEBUG
            printf("unsupported expression %s\n", e.description().c_str());
#endif
            SkASSERT(false);
    }
    if (discard) {
        int count = SlotCount(e.type());
        if (count > 0) {
            this->write(ByteCodeInstruction::kPop, count);
        }
        discard = false;
    }
}

class ByteCodeExternalValueLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeExternalValueLValue(ByteCodeGenerator* generator, const ExternalValue& value, int index)
        : INHERITED(*generator)
        , fCount(ByteCodeGenerator::SlotCount(value.type()))
        , fIndex(index) {}

    void load() override {
        fGenerator.write(ByteCodeInstruction::kReadExternal, fCount);
        fGenerator.write8(fIndex);
    }

    void store(bool discard) override {
        if (!discard) {
            fGenerator.write(ByteCodeInstruction::kDup, fCount);
        }
        fGenerator.write(ByteCodeInstruction::kWriteExternal, fCount);
        fGenerator.write8(fIndex);
    }

private:
    using INHERITED = LValue;

    int fCount;
    int fIndex;
};

class ByteCodeSwizzleLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeSwizzleLValue(ByteCodeGenerator* generator, const Swizzle& swizzle)
        : INHERITED(*generator)
        , fSwizzle(swizzle) {}

    void load() override {
        fGenerator.writeSwizzle(fSwizzle);
    }

    void store(bool discard) override {
        int count = fSwizzle.components().size();
        if (!discard) {
            fGenerator.write(ByteCodeInstruction::kDup, count);
        }
        // We already have the correct number of values on the stack, thanks to type checking.
        // The algorithm: Walk down the values on the stack, doing 'count' single-element stores.
        // For each value, use the corresponding swizzle component to offset the store location.
        //
        // Static locations: We (wastefully) call getLocation every time, but get good byte code.
        // Note that we could (but don't) store adjacent/sequential values with fewer instructions.
        //
        // Dynamic locations: ... are bad. We have to recompute the base address on each iteration,
        // because the stack doesn't let us retain that address between stores. Dynamic locations
        // are rare though, and swizzled writes to those are even rarer, so we just live with this.
        for (int i = count; i-- > 0;) {
            // If we have a swizzle-of-swizzle lvalue, we need to flatten that down to the final
            // component index. (getLocation can't handle this case).
            const Expression* expr = &fSwizzle;
            int component = i;
            do {
                component = expr->as<Swizzle>().components()[component];
                expr = expr->as<Swizzle>().base().get();
            } while (expr->is<Swizzle>());

            ByteCodeGenerator::Location location = fGenerator.getLocation(*expr);
            if (!location.isOnStack()) {
                fGenerator.write(location.selectStore(ByteCodeInstruction::kStore,
                                                      ByteCodeInstruction::kStoreGlobal),
                                 1);
                fGenerator.write8(location.fSlot + component);
            } else {
                fGenerator.write(ByteCodeInstruction::kPushImmediate);
                fGenerator.write32(component);
                fGenerator.write(ByteCodeInstruction::kAddI, 1);
                fGenerator.write(location.selectStore(ByteCodeInstruction::kStoreExtended,
                                                      ByteCodeInstruction::kStoreExtendedGlobal),
                                 1);
            }
        }
    }

private:
    const Swizzle& fSwizzle;

    using INHERITED = LValue;
};

class ByteCodeExpressionLValue : public ByteCodeGenerator::LValue {
public:
    ByteCodeExpressionLValue(ByteCodeGenerator* generator, const Expression& expr)
        : INHERITED(*generator)
        , fExpression(expr) {}

    void load() override {
        fGenerator.writeVariableExpression(fExpression);
    }

    void store(bool discard) override {
        int count = ByteCodeGenerator::SlotCount(fExpression.type());
        if (!discard) {
            fGenerator.write(ByteCodeInstruction::kDup, count);
        }
        ByteCodeGenerator::Location location = fGenerator.getLocation(fExpression);
        if (location.isOnStack()) {
            fGenerator.write(location.selectStore(ByteCodeInstruction::kStoreExtended,
                                                  ByteCodeInstruction::kStoreExtendedGlobal),
                             count);
        } else {
            fGenerator.write(location.selectStore(ByteCodeInstruction::kStore,
                                                  ByteCodeInstruction::kStoreGlobal),
                             count);
            fGenerator.write8(location.fSlot);
        }
    }

private:
    using INHERITED = LValue;

    const Expression& fExpression;
};

std::unique_ptr<ByteCodeGenerator::LValue> ByteCodeGenerator::getLValue(const Expression& e) {
    switch (e.kind()) {
        case Expression::Kind::kExternalValue: {
            const ExternalValue& value = e.as<ExternalValueReference>().value();
            int index = fOutput->fExternalValues.size();
            fOutput->fExternalValues.push_back(&value);
            SkASSERT(index <= 255);
            return std::unique_ptr<LValue>(new ByteCodeExternalValueLValue(this, value, index));
        }
        case Expression::Kind::kFieldAccess:
        case Expression::Kind::kIndex:
        case Expression::Kind::kVariableReference:
            return std::unique_ptr<LValue>(new ByteCodeExpressionLValue(this, e));
        case Expression::Kind::kSwizzle: {
            const Swizzle& s = e.as<Swizzle>();
            return swizzle_is_simple(s)
                    ? std::unique_ptr<LValue>(new ByteCodeExpressionLValue(this, e))
                    : std::unique_ptr<LValue>(new ByteCodeSwizzleLValue(this, s));
        }
        case Expression::Kind::kTernary:
        default:
#ifdef SK_DEBUG
            ABORT("unsupported lvalue %s\n", e.description().c_str());
#endif
            return nullptr;
    }
}

void ByteCodeGenerator::writeBlock(const Block& b) {
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        this->writeStatement(*stmt);
    }
}

void ByteCodeGenerator::setBreakTargets() {
    std::vector<DeferredLocation>& breaks = fBreakTargets.top();
    for (DeferredLocation& b : breaks) {
        b.set();
    }
    fBreakTargets.pop();
}

void ByteCodeGenerator::setContinueTargets() {
    std::vector<DeferredLocation>& continues = fContinueTargets.top();
    for (DeferredLocation& c : continues) {
        c.set();
    }
    fContinueTargets.pop();
}

void ByteCodeGenerator::writeBreakStatement(const BreakStatement& b) {
    // TODO: Include BranchIfAllFalse to top-most LoopNext
    this->write(ByteCodeInstruction::kLoopBreak);
}

void ByteCodeGenerator::writeContinueStatement(const ContinueStatement& c) {
    // TODO: Include BranchIfAllFalse to top-most LoopNext
    this->write(ByteCodeInstruction::kLoopContinue);
}

void ByteCodeGenerator::writeDoStatement(const DoStatement& d) {
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t start = fCode->size();
    this->writeStatement(*d.statement());
    this->write(ByteCodeInstruction::kLoopNext);
    this->writeExpression(*d.test());
    this->write(ByteCodeInstruction::kLoopMask);
    // TODO: Could shorten this with kBranchIfAnyTrue
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->write(ByteCodeInstruction::kBranch);
    this->write16(start);
    endLocation.set();
    this->write(ByteCodeInstruction::kLoopEnd);
}

void ByteCodeGenerator::writeForStatement(const ForStatement& f) {
    fContinueTargets.emplace();
    fBreakTargets.emplace();
    if (f.initializer()) {
        this->writeStatement(*f.initializer());
    }
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t start = fCode->size();
    if (f.test()) {
        this->writeExpression(*f.test());
        this->write(ByteCodeInstruction::kLoopMask);
    }
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(*f.statement());
    this->write(ByteCodeInstruction::kLoopNext);
    if (f.next()) {
        this->writeExpression(*f.next(), true);
    }
    this->write(ByteCodeInstruction::kBranch);
    this->write16(start);
    endLocation.set();
    this->write(ByteCodeInstruction::kLoopEnd);
}

void ByteCodeGenerator::writeIfStatement(const IfStatement& i) {
    this->writeExpression(*i.test());
    this->write(ByteCodeInstruction::kMaskPush);
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation falseLocation(this);
    this->writeStatement(*i.ifTrue());
    falseLocation.set();
    if (i.ifFalse()) {
        this->write(ByteCodeInstruction::kMaskNegate);
        this->write(ByteCodeInstruction::kBranchIfAllFalse);
        DeferredLocation endLocation(this);
        this->writeStatement(*i.ifFalse());
        endLocation.set();
    }
    this->write(ByteCodeInstruction::kMaskPop);
}

void ByteCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    if (fLoopCount) {
        fErrors.error(r.fOffset, "return not allowed inside loop");
        return;
    }
    int count = SlotCount(r.expression()->type());
    this->writeExpression(*r.expression());

    // Technically, the kReturn also pops fOutput->fLocalCount values from the stack, too, but we
    // haven't counted pushing those (they're outside the scope of our stack tracking). Instead,
    // we account for those in writeFunction().

    // This is all fine because we don't allow conditional returns, so we only return once anyway.
    this->write(ByteCodeInstruction::kReturn, count);
}

void ByteCodeGenerator::writeSwitchStatement(const SwitchStatement& r) {
    // not yet implemented
    abort();
}

void ByteCodeGenerator::writeVarDeclaration(const VarDeclaration& decl) {
    // we need to grab the location even if we don't use it, to ensure it has been allocated
    Location location = this->getLocation(decl.var());
    if (decl.value()) {
        this->writeExpression(*decl.value());
        int count = SlotCount(decl.value()->type());
        this->write(ByteCodeInstruction::kStore, count);
        this->write8(location.fSlot);
    }
}

void ByteCodeGenerator::writeStatement(const Statement& s) {
    switch (s.kind()) {
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::Kind::kBreak:
            this->writeBreakStatement(s.as<BreakStatement>());
            break;
        case Statement::Kind::kContinue:
            this->writeContinueStatement(s.as<ContinueStatement>());
            break;
        case Statement::Kind::kDiscard:
            // not yet implemented
            abort();
        case Statement::Kind::kDo:
            this->writeDoStatement(s.as<DoStatement>());
            break;
        case Statement::Kind::kExpression:
            this->writeExpression(*s.as<ExpressionStatement>().expression(), true);
            break;
        case Statement::Kind::kFor:
            this->writeForStatement(s.as<ForStatement>());
            break;
        case Statement::Kind::kIf:
            this->writeIfStatement(s.as<IfStatement>());
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::Kind::kSwitch:
            this->writeSwitchStatement(s.as<SwitchStatement>());
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>());
            break;
        case Statement::Kind::kInlineMarker:
        case Statement::Kind::kNop:
            break;
        default:
            SkASSERT(false);
    }
}

ByteCodeFunction::ByteCodeFunction(const FunctionDeclaration* declaration)
        : fName(declaration->name()) {
    fParameterCount = 0;
    for (const auto& p : declaration->parameters()) {
        int slots = ByteCodeGenerator::SlotCount(p->type());
        fParameters.push_back({ slots, (bool)(p->modifiers().fFlags & Modifiers::kOut_Flag) });
        fParameterCount += slots;
    }
}

}  // namespace SkSL
