/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BYTECODE
#define SKSL_BYTECODE

#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

enum class ByteCodeInstruction : uint8_t {
    kInvalid,
    kNop,
    // B = bool, F = float, I = int, S = signed, U = unsigned
    kAddF,
    kAddI,
    kAndB,
    kAndI,
    kBranch,
    kCompareIEQ,
    kCompareINEQ,
    kCompareFEQ,
    kCompareFGT,
    kCompareFGTEQ,
    kCompareFLT,
    kCompareFLTEQ,
    kCompareFNEQ,
    kCompareSGT,
    kCompareSGTEQ,
    kCompareSLT,
    kCompareSLTEQ,
    kCompareUGT,
    kCompareUGTEQ,
    kCompareULT,
    kCompareULTEQ,
    // Followed by a 16 bit address
    kConditionalBranch,
    // Pops and prints the top value from the stack
    kDebugPrint,
    kDivideF,
    kDivideS,
    kDivideU,
    // Duplicates the top stack value
    kDup,
    // Followed by a byte indicating number of slots to copy below the underlying element.
    // dupdown 2 yields: ... value3 value2 value1 => .. value2 value1 value3 value2 value2
    kDupDown,
    kFloatToInt,
    kSignedToFloat,
    kUnsignedToFloat,
    kLoad,
    // Followed by a byte indicating global slot to load
    kLoadGlobal,
    // Followed by a count byte (1-4), and then one byte per swizzle component (0-3).
    kLoadSwizzle,
    kNegateF,
    kNegateS,
    kMultiplyF,
    kMultiplyS,
    kMultiplyU,
    kNot,
    kOrB,
    kOrI,
    // Followed by a byte indicating parameter slot to load
    kParameter,
    kPop,
    // Followed by a 32 bit value containing the value to push
    kPushImmediate,
    kRemainderS,
    kRemainderU,
    // Followed by a byte indicating the number of slots being returned
    kReturn,
    kStore,
    kStoreGlobal,
    // Followed by a count byte (1-4), and then one byte per swizzle component (0-3). Expects the
    // stack to look like: ... target v1 v2 v3 v4, where the number of 'v's is equal to the number
    // of swizzle components. After the store, the target and all v's are popped from the stack.
    kStoreSwizzle,
    // Followed by two count bytes (1-4), and then one byte per swizzle component (0-3). The first
    // count byte provides the current vector size (the vector is the top n stack elements), and the
    // second count byte provides the swizzle component count.
    kSwizzle,
    kSubtractF,
    kSubtractI,
    // Followed by a byte indicating vector count. Modifies the next instruction to operate on the
    // indicated number of columns, e.g. kVector 2 kMultiplyf performs a float2 * float2 operation.
    kVector,
};

struct ByteCode;

struct ByteCodeFunction {
    ByteCodeFunction(const ByteCode* owner, const FunctionDeclaration* declaration)
        : fOwner(*owner)
        , fDeclaration(*declaration) {}

    const ByteCode& fOwner;
    const FunctionDeclaration& fDeclaration;
    int fParameterCount = 0;
    int fLocalCount = 0;
    std::vector<uint8_t> fCode;
};

struct ByteCode {
    int fGlobalCount = 0;
    int fInputCount = 0;
    // one entry per input slot, contains the global slot to which the input slot maps
    std::vector<uint8_t> fInputSlots;
    std::vector<std::unique_ptr<ByteCodeFunction>> fFunctions;
};

}

#endif
