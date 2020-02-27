/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkUtils.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLExternalValue.h"

namespace SkSL {

template <typename T>
static T read(const uint8_t** ip) {
    *ip += sizeof(T);
    return sk_unaligned_load<T>(*ip - sizeof(T));
}

#define DISASSEMBLE_0(inst, name) \
    case Instruction::inst:       \
        return String(name);

#define DISASSEMBLE_1(inst, name) \
    case Instruction::inst:       \
        return String::printf(name " $%d", read<Register>(ip).fIndex);

#define DISASSEMBLE_UNARY(inst, name)                                         \
    case Instruction::inst: {                                                 \
        Register target = read<Register>(ip);                                 \
        Register src = read<Register>(ip);                                    \
        return String::printf(name " $%d -> $%d", src.fIndex, target.fIndex); \
    }

#define DISASSEMBLE_VECTOR_UNARY(inst, name)                                           \
    case Instruction::inst: {                                                          \
        Register target = read<Register>(ip);                                          \
        Register src = read<Register>(ip);                                             \
        return String::printf(name " $%d -> $%d", src.fIndex, target.fIndex);          \
    }                                                                                  \
    case Instruction::inst##N: {                                                       \
        uint8_t count = read<uint8_t>(ip);                                             \
        Register target = read<Register>(ip);                                          \
        Register src = read<Register>(ip);                                             \
        return String::printf(name "%d $%d -> $%d", count, src.fIndex, target.fIndex); \
    }

#define DISASSEMBLE_BINARY(inst, name)                                                           \
    case Instruction::inst: {                                                                    \
        Register target = read<Register>(ip);                                                    \
        Register src1 = read<Register>(ip);                                                      \
        Register src2 = read<Register>(ip);                                                      \
        return String::printf(name " $%d, $%d -> $%d", src1.fIndex, src2.fIndex, target.fIndex); \
    }

#define DISASSEMBLE_VECTOR_BINARY(inst, name)                                                    \
    case Instruction::inst: {                                                                    \
        Register target = read<Register>(ip);                                                    \
        Register src1 = read<Register>(ip);                                                      \
        Register src2 = read<Register>(ip);                                                      \
        return String::printf(name " $%d, $%d -> $%d", src1.fIndex, src2.fIndex, target.fIndex); \
    }                                                                                            \
    case Instruction::inst##N: {                                                                 \
        uint8_t count = read<uint8_t>(ip);                                                       \
        Register target = read<Register>(ip);                                                    \
        Register src1 = read<Register>(ip);                                                      \
        Register src2 = read<Register>(ip);                                                      \
        return String::printf(name "%d $%d, $%d -> $%d", count, src1.fIndex, src2.fIndex,        \
                              target.fIndex);                                                    \
    }

// $x = register
// @x = memory cell
// &x = parameter
String ByteCode::disassemble(const uint8_t** ip) const {
    Instruction inst = read<Instruction>(ip);
    switch (inst) {
        DISASSEMBLE_VECTOR_BINARY(kAddF, "addF")
        DISASSEMBLE_VECTOR_BINARY(kAddI, "addI")
        DISASSEMBLE_BINARY(kAnd, "and")
        DISASSEMBLE_BINARY(kCompareEQF, "compare eqF")
        DISASSEMBLE_BINARY(kCompareEQI, "compare eqI")
        DISASSEMBLE_BINARY(kCompareNEQF, "compare neqF")
        DISASSEMBLE_BINARY(kCompareNEQI, "compare neqI")
        DISASSEMBLE_BINARY(kCompareGTF, "compare gtF")
        DISASSEMBLE_BINARY(kCompareGTS, "compare gtS")
        DISASSEMBLE_BINARY(kCompareGTU, "compare gtU")
        DISASSEMBLE_BINARY(kCompareGTEQF, "compare gteqF")
        DISASSEMBLE_BINARY(kCompareGTEQS, "compare gteqS")
        DISASSEMBLE_BINARY(kCompareGTEQU, "compare gteqU")
        DISASSEMBLE_BINARY(kCompareLTF, "compare ltF")
        DISASSEMBLE_BINARY(kCompareLTS, "compare ltS")
        DISASSEMBLE_BINARY(kCompareLTU, "compare ltU")
        DISASSEMBLE_BINARY(kCompareLTEQF, "compare lteqF")
        DISASSEMBLE_BINARY(kCompareLTEQS, "compare lteqS")
        DISASSEMBLE_BINARY(kCompareLTEQU, "compare lteqU")
        DISASSEMBLE_VECTOR_BINARY(kSubtractF, "subF")
        DISASSEMBLE_VECTOR_BINARY(kSubtractI, "subI")
        DISASSEMBLE_VECTOR_BINARY(kDivideF, "divF")
        DISASSEMBLE_VECTOR_BINARY(kDivideS, "divS")
        DISASSEMBLE_VECTOR_BINARY(kDivideU, "divU")
        DISASSEMBLE_VECTOR_BINARY(kRemainderS, "remS")
        DISASSEMBLE_VECTOR_BINARY(kRemainderU, "remU")
        DISASSEMBLE_VECTOR_BINARY(kRemainderF, "remF")
        DISASSEMBLE_VECTOR_BINARY(kMultiplyF, "mulF")
        DISASSEMBLE_VECTOR_BINARY(kMultiplyI, "mulI")
        DISASSEMBLE_BINARY(kOr, "or")
        DISASSEMBLE_BINARY(kXor, "xor")
        DISASSEMBLE_0(kNop, "nop")
        DISASSEMBLE_0(kAbort, "abort")
        case Instruction::kBoundsCheck: {
            Register r = read<Register>(ip);
            int length = read<int>(ip);
            return String::printf("boundsCheck 0 <= $%d < %d", r.fIndex, length);
        }
        case Instruction::kBranch:
            return String::printf("branch %d", read<Pointer>(ip).fAddress);
        case Instruction::kBranchIfAllFalse:
            return String::printf("branchIfAllFalse %d", read<Pointer>(ip).fAddress);
            DISASSEMBLE_0(kBreak, "break")
        case Instruction::kCall: {
            Register target = read<Register>(ip);
            uint8_t idx = read<uint8_t>(ip);
            Register args = read<Register>(ip);
            ByteCodeFunction* f = fFunctions[idx].get();
            return String::printf("call %s($%d...) -> $%d", f->fName.c_str(), args.fIndex,
                                  target.fIndex);
        }
        case Instruction::kCallExternal: {
            Register target = read<Register>(ip);
            uint8_t idx = read<uint8_t>(ip);
            uint8_t targetCount = read<uint8_t>(ip);
            Register args = read<Register>(ip);
            uint8_t argCount = read<uint8_t>(ip);
            ExternalValue* ev = fExternalValues[idx];
            return String::printf("callExternal %s($%d(%d)...) -> $%d(%d)",
                                  String(ev->fName).c_str(), args.fIndex, argCount, target.fIndex,
                                  targetCount);
        }
            DISASSEMBLE_0(kContinue, "continue")
            DISASSEMBLE_UNARY(kCopy, "copy")
            DISASSEMBLE_UNARY(kCos, "cos")
            DISASSEMBLE_UNARY(kFloatToSigned, "FtoS")
            DISASSEMBLE_UNARY(kFloatToUnsigned, "FtoU")
        case Instruction::kImmediate: {
            Register target = read<Register>(ip);
            Immediate src = read<Immediate>(ip);
            return String::printf("immediate (%d | %f) -> $%d", src.fInt, src.fFloat,
                                  target.fIndex);
        }
            DISASSEMBLE_UNARY(kInverse2x2, "inverse2x2")
            DISASSEMBLE_UNARY(kInverse3x3, "inverse3x3")
            DISASSEMBLE_UNARY(kInverse4x4, "inverse4x4")
            DISASSEMBLE_VECTOR_UNARY(kLoad, "load")
        case Instruction::kLoadDirect: {
            Register target = read<Register>(ip);
            Pointer src = read<Pointer>(ip);
            return String::printf("loadDirect @%d -> $%d", src.fAddress, target.fIndex);
        }
        case Instruction::kLoadDirectN: {
            uint8_t count = read<uint8_t>(ip);
            Register target = read<Register>(ip);
            Pointer src = read<Pointer>(ip);
            return String::printf("loadDirect%d @%d -> $%d", count, src.fAddress, target.fIndex);
        }
            DISASSEMBLE_VECTOR_UNARY(kLoadParameter, "loadParameter")
        case Instruction::kLoadParameterDirect: {
            Register target = read<Register>(ip);
            Pointer src = read<Pointer>(ip);
            return String::printf("loadParameterDirect &%d -> $%d", src.fAddress, target.fIndex);
        }
        case Instruction::kLoadParameterDirectN: {
            uint8_t count = read<uint8_t>(ip);
            Register target = read<Register>(ip);
            Pointer src = read<Pointer>(ip);
            return String::printf("loadParameterDirect%d &%d -> $%d", count, src.fAddress,
                                  target.fIndex);
        }
            DISASSEMBLE_VECTOR_UNARY(kLoadStack, "loadStack")
        case Instruction::kLoadStackDirect: {
            Register target = read<Register>(ip);
            Pointer src = read<Pointer>(ip);
            return String::printf("loadStackDirect @%d -> $%d", src.fAddress, target.fIndex);
        }
        case Instruction::kLoadStackDirectN: {
            uint8_t count = read<uint8_t>(ip);
            Register target = read<Register>(ip);
            Pointer src = read<Pointer>(ip);
            return String::printf("loadStackDirect%d @%d -> $%d", count, src.fAddress,
                                  target.fIndex);
        }
            DISASSEMBLE_0(kLoopBegin, "loopBegin")
            DISASSEMBLE_0(kLoopEnd, "loopEnd")
            DISASSEMBLE_1(kLoopMask, "loopMask")
            DISASSEMBLE_0(kLoopNext, "loopNext")
            DISASSEMBLE_0(kMaskNegate, "maskNegate")
            DISASSEMBLE_0(kMaskPop, "maskPop")
            DISASSEMBLE_1(kMaskPush, "maskPush")
        case Instruction::kMatrixMultiply: {
            Register target = read<Register>(ip);
            Register left = read<Register>(ip);
            Register right = read<Register>(ip);
            uint8_t leftColsAndRightRows = read<uint8_t>(ip);
            uint8_t leftRows = read<uint8_t>(ip);
            uint8_t rightColumns = read<uint8_t>(ip);
            return String::printf("matrixMultiply $%d, $%d, %d, %d, %d -> $%d", left.fIndex,
                                  right.fIndex, leftColsAndRightRows, leftRows, rightColumns,
                                  target.fIndex);
        }
        case Instruction::kMatrixToMatrix: {
            Register target = read<Register>(ip);
            Register src = read<Register>(ip);
            uint8_t srcColumns = read<uint8_t>(ip);
            uint8_t srcRows = read<uint8_t>(ip);
            uint8_t dstColumns = read<uint8_t>(ip);
            uint8_t dstRows = read<uint8_t>(ip);
            return String::printf("matrixToMatrix $%d, %dx%d to %dx%d -> $%d", src.fIndex,
                                  srcColumns, srcRows, dstColumns, dstRows, target.fIndex);
        }
            DISASSEMBLE_UNARY(kNegateF, "negateF")
            DISASSEMBLE_UNARY(kNegateS, "negateS")
            DISASSEMBLE_UNARY(kNot, "not")
        case Instruction::kReadExternal: {
            Register target = read<Register>(ip);
            uint8_t count = read<uint8_t>(ip);
            uint8_t index = read<uint8_t>(ip);
            return String::printf("readExternal %d, %d -> $%d", count, index, target.fIndex);
        }
            DISASSEMBLE_1(kPrint, "print")
            DISASSEMBLE_0(kReturn, "return")
            DISASSEMBLE_1(kReturnValue, "returnValue")
        case Instruction::kScalarToMatrix: {
            Register target = read<Register>(ip);
            Register src = read<Register>(ip);
            uint8_t columns = read<uint8_t>(ip);
            uint8_t rows = read<uint8_t>(ip);
            return String::printf("scalarToMatrix $%d, %dx%d -> $%d", src.fIndex, columns, rows,
                                  target.fIndex);
        }
        case Instruction::kSelect: {
            Register target = read<Register>(ip);
            Register test = read<Register>(ip);
            Register src1 = read<Register>(ip);
            Register src2 = read<Register>(ip);
            return String::printf("select $%d, $%d, $%d -> %d", test.fIndex, src1.fIndex,
                                  src2.fIndex, target.fIndex);
        }
            DISASSEMBLE_BINARY(kShiftLeft, "shiftLeft")
            DISASSEMBLE_BINARY(kShiftRightS, "shiftRightS")
            DISASSEMBLE_BINARY(kShiftRightU, "shiftRightU")
            DISASSEMBLE_UNARY(kSignedToFloat, "signedToFloat")
            DISASSEMBLE_UNARY(kSin, "sin")
        case Instruction::kSplat: {
            uint8_t count = read<uint8_t>(ip);
            Pointer target = read<Pointer>(ip);
            Register src = read<Register>(ip);
            return String::printf("splat%d $%d -> @%d", count, src.fIndex, target.fAddress);
        }
            DISASSEMBLE_UNARY(kSqrt, "sqrt")
            DISASSEMBLE_VECTOR_UNARY(kStore, "store")
        case Instruction::kStoreDirect: {
            Pointer target = read<Pointer>(ip);
            Register src = read<Register>(ip);
            return String::printf("store $%d -> @%d", src.fIndex, target.fAddress);
        }
        case Instruction::kStoreDirectN: {
            uint8_t count = read<uint8_t>(ip);
            Pointer target = read<Pointer>(ip);
            Register src = read<Register>(ip);
            return String::printf("store%d $%d -> @%d", count, src.fIndex, target.fAddress);
        }
            DISASSEMBLE_VECTOR_UNARY(kStoreParameter, "storeParameter")
        case Instruction::kStoreParameterDirect: {
            Pointer target = read<Pointer>(ip);
            Register src = read<Register>(ip);
            return String::printf("storeParameterDirect $%d -> &%d", src.fIndex, target.fAddress);
        }
        case Instruction::kStoreParameterDirectN: {
            uint8_t count = read<uint8_t>(ip);
            Pointer target = read<Pointer>(ip);
            Register src = read<Register>(ip);
            return String::printf("storeParameterDirect%d $%d -> &%d", count, src.fIndex,
                                  target.fAddress);
        }
            DISASSEMBLE_VECTOR_UNARY(kStoreStack, "storeStack")
        case Instruction::kStoreStackDirect: {
            Pointer target = read<Pointer>(ip);
            Register src = read<Register>(ip);
            return String::printf("storeStackDirect $%d -> @%d", src.fIndex, target.fAddress);
        }
        case Instruction::kStoreStackDirectN: {
            uint8_t count = read<uint8_t>(ip);
            Pointer target = read<Pointer>(ip);
            Register src = read<Register>(ip);
            return String::printf("storeStackDirect%d $%d -> @%d", count, src.fIndex,
                                  target.fAddress);
        }
            DISASSEMBLE_UNARY(kTan, "tan")
            DISASSEMBLE_UNARY(kUnsignedToFloat, "unsignedToFloat")
        case Instruction::kWriteExternal: {
            uint8_t index = read<uint8_t>(ip);
            uint8_t count = read<uint8_t>(ip);
            Register src = read<Register>(ip);
            return String::printf("writeExternal $%d, %d -> %d", src.fIndex, count, index);
        }
        default:
            SkASSERT(false);
            return String::printf("unsupported: %d", (int)inst);
    }
}

String ByteCode::disassembleFunction(const ByteCodeFunction* f) const {
    String result;
    const uint8_t* ip = f->fCode.data();
    const uint8_t* codeEnd = f->fCode.data() + f->fCode.size();
    while (ip < codeEnd) {
        result.append(this->disassemble(&ip));
        result.append("\n");
    }
    return result;
}

}  // namespace SkSL
