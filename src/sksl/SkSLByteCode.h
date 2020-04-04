/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BYTECODE
#define SKSL_BYTECODE

#include "include/private/SkOnce.h"
#include "src/sksl/SkSLString.h"

#include <memory>
#include <vector>

namespace SkSL {

class  ExternalValue;
struct FunctionDeclaration;

// GCC and Clang support the "labels as values" extension which we need to implement the interpreter
// using threaded code. Otherwise, we fall back to using a switch statement in a for loop.
#if defined(__GNUC__) || defined(__clang__)
    #define SKSLC_THREADED_CODE
    using instruction = void*;
#else
    using instruction = uint16_t;
#endif

#define VECTOR(name) name ## 4, name ## 3, name ## 2, name
#define VECTOR_MATRIX(name) name ## 4, name ## 3, name ## 2, name, name ## N

enum class ByteCodeInstruction : uint16_t {
    // B = bool, F = float, I = int, S = signed, U = unsigned
    // All binary VECTOR instructions (kAddF, KSubtractI, kCompareIEQ, etc.) are followed by a byte
    // indicating the count, even though it is redundant due to the count appearing in the opcode.
    // This is because the original opcodes are lost after we preprocess it into threaded code, and
    // we need to still be able to access the count so as to permit the implementation to use opcode
    // fallthrough.
    VECTOR_MATRIX(kAddF),
    VECTOR(kAddI),
    kAndB,
    kBranch,
    // Followed by a byte indicating the index of the function to call
    kCall,
    // Followed by three bytes indicating: the number of argument slots, the number of return slots,
    // and the index of the external value to call
    kCallExternal,
    // For dynamic array access: Followed by byte indicating length of array
    kClampIndex,
    VECTOR(kCompareIEQ),
    VECTOR(kCompareINEQ),
    VECTOR_MATRIX(kCompareFEQ),
    VECTOR_MATRIX(kCompareFNEQ),
    VECTOR(kCompareFGT),
    VECTOR(kCompareFGTEQ),
    VECTOR(kCompareFLT),
    VECTOR(kCompareFLTEQ),
    VECTOR(kCompareSGT),
    VECTOR(kCompareSGTEQ),
    VECTOR(kCompareSLT),
    VECTOR(kCompareSLTEQ),
    VECTOR(kCompareUGT),
    VECTOR(kCompareUGTEQ),
    VECTOR(kCompareULT),
    VECTOR(kCompareULTEQ),
    VECTOR(kConvertFtoI),
    VECTOR(kConvertStoF),
    VECTOR(kConvertUtoF),
    // Followed by a (redundant) byte indicating the count
    VECTOR(kCos),
    VECTOR_MATRIX(kDivideF),
    VECTOR(kDivideS),
    VECTOR(kDivideU),
    // Duplicates the top stack value. Followed by a (redundant) byte indicating the count.
    VECTOR_MATRIX(kDup),
    kInverse2x2,
    kInverse3x3,
    kInverse4x4,
    // kLoad/kLoadGlobal are followed by a byte indicating the count, and a byte indicating the
    // local/global slot to load
    VECTOR(kLoad),
    VECTOR(kLoadGlobal),
    VECTOR(kLoadUniform),
    // As kLoad/kLoadGlobal, then a count byte (1-4), and then one byte per swizzle component (0-3).
    kLoadSwizzle,
    kLoadSwizzleGlobal,
    kLoadSwizzleUniform,
    // kLoadExtended* are fallback load ops when we lack a specialization. They are followed by a
    // count byte, and get the slot to load from the top of the stack.
    kLoadExtended,
    kLoadExtendedGlobal,
    kLoadExtendedUniform,
    // Followed by four bytes: srcCols, srcRows, dstCols, dstRows. Consumes the src matrix from the
    // stack, and replaces it with the dst matrix. Per GLSL rules, there are no restrictions on
    // dimensions. Any overlapping values are copied, and any other values are filled in with the
    // identity matrix.
    kMatrixToMatrix,
    // Followed by three bytes: leftCols (== rightRows), leftRows, rightCols
    kMatrixMultiply,
    VECTOR_MATRIX(kNegateF),
    VECTOR(kNegateI),
    VECTOR_MATRIX(kMultiplyF),
    VECTOR(kMultiplyI),
    kNotB,
    kOrB,
    VECTOR_MATRIX(kPop),
    // Followed by a 32 bit value containing the value to push
    kPushImmediate,
    // Followed by a byte indicating external value to read
    VECTOR(kReadExternal),
    VECTOR(kRemainderF),
    VECTOR(kRemainderS),
    VECTOR(kRemainderU),
    // Followed by a byte indicating the number of slots to reserve on the stack (for later return)
    kReserve,
    // Followed by a byte indicating the number of slots being returned
    kReturn,
    // Followed by two bytes indicating columns and rows of matrix (2, 3, or 4 each).
    // Takes a single value from the top of the stack, and converts to a CxR matrix with that value
    // replicated along the diagonal (and zero elsewhere), per the GLSL matrix construction rules.
    kScalarToMatrix,
    // Followed by a byte indicating the number of bits to shift
    kShiftLeft,
    kShiftRightS,
    kShiftRightU,
    // Followed by a (redundant) byte indicating the count
    VECTOR(kSin),
    VECTOR(kSqrt),
    // kStore/kStoreGlobal are followed by a byte indicating the local/global slot to store
    VECTOR(kStore),
    VECTOR(kStoreGlobal),
    // Fallback stores. Followed by count byte, and get the slot to store from the top of the stack
    kStoreExtended,
    kStoreExtendedGlobal,
    // As kStore/kStoreGlobal, then a count byte (1-4), then one byte per swizzle component (0-3).
    // Expects the stack to look like: ... v1 v2 v3 v4, where the number of 'v's is equal to the
    // number of swizzle components. After the store, all v's are popped from the stack.
    kStoreSwizzle,
    kStoreSwizzleGlobal,
    // As above, but gets the store slot from the top of the stack (before values to be stored)
    kStoreSwizzleIndirect,
    kStoreSwizzleIndirectGlobal,
    // Followed by two count bytes (1-4), and then one byte per swizzle component (0-3). The first
    // count byte provides the current vector size (the vector is the top n stack elements), and the
    // second count byte provides the swizzle component count.
    kSwizzle,
    VECTOR_MATRIX(kSubtractF),
    VECTOR(kSubtractI),
    // Followed by a (redundant) byte indicating the count
    VECTOR(kTan),
    // Followed by a byte indicating external value to write
    VECTOR(kWriteExternal),
    kXorB,

    kMaskPush,
    kMaskPop,
    kMaskNegate,
    // Followed by count byte
    kMaskBlend,
    // Followed by address
    kBranchIfAllFalse,

    kLoopBegin,
    kLoopNext,
    kLoopMask,
    kLoopEnd,
    kLoopBreak,
    kLoopContinue,
};
#undef VECTOR

class ByteCodeFunction {
public:
    int getParameterCount() const { return fParameterCount; }
    int getReturnCount() const { return fReturnCount; }

    /**
     * Print bytecode disassembly to stdout.
     */
    void disassemble() const;

private:
    ByteCodeFunction(const FunctionDeclaration* declaration);

    friend class ByteCode;
    friend class ByteCodeGenerator;
    friend struct Interpreter;

    struct Parameter {
        int fSlotCount;
        bool fIsOutParameter;
    };

    SkSL::String fName;
    std::vector<Parameter> fParameters;
    int fParameterCount;
    int fReturnCount = 0;

    int fLocalCount = 0;
    int fStackCount = 0;
    int fConditionCount = 0;
    int fLoopCount = 0;
    mutable SkOnce fPreprocessOnce;
    std::vector<uint8_t> fCode;

    /**
     * Replace each opcode with the corresponding entry from the labels array.
     */
    void preprocess(const void* labels[]);
};

enum class TypeCategory {
    kBool,
    kSigned,
    kUnsigned,
    kFloat,
};

class SK_API ByteCode {
public:
    static constexpr int kVecWidth = 16;

    ByteCode() = default;

    const ByteCodeFunction* getFunction(const char* name) const {
        for (const auto& f : fFunctions) {
            if (f->fName == name) {
                return f.get();
            }
        }
        return nullptr;
    }

    /**
     * Invokes the specified function once, with the given arguments.
     * 'args', 'outReturn', and 'uniforms' are collections of 32-bit values (typically floats,
     * but possibly int32_t or uint32_t, depending on the types used in the SkSL).
     * Any 'out' or 'inout' parameters will result in the 'args' array being modified.
     * The return value is stored in 'outReturn' (may be null, to discard the return value).
     * 'uniforms' are mapped to 'uniform' globals, in order.
     */
    bool SKSL_WARN_UNUSED_RESULT run(const ByteCodeFunction*,
                                     float* args, int argCount,
                                     float* outReturn, int returnCount,
                                     const float* uniforms, int uniformCount) const;

    /**
     * Invokes the specified function with the given arguments, 'N' times. 'args' and 'outReturn'
     * are accepted and returned in structure-of-arrays form:
     *   args[0] points to an array of N values, the first argument for each invocation
     *   ...
     *   args[argCount - 1] points to an array of N values, the last argument for each invocation
     *
     * All values in 'args', 'outReturn', and 'uniforms' are 32-bit values (typically floats,
     * but possibly int32_t or uint32_t, depending on the types used in the SkSL).
     * Any 'out' or 'inout' parameters will result in the 'args' array being modified.
     * The return value is stored in 'outReturn' (may be null, to discard the return value).
     * 'uniforms' are mapped to 'uniform' globals, in order.
     */
    bool SKSL_WARN_UNUSED_RESULT runStriped(const ByteCodeFunction*, int N,
                                            float* args[], int argCount,
                                            float* outReturn[], int returnCount,
                                            const float* uniforms, int uniformCount) const;

    struct Uniform {
        SkSL::String fName;
        TypeCategory fType;
        int fColumns;
        int fRows;
        int fSlot;
    };

    int getUniformSlotCount() const { return fUniformSlotCount; }
    int getUniformCount() const { return fUniforms.size(); }
    int getUniformLocation(const char* name) const {
        for (int i = 0; i < (int)fUniforms.size(); ++i) {
            if (fUniforms[i].fName == name) {
                return fUniforms[i].fSlot;
            }
        }
        return -1;
    }
    const Uniform& getUniform(int i) const { return fUniforms[i]; }

private:
    ByteCode(const ByteCode&) = delete;
    ByteCode& operator=(const ByteCode&) = delete;

    friend class ByteCodeGenerator;
    friend struct Interpreter;

    int fGlobalSlotCount = 0;
    int fUniformSlotCount = 0;
    std::vector<Uniform> fUniforms;

    std::vector<std::unique_ptr<ByteCodeFunction>> fFunctions;
    std::vector<ExternalValue*> fExternalValues;
};

}

#endif
