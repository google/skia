/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VMGENERATOR
#define SKSL_VMGENERATOR

#include <algorithm>
#include <stack>
#include <unordered_map>

#include "include/private/SkTArray.h"
#include "src/core/SkSpan.h"
#include "src/core/SkVM.h"
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
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

// TODO: Detach from CodeGenerator, just make a function with a private helper class in the .cpp

class SkVMGenerator : public CodeGenerator {
public:
    SkVMGenerator(const Context* context,
                  const Program* program,
                  ErrorReporter* errors,
                  skvm::Builder* builder,
                  SkSpan<skvm::Val> uniforms,
                  skvm::Coord deviceCoord,  // Make this more general map of builtins to skvm::Val?
                  SkSpan<skvm::Val> params,
                  const char* function = "main");

    bool generateCode() override;

    // Holds scalars, vectors, or matrices
    struct Val {
        Val() = default;
        Val(int reserveCount) : fVals(reserveCount) {}
        Val(skvm::F32 x) : fVals({ x.id }) {}
        Val(skvm::I32 x) : fVals({ x.id }) {}

        explicit operator bool() const {
            return !fVals.empty() && std::all_of(fVals.begin(), fVals.end(), [](skvm::Val id) {
                return id != skvm::NA;
            });
        }

        void push_back(skvm::Val x) { fVals.push_back(x); }
        void push_back(skvm::F32 x) { fVals.push_back(x.id); }
        void push_back(skvm::I32 x) { fVals.push_back(x.id); }

        size_t size() const { return fVals.size(); }

        skvm::Val& operator[](int i)       { return fVals[i]; }
        skvm::Val  operator[](int i) const { return fVals[i]; }

        SkSTArray<4, skvm::Val, true> fVals;
    };

private:
#if 0
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
#endif

    using Location = size_t;
    static constexpr Location kInvalidLocation = ~static_cast<size_t>(0);

    /**
     * Returns the slot into which var should be stored, allocating space if necessary. Compound
     * variables (e.g. vectors) will consume more than one slot, with the getLocation return value
     * indicating where the first element should be stored.
     */
    Location getLocation(const Variable& v);

    /**
     * As above, but computes the address of an expression involving indexing & field access.
     * The address must not be dynamic (no computed indexing). If the address is known, it's
     * returned. If not, kInvalidLocation is returned.
     */
    Location getLocation(const Expression& e);

    skvm::F32 f32(skvm::Val id) { return {fBuilder, id}; }
    skvm::I32 i32(skvm::Val id) { return {fBuilder, id}; }

    // Shorthand for scalars
    skvm::F32 f32(const Val& v) { SkASSERT(v.size() == 1); return f32(v[0]); }
    skvm::I32 i32(const Val& v) { SkASSERT(v.size() == 1); return i32(v[0]); }

    template <typename Fn>
    Val unary_f(const Val& v, Fn&& fn) {
        Val result(v.size());
        for (size_t i = 0; i < v.size(); ++i) {
            result.push_back(fn(f32(v[i])));
        }
        return result;
    }

    template <typename Fn>
    Val unary_i(const Val& v, Fn&& fn) {
        Val result(v.size());
        for (size_t i = 0; i < v.size(); ++i) {
            result.push_back(fn(i32(v[i])));
        }
        return result;
    }

    skvm::I32 mask() { return fMask; }

    Val writeExpression(const Expression& expr);
    Val writeBinaryExpression(const BinaryExpression& b);
    Val writeConstructor(const Constructor& c);
    Val writeFunctionCall(const FunctionCall& c);
    Val writeIntrinsicCall(const FunctionCall& c);
    Val writePostfixExpression(const PostfixExpression& p);
    Val writePrefixExpression(const PrefixExpression& p);
    Val writeSwizzle(const Swizzle& swizzle);
    Val writeTernaryExpression(const TernaryExpression& t);
    Val writeVariableExpression(const Expression& expr);

    void writeStatement(const Statement& s);
    void writeBlock(const Block& b);
    void writeIfStatement(const IfStatement& stmt);
    void writeReturnStatement(const ReturnStatement& r);
    void writeVarDeclaration(const VarDeclaration& decl);

    void writeStore(const Expression& lhs, const Val& rhs);

    const Context& fContext;

    skvm::Builder* fBuilder;

    skvm::I32 fMask;
    skvm::I32 fReturned;

    skvm::Coord fDeviceCoord;
    const FunctionDefinition* fFunction;

    // Storage for parameters, locals, etc.
    std::vector<skvm::Val> fVariables;

    // [Variable, first slot in fVariables]
    std::unordered_map<const Variable*, Location> fVariableMap;
    size_t fReturnSlot;

//    const std::unordered_map<String, Intrinsic> fIntrinsics;

    friend class AutoMask;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
