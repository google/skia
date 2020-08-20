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
    switch (type.kind()) {
        case Type::Kind::kVector_Kind:
        case Type::Kind::kMatrix_Kind:
            return type_category(type.componentType());
        default:
            if (type.fName == "bool") {
                return TypeCategory::kBool;
            } else if (type.fName == "int" ||
                       type.fName == "short" ||
                       type.fName == "$intLiteral") {
                return TypeCategory::kSigned;
            } else if (type.fName == "uint" ||
                       type.fName == "ushort") {
                return TypeCategory::kUnsigned;
            } else {
                SkASSERT(type.fName == "float" ||
                         type.fName == "half" ||
                         type.fName == "$floatLiteral");
                return TypeCategory::kFloat;
            }
            ABORT("unsupported type: %s\n", type.displayName().c_str());
    }
}


ByteCodeGenerator::ByteCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                  ByteCode* output)
    : INHERITED(program, errors, nullptr)
    , fContext(*context)
    , fOutput(output)
    // If you're adding new intrinsics here, ensure that they're declared in sksl_interp.inc, so
    // they're available to "generic" interpreter programs (eg particles).
    // You can probably copy the declarations from sksl_gpu.inc.
    , fIntrinsics {
        { "atan",      ByteCodeInstruction::kATan },
        { "ceil",      ByteCodeInstruction::kCeil },
        { "clamp",     SpecialIntrinsic::kClamp },
        { "cos",       ByteCodeInstruction::kCos },
        { "dot",       SpecialIntrinsic::kDot },
        { "floor",     ByteCodeInstruction::kFloor },
        { "fract",     ByteCodeInstruction::kFract },
        { "inverse",   ByteCodeInstruction::kInverse2x2 },
        { "length",    SpecialIntrinsic::kLength },
        { "max",       SpecialIntrinsic::kMax },
        { "min",       SpecialIntrinsic::kMin },
        { "mix",       SpecialIntrinsic::kMix },
        { "normalize", SpecialIntrinsic::kNormalize },
        { "pow",       ByteCodeInstruction::kPow },
        { "sample",    SpecialIntrinsic::kSample },
        { "saturate",  SpecialIntrinsic::kSaturate },
        { "sin",       ByteCodeInstruction::kSin },
        { "sqrt",      ByteCodeInstruction::kSqrt },
        { "tan",       ByteCodeInstruction::kTan },

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
    if (type.kind() == Type::kOther_Kind) {
        return 0;
    } else if (type.kind() == Type::kStruct_Kind) {
        int slots = 0;
        for (const auto& f : type.fields()) {
            slots += SlotCount(*f.fType);
        }
        SkASSERT(slots <= 255);
        return slots;
    } else if (type.kind() == Type::kArray_Kind) {
        int columns = type.columns();
        SkASSERT(columns >= 0);
        int slots = columns * SlotCount(type.componentType());
        SkASSERT(slots <= 255);
        return slots;
    } else {
        return type.columns() * type.rows();
    }
}

static inline bool is_uniform(const SkSL::Variable& var) {
    return var.fModifiers.fFlags & Modifiers::kUniform_Flag;
}

static inline bool is_in(const SkSL::Variable& var) {
    return var.fModifiers.fFlags & Modifiers::kIn_Flag;
}

void ByteCodeGenerator::gatherUniforms(const Type& type, const String& name) {
    if (type.kind() == Type::kOther_Kind) {
        return;
    } else if (type.kind() == Type::kStruct_Kind) {
        for (const auto& f : type.fields()) {
            this->gatherUniforms(*f.fType, name + "." + f.fName);
        }
    } else if (type.kind() == Type::kArray_Kind) {
        for (int i = 0; i < type.columns(); ++i) {
            this->gatherUniforms(type.componentType(), String::printf("%s[%d]", name.c_str(), i));
        }
    } else {
        fOutput->fUniforms.push_back({ name, type_category(type), type.rows(), type.columns(),
                                       fOutput->fUniformSlotCount });
        fOutput->fUniformSlotCount += type.columns() * type.rows();
    }
}

bool ByteCodeGenerator::generateCode() {
    for (const auto& e : fProgram) {
        switch (e.fKind) {
            case ProgramElement::kFunction_Kind: {
                std::unique_ptr<ByteCodeFunction> f =
                        this->writeFunction(e.as<FunctionDefinition>());
                if (!f) {
                    return false;
                }
                fOutput->fFunctions.push_back(std::move(f));
                fFunctions.push_back(&e.as<FunctionDefinition>());
                break;
            }
            case ProgramElement::kVar_Kind: {
                const VarDeclarations& decl = e.as<VarDeclarations>();
                for (const auto& v : decl.fVars) {
                    const Variable* declVar = v->as<VarDeclaration>().fVar;
                    if (declVar->fType == *fContext.fFragmentProcessor_Type) {
                        fOutput->fChildFPCount++;
                    }
                    if (declVar->fModifiers.fLayout.fBuiltin >= 0 || is_in(*declVar)) {
                        continue;
                    }
                    if (is_uniform(*declVar)) {
                        this->gatherUniforms(declVar->fType, declVar->fName);
                    } else {
                        fOutput->fGlobalSlotCount += SlotCount(declVar->fType);
                    }
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
    std::unique_ptr<ByteCodeFunction> result(new ByteCodeFunction(&f.fDeclaration));
    fParameterCount = result->fParameterCount;
    fLoopCount = fMaxLoopCount = 0;
    fConditionCount = fMaxConditionCount = 0;
    fStackCount = fMaxStackCount = 0;
    fCode = &result->fCode;

    this->writeStatement(*f.fBody);
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

    const Type& returnType = f.fDeclaration.fReturnType;
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
    if (e.fKind == Expression::kVariableReference_Kind) {
        const Variable& var(e.as<VariableReference>().fVariable);
        if (var.fStorage == Variable::kGlobal_Storage) {
            return var.fModifiers.fLayout.fBuiltin;
        }
    }
    return -1;
}

// A "simple" Swizzle is based on a variable (or a compound variable like a struct or array), and
// that references consecutive values, such that it can be implemented using normal load/store ops
// with an offset. Note that all single-component swizzles (of suitable base types) are simple.
static bool swizzle_is_simple(const Swizzle& s) {
    // Builtin variables use dedicated instructions that don't allow subset loads
    if (expression_as_builtin(*s.fBase) >= 0) {
        return false;
    }

    switch (s.fBase->fKind) {
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            break;
        default:
            return false;
    }

    for (size_t i = 1; i < s.fComponents.size(); ++i) {
        if (s.fComponents[i] != s.fComponents[i - 1] + 1) {
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

        VEC_UNARY(kATan)
        VEC_UNARY(kCeil)
        VEC_UNARY(kCos)
        VEC_UNARY(kFloor)
        VEC_UNARY(kFract)
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
    switch (var.fStorage) {
        case Variable::kLocal_Storage: {
            for (int i = fLocals.size() - 1; i >= 0; --i) {
                if (fLocals[i] == &var) {
                    SkASSERT(fParameterCount + i <= 255);
                    return { fParameterCount + i, Storage::kLocal };
                }
            }
            int result = fParameterCount + fLocals.size();
            fLocals.push_back(&var);
            for (int i = 0; i < SlotCount(var.fType) - 1; ++i) {
                fLocals.push_back(nullptr);
            }
            SkASSERT(result <= 255);
            return { result, Storage::kLocal };
        }
        case Variable::kParameter_Storage: {
            int offset = 0;
            for (const auto& p : fFunction->fDeclaration.fParameters) {
                if (p == &var) {
                    SkASSERT(offset <= 255);
                    return { offset, Storage::kLocal };
                }
                offset += SlotCount(p->fType);
            }
            SkASSERT(false);
            return Location::MakeInvalid();
        }
        case Variable::kGlobal_Storage: {
            if (var.fType == *fContext.fFragmentProcessor_Type) {
                int offset = 0;
                for (const auto& e : fProgram) {
                    if (e.fKind == ProgramElement::kVar_Kind) {
                        const VarDeclarations& decl = e.as<VarDeclarations>();
                        for (const auto& v : decl.fVars) {
                            const Variable* declVar = v->as<VarDeclaration>().fVar;
                            if (declVar->fType != *fContext.fFragmentProcessor_Type) {
                                continue;
                            }
                            if (declVar == &var) {
                                SkASSERT(offset <= 255);
                                return { offset, Storage::kChildFP };
                            }
                            offset++;
                        }
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
            for (const auto& e : fProgram) {
                if (e.fKind == ProgramElement::kVar_Kind) {
                    const VarDeclarations& decl = e.as<VarDeclarations>();
                    for (const auto& v : decl.fVars) {
                        const Variable* declVar = v->as<VarDeclaration>().fVar;
                        if (declVar->fModifiers.fLayout.fBuiltin >= 0 || is_in(*declVar)) {
                            continue;
                        }
                        if (isUniform != is_uniform(*declVar)) {
                            continue;
                        }
                        if (declVar == &var) {
                            SkASSERT(offset <= 255);
                            return  { offset, isUniform ? Storage::kUniform : Storage::kGlobal };
                        }
                        offset += SlotCount(declVar->fType);
                    }
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
    switch (expr.fKind) {
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = expr.as<FieldAccess>();
            Location baseLoc = this->getLocation(*f.fBase);
            int offset = 0;
            for (int i = 0; i < f.fFieldIndex; ++i) {
                offset += SlotCount(*f.fBase->fType.fields()[i].fType);
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
        case Expression::kIndex_Kind: {
            const IndexExpression& i = expr.as<IndexExpression>();
            int stride = SlotCount(i.fType);
            int length = i.fBase->fType.columns();
            SkASSERT(length <= 255);
            int offset = -1;
            if (i.fIndex->isCompileTimeConstant()) {
                int64_t index = i.fIndex->getConstantInt();
                if (index < 0 || index >= length) {
                    fErrors.error(i.fIndex->fOffset, "Array index out of bounds.");
                    return Location::MakeInvalid();
                }
                offset = index * stride;
            } else {
                if (i.fIndex->hasSideEffects()) {
                    // Having a side-effect in an indexer is technically safe for an rvalue,
                    // but with lvalues we have to evaluate the indexer twice, so make it an error.
                    fErrors.error(i.fIndex->fOffset,
                            "Index expressions with side-effects not supported in byte code.");
                    return Location::MakeInvalid();
                }
                this->writeExpression(*i.fIndex);
                this->write(ByteCodeInstruction::kClampIndex);
                this->write8(length);
                if (stride != 1) {
                    this->write(ByteCodeInstruction::kPushImmediate);
                    this->write32(stride);
                    this->write(ByteCodeInstruction::kMultiplyI, 1);
                }
            }
            Location baseLoc = this->getLocation(*i.fBase);

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
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = expr.as<Swizzle>();
            SkASSERT(swizzle_is_simple(s));
            Location baseLoc = this->getLocation(*s.fBase);
            int offset = s.fComponents[0];
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
        case Expression::kVariableReference_Kind: {
            const Variable& var = expr.as<VariableReference>().fVariable;
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
    if (b.fOperator == Token::Kind::TK_EQ) {
        std::unique_ptr<LValue> lvalue = this->getLValue(*b.fLeft);
        this->writeExpression(*b.fRight);
        lvalue->store(discard);
        discard = false;
        return discard;
    }
    const Type& lType = b.fLeft->fType;
    const Type& rType = b.fRight->fType;
    bool lVecOrMtx = (lType.kind() == Type::kVector_Kind || lType.kind() == Type::kMatrix_Kind);
    bool rVecOrMtx = (rType.kind() == Type::kVector_Kind || rType.kind() == Type::kMatrix_Kind);
    Token::Kind op;
    std::unique_ptr<LValue> lvalue;
    if (is_assignment(b.fOperator)) {
        lvalue = this->getLValue(*b.fLeft);
        lvalue->load();
        op = remove_assignment(b.fOperator);
    } else {
        this->writeExpression(*b.fLeft);
        op = b.fOperator;
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
            this->writeExpression(*b.fRight);
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
            this->writeExpression(*b.fRight);
            this->write(ByteCodeInstruction::kOrB, 1);
            falseLocation.set();
            this->write(ByteCodeInstruction::kMaskPop);
            return false;
        }
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR: {
            SkASSERT(count == 1 && (tc == SkSL::TypeCategory::kSigned ||
                                    tc == SkSL::TypeCategory::kUnsigned));
            if (!b.fRight->isCompileTimeConstant()) {
                fErrors.error(b.fRight->fOffset, "Shift amounts must be constant");
                return false;
            }
            int64_t shift = b.fRight->getConstantInt();
            if (shift < 0 || shift > 31) {
                fErrors.error(b.fRight->fOffset, "Shift amount out of range");
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
    this->writeExpression(*b.fRight);
    if (lVecOrMtx && !rVecOrMtx) {
        for (int i = SlotCount(lType); i > 1; --i) {
            this->write(ByteCodeInstruction::kDup, 1);
        }
    }
    // Special case for M*V, V*M, M*M (but not V*V!)
    if (op == Token::Kind::TK_STAR && lVecOrMtx && rVecOrMtx &&
        !(lType.kind() == Type::kVector_Kind && rType.kind() == Type::kVector_Kind)) {
        this->write(ByteCodeInstruction::kMatrixMultiply,
                    SlotCount(b.fType) - (SlotCount(lType) + SlotCount(rType)));
        int rCols = rType.columns(),
            rRows = rType.rows(),
            lCols = lType.columns(),
            lRows = lType.rows();
        // M*V treats the vector as a column
        if (rType.kind() == Type::kVector_Kind) {
            std::swap(rCols, rRows);
        }
        SkASSERT(lCols == rRows);
        SkASSERT(SlotCount(b.fType) == lRows * rCols);
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
    this->write32(b.fValue ? ~0 : 0);
}

void ByteCodeGenerator::writeConstructor(const Constructor& c) {
    for (const auto& arg : c.fArguments) {
        this->writeExpression(*arg);
    }
    if (c.fArguments.size() == 1) {
        const Type& inType = c.fArguments[0]->fType;
        const Type& outType = c.fType;
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
        if (inType.kind() == Type::kMatrix_Kind && outType.kind() == Type::kMatrix_Kind) {
            this->write(ByteCodeInstruction::kMatrixToMatrix,
                        SlotCount(outType) - SlotCount(inType));
            this->write8(inType.columns());
            this->write8(inType.rows());
            this->write8(outType.columns());
            this->write8(outType.rows());
        } else if (inCount != outCount) {
            SkASSERT(inCount == 1);
            if (outType.kind() == Type::kMatrix_Kind) {
                this->write(ByteCodeInstruction::kScalarToMatrix, SlotCount(outType) - 1);
                this->write8(outType.columns());
                this->write8(outType.rows());
            } else {
                SkASSERT(outType.kind() == Type::kVector_Kind);
                for (; inCount != outCount; ++inCount) {
                    this->write(ByteCodeInstruction::kDup, 1);
                }
            }
        }
    }
}

void ByteCodeGenerator::writeExternalFunctionCall(const ExternalFunctionCall& f) {
    int argumentCount = 0;
    for (const auto& arg : f.fArguments) {
        this->writeExpression(*arg);
        argumentCount += SlotCount(arg->fType);
    }
    this->write(ByteCodeInstruction::kCallExternal, SlotCount(f.fType) - argumentCount);
    SkASSERT(argumentCount <= 255);
    this->write8(argumentCount);
    this->write8(SlotCount(f.fType));
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(f.fFunction);
    SkASSERT(index <= 255);
    this->write8(index);
}

void ByteCodeGenerator::writeExternalValue(const ExternalValueReference& e) {
    int count = SlotCount(e.fValue->type());
    this->write(ByteCodeInstruction::kReadExternal, count);
    int index = fOutput->fExternalValues.size();
    fOutput->fExternalValues.push_back(e.fValue);
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
    int count = SlotCount(expr.fType);
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
    this->write32(float_to_bits(f.fValue));
}

static bool is_generic_type(const Type* type, const Type* generic) {
    const std::vector<const Type*>& concrete(generic->coercibleTypes());
    return std::find(concrete.begin(), concrete.end(), type) != concrete.end();
}

void ByteCodeGenerator::writeIntrinsicCall(const FunctionCall& c) {
    auto found = fIntrinsics.find(c.fFunction.fName);
    if (found == fIntrinsics.end()) {
        fErrors.error(c.fOffset, String::printf("Unsupported intrinsic: '%s'",
                                                String(c.fFunction.fName).c_str()));
        return;
    }
    Intrinsic intrin = found->second;

    const auto& args = c.fArguments;
    const size_t nargs = args.size();
    SkASSERT(nargs >= 1);

    int count = SlotCount(args[0]->fType);

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
        if (nargs > 2 || args[0]->fType != *fContext.fFragmentProcessor_Type ||
            (nargs == 2 && (args[1]->fType != *fContext.fFloat2_Type &&
                            args[1]->fType != *fContext.fFloat3x3_Type))) {
            fErrors.error(c.fOffset, "Unsupported form of sample");
            return;
        }

        if (nargs == 2) {
            // Write our coords or matrix
            this->writeExpression(*args[1]);
            this->write(args[1]->fType == *fContext.fFloat3x3_Type
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

    if (intrin.is_special && (intrin.special == SpecialIntrinsic::kClamp ||
                              intrin.special == SpecialIntrinsic::kSaturate)) {
        // These intrinsics are extra-special, we need instructions interleaved with arguments
        bool saturate = (intrin.special == SpecialIntrinsic::kSaturate);
        SkASSERT(nargs == (saturate ? 1 : 3));
        int limitCount = saturate ? 1 : SlotCount(args[1]->fType);

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
        this->writeTypedInstruction(args[0]->fType,
                                    ByteCodeInstruction::kMaxS,
                                    ByteCodeInstruction::kMaxS,
                                    ByteCodeInstruction::kMaxF,
                                    count);

        // 'maxVal'
        if (saturate) {
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(float_to_bits(1.0f));
        } else {
            SkASSERT(limitCount == SlotCount(args[2]->fType));
            this->writeExpression(*args[2]);
        }
        dupSmallerType(limitCount);
        this->writeTypedInstruction(args[0]->fType,
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

            case SpecialIntrinsic::kDot: {
                SkASSERT(nargs == 2);
                SkASSERT(count == SlotCount(args[1]->fType));
                this->write(ByteCodeInstruction::kMultiplyF, count);
                for (int i = count-1; i --> 0;) {
                    this->write(ByteCodeInstruction::kAddF, 1);
                }
            } break;

            case SpecialIntrinsic::kLength: {
                SkASSERT(nargs == 1);
                this->write(ByteCodeInstruction::kDup, count);
                this->write(ByteCodeInstruction::kMultiplyF, count);
                for (int i = count-1; i --> 0;) {
                    this->write(ByteCodeInstruction::kAddF, 1);
                }
                this->write(ByteCodeInstruction::kSqrt, 1);
            } break;

            case SpecialIntrinsic::kMax:
            case SpecialIntrinsic::kMin: {
                SkASSERT(nargs == 2);
                // There are variants where the second argument is scalar
                dupSmallerType(SlotCount(args[1]->fType));
                if (intrin.special == SpecialIntrinsic::kMax) {
                    this->writeTypedInstruction(args[0]->fType,
                                                ByteCodeInstruction::kMaxS,
                                                ByteCodeInstruction::kMaxS,
                                                ByteCodeInstruction::kMaxF,
                                                count);
                } else {
                    this->writeTypedInstruction(args[0]->fType,
                                                ByteCodeInstruction::kMinS,
                                                ByteCodeInstruction::kMinS,
                                                ByteCodeInstruction::kMinF,
                                                count);
                }
            } break;

            case SpecialIntrinsic::kMix: {
                // Two main variants of mix to handle
                SkASSERT(nargs == 3);
                SkASSERT(count == SlotCount(args[1]->fType));
                int selectorCount = SlotCount(args[2]->fType);

                if (is_generic_type(&args[2]->fType, fContext.fGenBType_Type.get())) {
                    // mix(genType, genType, genBoolType)
                    SkASSERT(selectorCount == count);
                    this->write(ByteCodeInstruction::kMix, count);
                } else {
                    // mix(genType, genType, genType) or mix(genType, genType, float)
                    dupSmallerType(selectorCount);
                    this->write(ByteCodeInstruction::kLerp, count);
                }
            } break;

            case SpecialIntrinsic::kNormalize: {
                SkASSERT(nargs == 1);
                this->write(ByteCodeInstruction::kDup, count);
                this->write(ByteCodeInstruction::kDup, count);
                this->write(ByteCodeInstruction::kMultiplyF, count);
                for (int i = count-1; i --> 0;) {
                    this->write(ByteCodeInstruction::kAddF, 1);
                }
                this->write(ByteCodeInstruction::kSqrt, 1);
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
                this->writeTypedInstruction(args[0]->fType,
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
        if (f.fFunction.matches(fFunctions[i]->fDeclaration)) {
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
    if (int returnCount = SlotCount(f.fType)) {
        this->write(ByteCodeInstruction::kReserve, returnCount);
    }

    int argCount = f.fArguments.size();
    std::vector<std::unique_ptr<LValue>> lvalues;
    for (int i = 0; i < argCount; ++i) {
        const auto& param = f.fFunction.fParameters[i];
        const auto& arg = f.fArguments[i];
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
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
        const auto& param = f.fFunction.fParameters[i];
        const auto& arg = f.fArguments[i];
        if (param->fModifiers.fFlags & Modifiers::kOut_Flag) {
            pop();
            lvalues.back()->store(true);
            lvalues.pop_back();
        } else {
            popCount += SlotCount(arg->fType);
        }
    }
    pop();
}

void ByteCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    this->write(ByteCodeInstruction::kPushImmediate);
    this->write32(i.fValue);
}

void ByteCodeGenerator::writeNullLiteral(const NullLiteral& n) {
    // not yet implemented
    abort();
}

bool ByteCodeGenerator::writePrefixExpression(const PrefixExpression& p, bool discard) {
    switch (p.fOperator) {
        case Token::Kind::TK_PLUSPLUS: // fall through
        case Token::Kind::TK_MINUSMINUS: {
            SkASSERT(SlotCount(p.fOperand->fType) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.fOperand);
            lvalue->load();
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.fType) == TypeCategory::kFloat ? float_to_bits(1.0f) : 1);
            if (p.fOperator == Token::Kind::TK_PLUSPLUS) {
                this->writeTypedInstruction(p.fType,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            1);
            } else {
                this->writeTypedInstruction(p.fType,
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
            this->writeExpression(*p.fOperand);
            this->writeTypedInstruction(p.fType,
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateI,
                                        ByteCodeInstruction::kNegateF,
                                        SlotCount(p.fOperand->fType));
            break;
        }
        case Token::Kind::TK_LOGICALNOT:
        case Token::Kind::TK_BITWISENOT: {
            SkASSERT(SlotCount(p.fOperand->fType) == 1);
            SkDEBUGCODE(TypeCategory tc = type_category(p.fOperand->fType));
            SkASSERT((p.fOperator == Token::Kind::TK_LOGICALNOT && tc == TypeCategory::kBool) ||
                     (p.fOperator == Token::Kind::TK_BITWISENOT && (tc == TypeCategory::kSigned ||
                                                                 tc == TypeCategory::kUnsigned)));
            this->writeExpression(*p.fOperand);
            this->write(ByteCodeInstruction::kNotB, 1);
            break;
        }
        default:
            SkASSERT(false);
    }
    return discard;
}

bool ByteCodeGenerator::writePostfixExpression(const PostfixExpression& p, bool discard) {
    switch (p.fOperator) {
        case Token::Kind::TK_PLUSPLUS: // fall through
        case Token::Kind::TK_MINUSMINUS: {
            SkASSERT(SlotCount(p.fOperand->fType) == 1);
            std::unique_ptr<LValue> lvalue = this->getLValue(*p.fOperand);
            lvalue->load();
            // If we're not supposed to discard the result, then make a copy *before* the +/-
            if (!discard) {
                this->write(ByteCodeInstruction::kDup, 1);
            }
            this->write(ByteCodeInstruction::kPushImmediate);
            this->write32(type_category(p.fType) == TypeCategory::kFloat ? float_to_bits(1.0f) : 1);
            if (p.fOperator == Token::Kind::TK_PLUSPLUS) {
                this->writeTypedInstruction(p.fType,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddI,
                                            ByteCodeInstruction::kAddF,
                                            1);
            } else {
                this->writeTypedInstruction(p.fType,
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

    this->writeExpression(*s.fBase);
    this->write(ByteCodeInstruction::kSwizzle, s.fComponents.size() - s.fBase->fType.columns());
    this->write8(s.fBase->fType.columns());
    this->write8(s.fComponents.size());
    for (int c : s.fComponents) {
        this->write8(c);
    }
}

void ByteCodeGenerator::writeTernaryExpression(const TernaryExpression& t) {
    int count = SlotCount(t.fType);
    SkASSERT(count == SlotCount(t.fIfTrue->fType));
    SkASSERT(count == SlotCount(t.fIfFalse->fType));

    this->writeExpression(*t.fTest);
    this->write(ByteCodeInstruction::kMaskPush);
    this->writeExpression(*t.fIfTrue);
    this->write(ByteCodeInstruction::kMaskNegate);
    this->writeExpression(*t.fIfFalse);
    this->write(ByteCodeInstruction::kMaskBlend, count);
}

void ByteCodeGenerator::writeExpression(const Expression& e, bool discard) {
    switch (e.fKind) {
        case Expression::kBinary_Kind:
            discard = this->writeBinaryExpression(e.as<BinaryExpression>(), discard);
            break;
        case Expression::kBoolLiteral_Kind:
            this->writeBoolLiteral(e.as<BoolLiteral>());
            break;
        case Expression::kConstructor_Kind:
            this->writeConstructor(e.as<Constructor>());
            break;
        case Expression::kExternalFunctionCall_Kind:
            this->writeExternalFunctionCall(e.as<ExternalFunctionCall>());
            break;
        case Expression::kExternalValue_Kind:
            this->writeExternalValue(e.as<ExternalValueReference>());
            break;
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            this->writeVariableExpression(e);
            break;
        case Expression::kFloatLiteral_Kind:
            this->writeFloatLiteral(e.as<FloatLiteral>());
            break;
        case Expression::kFunctionCall_Kind:
            this->writeFunctionCall(e.as<FunctionCall>());
            break;
        case Expression::kIntLiteral_Kind:
            this->writeIntLiteral(e.as<IntLiteral>());
            break;
        case Expression::kNullLiteral_Kind:
            this->writeNullLiteral(e.as<NullLiteral>());
            break;
        case Expression::kPrefix_Kind:
            discard = this->writePrefixExpression(e.as<PrefixExpression>(), discard);
            break;
        case Expression::kPostfix_Kind:
            discard = this->writePostfixExpression(e.as<PostfixExpression>(), discard);
            break;
        case Expression::kSwizzle_Kind:
            this->writeSwizzle(e.as<Swizzle>());
            break;
        case Expression::kTernary_Kind:
            this->writeTernaryExpression(e.as<TernaryExpression>());
            break;
        default:
#ifdef SK_DEBUG
            printf("unsupported expression %s\n", e.description().c_str());
#endif
            SkASSERT(false);
    }
    if (discard) {
        int count = SlotCount(e.fType);
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
    typedef LValue INHERITED;

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
        int count = fSwizzle.fComponents.size();
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
            ByteCodeGenerator::Location location = fGenerator.getLocation(*fSwizzle.fBase);
            if (!location.isOnStack()) {
                fGenerator.write(location.selectStore(ByteCodeInstruction::kStore,
                                                      ByteCodeInstruction::kStoreGlobal),
                                 1);
                fGenerator.write8(location.fSlot + fSwizzle.fComponents[i]);
            } else {
                fGenerator.write(ByteCodeInstruction::kPushImmediate);
                fGenerator.write32(fSwizzle.fComponents[i]);
                fGenerator.write(ByteCodeInstruction::kAddI, 1);
                fGenerator.write(location.selectStore(ByteCodeInstruction::kStoreExtended,
                                                      ByteCodeInstruction::kStoreExtendedGlobal),
                                 1);
            }
        }
    }

private:
    const Swizzle& fSwizzle;

    typedef LValue INHERITED;
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
        int count = ByteCodeGenerator::SlotCount(fExpression.fType);
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
    typedef LValue INHERITED;

    const Expression& fExpression;
};

std::unique_ptr<ByteCodeGenerator::LValue> ByteCodeGenerator::getLValue(const Expression& e) {
    switch (e.fKind) {
        case Expression::kExternalValue_Kind: {
            const ExternalValue* value = e.as<ExternalValueReference>().fValue;
            int index = fOutput->fExternalValues.size();
            fOutput->fExternalValues.push_back(value);
            SkASSERT(index <= 255);
            return std::unique_ptr<LValue>(new ByteCodeExternalValueLValue(this, *value, index));
        }
        case Expression::kFieldAccess_Kind:
        case Expression::kIndex_Kind:
        case Expression::kVariableReference_Kind:
            return std::unique_ptr<LValue>(new ByteCodeExpressionLValue(this, e));
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = e.as<Swizzle>();
            return swizzle_is_simple(s)
                    ? std::unique_ptr<LValue>(new ByteCodeExpressionLValue(this, e))
                    : std::unique_ptr<LValue>(new ByteCodeSwizzleLValue(this, s));
        }
        case Expression::kTernary_Kind:
        default:
#ifdef SK_DEBUG
            ABORT("unsupported lvalue %s\n", e.description().c_str());
#endif
            return nullptr;
    }
}

void ByteCodeGenerator::writeBlock(const Block& b) {
    for (const auto& s : b.fStatements) {
        this->writeStatement(*s);
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
    this->writeStatement(*d.fStatement);
    this->write(ByteCodeInstruction::kLoopNext);
    this->writeExpression(*d.fTest);
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
    if (f.fInitializer) {
        this->writeStatement(*f.fInitializer);
    }
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t start = fCode->size();
    if (f.fTest) {
        this->writeExpression(*f.fTest);
        this->write(ByteCodeInstruction::kLoopMask);
    }
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(*f.fStatement);
    this->write(ByteCodeInstruction::kLoopNext);
    if (f.fNext) {
        this->writeExpression(*f.fNext, true);
    }
    this->write(ByteCodeInstruction::kBranch);
    this->write16(start);
    endLocation.set();
    this->write(ByteCodeInstruction::kLoopEnd);
}

void ByteCodeGenerator::writeIfStatement(const IfStatement& i) {
    this->writeExpression(*i.fTest);
    this->write(ByteCodeInstruction::kMaskPush);
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation falseLocation(this);
    this->writeStatement(*i.fIfTrue);
    falseLocation.set();
    if (i.fIfFalse) {
        this->write(ByteCodeInstruction::kMaskNegate);
        this->write(ByteCodeInstruction::kBranchIfAllFalse);
        DeferredLocation endLocation(this);
        this->writeStatement(*i.fIfFalse);
        endLocation.set();
    }
    this->write(ByteCodeInstruction::kMaskPop);
}

void ByteCodeGenerator::writeReturnStatement(const ReturnStatement& r) {
    if (fLoopCount || fConditionCount) {
        fErrors.error(r.fOffset, "return not allowed inside conditional or loop");
        return;
    }
    int count = SlotCount(r.fExpression->fType);
    this->writeExpression(*r.fExpression);

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

void ByteCodeGenerator::writeVarDeclarations(const VarDeclarations& v) {
    for (const auto& declStatement : v.fVars) {
        const VarDeclaration& decl = declStatement->as<VarDeclaration>();
        // we need to grab the location even if we don't use it, to ensure it has been allocated
        Location location = this->getLocation(*decl.fVar);
        if (decl.fValue) {
            this->writeExpression(*decl.fValue);
            int count = SlotCount(decl.fValue->fType);
            this->write(ByteCodeInstruction::kStore, count);
            this->write8(location.fSlot);
        }
    }
}

void ByteCodeGenerator::writeWhileStatement(const WhileStatement& w) {
    this->write(ByteCodeInstruction::kLoopBegin);
    size_t cond = fCode->size();
    this->writeExpression(*w.fTest);
    this->write(ByteCodeInstruction::kLoopMask);
    this->write(ByteCodeInstruction::kBranchIfAllFalse);
    DeferredLocation endLocation(this);
    this->writeStatement(*w.fStatement);
    this->write(ByteCodeInstruction::kLoopNext);
    this->write(ByteCodeInstruction::kBranch);
    this->write16(cond);
    endLocation.set();
    this->write(ByteCodeInstruction::kLoopEnd);
}

void ByteCodeGenerator::writeStatement(const Statement& s) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            this->writeBlock(s.as<Block>());
            break;
        case Statement::kBreak_Kind:
            this->writeBreakStatement(s.as<BreakStatement>());
            break;
        case Statement::kContinue_Kind:
            this->writeContinueStatement(s.as<ContinueStatement>());
            break;
        case Statement::kDiscard_Kind:
            // not yet implemented
            abort();
        case Statement::kDo_Kind:
            this->writeDoStatement(s.as<DoStatement>());
            break;
        case Statement::kExpression_Kind:
            this->writeExpression(*s.as<ExpressionStatement>().fExpression, true);
            break;
        case Statement::kFor_Kind:
            this->writeForStatement(s.as<ForStatement>());
            break;
        case Statement::kIf_Kind:
            this->writeIfStatement(s.as<IfStatement>());
            break;
        case Statement::kNop_Kind:
            break;
        case Statement::kReturn_Kind:
            this->writeReturnStatement(s.as<ReturnStatement>());
            break;
        case Statement::kSwitch_Kind:
            this->writeSwitchStatement(s.as<SwitchStatement>());
            break;
        case Statement::kVarDeclarations_Kind:
            this->writeVarDeclarations(*s.as<VarDeclarationsStatement>().fDeclaration);
            break;
        case Statement::kWhile_Kind:
            this->writeWhileStatement(s.as<WhileStatement>());
            break;
        default:
            SkASSERT(false);
    }
}

ByteCodeFunction::ByteCodeFunction(const FunctionDeclaration* declaration)
        : fName(declaration->fName) {
    fParameterCount = 0;
    for (const auto& p : declaration->fParameters) {
        int slots = ByteCodeGenerator::SlotCount(p->fType);
        fParameters.push_back({ slots, (bool)(p->fModifiers.fFlags & Modifiers::kOut_Flag) });
        fParameterCount += slots;
    }
}

}  // namespace SkSL
