/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STANDALONE

#include "include/core/SkPoint3.h"
#include "include/private/SkVx.h"
#include "src/core/SkUtils.h"   // sk_unaligned_load
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLByteCodeGenerator.h"
#include "src/sksl/SkSLExternalValue.h"

#include <vector>

namespace SkSL {

#if defined(SK_ENABLE_SKSL_INTERPRETER)

namespace Interpreter {

constexpr int VecWidth = ByteCode::kVecWidth;

using F32 = skvx::Vec<VecWidth, float>;
using I32 = skvx::Vec<VecWidth, int32_t>;
using U32 = skvx::Vec<VecWidth, uint32_t>;

#define READ8() (*(ip++))
#define READ16() (ip += 2, sk_unaligned_load<uint16_t>(ip - 2))
#define READ32() (ip += 4, sk_unaligned_load<uint32_t>(ip - 4))
#define READ_INST() (ip += sizeof(instruction), \
                     sk_unaligned_load<instruction>(ip - sizeof(instruction)))

#define VECTOR_DISASSEMBLE(op, text)                                \
    case ByteCodeInstruction::op: printf(text); ++ip; break;        \
    case ByteCodeInstruction::op##2: printf(text "2"); ++ip; break; \
    case ByteCodeInstruction::op##3: printf(text "3"); ++ip; break; \
    case ByteCodeInstruction::op##4: printf(text "4"); ++ip; break;

#define VECTOR_DISASSEMBLE_NO_COUNT(op, text)                 \
    case ByteCodeInstruction::op: printf(text); break;        \
    case ByteCodeInstruction::op##2: printf(text "2"); break; \
    case ByteCodeInstruction::op##3: printf(text "3"); break; \
    case ByteCodeInstruction::op##4: printf(text "4"); break;

#define VECTOR_MATRIX_DISASSEMBLE(op, text) \
    VECTOR_DISASSEMBLE(op, text)            \
    case ByteCodeInstruction::op##N: printf(text "N %d", READ8()); break;

#define VECTOR_MATRIX_DISASSEMBLE_NO_COUNT(op, text) \
    VECTOR_DISASSEMBLE_NO_COUNT(op, text)            \
    case ByteCodeInstruction::op##N: printf(text "N %d", READ8()); break;

static const uint8_t* disassemble_instruction(const uint8_t* ip) {
    switch ((ByteCodeInstruction) (intptr_t) READ_INST()) {
        VECTOR_MATRIX_DISASSEMBLE(kAddF, "addf")
        VECTOR_DISASSEMBLE(kAddI, "addi")
        case ByteCodeInstruction::kAndB: printf("andb"); break;
        case ByteCodeInstruction::kBranch: printf("branch %d", READ16()); break;
        case ByteCodeInstruction::kCall: printf("call %d", READ8()); break;
        case ByteCodeInstruction::kCallExternal: {
            int argumentCount = READ8();
            int returnCount = READ8();
            int externalValue = READ8();
            printf("callexternal %d, %d, %d", argumentCount, returnCount, externalValue);
            break;
        }
        case ByteCodeInstruction::kClampIndex: printf("clampindex %d", READ8()); break;
        VECTOR_DISASSEMBLE(kCompareIEQ, "compareieq")
        VECTOR_DISASSEMBLE(kCompareINEQ, "compareineq")
        VECTOR_MATRIX_DISASSEMBLE(kCompareFEQ, "comparefeq")
        VECTOR_MATRIX_DISASSEMBLE(kCompareFNEQ, "comparefneq")
        VECTOR_DISASSEMBLE(kCompareFGT, "comparefgt")
        VECTOR_DISASSEMBLE(kCompareFGTEQ, "comparefgteq")
        VECTOR_DISASSEMBLE(kCompareFLT, "compareflt")
        VECTOR_DISASSEMBLE(kCompareFLTEQ, "compareflteq")
        VECTOR_DISASSEMBLE(kCompareSGT, "comparesgt")
        VECTOR_DISASSEMBLE(kCompareSGTEQ, "comparesgteq")
        VECTOR_DISASSEMBLE(kCompareSLT, "compareslt")
        VECTOR_DISASSEMBLE(kCompareSLTEQ, "compareslteq")
        VECTOR_DISASSEMBLE(kCompareUGT, "compareugt")
        VECTOR_DISASSEMBLE(kCompareUGTEQ, "compareugteq")
        VECTOR_DISASSEMBLE(kCompareULT, "compareult")
        VECTOR_DISASSEMBLE(kCompareULTEQ, "compareulteq")
        VECTOR_DISASSEMBLE_NO_COUNT(kConvertFtoI, "convertftoi")
        VECTOR_DISASSEMBLE_NO_COUNT(kConvertStoF, "convertstof")
        VECTOR_DISASSEMBLE_NO_COUNT(kConvertUtoF, "convertutof")
        VECTOR_DISASSEMBLE(kCos, "cos")
        VECTOR_MATRIX_DISASSEMBLE(kDivideF, "dividef")
        VECTOR_DISASSEMBLE(kDivideS, "divideS")
        VECTOR_DISASSEMBLE(kDivideU, "divideu")
        VECTOR_MATRIX_DISASSEMBLE(kDup, "dup")
        case ByteCodeInstruction::kInverse2x2: printf("inverse2x2"); break;
        case ByteCodeInstruction::kInverse3x3: printf("inverse3x3"); break;
        case ByteCodeInstruction::kInverse4x4: printf("inverse4x4"); break;
        case ByteCodeInstruction::kLoad: printf("load %d", READ16() >> 8); break;
        case ByteCodeInstruction::kLoad2: printf("load2 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kLoad3: printf("load3 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kLoad4: printf("load4 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kLoadGlobal: printf("loadglobal %d", READ16() >> 8); break;
        case ByteCodeInstruction::kLoadGlobal2: printf("loadglobal2 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kLoadGlobal3: printf("loadglobal3 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kLoadGlobal4: printf("loadglobal4 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kLoadSwizzle: {
            int target = READ8();
            int count = READ8();
            printf("loadswizzle %d %d", target, count);
            for (int i = 0; i < count; ++i) {
                printf(", %d", READ8());
            }
            break;
        }
        case ByteCodeInstruction::kLoadSwizzleGlobal: {
            int target = READ8();
            int count = READ8();
            printf("loadswizzleglobal %d %d", target, count);
            for (int i = 0; i < count; ++i) {
                printf(", %d", READ8());
            }
            break;
        }
        case ByteCodeInstruction::kLoadExtended: printf("loadextended %d", READ8()); break;
        case ByteCodeInstruction::kLoadExtendedGlobal: printf("loadextendedglobal %d", READ8());
            break;
        case ByteCodeInstruction::kMatrixToMatrix: {
            int srcCols = READ8();
            int srcRows = READ8();
            int dstCols = READ8();
            int dstRows = READ8();
            printf("matrixtomatrix %dx%d %dx%d", srcCols, srcRows, dstCols, dstRows);
            break;
        }
        case ByteCodeInstruction::kMatrixMultiply: {
            int lCols = READ8();
            int lRows = READ8();
            int rCols = READ8();
            printf("matrixmultiply %dx%d %dx%d", lCols, lRows, rCols, lCols);
            break;
        }
        VECTOR_MATRIX_DISASSEMBLE(kMultiplyF, "multiplyf")
        VECTOR_DISASSEMBLE(kMultiplyI, "multiplyi")
        VECTOR_MATRIX_DISASSEMBLE_NO_COUNT(kNegateF, "negatef")
        VECTOR_DISASSEMBLE_NO_COUNT(kNegateI, "negatei")
        case ByteCodeInstruction::kNotB: printf("notb"); break;
        case ByteCodeInstruction::kOrB: printf("orb"); break;
        VECTOR_MATRIX_DISASSEMBLE_NO_COUNT(kPop, "pop")
        case ByteCodeInstruction::kPushImmediate: {
            uint32_t v = READ32();
            union { uint32_t u; float f; } pun = { v };
            printf("pushimmediate %s", (to_string(v) + "(" + to_string(pun.f) + ")").c_str());
            break;
        }
        case ByteCodeInstruction::kReadExternal: printf("readexternal %d", READ16() >> 8); break;
        case ByteCodeInstruction::kReadExternal2: printf("readexternal2 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kReadExternal3: printf("readexternal3 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kReadExternal4: printf("readexternal4 %d", READ16() >> 8); break;
        VECTOR_DISASSEMBLE(kRemainderF, "remainderf")
        VECTOR_DISASSEMBLE(kRemainderS, "remainders")
        VECTOR_DISASSEMBLE(kRemainderU, "remainderu")
        case ByteCodeInstruction::kReserve: printf("reserve %d", READ8()); break;
        case ByteCodeInstruction::kReturn: printf("return %d", READ8()); break;
        case ByteCodeInstruction::kScalarToMatrix: {
            int cols = READ8();
            int rows = READ8();
            printf("scalartomatrix %dx%d", cols, rows);
            break;
        }
        VECTOR_DISASSEMBLE(kSin, "sin")
        VECTOR_DISASSEMBLE_NO_COUNT(kSqrt, "sqrt")
        case ByteCodeInstruction::kStore: printf("store %d", READ8()); break;
        case ByteCodeInstruction::kStore2: printf("store2 %d", READ8()); break;
        case ByteCodeInstruction::kStore3: printf("store3 %d", READ8()); break;
        case ByteCodeInstruction::kStore4: printf("store4 %d", READ8()); break;
        case ByteCodeInstruction::kStoreGlobal: printf("storeglobal %d", READ8()); break;
        case ByteCodeInstruction::kStoreGlobal2: printf("storeglobal2 %d", READ8()); break;
        case ByteCodeInstruction::kStoreGlobal3: printf("storeglobal3 %d", READ8()); break;
        case ByteCodeInstruction::kStoreGlobal4: printf("storeglobal4 %d", READ8()); break;
        case ByteCodeInstruction::kStoreSwizzle: {
            int target = READ8();
            int count = READ8();
            printf("storeswizzle %d %d", target, count);
            for (int i = 0; i < count; ++i) {
                printf(", %d", READ8());
            }
            break;
        }
        case ByteCodeInstruction::kStoreSwizzleGlobal: {
            int target = READ8();
            int count = READ8();
            printf("storeswizzleglobal %d %d", target, count);
            for (int i = 0; i < count; ++i) {
                printf(", %d", READ8());
            }
            break;
        }
        case ByteCodeInstruction::kStoreSwizzleIndirect: {
            int count = READ8();
            printf("storeswizzleindirect %d", count);
            for (int i = 0; i < count; ++i) {
                printf(", %d", READ8());
            }
            break;
        }
        case ByteCodeInstruction::kStoreSwizzleIndirectGlobal: {
            int count = READ8();
            printf("storeswizzleindirectglobal %d", count);
            for (int i = 0; i < count; ++i) {
                printf(", %d", READ8());
            }
            break;
        }
        case ByteCodeInstruction::kStoreExtended: printf("storeextended %d", READ8()); break;
        case ByteCodeInstruction::kStoreExtendedGlobal: printf("storeextendedglobal %d", READ8());
            break;
        VECTOR_MATRIX_DISASSEMBLE(kSubtractF, "subtractf")
        VECTOR_DISASSEMBLE(kSubtractI, "subtracti")
        case ByteCodeInstruction::kSwizzle: {
            printf("swizzle %d, ", READ8());
            int count = READ8();
            printf("%d", count);
            for (int i = 0; i < count; ++i) {
                printf(", %d", READ8());
            }
            break;
        }
        VECTOR_DISASSEMBLE(kTan, "tan")
        case ByteCodeInstruction::kWriteExternal: printf("writeexternal %d", READ16() >> 8); break;
        case ByteCodeInstruction::kWriteExternal2: printf("writeexternal2 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kWriteExternal3: printf("writeexternal3 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kWriteExternal4: printf("writeexternal4 %d", READ16() >> 8); break;
        case ByteCodeInstruction::kXorB: printf("xorb"); break;
        case ByteCodeInstruction::kMaskPush: printf("maskpush"); break;
        case ByteCodeInstruction::kMaskPop: printf("maskpop"); break;
        case ByteCodeInstruction::kMaskNegate: printf("masknegate"); break;
        case ByteCodeInstruction::kMaskBlend: printf("maskblend %d", READ8()); break;
        case ByteCodeInstruction::kBranchIfAllFalse:
            printf("branchifallfalse %d", READ16());
            break;
        case ByteCodeInstruction::kLoopBegin: printf("loopbegin"); break;
        case ByteCodeInstruction::kLoopNext: printf("loopnext"); break;
        case ByteCodeInstruction::kLoopMask: printf("loopmask"); break;
        case ByteCodeInstruction::kLoopEnd: printf("loopend"); break;
        case ByteCodeInstruction::kLoopContinue: printf("loopcontinue"); break;
        case ByteCodeInstruction::kLoopBreak: printf("loopbreak"); break;
        default:
            ip -= sizeof(instruction);
            printf("unknown(%d)\n", (int) (intptr_t) READ_INST());
            SkASSERT(false);
    }
    return ip;
}

#ifdef SKSLC_THREADED_CODE
    #define LABEL(name) name:
    #ifdef TRACE
        #define NEXT() goto next
    #else
        #define NEXT() goto *READ_INST()
    #endif
#else
    #define LABEL(name) case ByteCodeInstruction::name:
    #define NEXT() continue
#endif

#define VECTOR_BINARY_OP(base, field, op)             \
    LABEL(base ## 4)                                  \
        sp[-4] = sp[-4].field op sp[0].field;         \
        POP();                                        \
        /* fall through */                            \
    LABEL(base ## 3) {                                \
        sp[-ip[0]] = sp[-ip[0]].field op sp[0].field; \
        POP();                                        \
    }   /* fall through */                            \
    LABEL(base ## 2) {                                \
        sp[-ip[0]] = sp[-ip[0]].field op sp[0].field; \
        POP();                                        \
    }   /* fall through */                            \
    LABEL(base) {                                     \
        sp[-ip[0]] = sp[-ip[0]].field op sp[0].field; \
        POP();                                        \
        ++ip;                                         \
        NEXT();                                       \
    }

#define VECTOR_MATRIX_BINARY_OP(base, field, op)          \
    VECTOR_BINARY_OP(base, field, op)                     \
    LABEL(base ## N) {                                    \
        int count = READ8();                              \
        for (int i = count; i > 0; --i) {                 \
            sp[-count] = sp[-count].field op sp[0].field; \
            POP();                                        \
        }                                                 \
        NEXT();                                           \
    }

#define VECTOR_BINARY_FN(base, field, fn)               \
    LABEL(base ## 4)                                    \
        sp[-4] = fn(sp[-4].field, sp[0].field);         \
        POP();                                          \
        /* fall through */                              \
    LABEL(base ## 3) {                                  \
        sp[-ip[0]] = fn(sp[-ip[0]].field, sp[0].field); \
        POP();                                          \
    }   /* fall through */                              \
    LABEL(base ## 2) {                                  \
        sp[-ip[0]] = fn(sp[-ip[0]].field, sp[0].field); \
        POP();                                          \
    }   /* fall through */                              \
    LABEL(base) {                                       \
        sp[-ip[0]] = fn(sp[-ip[0]].field, sp[0].field); \
        POP();                                          \
        ++ip;                                           \
        NEXT();                                         \
    }

#define VECTOR_UNARY_FN(base, fn, field)         \
    LABEL(base ## 4)  sp[-3] = fn(sp[-3].field); \
    LABEL(base ## 3)  sp[-2] = fn(sp[-2].field); \
    LABEL(base ## 2)  sp[-1] = fn(sp[-1].field); \
    LABEL(base)       sp[ 0] = fn(sp[ 0].field); \
                      NEXT();

#define VECTOR_UNARY_FN_VEC(base, fn)                     \
    LABEL(base ## 4)                                      \
    LABEL(base ## 3)                                      \
    LABEL(base ## 2)                                      \
    LABEL(base) {                                         \
        int count = READ8();                              \
        float* v = (float*)sp - count + 1;                \
        for (int i = VecWidth * count; i > 0; --i, ++v) { \
            *v = fn(*v);                                  \
        }                                                 \
        NEXT();                                           \
    }

#define VECTOR_LABELS(base) \
    &&base ## 4,            \
    &&base ## 3,            \
    &&base ## 2,            \
    &&base

#define VECTOR_MATRIX_LABELS(base) \
    VECTOR_LABELS(base),           \
    &&base ## N

// If you trip this assert, it means that the order of the opcodes listed in ByteCodeInstruction
// does not match the order of the opcodes listed in the 'labels' array in innerRun().
#define CHECK_LABEL(name) \
    SkASSERT(labels[(int) ByteCodeInstruction::name] == &&name)

#define CHECK_VECTOR_LABELS(name) \
    CHECK_LABEL(name ## 4);       \
    CHECK_LABEL(name ## 3);       \
    CHECK_LABEL(name ## 2);       \
    CHECK_LABEL(name)

#define CHECK_VECTOR_MATRIX_LABELS(name) \
    CHECK_VECTOR_LABELS(name);           \
    CHECK_LABEL(name ## N)

union VValue {
    VValue() {}

    VValue(F32 f)
        : fFloat(f) {
    }

    VValue(I32 s)
        : fSigned(s) {
    }

    VValue(U32 u)
        : fUnsigned(u) {
    }

    F32 fFloat;
    I32 fSigned;
    U32 fUnsigned;
};

struct StackFrame {
    const uint8_t* fCode;
    const uint8_t* fIP;
    VValue* fStack;
    int fParameterCount;
};

// TODO: trunc on integers?
template <typename T>
static T vec_mod(T a, T b) {
    return a - skvx::trunc(a / b) * b;
}

#define spf(index)  sp[index].fFloat

static void call_external(const ByteCode* byteCode, const uint8_t*& ip, VValue*& sp,
                          int baseIndex, I32 mask) {
    int argumentCount = READ8();
    int returnCount = READ8();
    int target = READ8();
    ExternalValue* v = byteCode->fExternalValues[target];
    sp -= argumentCount - 1;

    float tmpArgs[4];
    float tmpReturn[4];
    SkASSERT(argumentCount <= (int)SK_ARRAY_COUNT(tmpArgs));
    SkASSERT(returnCount <= (int)SK_ARRAY_COUNT(tmpReturn));

    for (int i = 0; i < VecWidth; ++i) {
        if (mask[i]) {
            for (int j = 0; j < argumentCount; ++j) {
                tmpArgs[j] = sp[j].fFloat[i];
            }
            v->call(baseIndex + i, tmpArgs, tmpReturn);
            for (int j = 0; j < returnCount; ++j) {
                sp[j].fFloat[i] = tmpReturn[j];
            }
        }
    }
    sp += returnCount - 1;
}

static void inverse2x2(VValue* sp) {
    F32 a = sp[-3].fFloat,
        b = sp[-2].fFloat,
        c = sp[-1].fFloat,
        d = sp[ 0].fFloat;
    F32 idet = F32(1) / (a*d - b*c);
    sp[-3].fFloat = d * idet;
    sp[-2].fFloat = -b * idet;
    sp[-1].fFloat = -c * idet;
    sp[ 0].fFloat = a * idet;
}

static void inverse3x3(VValue* sp) {
    F32 a11 = sp[-8].fFloat, a12 = sp[-5].fFloat, a13 = sp[-2].fFloat,
        a21 = sp[-7].fFloat, a22 = sp[-4].fFloat, a23 = sp[-1].fFloat,
        a31 = sp[-6].fFloat, a32 = sp[-3].fFloat, a33 = sp[ 0].fFloat;
    F32 idet = F32(1) / (a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 -
                         a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31);
    sp[-8].fFloat = (a22 * a33 - a23 * a32) * idet;
    sp[-7].fFloat = (a23 * a31 - a21 * a33) * idet;
    sp[-6].fFloat = (a21 * a32 - a22 * a31) * idet;
    sp[-5].fFloat = (a13 * a32 - a12 * a33) * idet;
    sp[-4].fFloat = (a11 * a33 - a13 * a31) * idet;
    sp[-3].fFloat = (a12 * a31 - a11 * a32) * idet;
    sp[-2].fFloat = (a12 * a23 - a13 * a22) * idet;
    sp[-1].fFloat = (a13 * a21 - a11 * a23) * idet;
    sp[ 0].fFloat = (a11 * a22 - a12 * a21) * idet;
}

static void inverse4x4(VValue* sp) {
    F32 a00 = spf(-15), a10 = spf(-11), a20 = spf( -7), a30 = spf( -3),
        a01 = spf(-14), a11 = spf(-10), a21 = spf( -6), a31 = spf( -2),
        a02 = spf(-13), a12 = spf( -9), a22 = spf( -5), a32 = spf( -1),
        a03 = spf(-12), a13 = spf( -8), a23 = spf( -4), a33 = spf(  0);

    F32 b00 = a00 * a11 - a01 * a10,
        b01 = a00 * a12 - a02 * a10,
        b02 = a00 * a13 - a03 * a10,
        b03 = a01 * a12 - a02 * a11,
        b04 = a01 * a13 - a03 * a11,
        b05 = a02 * a13 - a03 * a12,
        b06 = a20 * a31 - a21 * a30,
        b07 = a20 * a32 - a22 * a30,
        b08 = a20 * a33 - a23 * a30,
        b09 = a21 * a32 - a22 * a31,
        b10 = a21 * a33 - a23 * a31,
        b11 = a22 * a33 - a23 * a32;

    F32 idet = F32(1) /
               (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

    b00 *= idet;
    b01 *= idet;
    b02 *= idet;
    b03 *= idet;
    b04 *= idet;
    b05 *= idet;
    b06 *= idet;
    b07 *= idet;
    b08 *= idet;
    b09 *= idet;
    b10 *= idet;
    b11 *= idet;

    spf(-15) = a11 * b11 - a12 * b10 + a13 * b09;
    spf(-14) = a02 * b10 - a01 * b11 - a03 * b09;
    spf(-13) = a31 * b05 - a32 * b04 + a33 * b03;
    spf(-12) = a22 * b04 - a21 * b05 - a23 * b03;
    spf(-11) = a12 * b08 - a10 * b11 - a13 * b07;
    spf(-10) = a00 * b11 - a02 * b08 + a03 * b07;
    spf( -9) = a32 * b02 - a30 * b05 - a33 * b01;
    spf( -8) = a20 * b05 - a22 * b02 + a23 * b01;
    spf( -7) = a10 * b10 - a11 * b08 + a13 * b06;
    spf( -6) = a01 * b08 - a00 * b10 - a03 * b06;
    spf( -5) = a30 * b04 - a31 * b02 + a33 * b00;
    spf( -4) = a21 * b02 - a20 * b04 - a23 * b00;
    spf( -3) = a11 * b07 - a10 * b09 - a12 * b06;
    spf( -2) = a00 * b09 - a01 * b07 + a02 * b06;
    spf( -1) = a31 * b01 - a30 * b03 - a32 * b00;
    spf(  0) = a20 * b03 - a21 * b01 + a22 * b00;
}

static bool innerRun(const ByteCode* byteCode, const ByteCodeFunction* f, VValue* stack,
                     float* outReturn[], VValue globals[], bool stripedOutput, int N,
                     int baseIndex) {
#ifdef SKSLC_THREADED_CODE
    static const void* labels[] = {
        // If you aren't familiar with it, the &&label syntax is the GCC / Clang "labels as values"
        // extension. If you add anything to this array, be sure to add the corresponding
        // CHECK_LABEL() or CHECK_*_LABELS() assert below.
        VECTOR_MATRIX_LABELS(kAddF),
        VECTOR_LABELS(kAddI),
        &&kAndB,
        &&kBranch,
        &&kCall,
        &&kCallExternal,
        &&kClampIndex,
        VECTOR_LABELS(kCompareIEQ),
        VECTOR_LABELS(kCompareINEQ),
        VECTOR_MATRIX_LABELS(kCompareFEQ),
        VECTOR_MATRIX_LABELS(kCompareFNEQ),
        VECTOR_LABELS(kCompareFGT),
        VECTOR_LABELS(kCompareFGTEQ),
        VECTOR_LABELS(kCompareFLT),
        VECTOR_LABELS(kCompareFLTEQ),
        VECTOR_LABELS(kCompareSGT),
        VECTOR_LABELS(kCompareSGTEQ),
        VECTOR_LABELS(kCompareSLT),
        VECTOR_LABELS(kCompareSLTEQ),
        VECTOR_LABELS(kCompareUGT),
        VECTOR_LABELS(kCompareUGTEQ),
        VECTOR_LABELS(kCompareULT),
        VECTOR_LABELS(kCompareULTEQ),
        VECTOR_LABELS(kConvertFtoI),
        VECTOR_LABELS(kConvertStoF),
        VECTOR_LABELS(kConvertUtoF),
        VECTOR_LABELS(kCos),
        VECTOR_MATRIX_LABELS(kDivideF),
        VECTOR_LABELS(kDivideS),
        VECTOR_LABELS(kDivideU),
        VECTOR_MATRIX_LABELS(kDup),
        &&kInverse2x2,
        &&kInverse3x3,
        &&kInverse4x4,
        VECTOR_LABELS(kLoad),
        VECTOR_LABELS(kLoadGlobal),
        &&kLoadSwizzle,
        &&kLoadSwizzleGlobal,
        &&kLoadExtended,
        &&kLoadExtendedGlobal,
        &&kMatrixToMatrix,
        &&kMatrixMultiply,
        VECTOR_MATRIX_LABELS(kNegateF),
        VECTOR_LABELS(kNegateI),
        VECTOR_MATRIX_LABELS(kMultiplyF),
        VECTOR_LABELS(kMultiplyI),
        &&kNotB,
        &&kOrB,
        VECTOR_MATRIX_LABELS(kPop),
        &&kPushImmediate,
        VECTOR_LABELS(kReadExternal),
        VECTOR_LABELS(kRemainderF),
        VECTOR_LABELS(kRemainderS),
        VECTOR_LABELS(kRemainderU),
        &&kReserve,
        &&kReturn,
        &&kScalarToMatrix,
        VECTOR_LABELS(kSin),
        VECTOR_LABELS(kSqrt),
        VECTOR_LABELS(kStore),
        VECTOR_LABELS(kStoreGlobal),
        &&kStoreExtended,
        &&kStoreExtendedGlobal,
        &&kStoreSwizzle,
        &&kStoreSwizzleGlobal,
        &&kStoreSwizzleIndirect,
        &&kStoreSwizzleIndirectGlobal,
        &&kSwizzle,
        VECTOR_MATRIX_LABELS(kSubtractF),
        VECTOR_LABELS(kSubtractI),
        VECTOR_LABELS(kTan),
        VECTOR_LABELS(kWriteExternal),
        &&kXorB,

        &&kMaskPush,
        &&kMaskPop,
        &&kMaskNegate,
        &&kMaskBlend,
        &&kBranchIfAllFalse,

        &&kLoopBegin,
        &&kLoopNext,
        &&kLoopMask,
        &&kLoopEnd,
        &&kLoopBreak,
        &&kLoopContinue,
    };
    // Verify that the order of the labels array matches the order of the ByteCodeInstruction enum.
    CHECK_VECTOR_MATRIX_LABELS(kAddF);
    CHECK_VECTOR_LABELS(kAddI);
    CHECK_LABEL(kAndB);
    CHECK_LABEL(kBranch);
    CHECK_LABEL(kCall);
    CHECK_LABEL(kCallExternal);
    CHECK_LABEL(kClampIndex);
    CHECK_VECTOR_LABELS(kCompareIEQ);
    CHECK_VECTOR_LABELS(kCompareINEQ);
    CHECK_VECTOR_MATRIX_LABELS(kCompareFEQ);
    CHECK_VECTOR_MATRIX_LABELS(kCompareFNEQ);
    CHECK_VECTOR_LABELS(kCompareFGT);
    CHECK_VECTOR_LABELS(kCompareFGTEQ);
    CHECK_VECTOR_LABELS(kCompareFLT);
    CHECK_VECTOR_LABELS(kCompareFLTEQ);
    CHECK_VECTOR_LABELS(kCompareSGT);
    CHECK_VECTOR_LABELS(kCompareSGTEQ);
    CHECK_VECTOR_LABELS(kCompareSLT);
    CHECK_VECTOR_LABELS(kCompareSLTEQ);
    CHECK_VECTOR_LABELS(kCompareUGT);
    CHECK_VECTOR_LABELS(kCompareUGTEQ);
    CHECK_VECTOR_LABELS(kCompareULT);
    CHECK_VECTOR_LABELS(kCompareULTEQ);
    CHECK_VECTOR_LABELS(kConvertFtoI);
    CHECK_VECTOR_LABELS(kConvertStoF);
    CHECK_VECTOR_LABELS(kConvertUtoF);
    CHECK_VECTOR_LABELS(kCos);
    CHECK_VECTOR_MATRIX_LABELS(kDivideF);
    CHECK_VECTOR_LABELS(kDivideS);
    CHECK_VECTOR_LABELS(kDivideU);
    CHECK_VECTOR_MATRIX_LABELS(kDup);
    CHECK_LABEL(kInverse2x2);
    CHECK_LABEL(kInverse3x3);
    CHECK_LABEL(kInverse4x4);
    CHECK_VECTOR_LABELS(kLoad);
    CHECK_VECTOR_LABELS(kLoadGlobal);
    CHECK_LABEL(kLoadSwizzle);
    CHECK_LABEL(kLoadSwizzleGlobal);
    CHECK_LABEL(kLoadExtended);
    CHECK_LABEL(kLoadExtendedGlobal);
    CHECK_LABEL(kMatrixToMatrix);
    CHECK_LABEL(kMatrixMultiply);
    CHECK_VECTOR_MATRIX_LABELS(kNegateF);
    CHECK_VECTOR_LABELS(kNegateI);
    CHECK_VECTOR_MATRIX_LABELS(kMultiplyF);
    CHECK_VECTOR_LABELS(kMultiplyI);
    CHECK_LABEL(kNotB);
    CHECK_LABEL(kOrB);
    CHECK_VECTOR_MATRIX_LABELS(kPop);
    CHECK_LABEL(kPushImmediate);
    CHECK_VECTOR_LABELS(kReadExternal);
    CHECK_VECTOR_LABELS(kRemainderF);
    CHECK_VECTOR_LABELS(kRemainderS);
    CHECK_VECTOR_LABELS(kRemainderU);
    CHECK_LABEL(kReserve);
    CHECK_LABEL(kReturn);
    CHECK_LABEL(kScalarToMatrix);
    CHECK_VECTOR_LABELS(kSin);
    CHECK_VECTOR_LABELS(kSqrt);
    CHECK_VECTOR_LABELS(kStore);
    CHECK_VECTOR_LABELS(kStoreGlobal);
    CHECK_LABEL(kStoreExtended);
    CHECK_LABEL(kStoreExtendedGlobal);
    CHECK_LABEL(kStoreSwizzle);
    CHECK_LABEL(kStoreSwizzleGlobal);
    CHECK_LABEL(kStoreSwizzleIndirect);
    CHECK_LABEL(kStoreSwizzleIndirectGlobal);
    CHECK_LABEL(kSwizzle);
    CHECK_VECTOR_MATRIX_LABELS(kSubtractF);
    CHECK_VECTOR_LABELS(kSubtractI);
    CHECK_VECTOR_LABELS(kTan);
    CHECK_VECTOR_LABELS(kWriteExternal);
    CHECK_LABEL(kXorB);
    CHECK_LABEL(kMaskPush);
    CHECK_LABEL(kMaskPop);
    CHECK_LABEL(kMaskNegate);
    CHECK_LABEL(kMaskBlend);
    CHECK_LABEL(kBranchIfAllFalse);
    CHECK_LABEL(kLoopBegin);
    CHECK_LABEL(kLoopNext);
    CHECK_LABEL(kLoopMask);
    CHECK_LABEL(kLoopEnd);
    CHECK_LABEL(kLoopBreak);
    CHECK_LABEL(kLoopContinue);
    f->fPreprocessOnce([f] { ((ByteCodeFunction*)f)->preprocess(labels); });
#endif

    // Needs to be the first N non-negative integers, at least as large as VecWidth
    static const Interpreter::I32 gLanes = {
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
    };

    VValue* sp = stack + f->fParameterCount + f->fLocalCount - 1;

    #define POP() (*(sp--))
    #define PUSH(v) (sp[1] = v, ++sp)

    const uint8_t* code = f->fCode.data();
    const uint8_t* ip = code;
    std::vector<StackFrame> frames;

    I32 condStack[16];  // Independent condition masks
    I32 maskStack[16];  // Combined masks (eg maskStack[0] & maskStack[1] & ...)
    I32 contStack[16];  // Continue flags for loops
    I32 loopStack[16];  // Loop execution masks
    condStack[0] = maskStack[0] = (gLanes < N);
    contStack[0] = I32( 0);
    loopStack[0] = I32(~0);
    I32* condPtr = condStack;
    I32* maskPtr = maskStack;
    I32* contPtr = contStack;
    I32* loopPtr = loopStack;

    if (f->fConditionCount + 1 > (int)SK_ARRAY_COUNT(condStack) ||
        f->fLoopCount + 1 > (int)SK_ARRAY_COUNT(loopStack)) {
        return false;
    }

    auto mask = [&]() { return *maskPtr & *loopPtr; };

#ifdef SKSLC_THREADED_CODE
    // If the "labels as values" extension is available, we implement this using threaded code.
    // Instead of opcodes, the code directly contains the addresses of the labels to jump to. Then
    // the code for each opcode simply grabs the address of the next opcode and uses a goto to jump
    // there.
    NEXT();
#else
    // Otherwise, we have to use a switch statement and a loop to execute the right label.
    for (;;) {
        #ifdef TRACE
            printf("at %3d ", (int) (ip - code));
            disassemble_instruction(ip);
            printf(" (stack: %d)\n", (int) (sp - stack) + 1);
        #endif
        switch ((ByteCodeInstruction) READ16()) {
#endif

    VECTOR_MATRIX_BINARY_OP(kAddF, fFloat, +)
    VECTOR_BINARY_OP(kAddI, fSigned, +)

    // Booleans are integer masks: 0/~0 for false/true. So bitwise ops do what we want:
    LABEL(kAndB)
        sp[-1] = sp[-1].fSigned & sp[0].fSigned;
        POP();
        NEXT();
    LABEL(kNotB)
        sp[0] = ~sp[0].fSigned;
        NEXT();
    LABEL(kOrB)
        sp[-1] = sp[-1].fSigned | sp[0].fSigned;
        POP();
        NEXT();
    LABEL(kXorB)
        sp[-1] = sp[-1].fSigned ^ sp[0].fSigned;
        POP();
        NEXT();

    LABEL(kBranch)
        ip = code + READ16();
        NEXT();

    LABEL(kCall) {
        // Precursor code reserved space for the return value, and pushed all parameters to
        // the stack. Update our bottom of stack to point at the first parameter, and our
        // sp to point past those parameters (plus space for locals).
        int target = READ8();
        const ByteCodeFunction* fun = byteCode->fFunctions[target].get();
#ifdef SKSLC_THREADED_CODE
        fun->fPreprocessOnce([fun] { ((ByteCodeFunction*)fun)->preprocess(labels); });
#endif
        if (skvx::any(mask())) {
            frames.push_back({ code, ip, stack, fun->fParameterCount });
            ip = code = fun->fCode.data();
            stack = sp - fun->fParameterCount + 1;
            sp = stack + fun->fParameterCount + fun->fLocalCount - 1;
        }
        NEXT();
    }

    LABEL(kCallExternal) {
        call_external(byteCode, ip, sp, baseIndex, mask());
        NEXT();
    }

    LABEL(kClampIndex) {
        int length = READ8();
        if (skvx::any(mask() & ((sp[0].fSigned < 0) | (sp[0].fSigned >= length)))) {
            return false;
        }
        NEXT();
    }

    VECTOR_BINARY_OP(kCompareIEQ, fSigned, ==)
    VECTOR_MATRIX_BINARY_OP(kCompareFEQ, fFloat, ==)
    VECTOR_BINARY_OP(kCompareINEQ, fSigned, !=)
    VECTOR_MATRIX_BINARY_OP(kCompareFNEQ, fFloat, !=)
    VECTOR_BINARY_OP(kCompareSGT, fSigned, >)
    VECTOR_BINARY_OP(kCompareUGT, fUnsigned, >)
    VECTOR_BINARY_OP(kCompareFGT, fFloat, >)
    VECTOR_BINARY_OP(kCompareSGTEQ, fSigned, >=)
    VECTOR_BINARY_OP(kCompareUGTEQ, fUnsigned, >=)
    VECTOR_BINARY_OP(kCompareFGTEQ, fFloat, >=)
    VECTOR_BINARY_OP(kCompareSLT, fSigned, <)
    VECTOR_BINARY_OP(kCompareULT, fUnsigned, <)
    VECTOR_BINARY_OP(kCompareFLT, fFloat, <)
    VECTOR_BINARY_OP(kCompareSLTEQ, fSigned, <=)
    VECTOR_BINARY_OP(kCompareULTEQ, fUnsigned, <=)
    VECTOR_BINARY_OP(kCompareFLTEQ, fFloat, <=)

    LABEL(kConvertFtoI4) sp[-3] = skvx::cast<int>(sp[-3].fFloat);
    LABEL(kConvertFtoI3) sp[-2] = skvx::cast<int>(sp[-2].fFloat);
    LABEL(kConvertFtoI2) sp[-1] = skvx::cast<int>(sp[-1].fFloat);
    LABEL(kConvertFtoI)  sp[ 0] = skvx::cast<int>(sp[ 0].fFloat);
                         NEXT();

    LABEL(kConvertStoF4) sp[-3] = skvx::cast<float>(sp[-3].fSigned);
    LABEL(kConvertStoF3) sp[-2] = skvx::cast<float>(sp[-2].fSigned);
    LABEL(kConvertStoF2) sp[-1] = skvx::cast<float>(sp[-1].fSigned);
    LABEL(kConvertStoF)  sp[ 0] = skvx::cast<float>(sp[ 0].fSigned);
                         NEXT();

    LABEL(kConvertUtoF4) sp[-3] = skvx::cast<float>(sp[-3].fUnsigned);
    LABEL(kConvertUtoF3) sp[-2] = skvx::cast<float>(sp[-2].fUnsigned);
    LABEL(kConvertUtoF2) sp[-1] = skvx::cast<float>(sp[-1].fUnsigned);
    LABEL(kConvertUtoF)  sp[ 0] = skvx::cast<float>(sp[ 0].fUnsigned);
                         NEXT();

    VECTOR_UNARY_FN_VEC(kCos, cosf)

    VECTOR_BINARY_OP(kDivideS, fSigned, /)
    VECTOR_BINARY_OP(kDivideU, fUnsigned, /)
    VECTOR_MATRIX_BINARY_OP(kDivideF, fFloat, /)

    LABEL(kDup4) PUSH(sp[1 - ip[0]]);
    LABEL(kDup3) PUSH(sp[1 - ip[0]]);
    LABEL(kDup2) PUSH(sp[1 - ip[0]]);
    LABEL(kDup)  PUSH(sp[1 - ip[0]]);
                 ++ip;
                 NEXT();

    LABEL(kDupN) {
        int count = READ8();
        memcpy(sp + 1, sp - count + 1, count * sizeof(VValue));
        sp += count;
        NEXT();
    }

    LABEL(kInverse2x2) {
        inverse2x2(sp);
        NEXT();
    }
    LABEL(kInverse3x3) {
        inverse3x3(sp);
        NEXT();
    }
    LABEL(kInverse4x4) {
        inverse4x4(sp);
        NEXT();
    }

    LABEL(kLoad4) sp[4] = stack[ip[1] + 3];
    LABEL(kLoad3) sp[3] = stack[ip[1] + 2];
    LABEL(kLoad2) sp[2] = stack[ip[1] + 1];
    LABEL(kLoad)  sp[1] = stack[ip[1] + 0];
                  sp += ip[0];
                  ip += 2;
                  NEXT();

    LABEL(kLoadGlobal4) sp[4] = globals[ip[1] + 3];
    LABEL(kLoadGlobal3) sp[3] = globals[ip[1] + 2];
    LABEL(kLoadGlobal2) sp[2] = globals[ip[1] + 1];
    LABEL(kLoadGlobal)  sp[1] = globals[ip[1] + 0];
                        sp += ip[0];
                        ip += 2;
                        NEXT();

    LABEL(kLoadExtended) {
        int count = READ8();
        I32 src = POP().fSigned;
        I32 m = mask();
        for (int i = 0; i < count; ++i) {
            for (int j = 0; j < VecWidth; ++j) {
                if (m[j]) {
                    sp[i + 1].fSigned[j] = stack[src[j] + i].fSigned[j];
                }
            }
        }
        sp += count;
        NEXT();
    }

    LABEL(kLoadExtendedGlobal) {
        int count = READ8();
        I32 src = POP().fSigned;
        I32 m = mask();
        for (int i = 0; i < count; ++i) {
            for (int j = 0; j < VecWidth; ++j) {
                if (m[j]) {
                    sp[i + 1].fSigned[j] = globals[src[j] + i].fSigned[j];
                }
            }
        }
        sp += count;
        NEXT();
    }

    LABEL(kLoadSwizzle) {
        int src = READ8();
        int count = READ8();
        for (int i = 0; i < count; ++i) {
            PUSH(stack[src + *(ip + i)]);
        }
        ip += count;
        NEXT();
    }

    LABEL(kLoadSwizzleGlobal) {
        int src = READ8();
        int count = READ8();
        for (int i = 0; i < count; ++i) {
            PUSH(globals[src + *(ip + i)]);
        }
        ip += count;
        NEXT();
    }

    LABEL(kMatrixToMatrix) {
        int srcCols = READ8();
        int srcRows = READ8();
        int dstCols = READ8();
        int dstRows = READ8();
        SkASSERT(srcCols >= 2 && srcCols <= 4);
        SkASSERT(srcRows >= 2 && srcRows <= 4);
        SkASSERT(dstCols >= 2 && dstCols <= 4);
        SkASSERT(dstRows >= 2 && dstRows <= 4);
        F32 tmp[16];
        memset(tmp, 0, sizeof(tmp));
        tmp[0] = tmp[5] = tmp[10] = tmp[15] = F32(1.0f);
        for (int c = srcCols - 1; c >= 0; --c) {
            for (int r = srcRows - 1; r >= 0; --r) {
                tmp[c*4 + r] = POP().fFloat;
            }
        }
        for (int c = 0; c < dstCols; ++c) {
            for (int r = 0; r < dstRows; ++r) {
                PUSH(tmp[c*4 + r]);
            }
        }
        NEXT();
    }

    LABEL(kMatrixMultiply) {
        int lCols = READ8();
        int lRows = READ8();
        int rCols = READ8();
        int rRows = lCols;
        F32 tmp[16] = { 0.0f };
        F32* B = &(sp - (rCols * rRows) + 1)->fFloat;
        F32* A = B - (lCols * lRows);
        for (int c = 0; c < rCols; ++c) {
            for (int r = 0; r < lRows; ++r) {
                for (int j = 0; j < lCols; ++j) {
                    tmp[c*lRows + r] += A[j*lRows + r] * B[c*rRows + j];
                }
            }
        }
        sp -= (lCols * lRows) + (rCols * rRows);
        memcpy(sp + 1, tmp, rCols * lRows * sizeof(VValue));
        sp += (rCols * lRows);
        NEXT();
    }

    VECTOR_BINARY_OP(kMultiplyI, fSigned, *)
    VECTOR_MATRIX_BINARY_OP(kMultiplyF, fFloat, *)

    LABEL(kNegateF4) sp[-3] = -sp[-3].fFloat;
    LABEL(kNegateF3) sp[-2] = -sp[-2].fFloat;
    LABEL(kNegateF2) sp[-1] = -sp[-1].fFloat;
    LABEL(kNegateF)  sp[ 0] = -sp[ 0].fFloat;
                     NEXT();

    LABEL(kNegateFN) {
        int count = READ8();
        for (int i = count - 1; i >= 0; --i) {
            sp[-i] = -sp[-i].fFloat;
        }
        NEXT();
    }

    LABEL(kNegateI4) sp[-3] = -sp[-3].fSigned;
    LABEL(kNegateI3) sp[-2] = -sp[-2].fSigned;
    LABEL(kNegateI2) sp[-1] = -sp[-1].fSigned;
    LABEL(kNegateI)  sp[ 0] = -sp[ 0].fSigned;
                     NEXT();

    LABEL(kPop4) POP();
    LABEL(kPop3) POP();
    LABEL(kPop2) POP();
    LABEL(kPop)  POP();
                 NEXT();

    LABEL(kPopN)
        sp -= READ8();
        NEXT();

    LABEL(kPushImmediate)
        PUSH(U32(READ32()));
        NEXT();

    LABEL(kReadExternal)
    LABEL(kReadExternal2)
    LABEL(kReadExternal3)
    LABEL(kReadExternal4) {
        int count = READ8();
        int src = READ8();
        float tmp[4];
        I32 m = mask();
        for (int i = 0; i < VecWidth; ++i) {
            if (m[i]) {
                byteCode->fExternalValues[src]->read(baseIndex + i, tmp);
                for (int j = 0; j < count; ++j) {
                    sp[j + 1].fFloat[i] = tmp[j];
                }
            }
        }
        sp += count;
        NEXT();
    }

    VECTOR_BINARY_FN(kRemainderF, fFloat, vec_mod<F32>)
    VECTOR_BINARY_FN(kRemainderS, fSigned, vec_mod<I32>)
    VECTOR_BINARY_FN(kRemainderU, fUnsigned, vec_mod<U32>)

    LABEL(kReserve)
        sp += READ8();
        NEXT();

    LABEL(kReturn) {
        int count = READ8();
        if (frames.empty()) {
            if (outReturn) {
                VValue* src = sp - count + 1;
                if (stripedOutput) {
                    for (int i = 0; i < count; ++i) {
                        memcpy(outReturn[i], &src->fFloat, N * sizeof(float));
                        ++src;
                    }
                } else {
                    float* outPtr = outReturn[0];
                    for (int i = 0; i < count; ++i) {
                        for (int j = 0; j < N; ++j) {
                            outPtr[count * j] = src->fFloat[j];
                        }
                        ++outPtr;
                        ++src;
                    }
                }
            }
            return true;
        } else {
            // When we were called, the caller reserved stack space for their copy of our
            // return value, then 'stack' was positioned after that, where our parameters
            // were placed. Copy our return values to their reserved area.
            memcpy(stack - count, sp - count + 1, count * sizeof(VValue));

            // Now move the stack pointer to the end of the passed-in parameters. This odd
            // calling convention requires the caller to pop the arguments after calling,
            // but allows them to store any out-parameters back during that unwinding.
            // After that sequence finishes, the return value will be the top of the stack.
            const StackFrame& frame(frames.back());
            sp = stack + frame.fParameterCount - 1;
            stack = frame.fStack;
            code = frame.fCode;
            ip = frame.fIP;
            frames.pop_back();
            NEXT();
        }
    }

    LABEL(kScalarToMatrix) {
        int cols = READ8();
        int rows = READ8();
        VValue v = POP();
        for (int c = 0; c < cols; ++c) {
            for (int r = 0; r < rows; ++r) {
                PUSH(c == r ? v : F32(0.0f));
            }
        }
        NEXT();
    }

    VECTOR_UNARY_FN_VEC(kSin, sinf)
    VECTOR_UNARY_FN(kSqrt, skvx::sqrt, fFloat)

    LABEL(kStore4)
        stack[*ip+3] = skvx::if_then_else(mask(), POP().fFloat, stack[*ip+3].fFloat);
    LABEL(kStore3)
        stack[*ip+2] = skvx::if_then_else(mask(), POP().fFloat, stack[*ip+2].fFloat);
    LABEL(kStore2)
        stack[*ip+1] = skvx::if_then_else(mask(), POP().fFloat, stack[*ip+1].fFloat);
    LABEL(kStore)
        stack[*ip+0] = skvx::if_then_else(mask(), POP().fFloat, stack[*ip+0].fFloat);
        ++ip;
        NEXT();

    LABEL(kStoreGlobal4)
        globals[*ip+3] = skvx::if_then_else(mask(), POP().fFloat, globals[*ip+3].fFloat);
    LABEL(kStoreGlobal3)
        globals[*ip+2] = skvx::if_then_else(mask(), POP().fFloat, globals[*ip+2].fFloat);
    LABEL(kStoreGlobal2)
        globals[*ip+1] = skvx::if_then_else(mask(), POP().fFloat, globals[*ip+1].fFloat);
    LABEL(kStoreGlobal)
        globals[*ip+0] = skvx::if_then_else(mask(), POP().fFloat, globals[*ip+0].fFloat);
        ++ip;
        NEXT();

    LABEL(kStoreExtended) {
        int count = READ8();
        I32 target = POP().fSigned;
        VValue* src = sp - count + 1;
        I32 m = mask();
        for (int i = 0; i < count; ++i) {
            for (int j = 0; j < VecWidth; ++j) {
                if (m[j]) {
                    stack[target[j] + i].fSigned[j] = src[i].fSigned[j];
                }
            }
        }
        sp -= count;
        NEXT();
    }
    LABEL(kStoreExtendedGlobal) {
        int count = READ8();
        I32 target = POP().fSigned;
        VValue* src = sp - count + 1;
        I32 m = mask();
        for (int i = 0; i < count; ++i) {
            for (int j = 0; j < VecWidth; ++j) {
                if (m[j]) {
                    globals[target[j] + i].fSigned[j] = src[i].fSigned[j];
                }
            }
        }
        sp -= count;
        NEXT();
    }

    LABEL(kStoreSwizzle) {
        int target = READ8();
        int count = READ8();
        for (int i = count - 1; i >= 0; --i) {
            stack[target + *(ip + i)] = skvx::if_then_else(
                    mask(), POP().fFloat, stack[target + *(ip + i)].fFloat);
        }
        ip += count;
        NEXT();
    }

    LABEL(kStoreSwizzleGlobal) {
        int target = READ8();
        int count = READ8();
        for (int i = count - 1; i >= 0; --i) {
            globals[target + *(ip + i)] = skvx::if_then_else(
                    mask(), POP().fFloat, globals[target + *(ip + i)].fFloat);
        }
        ip += count;
        NEXT();
    }

    LABEL(kStoreSwizzleIndirect) {
        int count = READ8();
        I32 target = POP().fSigned;
        I32 m = mask();
        for (int i = count - 1; i >= 0; --i) {
            I32 v = POP().fSigned;
            for (int j = 0; j < VecWidth; ++j) {
                if (m[j]) {
                    stack[target[j] + *(ip + i)].fSigned[j] = v[j];
                }
            }
        }
        ip += count;
        NEXT();
    }

    LABEL(kStoreSwizzleIndirectGlobal) {
        int count = READ8();
        I32 target = POP().fSigned;
        I32 m = mask();
        for (int i = count - 1; i >= 0; --i) {
            I32 v = POP().fSigned;
            for (int j = 0; j < VecWidth; ++j) {
                if (m[j]) {
                    globals[target[j] + *(ip + i)].fSigned[j] = v[j];
                }
            }
        }
        ip += count;
        NEXT();
    }

    VECTOR_BINARY_OP(kSubtractI, fSigned, -)
    VECTOR_MATRIX_BINARY_OP(kSubtractF, fFloat, -)

    LABEL(kSwizzle) {
        VValue tmp[4];
        for (int i = READ8() - 1; i >= 0; --i) {
            tmp[i] = POP();
        }
        for (int i = READ8() - 1; i >= 0; --i) {
            PUSH(tmp[READ8()]);
        }
        NEXT();
    }

    VECTOR_UNARY_FN_VEC(kTan, tanf)

    LABEL(kWriteExternal4)
    LABEL(kWriteExternal3)
    LABEL(kWriteExternal2)
    LABEL(kWriteExternal) {
        int count = READ8();
        int target = READ8();
        float tmp[4];
        I32 m = mask();
        sp -= count;
        for (int i = 0; i < VecWidth; ++i) {
            if (m[i]) {
                for (int j = 0; j < count; ++j) {
                    tmp[j] = sp[j + 1].fFloat[i];
                }
                byteCode->fExternalValues[target]->write(baseIndex + i, tmp);
            }
        }
        NEXT();
    }

    LABEL(kMaskPush)
        condPtr[1] = POP().fSigned;
        maskPtr[1] = maskPtr[0] & condPtr[1];
        ++condPtr; ++maskPtr;
        NEXT();
    LABEL(kMaskPop)
        --condPtr; --maskPtr;
        NEXT();
    LABEL(kMaskNegate)
        maskPtr[0] = maskPtr[-1] & ~condPtr[0];
        NEXT();
    LABEL(kMaskBlend) {
        int count = READ8();
        I32 m = condPtr[0];
        --condPtr; --maskPtr;
        for (int i = 0; i < count; ++i) {
            sp[-count] = skvx::if_then_else(m, sp[-count].fFloat, sp[0].fFloat);
            --sp;
        }
        NEXT();
    }
    LABEL(kBranchIfAllFalse) {
        int target = READ16();
        if (!skvx::any(mask())) {
            ip = code + target;
        }
        NEXT();
    }

    LABEL(kLoopBegin)
        contPtr[1] = 0;
        loopPtr[1] = loopPtr[0];
        ++contPtr; ++loopPtr;
        NEXT();
    LABEL(kLoopNext)
        *loopPtr |= *contPtr;
        *contPtr = 0;
        NEXT();
    LABEL(kLoopMask)
        *loopPtr &= POP().fSigned;
        NEXT();
    LABEL(kLoopEnd)
        --contPtr; --loopPtr;
        NEXT();
    LABEL(kLoopBreak)
        *loopPtr &= ~mask();
        NEXT();
    LABEL(kLoopContinue) {
        I32 m = mask();
        *contPtr |=  m;
        *loopPtr &= ~m;
        NEXT();
    }
#ifdef SKSLC_THREADED_CODE
    #ifdef TRACE
        next:
            printf("at %3d (stack: %d) (disable threaded code for disassembly)\n",
                   (int) (ip - code), (int) (sp - stack) + 1);
            goto *READ_INST();
    #endif
#else
        }
    }
#endif
}

} // namespace Interpreter

#endif // SK_ENABLE_SKSL_INTERPRETER

#undef spf

void ByteCodeFunction::disassemble() const {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    const uint8_t* ip = fCode.data();
    while (ip < fCode.data() + fCode.size()) {
        printf("%d: ", (int)(ip - fCode.data()));
        ip = Interpreter::disassemble_instruction(ip);
        printf("\n");
    }
#endif
}

#define VECTOR_PREPROCESS(base)          \
    case ByteCodeInstruction::base ## 4: \
    case ByteCodeInstruction::base ## 3: \
    case ByteCodeInstruction::base ## 2: \
    case ByteCodeInstruction::base: READ8(); break;

#define VECTOR_PREPROCESS_NO_COUNT(base) \
    case ByteCodeInstruction::base ## 4: \
    case ByteCodeInstruction::base ## 3: \
    case ByteCodeInstruction::base ## 2: \
    case ByteCodeInstruction::base: break;

#define VECTOR_MATRIX_PREPROCESS(base) \
    VECTOR_PREPROCESS(base)            \
    case ByteCodeInstruction::base ## N: READ8(); break;

#define VECTOR_MATRIX_PREPROCESS_NO_COUNT(base) \
    VECTOR_PREPROCESS_NO_COUNT(base)            \
    case ByteCodeInstruction::base ## N: READ8(); break;

void ByteCodeFunction::preprocess(const void* labels[]) {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
#ifdef TRACE
    this->disassemble();
#endif
    uint8_t* ip = fCode.data();
    while (ip < fCode.data() + fCode.size()) {
        ByteCodeInstruction inst = (ByteCodeInstruction) (intptr_t) READ_INST();
        const void* label = labels[(int) inst];
        memcpy(ip - sizeof(instruction), &label, sizeof(label));
        switch (inst) {
            VECTOR_MATRIX_PREPROCESS(kAddF)
            VECTOR_PREPROCESS(kAddI)
            case ByteCodeInstruction::kAndB: break;
            case ByteCodeInstruction::kBranch: READ16(); break;
            case ByteCodeInstruction::kCall: READ8(); break;
            case ByteCodeInstruction::kCallExternal: {
                READ8();
                READ8();
                READ8();
                break;
            }
            case ByteCodeInstruction::kClampIndex: READ8(); break;
            VECTOR_PREPROCESS(kCompareIEQ)
            VECTOR_PREPROCESS(kCompareINEQ)
            VECTOR_MATRIX_PREPROCESS(kCompareFEQ)
            VECTOR_MATRIX_PREPROCESS(kCompareFNEQ)
            VECTOR_PREPROCESS(kCompareFGT)
            VECTOR_PREPROCESS(kCompareFGTEQ)
            VECTOR_PREPROCESS(kCompareFLT)
            VECTOR_PREPROCESS(kCompareFLTEQ)
            VECTOR_PREPROCESS(kCompareSGT)
            VECTOR_PREPROCESS(kCompareSGTEQ)
            VECTOR_PREPROCESS(kCompareSLT)
            VECTOR_PREPROCESS(kCompareSLTEQ)
            VECTOR_PREPROCESS(kCompareUGT)
            VECTOR_PREPROCESS(kCompareUGTEQ)
            VECTOR_PREPROCESS(kCompareULT)
            VECTOR_PREPROCESS(kCompareULTEQ)
            VECTOR_PREPROCESS_NO_COUNT(kConvertFtoI)
            VECTOR_PREPROCESS_NO_COUNT(kConvertStoF)
            VECTOR_PREPROCESS_NO_COUNT(kConvertUtoF)
            VECTOR_PREPROCESS(kCos)
            VECTOR_MATRIX_PREPROCESS(kDivideF)
            VECTOR_PREPROCESS(kDivideS)
            VECTOR_PREPROCESS(kDivideU)
            VECTOR_MATRIX_PREPROCESS(kDup)

            case ByteCodeInstruction::kInverse2x2:
            case ByteCodeInstruction::kInverse3x3:
            case ByteCodeInstruction::kInverse4x4: break;

            case ByteCodeInstruction::kLoad:
            case ByteCodeInstruction::kLoad2:
            case ByteCodeInstruction::kLoad3:
            case ByteCodeInstruction::kLoad4:
            case ByteCodeInstruction::kLoadGlobal:
            case ByteCodeInstruction::kLoadGlobal2:
            case ByteCodeInstruction::kLoadGlobal3:
            case ByteCodeInstruction::kLoadGlobal4: READ16(); break;

            case ByteCodeInstruction::kLoadSwizzle:
            case ByteCodeInstruction::kLoadSwizzleGlobal: {
                READ8();
                int count = READ8();
                ip += count;
                break;
            }

            case ByteCodeInstruction::kLoadExtended:
            case ByteCodeInstruction::kLoadExtendedGlobal:
                READ8();
                break;

            case ByteCodeInstruction::kMatrixToMatrix: {
                READ8();
                READ8();
                READ8();
                READ8();
                break;
            }
            case ByteCodeInstruction::kMatrixMultiply: {
                READ8();
                READ8();
                READ8();
                break;
            }
            VECTOR_MATRIX_PREPROCESS(kMultiplyF)
            VECTOR_PREPROCESS(kMultiplyI)
            VECTOR_MATRIX_PREPROCESS_NO_COUNT(kNegateF)
            VECTOR_PREPROCESS_NO_COUNT(kNegateI)
            case ByteCodeInstruction::kNotB: break;
            case ByteCodeInstruction::kOrB: break;
            VECTOR_MATRIX_PREPROCESS_NO_COUNT(kPop)
            case ByteCodeInstruction::kPushImmediate: READ32(); break;

            case ByteCodeInstruction::kReadExternal:
            case ByteCodeInstruction::kReadExternal2:
            case ByteCodeInstruction::kReadExternal3:
            case ByteCodeInstruction::kReadExternal4: READ16(); break;

            VECTOR_PREPROCESS(kRemainderF)
            VECTOR_PREPROCESS(kRemainderS)
            VECTOR_PREPROCESS(kRemainderU)
            case ByteCodeInstruction::kReserve: READ8(); break;
            case ByteCodeInstruction::kReturn: READ8(); break;
            case ByteCodeInstruction::kScalarToMatrix: READ8(); READ8(); break;
            VECTOR_PREPROCESS(kSin)
            VECTOR_PREPROCESS_NO_COUNT(kSqrt)

            case ByteCodeInstruction::kStore:
            case ByteCodeInstruction::kStore2:
            case ByteCodeInstruction::kStore3:
            case ByteCodeInstruction::kStore4:
            case ByteCodeInstruction::kStoreGlobal:
            case ByteCodeInstruction::kStoreGlobal2:
            case ByteCodeInstruction::kStoreGlobal3:
            case ByteCodeInstruction::kStoreGlobal4: READ8(); break;

            case ByteCodeInstruction::kStoreSwizzle:
            case ByteCodeInstruction::kStoreSwizzleGlobal: {
                READ8();
                int count = READ8();
                ip += count;
                break;
            }

            case ByteCodeInstruction::kStoreSwizzleIndirect:
            case ByteCodeInstruction::kStoreSwizzleIndirectGlobal: {
                int count = READ8();
                ip += count;
                break;
            }

            case ByteCodeInstruction::kStoreExtended: READ8(); break;
            case ByteCodeInstruction::kStoreExtendedGlobal: READ8(); break;

            VECTOR_MATRIX_PREPROCESS(kSubtractF)
            VECTOR_PREPROCESS(kSubtractI)

            case ByteCodeInstruction::kSwizzle: {
                READ8();
                int count = READ8();
                ip += count;
                break;
            }
            VECTOR_PREPROCESS(kTan)
            case ByteCodeInstruction::kWriteExternal:
            case ByteCodeInstruction::kWriteExternal2:
            case ByteCodeInstruction::kWriteExternal3:
            case ByteCodeInstruction::kWriteExternal4: READ16(); break;

            case ByteCodeInstruction::kXorB: break;
            case ByteCodeInstruction::kMaskPush: break;
            case ByteCodeInstruction::kMaskPop: break;
            case ByteCodeInstruction::kMaskNegate: break;
            case ByteCodeInstruction::kMaskBlend: READ8(); break;
            case ByteCodeInstruction::kBranchIfAllFalse: READ16(); break;
            case ByteCodeInstruction::kLoopBegin: break;
            case ByteCodeInstruction::kLoopNext: break;
            case ByteCodeInstruction::kLoopMask: break;
            case ByteCodeInstruction::kLoopEnd: break;
            case ByteCodeInstruction::kLoopContinue:  break;
            case ByteCodeInstruction::kLoopBreak: break;
            default:
                ip -= 2;
                printf("unknown(%d)\n", READ16());
                SkASSERT(false);
        }
    }
#endif
}

bool ByteCode::run(const ByteCodeFunction* f, float* args, float* outReturn, int N,
                   const float* uniforms, int uniformCount) const {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    Interpreter::VValue stack[128];
    int stackNeeded = f->fParameterCount + f->fLocalCount + f->fStackCount;
    if (stackNeeded > (int)SK_ARRAY_COUNT(stack)) {
        return false;
    }

    if (uniformCount != (int)fInputSlots.size()) {
        return false;
    }

    Interpreter::VValue globals[32];
    if (fGlobalCount > (int)SK_ARRAY_COUNT(globals)) {
        return false;
    }
    for (uint8_t slot : fInputSlots) {
        globals[slot].fFloat = *uniforms++;
    }

    int baseIndex = 0;

    while (N) {
        int w = std::min(N, Interpreter::VecWidth);

        // Transpose args into stack
        {
            float* src = args;
            for (int i = 0; i < w; ++i) {
                float* dst = (float*)stack + i;
                for (int j = f->fParameterCount; j > 0; --j) {
                    *dst = *src++;
                    dst += Interpreter::VecWidth;
                }
            }
        }

        bool stripedOutput = false;
        float** outArray = outReturn ? &outReturn : nullptr;
        if (!innerRun(this, f, stack, outArray, globals, stripedOutput, w, baseIndex)) {
            return false;
        }

        // Transpose out parameters back
        {
            float* dst = args;
            for (int i = 0; i < w; ++i) {
                float* src = (float*)stack + i;
                for (const auto& p : f->fParameters) {
                    if (p.fIsOutParameter) {
                        for (int j = p.fSlotCount; j > 0; --j) {
                            *dst++ = *src;
                            src += Interpreter::VecWidth;
                        }
                    } else {
                        dst += p.fSlotCount;
                        src += p.fSlotCount * Interpreter::VecWidth;
                    }
                }
            }
        }

        args += f->fParameterCount * w;
        if (outReturn) {
            outReturn += f->fReturnCount * w;
        }
        N -= w;
        baseIndex += w;
    }
    return true;
#else
    SkDEBUGFAIL("ByteCode interpreter not enabled");
    return false;
#endif
}

bool ByteCode::runStriped(const ByteCodeFunction* f, float* args[], int nargs, int N,
                          const float* uniforms, int uniformCount,
                          float* outArgs[], int outCount) const {
#if defined(SK_ENABLE_SKSL_INTERPRETER)
    Interpreter::VValue stack[128];
    int stackNeeded = f->fParameterCount + f->fLocalCount + f->fStackCount;
    if (stackNeeded > (int)SK_ARRAY_COUNT(stack)) {
        return false;
    }

    if (nargs != f->fParameterCount ||
        outCount != f->fReturnCount ||
        uniformCount != (int)fInputSlots.size()) {
        return false;
    }

    Interpreter::VValue globals[32];
    if (fGlobalCount > (int)SK_ARRAY_COUNT(globals)) {
        return false;
    }
    for (uint8_t slot : fInputSlots) {
        globals[slot].fFloat = *uniforms++;
    }

    // innerRun just takes outArgs, so clear it if the count is zero
    if (outCount == 0) {
        outArgs = nullptr;
    }

    int baseIndex = 0;

    while (N) {
        int w = std::min(N, Interpreter::VecWidth);

        // Copy args into stack
        for (int i = 0; i < nargs; ++i) {
            memcpy(stack + i, args[i], w * sizeof(float));
        }

        bool stripedOutput = true;
        if (!innerRun(this, f, stack, outArgs, globals, stripedOutput, w, baseIndex)) {
            return false;
        }

        // Copy out parameters back
        int slot = 0;
        for (const auto& p : f->fParameters) {
            if (p.fIsOutParameter) {
                for (int i = slot; i < slot + p.fSlotCount; ++i) {
                    memcpy(args[i], stack + i, w * sizeof(float));
                }
            }
            slot += p.fSlotCount;
        }

        // Step each argument pointer ahead
        for (int i = 0; i < nargs; ++i) {
            args[i] += w;
        }
        N -= w;
        baseIndex += w;
    }

    return true;
#else
    SkDEBUGFAIL("ByteCode interpreter not enabled");
    return false;
#endif
}

} // namespace SkSL

#endif
