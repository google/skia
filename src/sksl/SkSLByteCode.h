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

class ExternalValue;
class FunctionDeclaration;
class OutputStream;

enum class ByteCodeInstruction : uint8_t {
    // B = bool, F = float, I = int, S = signed, U = unsigned

    kAbs,   // N
    kAddF,  // N
    kAddI,  // N
    kAndB,  // N
    kACos,  // N
    kASin,  // N
    kATan,  // N
    kATan2, // N
    kBranch,
    // Followed by a byte indicating the index of the function to call
    kCall,
    // Followed by three bytes indicating: the number of argument slots, the number of return slots,
    // and the index of the external value to call
    kCallExternal,
    kCeil,  // N
    // For dynamic array access: Followed by byte indicating length of array
    kClampIndex,
    kCompareIEQ,    // N
    kCompareINEQ,   // N
    kCompareFEQ,    // N
    kCompareFNEQ,   // N
    kCompareFGT,    // N
    kCompareFGTEQ,  // N
    kCompareFLT,    // N
    kCompareFLTEQ,  // N
    kCompareSGT,    // N
    kCompareSGTEQ,  // N
    kCompareSLT,    // N
    kCompareSLTEQ,  // N
    kCompareUGT,    // N
    kCompareUGTEQ,  // N
    kCompareULT,    // N
    kCompareULTEQ,  // N
    kConvertFtoI,   // N
    kConvertStoF,   // N
    kConvertUtoF,   // N
    kCos,           // N
    kDivideF,       // N
    kDivideS,       // N
    kDivideU,       // N
    // Duplicates the top N stack values
    kDup,    // N
    kExp,    // N
    kExp2,   // N
    kFloor,  // N
    kFract,  // N
    kInverse2x2,
    kInverse3x3,
    kInverse4x4,
    // A1, A2, .., B1, B2, .., T1, T2, .. -> lerp(A1, B1, T1), lerp(A2, B2, T2), ..
    kLerp,  // N
    kLoad,                 // N, slot
    kLoadGlobal,           // N, slot
    kLoadUniform,          // N, slot
    // Indirect loads get the slot to load from the top of the stack
    kLoadExtended,         // N
    kLoadExtendedGlobal,   // N
    kLoadExtendedUniform,  // N
    // Loads "sk_FragCoord" [X, Y, Z, 1/W]
    kLoadFragCoord,
    kLog,   // N
    kLog2,  // N
    // Followed by four bytes: srcCols, srcRows, dstCols, dstRows. Consumes the src matrix from the
    // stack, and replaces it with the dst matrix. Per GLSL rules, there are no restrictions on
    // dimensions. Any overlapping values are copied, and any other values are filled in with the
    // identity matrix.
    kMatrixToMatrix,
    // Followed by three bytes: leftCols (== rightRows), leftRows, rightCols
    kMatrixMultiply,
    kMaxF,  // N
    kMaxS,  // N  --  SkSL only declares signed versions of min/max
    kMinF,  // N
    kMinS,  // N
    // Masked selection: Stack is ... A1, A2, A3, B1, B2, B3, M1, M2, M3
    //                   Result:      M1 ? B1 : A1, M2 ? B2 : A2, M3 ? B3 : A3
    kMix,        // N
    kMod,        // N
    kNegateF,    // N
    kNegateI,    // N
    kMultiplyF,  // N
    kMultiplyI,  // N
    kNotB,       // N
    kOrB,        // N
    kPop,        // N
    kPow,        // N
    // Followed by a 32 bit value containing the value to push
    kPushImmediate,
    kReadExternal,  // N, slot
    kRemainderF,    // N
    kRemainderS,    // N
    kRemainderU,    // N
    // Followed by a byte indicating the number of slots to reserve on the stack (for later return)
    kReserve,
    // Followed by a byte indicating the number of slots being returned
    kReturn,
    kInvSqrt,  // N
    // kSample* are followed by a byte indicating the FP slot to sample, and produce (R, G, B, A)
    // Does "pass-through" sampling at the same coords as the parent
    kSample,
    // Expects stack to contain (X, Y)
    kSampleExplicit,
    // Expects stack to contain a 3x3 matrix (applied to parent's sample coords)
    kSampleMatrix,
    // Followed by two bytes indicating columns and rows of matrix (2, 3, or 4 each).
    // Takes a single value from the top of the stack, and converts to a CxR matrix with that value
    // replicated along the diagonal (and zero elsewhere), per the GLSL matrix construction rules.
    kScalarToMatrix,
    // Followed by a byte indicating the number of bits to shift
    kShiftLeft,
    kShiftRightS,
    kShiftRightU,
    kSign,  // N
    kSin,   // N
    kSqrt,  // N
    kStep,  // N
    kStore,                // N, slot
    kStoreGlobal,          // N, slot
    // Indirect stores get the slot to store from the top of the stack
    kStoreExtended,        // N
    kStoreExtendedGlobal,  // N
    // Followed by two count bytes (1-4), and then one byte per swizzle component (0-3). The first
    // count byte provides the current vector size (the vector is the top n stack elements), and the
    // second count byte provides the swizzle component count.
    kSwizzle,
    kSubtractF,  // N
    kSubtractI,  // N
    kTan,        // N
    kWriteExternal,  // N, slot
    kXorB,       // N

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

class ByteCodeFunction {
public:
    int getParameterCount() const { return fParameterCount; }
    int getReturnCount() const { return fReturnCount; }
    int getLocalCount() const { return fLocalCount; }

    const uint8_t* code() const { return fCode.data(); }
    size_t         size() const { return fCode.size(); }

    /**
     * Print bytecode disassembly to 'out', or SkDebugf if not supplied.
     */
    void disassemble(OutputStream* out = nullptr) const;

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
    std::vector<uint8_t> fCode;
};

enum class TypeCategory {
    kBool,
    kSigned,
    kUnsigned,
    kFloat,
};

class SK_API ByteCode {
public:
    static constexpr int kVecWidth = 8;

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

    /**
     * Some byte code programs can't be executed by the interpreter, due to unsupported features.
     * They may still be used to convert to other formats, or for reflection of uniforms.
     */
    bool canRun() const { return fChildFPCount == 0 && !fUsesFragCoord; }

    /**
     * Print bytecode disassembly to 'out', or SkDebugf if not supplied.
     */
    void disassemble(OutputStream* out = nullptr) const;

private:
    ByteCode(const ByteCode&) = delete;
    ByteCode& operator=(const ByteCode&) = delete;

    friend class ByteCodeGenerator;
    friend struct Interpreter;

    int fGlobalSlotCount = 0;
    int fUniformSlotCount = 0;
    int fChildFPCount = 0;
    bool fUsesFragCoord = false;
    std::vector<Uniform> fUniforms;

    std::vector<std::unique_ptr<ByteCodeFunction>> fFunctions;
    std::vector<const ExternalValue*> fExternalValues;
};

}  // namespace SkSL

#endif
