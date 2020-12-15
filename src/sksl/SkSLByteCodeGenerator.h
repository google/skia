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
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/spirv.h"

namespace SkSL {

class ByteCodeGenerator : public CodeGenerator {
public:
    class LValue {
    public:
        LValue(ByteCodeGenerator& generator)
            : fGenerator(generator) {}

        virtual ~LValue() {}

        /**
         * Stack before call: ... lvalue
         * Stack after call: ... lvalue load
         */
        virtual void load() = 0;

        /**
         * Stack before call: ... lvalue value
         * Stack after call: ...
         */
        virtual void store(bool discard) = 0;

    protected:
        ByteCodeGenerator& fGenerator;
    };

    ByteCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                      ByteCode* output);

    bool generateCode() override;

    void write8(uint8_t b);

    void write16(uint16_t b);

    void write32(uint32_t b);

    void write(ByteCodeInstruction inst, int count = kUnusedStackCount);

    /**
     * Based on 'type', writes the s (signed), u (unsigned), or f (float) instruction.
     */
    void writeTypedInstruction(const Type& type, ByteCodeInstruction s, ByteCodeInstruction u,
                               ByteCodeInstruction f, int count);

    static int SlotCount(const Type& type);

private:
    static constexpr int kUnusedStackCount = INT32_MAX;
    static int StackUsage(ByteCodeInstruction, int count);

    // reserves 16 bits in the output code, to be filled in later with an address once we determine
    // it
    class DeferredLocation {
    public:
        DeferredLocation(ByteCodeGenerator* generator)
            : fGenerator(*generator)
            , fOffset(generator->fCode->size()) {
            generator->write16(0);
        }

#ifdef SK_DEBUG
        ~DeferredLocation() {
            SkASSERT(fSet);
        }
#endif

        void set() {
            int target = fGenerator.fCode->size();
            SkASSERT(target <= 65535);
            (*fGenerator.fCode)[fOffset] = target;
            (*fGenerator.fCode)[fOffset + 1] = target >> 8;
#ifdef SK_DEBUG
            fSet = true;
#endif
        }

    private:
        ByteCodeGenerator& fGenerator;
        size_t fOffset;
#ifdef SK_DEBUG
        bool fSet = false;
#endif
    };

    // Intrinsics which do not simply map to a single opcode
    enum class SpecialIntrinsic {
        kAll,
        kAny,
        kATan,
        kClamp,
        kDistance,
        kDot,
        kLength,
        kMax,
        kMin,
        kMix,
        kMod,
        kNormalize,
        kSample,
        kSaturate,
        kSmoothstep,
        kStep,
    };

    struct Intrinsic {
        Intrinsic(SpecialIntrinsic    s) : is_special(true), special(s) {}
        Intrinsic(ByteCodeInstruction i) : Intrinsic(i, i, i) {}
        // Workaround: We should be able to leave special uninitialized here, and were for a long
        // time. Unrelated changes have made valgrind suddenly start complaining about us accessing
        // uninitialized memory in the code:
        //     if (intrin.is_special && intrin.special == SpecialIntrinsic::kSample)
        // despite intrin.is_special being false at the time and therefore, one would think, not
        // actually accessing intrin.special. I'm not sure whether this is a buggy optimization on
        // clang's part or a false positive on valgrind's part, but either way initializing the
        // field works around it.
        Intrinsic(ByteCodeInstruction f,
                  ByteCodeInstruction s,
                  ByteCodeInstruction u) : is_special(false), special((SpecialIntrinsic) -1),
                                           inst_f(f), inst_s(s), inst_u(u) {}

        bool                is_special;
        SpecialIntrinsic    special;
        ByteCodeInstruction inst_f;
        ByteCodeInstruction inst_s;
        ByteCodeInstruction inst_u;
    };


    // Similar to Variable::Storage, but locals and parameters are grouped together, and globals
    // are further subidivided into uniforms and other (writable) globals.
    enum class Storage {
        kLocal,    // include parameters
        kGlobal,   // non-uniform globals
        kUniform,  // uniform globals
        kChildFP,  // child fragment processors
    };

    struct Location {
        int     fSlot;
        Storage fStorage;

        // Not really invalid, but a "safe" placeholder to be more explicit at call-sites
        static Location MakeInvalid() { return { 0, Storage::kLocal }; }

        Location makeOnStack() {
            SkASSERT(fStorage != Storage::kChildFP);
            return { -1, fStorage };
        }
        bool isOnStack() const { return fSlot < 0; }

        Location operator+(int offset) {
            SkASSERT(fStorage != Storage::kChildFP);
            SkASSERT(fSlot >= 0);
            return { fSlot + offset, fStorage };
        }

        ByteCodeInstruction selectLoad(ByteCodeInstruction local,
                                       ByteCodeInstruction global,
                                       ByteCodeInstruction uniform) const {
            switch (fStorage) {
                case Storage::kLocal:   return local;
                case Storage::kGlobal:  return global;
                case Storage::kUniform: return uniform;
                case Storage::kChildFP: ABORT("Trying to load an FP"); break;
            }
            return local;
        }

        ByteCodeInstruction selectStore(ByteCodeInstruction local,
                                        ByteCodeInstruction global) const {
            switch (fStorage) {
                case Storage::kLocal:   return local;
                case Storage::kGlobal:  return global;
                case Storage::kUniform: ABORT("Trying to store to a uniform"); break;
                case Storage::kChildFP: ABORT("Trying to store an FP"); break;
            }
            return local;
        }
    };

    /**
     * Returns the local slot into which var should be stored, allocating a new slot if it has not
     * already been assigned one. Compound variables (e.g. vectors) will consume more than one local
     * slot, with the getLocation return value indicating where the first element should be stored.
     */
    Location getLocation(const Variable& var);

    /**
     * As above, but computes the (possibly dynamic) address of an expression involving indexing &
     * field access. If the address is known, it's returned. If not, -1 is returned, and the
     * location will be left on the top of the stack.
     */
    Location getLocation(const Expression& expr);

    void gatherUniforms(const Type& type, const String& name);

    std::unique_ptr<ByteCodeFunction> writeFunction(const FunctionDefinition& f);

    void writeVarDeclaration(const VarDeclaration& decl);

    void writeVariableExpression(const Expression& expr);

    void writeExpression(const Expression& expr, bool discard = false);

    /**
     * Pushes whatever values are required by the lvalue onto the stack, and returns an LValue
     * permitting loads and stores to it.
     */
    std::unique_ptr<LValue> getLValue(const Expression& expr);

    void writeIntrinsicCall(const FunctionCall& c);
    void writeFunctionCall(const FunctionCall& c);
    void writeConstructor(const Constructor& c);
    void writeExternalFunctionCall(const ExternalFunctionCall& c);
    void writeExternalValue(const ExternalValueReference& r);
    void writeSwizzle(const Swizzle& swizzle);
    bool writeBinaryExpression(const BinaryExpression& b, bool discard);
    void writeTernaryExpression(const TernaryExpression& t);
    bool writePrefixExpression(const PrefixExpression& p, bool discard);
    bool writePostfixExpression(const PostfixExpression& p, bool discard);

    void writeNullLiteral(const NullLiteral& n);
    void writeBoolLiteral(const BoolLiteral& b);
    void writeIntLiteral(const IntLiteral& i);
    void writeFloatLiteral(const FloatLiteral& f);

    void writeStatement(const Statement& s);
    void writeBlock(const Block& b);
    void writeBreakStatement(const BreakStatement& b);
    void writeContinueStatement(const ContinueStatement& c);
    void writeIfStatement(const IfStatement& stmt);
    void writeForStatement(const ForStatement& f);
    void writeDoStatement(const DoStatement& d);
    void writeSwitchStatement(const SwitchStatement& s);
    void writeReturnStatement(const ReturnStatement& r);

    // Some intrinsics are complex enough to warrant their own functions:
    void writeSmoothstep(const ExpressionArray& args);

    // updates the current set of breaks to branch to the current location
    void setBreakTargets();

    // updates the current set of continues to branch to the current location
    void setContinueTargets();

    void enterLoop() {
        fLoopCount++;
        fMaxLoopCount = std::max(fMaxLoopCount, fLoopCount);
    }

    void exitLoop() {
        SkASSERT(fLoopCount > 0);
        fLoopCount--;
    }

    void enterCondition() {
        fConditionCount++;
        fMaxConditionCount = std::max(fMaxConditionCount, fConditionCount);
    }

    void exitCondition() {
        SkASSERT(fConditionCount > 0);
        fConditionCount--;
    }

    const Context& fContext;

    ByteCode* fOutput;

    const FunctionDefinition* fFunction;

    std::vector<uint8_t>* fCode;

    std::vector<const Variable*> fLocals;

    std::stack<std::vector<DeferredLocation>> fContinueTargets;

    std::stack<std::vector<DeferredLocation>> fBreakTargets;

    std::vector<const FunctionDefinition*> fFunctions;

    int fParameterCount;
    int fStackCount;
    int fMaxStackCount;

    int fLoopCount;
    int fMaxLoopCount;
    int fConditionCount;
    int fMaxConditionCount;

    // Holds variables synthesized during output, for lifetime purposes
    SymbolTable fSynthetics;

    const std::unordered_map<String, Intrinsic> fIntrinsics;

    friend class DeferredLocation;
    friend class ByteCodeExpressionLValue;
    friend class ByteCodeSwizzleLValue;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
