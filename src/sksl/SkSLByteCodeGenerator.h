/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BYTECODEGENERATOR
#define SKSL_BYTECODEGENERATOR

#include <algorithm>
#include <stack>
#include <unordered_map>

#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCodeGenerator.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLExternalValueReference.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLNullLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/ir/SkSLWhileStatement.h"
#include "src/sksl/spirv.h"

namespace SkSL {

class ByteCodeGenerator : public CodeGenerator {
public:
    ByteCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                      ByteCode* output);

    bool generateCode() override;

private:
    // Intrinsics which do not simply map to a single opcode
    enum class SpecialIntrinsic {
        kDot,
    };

    struct Intrinsic {
        Intrinsic(ByteCode::Instruction instruction)
            : fIsSpecial(false)
            , fValue(instruction) {}

        Intrinsic(SpecialIntrinsic special)
            : fIsSpecial(true)
            , fValue(special) {}

        bool fIsSpecial;

        union Value {
            Value(ByteCode::Instruction instruction)
                : fInstruction(instruction) {}

            Value(SpecialIntrinsic special)
                : fSpecial(special) {}

            ByteCode::Instruction fInstruction;
            SpecialIntrinsic fSpecial;
        } fValue;
    };

    class LValue {
    public:
        LValue(ByteCodeGenerator& generator)
            : fGenerator(generator) {}

        virtual ~LValue() {}

        virtual ByteCode::Register load() = 0;

        virtual void store(ByteCode::Register src) = 0;

    protected:
        ByteCodeGenerator& fGenerator;
    };

    struct Location {
        enum {
            kPointer_Kind,
            kRegister_Kind
        } fKind;

        union {
            ByteCode::Pointer fPointer;
            ByteCode::Register fRegister;
        };

        Location(ByteCode::Pointer p)
            : fKind(kPointer_Kind)
            , fPointer(p) {}

        Location(ByteCode::Register r)
            : fKind(kRegister_Kind)
            , fRegister(r) {}

        /**
         * Returns this location offset by 'offset' bytes. For pointers, this is a compile-time
         * operation, while for registers there will be CPU instructions output to handle the
         * runtime calculation of the address.
         */
        Location offset(ByteCodeGenerator& generator, int offset) {
            if (!offset) {
                return *this;
            }
            if (fKind == kPointer_Kind) {
                return Location(fPointer + offset);
            }
            ByteCode::Register a = generator.next();
            generator.write(ByteCode::Instruction::kImmediate);
            generator.write(a);
            generator.write(ByteCode::Immediate{offset});
            ByteCode::Register result = generator.next();
            generator.write(ByteCode::Instruction::kAddI);
            generator.write(result);
            generator.write(fRegister);
            generator.write(a);
            return result;
        }

        /**
         * Returns this location offset by the number of bytes stored in the 'offset' register. This
         * will output the necessary CPU instructions to perform the math and return a new register
         * location.
         */
        Location offset(ByteCodeGenerator& generator, ByteCode::Register offset) {
            ByteCode::Register current;
            switch (fKind) {
                case kPointer_Kind:
                    current = generator.next();
                    generator.write(ByteCode::Instruction::kImmediate);
                    generator.write(current);
                    generator.write(ByteCode::Immediate{fPointer.fAddress});
                    break;
                case kRegister_Kind:
                    current = fRegister;
            }
            ByteCode::Register result = generator.next();
            generator.write(ByteCode::Instruction::kAddI);
            generator.write(result);
            generator.write(current);
            generator.write(offset);
            return result;
        }
    };

    // reserves 16 bits in the output code, to be filled in later with an address once we determine
    // it
    class DeferredLocation {
    public:
        explicit DeferredLocation(ByteCodeGenerator* generator)
            : fGenerator(*generator)
            , fOffset(generator->fCode->size()) {
            generator->write(ByteCode::Pointer{65535});
        }

        void set() {
            SkASSERT(fGenerator.fCode->size() <= ByteCode::kPointerMax);
            static_assert(sizeof(ByteCode::Pointer) == 2,
                          "failed assumption that ByteCode::Pointer is uint16_t");
            ByteCode::Pointer target{(uint16_t) fGenerator.fCode->size()};
            memcpy(&(*fGenerator.fCode)[fOffset], &target, sizeof(target));
        }

    private:
        ByteCodeGenerator& fGenerator;
        size_t fOffset;
    };

    template<typename T>
    void write(T value) {
        size_t n = fCode->size();
        fCode->resize(n + sizeof(value));
        memcpy(fCode->data() + n, &value, sizeof(value));
    }

    ByteCode::Register next();

    /**
     * Based on 'type', writes the s (signed), u (unsigned), or f (float) instruction.
     */
    void writeTypedInstruction(const Type& type, ByteCode::Instruction s, ByteCode::Instruction u,
                               ByteCode::Instruction f);

    ByteCode::Instruction getLoadInstruction(Location location, Variable::Storage storage);

    ByteCode::Instruction getStoreInstruction(Location location, Variable::Storage storage);

    static int SlotCount(const Type& type);

    Location getLocation(const Variable& var);

    Location getLocation(const Expression& expr);

    Variable::Storage getStorage(const Expression& expr);

    std::unique_ptr<LValue> getLValue(const Expression& expr);

    void writeFunction(const FunctionDefinition& f);

    // For compound values, the various write functions return the register containing the first
    // component. Subsequent components will be in subsequent registers.

    ByteCode::Register writeBinaryInstruction(const Type& operandType, ByteCode::Register left,
                                              ByteCode::Register right, ByteCode::Instruction s,
                                              ByteCode::Instruction u, ByteCode::Instruction f);

    ByteCode::Register writeBinaryExpression(const BinaryExpression& expr);

    ByteCode::Register writeConstructor(const Constructor& c);

    ByteCode::Register writeIntrinsicCall(const FunctionCall& c,
                                          std::vector<ByteCode::Register> args);

    ByteCode::Register writeFunctionCall(const FunctionCall& c);

    ByteCode::Register incOrDec(Token::Kind op, Expression& operand, bool prefix);

    ByteCode::Register writePostfixExpression(const PostfixExpression& p);

    ByteCode::Register writePrefixExpression(const PrefixExpression& p);

    ByteCode::Register writeSwizzle(const Swizzle& s);

    ByteCode::Register writeTernaryExpression(const TernaryExpression& t);

    ByteCode::Register writeVariableExpression(const Expression& e);

    ByteCode::Register writeExpression(const Expression& expr);

    void writeBlock(const Block& b);

    void writeDoStatement(const DoStatement& d);

    void writeForStatement(const ForStatement& f);

    void writeIfStatement(const IfStatement& i);

    void writeReturn(const ReturnStatement& r);

    void writeVarDeclarations(const VarDeclarations& v);

    void writeWhileStatement(const WhileStatement& w);

    void writeStatement(const Statement& s);

    const Context& fContext;

    ByteCode* fOutput;

    uint16_t fNextRegister = 0;

    const FunctionDefinition* fFunction;

    std::vector<const FunctionDefinition*> fFunctions;

    std::vector<uint8_t>* fCode;

    std::vector<const Variable*> fLocals;

    int fParameterCount;

    const std::unordered_map<String, Intrinsic> fIntrinsics;

    friend class DeferredLocation;
    friend class ByteCodeSimpleLValue;
    friend class ByteCodeSwizzleLValue;

    typedef CodeGenerator INHERITED;
};

template<>
inline void ByteCodeGenerator::write(ByteCodeGenerator::Location loc) {
    switch (loc.fKind) {
        case ByteCodeGenerator::Location::kPointer_Kind:
            this->write(loc.fPointer);
            break;
        case ByteCodeGenerator::Location::kRegister_Kind:
            this->write(loc.fRegister);
            break;
    }
}

}

#endif
