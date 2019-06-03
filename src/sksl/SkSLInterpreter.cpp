/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STANDALONE

#include "include/core/SkPoint3.h"
#include "src/core/SkRasterPipeline.h"
#include "src/sksl/SkSLByteCodeGenerator.h"
#include "src/sksl/SkSLExternalValue.h"
#include "src/sksl/SkSLInterpreter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

static constexpr int UNINITIALIZED = 0xDEADBEEF;

Interpreter::Interpreter(std::unique_ptr<Program> program,
                         std::unique_ptr<ByteCode> byteCode,
                         Interpreter::Value inputs[])
    : fProgram(std::move(program))
    , fByteCode(std::move(byteCode))
    , fGlobals(fByteCode->fGlobalCount, UNINITIALIZED) {
    this->setInputs(inputs);
}

void Interpreter::setInputs(Interpreter::Value inputs[]) {
    for (uint8_t slot : fByteCode->fInputSlots) {
        fGlobals[slot] = *inputs++;
    }
}

void Interpreter::run(const ByteCodeFunction& f, Interpreter::Value args[],
                      Interpreter::Value* outReturn) {
#ifdef TRACE
    this->disassemble(f);
#endif
    Value smallStack[128];
    std::unique_ptr<Value[]> largeStack;
    Value* stack = smallStack;
    if ((int) SK_ARRAY_COUNT(smallStack) < f.fStackCount) {
        largeStack.reset(new Value[f.fStackCount]);
        stack = largeStack.get();
    }

    if (f.fParameterCount) {
        memcpy(stack, args, f.fParameterCount * sizeof(Value));
    }
    this->innerRun(f, stack, outReturn);

    for (const Variable* p : f.fDeclaration.fParameters) {
        const int nvalues = ByteCodeGenerator::SlotCount(p->fType);
        if (p->fModifiers.fFlags & Modifiers::kOut_Flag) {
            memcpy(args, stack, nvalues * sizeof(Value));
        }
        args  += nvalues;
        stack += nvalues;
    }
}

template <typename T>
static T unaligned_load(const void* ptr) {
    T val;
    memcpy(&val, ptr, sizeof(val));
    return val;
}

#define READ8() (*(ip++))
#define READ16() (ip += 2, unaligned_load<uint16_t>(ip - 2))
#define READ32() (ip += 4, unaligned_load<uint32_t>(ip - 4))

#define VECTOR_DISASSEMBLE(op, text)                          \
    case ByteCodeInstruction::op: printf(text); break;        \
    case ByteCodeInstruction::op##2: printf(text "2"); break; \
    case ByteCodeInstruction::op##3: printf(text "3"); break; \
    case ByteCodeInstruction::op##4: printf(text "4"); break;

#define VECTOR_MATRIX_DISASSEMBLE(op, text)                   \
    case ByteCodeInstruction::op: printf(text); break;        \
    case ByteCodeInstruction::op##2: printf(text "2"); break; \
    case ByteCodeInstruction::op##3: printf(text "3"); break; \
    case ByteCodeInstruction::op##4: printf(text "4"); break; \
    case ByteCodeInstruction::op##N: printf(text "N %d", READ8()); break;

static const uint8_t* disassemble_instruction(const uint8_t* ip) {
    switch ((ByteCodeInstruction) READ16()) {
        VECTOR_MATRIX_DISASSEMBLE(kAddF, "addf")
        VECTOR_DISASSEMBLE(kAddI, "addi")
        VECTOR_DISASSEMBLE(kAndB, "andb")
        VECTOR_DISASSEMBLE(kAndI, "andb")
        case ByteCodeInstruction::kBranch: printf("branch %d", READ16()); break;
        case ByteCodeInstruction::kCall: printf("call %d", READ8()); break;
        case ByteCodeInstruction::kCallExternal: {
            int argumentCount = READ8();
            int returnCount = READ8();
            int externalValue = READ8();
            printf("callexternal %d, %d, %d", argumentCount, returnCount, externalValue);
            break;
        }
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
        case ByteCodeInstruction::kConditionalBranch:
            printf("conditionalbranch %d", READ16());
            break;
        VECTOR_DISASSEMBLE(kConvertFtoI, "convertftoi")
        VECTOR_DISASSEMBLE(kConvertStoF, "convertstof")
        VECTOR_DISASSEMBLE(kConvertUtoF, "convertutof")
        VECTOR_DISASSEMBLE(kCos, "cos")
        case ByteCodeInstruction::kDebugPrint: printf("debugprint"); break;
        VECTOR_MATRIX_DISASSEMBLE(kDivideF, "dividef")
        VECTOR_DISASSEMBLE(kDivideS, "divideS")
        VECTOR_DISASSEMBLE(kDivideU, "divideu")
        VECTOR_MATRIX_DISASSEMBLE(kDup, "dup")
        case ByteCodeInstruction::kLoad: printf("load %d", READ8()); break;
        case ByteCodeInstruction::kLoad2: printf("load2 %d", READ8()); break;
        case ByteCodeInstruction::kLoad3: printf("load3 %d", READ8()); break;
        case ByteCodeInstruction::kLoad4: printf("load4 %d", READ8()); break;
        case ByteCodeInstruction::kLoadGlobal: printf("loadglobal %d", READ8()); break;
        case ByteCodeInstruction::kLoadGlobal2: printf("loadglobal2 %d", READ8()); break;
        case ByteCodeInstruction::kLoadGlobal3: printf("loadglobal3 %d", READ8()); break;
        case ByteCodeInstruction::kLoadGlobal4: printf("loadglobal4 %d", READ8()); break;
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
        VECTOR_DISASSEMBLE(kMix, "mix")
        VECTOR_MATRIX_DISASSEMBLE(kMultiplyF, "multiplyf")
        VECTOR_DISASSEMBLE(kMultiplyI, "multiplyi")
        VECTOR_MATRIX_DISASSEMBLE(kNegateF, "negatef")
        VECTOR_DISASSEMBLE(kNegateI, "negatei")
        VECTOR_DISASSEMBLE(kNot, "not")
        VECTOR_DISASSEMBLE(kOrB, "orb")
        VECTOR_DISASSEMBLE(kOrI, "ori")
        VECTOR_MATRIX_DISASSEMBLE(kPop, "pop")
        case ByteCodeInstruction::kPushImmediate: {
            uint32_t v = READ32();
            union { uint32_t u; float f; } pun = { v };
            printf("pushimmediate %s", (to_string(v) + "(" + to_string(pun.f) + ")").c_str());
            break;
        }
        case ByteCodeInstruction::kReadExternal: printf("readexternal %d", READ8()); break;
        case ByteCodeInstruction::kReadExternal2: printf("readexternal2 %d", READ8()); break;
        case ByteCodeInstruction::kReadExternal3: printf("readexternal3 %d", READ8()); break;
        case ByteCodeInstruction::kReadExternal4: printf("readexternal4 %d", READ8()); break;
        VECTOR_DISASSEMBLE(kRemainderF, "remainderf")
        VECTOR_DISASSEMBLE(kRemainderS, "remainders")
        VECTOR_DISASSEMBLE(kRemainderU, "remainderu")
        case ByteCodeInstruction::kReturn: printf("return %d", READ8()); break;
        case ByteCodeInstruction::kScalarToMatrix: {
            int cols = READ8();
            int rows = READ8();
            printf("scalartomatrix %dx%d", cols, rows);
            break;
        }
        VECTOR_DISASSEMBLE(kSin, "sin")
        VECTOR_DISASSEMBLE(kSqrt, "sqrt")
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
        case ByteCodeInstruction::kWriteExternal: printf("writeexternal %d", READ8()); break;
        case ByteCodeInstruction::kWriteExternal2: printf("writeexternal2 %d", READ8()); break;
        case ByteCodeInstruction::kWriteExternal3: printf("writeexternal3 %d", READ8()); break;
        case ByteCodeInstruction::kWriteExternal4: printf("writeexternal4 %d", READ8()); break;
        case ByteCodeInstruction::kMaskPush: printf("maskpush"); break;
        case ByteCodeInstruction::kMaskPop: printf("maskpop"); break;
        case ByteCodeInstruction::kMaskBlend: printf("maskblend"); break;
        default: printf("unknown(%d)\n", *(ip - 1)); SkASSERT(false);
    }
    return ip;
}

void Interpreter::disassemble(const ByteCodeFunction& f) {
    const uint8_t* ip = f.fCode.data();
    while (ip < f.fCode.data() + f.fCode.size()) {
        printf("%d: ", (int) (ip - f.fCode.data()));
        ip = disassemble_instruction(ip);
        printf("\n");
    }
}

#define VECTOR_BINARY_OP(base, field, op)                             \
    case ByteCodeInstruction::base ## 4:                              \
        sp[-4] = sp[-4].field op sp[0].field;                         \
        POP();                                                        \
        /* fall through */                                            \
    case ByteCodeInstruction::base ## 3: {                            \
        int count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count] = sp[count].field op sp[0].field;                   \
        POP();                                                        \
    }   /* fall through */                                            \
    case ByteCodeInstruction::base ## 2: {                            \
        int count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count] = sp[count].field op sp[0].field;                   \
        POP();                                                        \
    }   /* fall through */                                            \
    case ByteCodeInstruction::base: {                                 \
        int count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count] = sp[count].field op sp[0].field;                   \
        POP();                                                        \
        break;                                                        \
    }

#define VECTOR_MATRIX_BINARY_OP(base, field, op)                      \
    VECTOR_BINARY_OP(base, field, op)                                 \
    case ByteCodeInstruction::base ## N: {                            \
        int count = READ8();                                          \
        for (int i = count; i > 0; --i) {                             \
            sp[-count] = sp[-count].field op sp[0].field;             \
            POP();                                                    \
        }                                                             \
        break;                                                        \
    }

#define VECTOR_BINARY_FN(base, field, fn)                              \
    case ByteCodeInstruction::base ## 4:                               \
        sp[-4] = fn(sp[-4].field, sp[0].field);                        \
        POP();                                                         \
        /* fall through */                                             \
    case ByteCodeInstruction::base ## 3: {                             \
        int target = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[target] = fn(sp[target].field, sp[0].field);                \
        POP();                                                         \
    }   /* fall through */                                             \
    case ByteCodeInstruction::base ## 2: {                             \
        int target = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[target] = fn(sp[target].field, sp[0].field);                \
        POP();                                                         \
    }   /* fall through */                                             \
    case ByteCodeInstruction::base: {                                  \
        int target = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[target] = fn(sp[target].field, sp[0].field);                \
        POP();                                                         \
        break;                                                         \
    }

#define VECTOR_UNARY_FN(base, fn, field)                            \
    case ByteCodeInstruction::base ## 4: sp[-3] = fn(sp[-3].field); \
    case ByteCodeInstruction::base ## 3: sp[-2] = fn(sp[-2].field); \
    case ByteCodeInstruction::base ## 2: sp[-1] = fn(sp[-1].field); \
    case ByteCodeInstruction::base:      sp[ 0] = fn(sp[ 0].field); \
                                         break;

struct StackFrame {
    const uint8_t* fCode;
    const uint8_t* fIP;
    Interpreter::Value* fStack;
};

static float mix(float start, float end, float t) {
    return start * (1 - t) + end * t;
}

void Interpreter::innerRun(const ByteCodeFunction& f, Value* stack, Value* outReturn) {
    Value* sp = stack + f.fParameterCount + f.fLocalCount - 1;

    auto POP =  [&]          { SkASSERT(sp     >= stack); return *(sp--); };
    auto PUSH = [&](Value v) { SkASSERT(sp + 1 >= stack); *(++sp) = v;    };

    const uint8_t* code = f.fCode.data();
    const uint8_t* ip = code;
    std::vector<StackFrame> frames;

    for (;;) {
#ifdef TRACE
        printf("at %3d  ", (int) (ip - code));
        disassemble_instruction(ip);
        printf("\n");
#endif
        ByteCodeInstruction inst = (ByteCodeInstruction) READ16();
        switch (inst) {
            VECTOR_BINARY_OP(kAddI, fSigned, +)
            VECTOR_MATRIX_BINARY_OP(kAddF, fFloat, +)
            VECTOR_BINARY_OP(kAndB, fBool, &&)

            case ByteCodeInstruction::kBranch:
                ip = code + READ16();
                break;

            case ByteCodeInstruction::kCall: {
                // Precursor code has pushed all parameters to the stack. Update our bottom of
                // stack to point at the first parameter, and our sp to point past those parameters
                // (plus space for locals).
                int target = READ8();
                const ByteCodeFunction* fun = fByteCode->fFunctions[target].get();
                frames.push_back({ code, ip, stack });
                ip = code = fun->fCode.data();
                stack = sp - fun->fParameterCount + 1;
                sp = stack + fun->fParameterCount + fun->fLocalCount - 1;
                break;
            }

            case ByteCodeInstruction::kCallExternal: {
                int argumentCount = READ8();
                int returnCount = READ8();
                int target = READ8();
                ExternalValue* v = fByteCode->fExternalValues[target];
                sp -= argumentCount - 1;

                Value tmp[4];
                SkASSERT(returnCount <= (int)SK_ARRAY_COUNT(tmp));
                v->call(sp, tmp);
                memcpy(sp, tmp, returnCount * sizeof(Value));
                sp += returnCount - 1;
                break;
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

            case ByteCodeInstruction::kConditionalBranch: {
                int target = READ16();
                if (POP().fBool) {
                    ip = code + target;
                }
                break;
            }

            case ByteCodeInstruction::kConvertFtoI4: sp[-3].fSigned = (int)sp[-3].fFloat;
            case ByteCodeInstruction::kConvertFtoI3: sp[-2].fSigned = (int)sp[-2].fFloat;
            case ByteCodeInstruction::kConvertFtoI2: sp[-1].fSigned = (int)sp[-1].fFloat;
            case ByteCodeInstruction::kConvertFtoI:  sp[ 0].fSigned = (int)sp[ 0].fFloat;
                                                     break;

            case ByteCodeInstruction::kConvertStoF4: sp[-3].fFloat = sp[-3].fSigned;
            case ByteCodeInstruction::kConvertStoF3: sp[-2].fFloat = sp[-2].fSigned;
            case ByteCodeInstruction::kConvertStoF2: sp[-1].fFloat = sp[-1].fSigned;
            case ByteCodeInstruction::kConvertStoF : sp[ 0].fFloat = sp[ 0].fSigned;
                                                     break;

            case ByteCodeInstruction::kConvertUtoF4: sp[-3].fFloat = sp[-3].fUnsigned;
            case ByteCodeInstruction::kConvertUtoF3: sp[-2].fFloat = sp[-2].fUnsigned;
            case ByteCodeInstruction::kConvertUtoF2: sp[-1].fFloat = sp[-1].fUnsigned;
            case ByteCodeInstruction::kConvertUtoF : sp[ 0].fFloat = sp[ 0].fUnsigned;
                                                     break;

            VECTOR_UNARY_FN(kCos, cosf, fFloat)

            case ByteCodeInstruction::kCross: {
                SkPoint3 cross = SkPoint3::CrossProduct(SkPoint3::Make(sp[-5].fFloat,
                                                                       sp[-4].fFloat,
                                                                       sp[-3].fFloat),
                                                        SkPoint3::Make(sp[-2].fFloat,
                                                                       sp[-1].fFloat,
                                                                       sp[ 0].fFloat));
                sp -= 3;
                sp[-2] = cross.fX;
                sp[-1] = cross.fY;
                sp[ 0] = cross.fZ;
                break;
            }

            case ByteCodeInstruction::kDebugPrint: {
                Value v = POP();
                printf("Debug: %d(int), %d(uint), %f(float)\n", v.fSigned, v.fUnsigned, v.fFloat);
                break;
            }

            VECTOR_BINARY_OP(kDivideS, fSigned, /)
            VECTOR_BINARY_OP(kDivideU, fUnsigned, /)
            VECTOR_MATRIX_BINARY_OP(kDivideF, fFloat, /)

            case ByteCodeInstruction::kDup4: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup3: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup2: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup : PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
                                             break;

            case ByteCodeInstruction::kDupN: {
                int count = READ8();
                memcpy(sp + 1, sp - count + 1, count * sizeof(Value));
                sp += count;
                break;
            }

            case ByteCodeInstruction::kLoad4: sp[4] = stack[*ip + 3];
            case ByteCodeInstruction::kLoad3: sp[3] = stack[*ip + 2];
            case ByteCodeInstruction::kLoad2: sp[2] = stack[*ip + 1];
            case ByteCodeInstruction::kLoad : sp[1] = stack[*ip + 0];
                                              ++ip;
                                              sp += (int)inst - (int)ByteCodeInstruction::kLoad + 1;
                                              break;

            case ByteCodeInstruction::kLoadGlobal4: sp[4] = fGlobals[*ip + 3];
            case ByteCodeInstruction::kLoadGlobal3: sp[3] = fGlobals[*ip + 2];
            case ByteCodeInstruction::kLoadGlobal2: sp[2] = fGlobals[*ip + 1];
            case ByteCodeInstruction::kLoadGlobal : sp[1] = fGlobals[*ip + 0];
                                                    ++ip;
                                                    sp += (int)inst -
                                                          (int)ByteCodeInstruction::kLoadGlobal + 1;
                                                    break;

            case ByteCodeInstruction::kLoadExtended: {
                int count = READ8();
                int src = POP().fSigned;
                memcpy(sp + 1, &stack[src], count * sizeof(Value));
                sp += count;
                break;
            }

            case ByteCodeInstruction::kLoadExtendedGlobal: {
                int count = READ8();
                int src = POP().fSigned;
                SkASSERT(src + count <= (int) fGlobals.size());
                memcpy(sp + 1, &fGlobals[src], count * sizeof(Value));
                sp += count;
                break;
            }

            case ByteCodeInstruction::kLoadSwizzle: {
                int src = READ8();
                int count = READ8();
                for (int i = 0; i < count; ++i) {
                    PUSH(stack[src + *(ip + i)]);
                }
                ip += count;
                break;
            }

            case ByteCodeInstruction::kLoadSwizzleGlobal: {
                int src = READ8();
                SkASSERT(src < (int) fGlobals.size());
                int count = READ8();
                for (int i = 0; i < count; ++i) {
                    PUSH(fGlobals[src + *(ip + i)]);
                }
                ip += count;
                break;
            }

            case ByteCodeInstruction::kMatrixToMatrix: {
                int srcCols = READ8();
                int srcRows = READ8();
                int dstCols = READ8();
                int dstRows = READ8();
                SkASSERT(srcCols >= 2 && srcCols <= 4);
                SkASSERT(srcRows >= 2 && srcRows <= 4);
                SkASSERT(dstCols >= 2 && dstCols <= 4);
                SkASSERT(dstRows >= 2 && dstRows <= 4);
                SkMatrix44 m;
                for (int c = srcCols - 1; c >= 0; --c) {
                    for (int r = srcRows - 1; r >= 0; --r) {
                        m.set(r, c, POP().fFloat);
                    }
                }
                for (int c = 0; c < dstCols; ++c) {
                    for (int r = 0; r < dstRows; ++r) {
                        PUSH(m.get(r, c));
                    }
                }
                break;
            }

            case ByteCodeInstruction::kMatrixMultiply: {
                int lCols = READ8();
                int lRows = READ8();
                int rCols = READ8();
                int rRows = lCols;
                float tmp[16] = { 0.0f };
                float* B = &(sp - (rCols * rRows) + 1)->fFloat;
                float* A = B - (lCols * lRows);
                for (int c = 0; c < rCols; ++c) {
                    for (int r = 0; r < lRows; ++r) {
                        for (int j = 0; j < lCols; ++j) {
                            tmp[c*lRows + r] += A[j*lRows + r] * B[c*rRows + j];
                        }
                    }
                }
                sp -= (lCols * lRows) + (rCols * rRows);
                memcpy(sp + 1, tmp, rCols * lRows * sizeof(Value));
                sp += (rCols * lRows);
                break;
            }

            // stack looks like: X1 Y1 Z1 W1 X2 Y2 Z2 W2 T
            case ByteCodeInstruction::kMix4:
                sp[-5] = mix(sp[-5].fFloat, sp[-1].fFloat, sp[0].fFloat);
                // fall through
            case ByteCodeInstruction::kMix3: {
                int count = (int) inst - (int) ByteCodeInstruction::kMix + 1;
                int target = 2 - count * 2;
                sp[target] = mix(sp[target].fFloat, sp[2 - count].fFloat, sp[0].fFloat);
                // fall through
            }
            case ByteCodeInstruction::kMix2: {
                int count = (int) inst - (int) ByteCodeInstruction::kMix + 1;
                int target = 1 - count * 2;
                sp[target] = mix(sp[target].fFloat, sp[1 - count].fFloat, sp[0].fFloat);
                // fall through
            }
            case ByteCodeInstruction::kMix: {
                int count = (int) inst - (int) ByteCodeInstruction::kMix + 1;
                int target = -count * 2;
                sp[target] = mix(sp[target].fFloat, sp[-count].fFloat, sp[0].fFloat);
                sp -= 1 + count;
                break;
            }

            VECTOR_BINARY_OP(kMultiplyI, fSigned, *)
            VECTOR_MATRIX_BINARY_OP(kMultiplyF, fFloat, *)

            case ByteCodeInstruction::kNot:
                sp[0].fBool = !sp[0].fBool;
                break;

            case ByteCodeInstruction::kNegateF4: sp[-3] = -sp[-3].fFloat;
            case ByteCodeInstruction::kNegateF3: sp[-2] = -sp[-2].fFloat;
            case ByteCodeInstruction::kNegateF2: sp[-1] = -sp[-1].fFloat;
            case ByteCodeInstruction::kNegateF : sp[ 0] = -sp[ 0].fFloat;
                                                 break;

            case ByteCodeInstruction::kNegateFN: {
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    sp[-i] = -sp[-i].fFloat;
                }
                break;
            }

            case ByteCodeInstruction::kNegateI4: sp[-3] = -sp[-3].fSigned;
            case ByteCodeInstruction::kNegateI3: sp[-2] = -sp[-2].fSigned;
            case ByteCodeInstruction::kNegateI2: sp[-1] = -sp[-1].fSigned;
            case ByteCodeInstruction::kNegateI : sp[ 0] = -sp [0].fSigned;
                                                 break;

            VECTOR_BINARY_OP(kOrB, fBool, ||)

            case ByteCodeInstruction::kPop4: POP();
            case ByteCodeInstruction::kPop3: POP();
            case ByteCodeInstruction::kPop2: POP();
            case ByteCodeInstruction::kPop : POP();
                                             break;

            case ByteCodeInstruction::kPopN:
                sp -= READ8();
                break;

            case ByteCodeInstruction::kPushImmediate:
                PUSH(READ32());
                break;

            case ByteCodeInstruction::kReadExternal:  // fall through
            case ByteCodeInstruction::kReadExternal2: // fall through
            case ByteCodeInstruction::kReadExternal3: // fall through
            case ByteCodeInstruction::kReadExternal4: {
                int src = READ8();
                fByteCode->fExternalValues[src]->read(sp + 1);
                sp += (int) inst - (int) ByteCodeInstruction::kReadExternal + 1;
                break;
            }

            VECTOR_BINARY_FN(kRemainderF, fFloat, fmodf)
            VECTOR_BINARY_OP(kRemainderS, fSigned, %)
            VECTOR_BINARY_OP(kRemainderU, fUnsigned, %)

            case ByteCodeInstruction::kReturn: {
                int count = READ8();
                if (frames.empty()) {
                    if (outReturn) {
                        memcpy(outReturn, sp - count + 1, count * sizeof(Value));
                    }
                    return;
                } else {
                    // When we were called, 'stack' was positioned at the old top-of-stack (where
                    // our parameters were placed). So copy our return values to that same spot.
                    memmove(stack, sp - count + 1, count * sizeof(Value));

                    // Now move the stack pointer to the end of the just-pushed return values,
                    // and restore everything else.
                    const StackFrame& frame(frames.back());
                    sp = stack + count - 1;
                    stack = frame.fStack;
                    code = frame.fCode;
                    ip = frame.fIP;
                    frames.pop_back();
                    break;
                }
            }

            case ByteCodeInstruction::kScalarToMatrix: {
                int cols = READ8();
                int rows = READ8();
                Value v = POP();
                for (int c = 0; c < cols; ++c) {
                    for (int r = 0; r < rows; ++r) {
                        PUSH(c == r ? v : 0.0f);
                    }
                }
                break;
            }

            VECTOR_UNARY_FN(kSin, sinf, fFloat)
            VECTOR_UNARY_FN(kSqrt, sqrtf, fFloat)

            case ByteCodeInstruction::kStore4: stack[*ip + 3] = POP();
            case ByteCodeInstruction::kStore3: stack[*ip + 2] = POP();
            case ByteCodeInstruction::kStore2: stack[*ip + 1] = POP();
            case ByteCodeInstruction::kStore : stack[*ip + 0] = POP();
                                               ++ip;
                                               break;

            case ByteCodeInstruction::kStoreGlobal4: fGlobals[*ip + 3] = POP();
            case ByteCodeInstruction::kStoreGlobal3: fGlobals[*ip + 2] = POP();
            case ByteCodeInstruction::kStoreGlobal2: fGlobals[*ip + 1] = POP();
            case ByteCodeInstruction::kStoreGlobal : fGlobals[*ip + 0] = POP();
                                                     ++ip;
                                                     break;

            case ByteCodeInstruction::kStoreExtended: {
                int count = READ8();
                int target = POP().fSigned;
                memcpy(&stack[target], sp - count + 1, count * sizeof(Value));
                sp -= count;
                break;
            }
            case ByteCodeInstruction::kStoreExtendedGlobal: {
                int count = READ8();
                int target = POP().fSigned;
                SkASSERT(target + count <= (int) fGlobals.size());
                memcpy(&fGlobals[target], sp - count + 1, count * sizeof(Value));
                sp -= count;
                break;
            }

            case ByteCodeInstruction::kStoreSwizzle: {
                int target = READ8();
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    stack[target + *(ip + i)] = POP();
                }
                ip += count;
                break;
            }

            case ByteCodeInstruction::kStoreSwizzleGlobal: {
                int target = READ8();
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    fGlobals[target + *(ip + i)] = POP();
                }
                ip += count;
                break;
            }
            case ByteCodeInstruction::kStoreSwizzleIndirect: {
                int target = POP().fSigned;
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    stack[target + *(ip + i)] = POP();
                }
                ip += count;
                break;
            }
            case ByteCodeInstruction::kStoreSwizzleIndirectGlobal: {
                int target = POP().fSigned;
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    fGlobals[target + *(ip + i)] = POP();
                }
                ip += count;
                break;
            }

            VECTOR_BINARY_OP(kSubtractI, fSigned, -)
            VECTOR_MATRIX_BINARY_OP(kSubtractF, fFloat, -)

            case ByteCodeInstruction::kSwizzle: {
                Value tmp[4];
                for (int i = READ8() - 1; i >= 0; --i) {
                    tmp[i] = POP();
                }
                for (int i = READ8() - 1; i >= 0; --i) {
                    PUSH(tmp[READ8()]);
                }
                break;
            }

            VECTOR_UNARY_FN(kTan, tanf, fFloat)

            case ByteCodeInstruction::kWriteExternal:  // fall through
            case ByteCodeInstruction::kWriteExternal2: // fall through
            case ByteCodeInstruction::kWriteExternal3: // fall through
            case ByteCodeInstruction::kWriteExternal4: {
                int count = (int) inst - (int) ByteCodeInstruction::kWriteExternal + 1;
                int target = READ8();
                fByteCode->fExternalValues[target]->write(sp - count + 1);
                sp -= count;
                break;
            }

            default:
                SkDEBUGFAILF("unsupported instruction %d\n", (int) inst);
        }
#ifdef TRACE
        int stackSize = (int) (sp - stack + 1);
        printf("STACK(%d):", stackSize);
        for (int i = 0; i < stackSize; ++i) {
            printf(" %d(%g)", stack[i].fSigned, stack[i].fFloat);
        }
        printf("\n");
#endif
    }
}

VecInterpreter::VecInterpreter(std::unique_ptr<Program> program,
                               std::unique_ptr<ByteCode> byteCode,
                               VecInterpreter::SValue inputs[])
    : fProgram(std::move(program))
    , fByteCode(std::move(byteCode))
    , fGlobals(fByteCode->fGlobalCount, int4(UNINITIALIZED)) {
    this->setInputs(inputs);
}

void VecInterpreter::setInputs(VecInterpreter::SValue inputs[]) {
    for (uint8_t slot : fByteCode->fInputSlots) {
        fGlobals[slot] = uint4((*inputs++).fUnsigned);
    }
}

void VecInterpreter::run(const ByteCodeFunction& f, VecInterpreter::SValue args[],
                         VecInterpreter::SValue* outReturn) {
#ifdef TRACE
    this->disassemble(f);
#endif
    VValue smallStack[128];
    VValue* stack = smallStack;

    // Transpose args into stack
    for (int i = 0; i < f.fParameterCount; ++i) {
        stack[i].fSigned[0] = args[f.fParameterCount * 0 + i].fSigned;
        stack[i].fSigned[1] = args[f.fParameterCount * 1 + i].fSigned;
        stack[i].fSigned[2] = args[f.fParameterCount * 2 + i].fSigned;
        stack[i].fSigned[3] = args[f.fParameterCount * 3 + i].fSigned;
    }

    this->innerRun(f, stack, outReturn);

    for (const Variable* p : f.fDeclaration.fParameters) {
        const int nvalues = ByteCodeGenerator::SlotCount(p->fType);
        if (p->fModifiers.fFlags & Modifiers::kOut_Flag) {
            for (int i = 0; i < nvalues; ++i) {
                args[f.fParameterCount * 0 + i].fSigned = stack[i].fSigned[0];
                args[f.fParameterCount * 1 + i].fSigned = stack[i].fSigned[1];
                args[f.fParameterCount * 2 + i].fSigned = stack[i].fSigned[2];
                args[f.fParameterCount * 3 + i].fSigned = stack[i].fSigned[3];
            }
        }
        args  += nvalues;
        stack += nvalues;
    }
}

void VecInterpreter::disassemble(const ByteCodeFunction& f) {
    const uint8_t* ip = f.fCode.data();
    while (ip < f.fCode.data() + f.fCode.size()) {
        printf("%d: ", (int) (ip - f.fCode.data()));
        ip = disassemble_instruction(ip);
        printf("\n");
    }
}

#define VECTOR_UNARY_FN_VEC(base, fn, field)                                    \
    case ByteCodeInstruction::base ## 4: sp[-3].field[0] = fn(sp[-3].field[0]); \
                                         sp[-3].field[1] = fn(sp[-3].field[1]); \
                                         sp[-3].field[2] = fn(sp[-3].field[2]); \
                                         sp[-3].field[3] = fn(sp[-3].field[3]); \
    case ByteCodeInstruction::base ## 3: sp[-2].field[0] = fn(sp[-2].field[0]); \
                                         sp[-2].field[1] = fn(sp[-2].field[1]); \
                                         sp[-2].field[2] = fn(sp[-2].field[2]); \
                                         sp[-2].field[3] = fn(sp[-2].field[3]); \
    case ByteCodeInstruction::base ## 2: sp[-1].field[0] = fn(sp[-1].field[0]); \
                                         sp[-1].field[1] = fn(sp[-1].field[1]); \
                                         sp[-1].field[2] = fn(sp[-1].field[2]); \
                                         sp[-1].field[3] = fn(sp[-1].field[3]); \
    case ByteCodeInstruction::base     : sp[ 0].field[0] = fn(sp[ 0].field[0]); \
                                         sp[ 0].field[1] = fn(sp[ 0].field[1]); \
                                         sp[ 0].field[2] = fn(sp[ 0].field[2]); \
                                         sp[ 0].field[3] = fn(sp[ 0].field[3]); \
                                         break;

struct VecStackFrame {
    const uint8_t* fCode;
    const uint8_t* fIP;
    VecInterpreter::VValue* fStack;
};

static VecInterpreter::float4 mix(VecInterpreter::float4 start,
                                  VecInterpreter::float4 end,
                                  VecInterpreter::float4 t) {
    return start * (1 - t) + end * t;
}

// TODO: trunc on integers?
template <typename T>
static T vec_mod(T a, T b) {
    return a - skvx::trunc(a / b) * b;
}

void VecInterpreter::innerRun(const ByteCodeFunction& f, VValue* stack, SValue* outReturn) {
    VValue* sp = stack + f.fParameterCount + f.fLocalCount - 1;

    auto POP =  [&]           { SkASSERT(sp     >= stack); return *(sp--); };
    auto PUSH = [&](VValue v) { SkASSERT(sp + 1 >= stack); *(++sp) = v;    };

    const uint8_t* code = f.fCode.data();
    const uint8_t* ip = code;
    std::vector<VecStackFrame> frames;
    int4 maskStack[32] = { int4(~0) };
    int4* mask = maskStack;

    for (;;) {
#ifdef TRACE
        printf("at %3d  ", (int) (ip - code));
        disassemble_instruction(ip);
        printf("\n");
#endif
        ByteCodeInstruction inst = (ByteCodeInstruction) READ16();
        switch (inst) {
            VECTOR_BINARY_OP(kAddI, fSigned, +)
            VECTOR_MATRIX_BINARY_OP(kAddF, fFloat, +)

            // Booleans are integer masks: 0/~0 for false/true. So bitwise ops do what we want:
            VECTOR_BINARY_OP(kAndB, fSigned, &)
            VECTOR_BINARY_OP(kOrB, fSigned, |)
            case ByteCodeInstruction::kNot:
                sp[0].fSigned = ~sp[0].fSigned;
                break;

            case ByteCodeInstruction::kBranch:
                ip = code + READ16();
                break;

            case ByteCodeInstruction::kCall: {
                // Precursor code has pushed all parameters to the stack. Update our bottom of
                // stack to point at the first parameter, and our sp to point past those parameters
                // (plus space for locals).
                int target = READ8();
                const ByteCodeFunction* fun = fByteCode->fFunctions[target].get();
                if (skvx::any(*mask)) {
                    frames.push_back({ code, ip, stack });
                    ip = code = fun->fCode.data();
                    stack = sp - fun->fParameterCount + 1;
                    sp = stack + fun->fParameterCount + fun->fLocalCount - 1;
                } else {
                    sp -= fun->fParameterCount;
                    sp += fun->fReturnCount;
                }
                break;
            }

            case ByteCodeInstruction::kCallExternal:
                SkDEBUGFAIL("Not implemented"); break;

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

            case ByteCodeInstruction::kConditionalBranch:
                SkDEBUGFAIL("Not implemented"); break;

            case ByteCodeInstruction::kConvertFtoI4: sp[-3] = skvx::cast<int>(sp[-3].fFloat);
            case ByteCodeInstruction::kConvertFtoI3: sp[-2] = skvx::cast<int>(sp[-2].fFloat);
            case ByteCodeInstruction::kConvertFtoI2: sp[-1] = skvx::cast<int>(sp[-1].fFloat);
            case ByteCodeInstruction::kConvertFtoI:  sp[ 0] = skvx::cast<int>(sp[ 0].fFloat);
                                                     break;

            case ByteCodeInstruction::kConvertStoF4: sp[-3] = skvx::cast<float>(sp[-3].fSigned);
            case ByteCodeInstruction::kConvertStoF3: sp[-2] = skvx::cast<float>(sp[-2].fSigned);
            case ByteCodeInstruction::kConvertStoF2: sp[-1] = skvx::cast<float>(sp[-1].fSigned);
            case ByteCodeInstruction::kConvertStoF : sp[ 0] = skvx::cast<float>(sp[ 0].fSigned);
                                                     break;

            case ByteCodeInstruction::kConvertUtoF4: sp[-3] = skvx::cast<float>(sp[-3].fUnsigned);
            case ByteCodeInstruction::kConvertUtoF3: sp[-2] = skvx::cast<float>(sp[-2].fUnsigned);
            case ByteCodeInstruction::kConvertUtoF2: sp[-1] = skvx::cast<float>(sp[-1].fUnsigned);
            case ByteCodeInstruction::kConvertUtoF : sp[ 0] = skvx::cast<float>(sp[ 0].fUnsigned);
                                                     break;

            VECTOR_UNARY_FN_VEC(kCos, cosf, fFloat)

            case ByteCodeInstruction::kCross: {
                float4 ax = sp[-5].fFloat, ay = sp[-4].fFloat, az = sp[-3].fFloat,
                       bx = sp[-2].fFloat, by = sp[-2].fFloat, bz = sp[ 0].fFloat;
                float4 cx = ay*bz - az*by,
                       cy = az*bx - ax*bz,
                       cz = ax*by - ay*bx;
                sp -= 3;
                sp[-2] = cx;
                sp[-1] = cy;
                sp[ 0] = cz;
                break;
            }

            case ByteCodeInstruction::kDebugPrint:
                SkDEBUGFAIL("Not implemented"); break;

            VECTOR_BINARY_OP(kDivideS, fSigned, /)
            VECTOR_BINARY_OP(kDivideU, fUnsigned, /)
            VECTOR_MATRIX_BINARY_OP(kDivideF, fFloat, /)

            case ByteCodeInstruction::kDup4: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup3: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup2: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup : PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
                                             break;

            case ByteCodeInstruction::kDupN: {
                int count = READ8();
                memcpy(sp + 1, sp - count + 1, count * sizeof(VValue));
                sp += count;
                break;
            }

            case ByteCodeInstruction::kLoad4: sp[4] = stack[*ip + 3];
            case ByteCodeInstruction::kLoad3: sp[3] = stack[*ip + 2];
            case ByteCodeInstruction::kLoad2: sp[2] = stack[*ip + 1];
            case ByteCodeInstruction::kLoad : sp[1] = stack[*ip + 0];
                                              ++ip;
                                              sp += (int)inst - (int)ByteCodeInstruction::kLoad + 1;
                                              break;

            case ByteCodeInstruction::kLoadGlobal4: sp[4] = fGlobals[*ip + 3];
            case ByteCodeInstruction::kLoadGlobal3: sp[3] = fGlobals[*ip + 2];
            case ByteCodeInstruction::kLoadGlobal2: sp[2] = fGlobals[*ip + 1];
            case ByteCodeInstruction::kLoadGlobal : sp[1] = fGlobals[*ip + 0];
                                                    ++ip;
                                                    sp += (int)inst -
                                                          (int)ByteCodeInstruction::kLoadGlobal + 1;
                                                    break;

            case ByteCodeInstruction::kLoadExtended: {
                int count = READ8();
                int4 src = POP().fSigned;
                int4 m = *mask;
                for (int i = 0; i < count; ++i) {
                    if (m[0]) { sp[i + 1].fUnsigned[0] = stack[src[0]].fUnsigned[0]; }
                    if (m[1]) { sp[i + 1].fUnsigned[1] = stack[src[1]].fUnsigned[1]; }
                    if (m[2]) { sp[i + 1].fUnsigned[2] = stack[src[2]].fUnsigned[2]; }
                    if (m[3]) { sp[i + 1].fUnsigned[3] = stack[src[3]].fUnsigned[3]; }
                }
                sp += count;
                break;
            }

            case ByteCodeInstruction::kLoadExtendedGlobal: {
                int count = READ8();
                int4 src = POP().fSigned;
                int4 m = *mask;
                for (int i = 0; i < count; ++i) {
                    if (m[0]) { sp[i + 1].fUnsigned[0] = fGlobals[src[0]].fUnsigned[0]; }
                    if (m[1]) { sp[i + 1].fUnsigned[1] = fGlobals[src[1]].fUnsigned[1]; }
                    if (m[2]) { sp[i + 1].fUnsigned[2] = fGlobals[src[2]].fUnsigned[2]; }
                    if (m[3]) { sp[i + 1].fUnsigned[3] = fGlobals[src[3]].fUnsigned[3]; }
                }
                sp += count;
                break;
            }

            case ByteCodeInstruction::kLoadSwizzle: {
                int src = READ8();
                int count = READ8();
                for (int i = 0; i < count; ++i) {
                    PUSH(stack[src + *(ip + i)]);
                }
                ip += count;
                break;
            }

            case ByteCodeInstruction::kLoadSwizzleGlobal: {
                int src = READ8();
                SkASSERT(src < (int)fGlobals.size());
                int count = READ8();
                for (int i = 0; i < count; ++i) {
                    PUSH(fGlobals[src + *(ip + i)]);
                }
                ip += count;
                break;
            }

            case ByteCodeInstruction::kMatrixToMatrix: {
                int srcCols = READ8();
                int srcRows = READ8();
                int dstCols = READ8();
                int dstRows = READ8();
                SkASSERT(srcCols >= 2 && srcCols <= 4);
                SkASSERT(srcRows >= 2 && srcRows <= 4);
                SkASSERT(dstCols >= 2 && dstCols <= 4);
                SkASSERT(dstRows >= 2 && dstRows <= 4);
                VValue tmp[16];
                memset(tmp, 0, sizeof(tmp));
                tmp[0] = tmp[5] = tmp[10] = tmp[15] = float4(1.0f);
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
                break;
            }

            case ByteCodeInstruction::kMatrixMultiply: {
                int lCols = READ8();
                int lRows = READ8();
                int rCols = READ8();
                int rRows = lCols;
                float4 tmp[16] = { 0.0f };
                float4* B = &(sp - (rCols * rRows) + 1)->fFloat;
                float4* A = B - (lCols * lRows);
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
                break;
            }

            // stack looks like: X1 Y1 Z1 W1 X2 Y2 Z2 W2 T
            case ByteCodeInstruction::kMix4:
                sp[-5] = mix(sp[-5].fFloat, sp[-1].fFloat, sp[0].fFloat);
                // fall through
            case ByteCodeInstruction::kMix3: {
                int count = (int) inst - (int) ByteCodeInstruction::kMix + 1;
                int target = 2 - count * 2;
                sp[target] = mix(sp[target].fFloat, sp[2 - count].fFloat, sp[0].fFloat);
                // fall through
            }
            case ByteCodeInstruction::kMix2: {
                int count = (int) inst - (int) ByteCodeInstruction::kMix + 1;
                int target = 1 - count * 2;
                sp[target] = mix(sp[target].fFloat, sp[1 - count].fFloat, sp[0].fFloat);
                // fall through
            }
            case ByteCodeInstruction::kMix: {
                int count = (int) inst - (int) ByteCodeInstruction::kMix + 1;
                int target = -count * 2;
                sp[target] = mix(sp[target].fFloat, sp[-count].fFloat, sp[0].fFloat);
                sp -= 1 + count;
                break;
            }

            VECTOR_BINARY_OP(kMultiplyI, fSigned, *)
            VECTOR_MATRIX_BINARY_OP(kMultiplyF, fFloat, *)

            case ByteCodeInstruction::kNegateF4: sp[-3] = -sp[-3].fFloat;
            case ByteCodeInstruction::kNegateF3: sp[-2] = -sp[-2].fFloat;
            case ByteCodeInstruction::kNegateF2: sp[-1] = -sp[-1].fFloat;
            case ByteCodeInstruction::kNegateF : sp[ 0] = -sp[ 0].fFloat;
                                                 break;

            case ByteCodeInstruction::kNegateFN: {
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    sp[-i] = -sp[-i].fFloat;
                }
                break;
            }

            case ByteCodeInstruction::kNegateI4: sp[-3] = -sp[-3].fSigned;
            case ByteCodeInstruction::kNegateI3: sp[-2] = -sp[-2].fSigned;
            case ByteCodeInstruction::kNegateI2: sp[-1] = -sp[-1].fSigned;
            case ByteCodeInstruction::kNegateI : sp[ 0] = -sp [0].fSigned;
                                                 break;

            case ByteCodeInstruction::kPop4: POP();
            case ByteCodeInstruction::kPop3: POP();
            case ByteCodeInstruction::kPop2: POP();
            case ByteCodeInstruction::kPop : POP();
                                             break;

            case ByteCodeInstruction::kPopN:
                sp -= READ8();
                break;

            case ByteCodeInstruction::kPushImmediate:
                PUSH(uint4(READ32()));
                break;

            case ByteCodeInstruction::kReadExternal:
            case ByteCodeInstruction::kReadExternal2:
            case ByteCodeInstruction::kReadExternal3:
            case ByteCodeInstruction::kReadExternal4:
                SkDEBUGFAIL("Not implemented"); break;

            VECTOR_BINARY_FN(kRemainderF, fFloat, vec_mod<float4>)
            VECTOR_BINARY_FN(kRemainderS, fSigned, vec_mod<int4>)
            VECTOR_BINARY_FN(kRemainderU, fUnsigned, vec_mod<uint4>)

            case ByteCodeInstruction::kReturn: {
                int count = READ8();
                if (frames.empty()) {
                    if (outReturn) {
                        VValue* src = sp - count + 1;
                        for (int i = 0; i < count; ++i) {
                            outReturn[count * 0].fUnsigned = src->fUnsigned[0];
                            outReturn[count * 1].fUnsigned = src->fUnsigned[1];
                            outReturn[count * 2].fUnsigned = src->fUnsigned[2];
                            outReturn[count * 3].fUnsigned = src->fUnsigned[3];
                            ++outReturn;
                            ++src;
                        }
                    }
                    return;
                } else {
                    // When we were called, 'stack' was positioned at the old top-of-stack (where
                    // our parameters were placed). So copy our return values to that same spot.
                    memmove(stack, sp - count + 1, count * sizeof(VValue));

                    // Now move the stack pointer to the end of the just-pushed return values,
                    // and restore everything else.
                    const VecStackFrame& frame(frames.back());
                    sp = stack + count - 1;
                    stack = frame.fStack;
                    code = frame.fCode;
                    ip = frame.fIP;
                    frames.pop_back();
                    break;
                }
            }

            case ByteCodeInstruction::kScalarToMatrix: {
                int cols = READ8();
                int rows = READ8();
                VValue v = POP();
                for (int c = 0; c < cols; ++c) {
                    for (int r = 0; r < rows; ++r) {
                        PUSH(c == r ? v : float4(0.0f));
                    }
                }
                break;
            }

            VECTOR_UNARY_FN_VEC(kSin, sinf, fFloat)
            VECTOR_UNARY_FN(kSqrt, skvx::sqrt, fFloat)

            case ByteCodeInstruction::kStore4:
                stack[*ip+3] = skvx::if_then_else(*mask, POP().fFloat, stack[*ip+3].fFloat);
            case ByteCodeInstruction::kStore3:
                stack[*ip+2] = skvx::if_then_else(*mask, POP().fFloat, stack[*ip+2].fFloat);
            case ByteCodeInstruction::kStore2:
                stack[*ip+1] = skvx::if_then_else(*mask, POP().fFloat, stack[*ip+1].fFloat);
            case ByteCodeInstruction::kStore :
                stack[*ip+0] = skvx::if_then_else(*mask, POP().fFloat, stack[*ip+0].fFloat);
                ++ip;
                break;

            case ByteCodeInstruction::kStoreGlobal4:
                fGlobals[*ip+3] = skvx::if_then_else(*mask, POP().fFloat, fGlobals[*ip+3].fFloat);
            case ByteCodeInstruction::kStoreGlobal3:
                fGlobals[*ip+3] = skvx::if_then_else(*mask, POP().fFloat, fGlobals[*ip+3].fFloat);
            case ByteCodeInstruction::kStoreGlobal2:
                fGlobals[*ip+3] = skvx::if_then_else(*mask, POP().fFloat, fGlobals[*ip+3].fFloat);
            case ByteCodeInstruction::kStoreGlobal :
                fGlobals[*ip+3] = skvx::if_then_else(*mask, POP().fFloat, fGlobals[*ip+3].fFloat);
                ++ip;
                break;

            case ByteCodeInstruction::kStoreExtended: {
                int count = READ8();
                int4 target = POP().fSigned;
                VValue* src = sp - count + 1;
                int4 m = *mask;
                for (int i = 0; i < count; ++i) {
                    if (m[0]) { stack[target[0]].fSigned[0] = src[i].fSigned[0]; }
                    if (m[1]) { stack[target[1]].fSigned[1] = src[i].fSigned[1]; }
                    if (m[2]) { stack[target[2]].fSigned[2] = src[i].fSigned[2]; }
                    if (m[3]) { stack[target[3]].fSigned[3] = src[i].fSigned[3]; }
                }
                sp -= count;
                break;
            }
            case ByteCodeInstruction::kStoreExtendedGlobal: {
                int count = READ8();
                int4 target = POP().fSigned;
                VValue* src = sp - count + 1;
                int4 m = *mask;
                for (int i = 0; i < count; ++i) {
                    if (m[0]) { fGlobals[target[0]].fSigned[0] = src[i].fSigned[0]; }
                    if (m[1]) { fGlobals[target[1]].fSigned[1] = src[i].fSigned[1]; }
                    if (m[2]) { fGlobals[target[2]].fSigned[2] = src[i].fSigned[2]; }
                    if (m[3]) { fGlobals[target[3]].fSigned[3] = src[i].fSigned[3]; }
                }
                sp -= count;
                break;
            }

            case ByteCodeInstruction::kStoreSwizzle: {
                int target = READ8();
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    stack[target + *(ip + i)] = skvx::if_then_else(
                            *mask, POP().fFloat, stack[target + *(ip + i)].fFloat);
                }
                ip += count;
                break;
            }

            case ByteCodeInstruction::kStoreSwizzleGlobal: {
                int target = READ8();
                int count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    fGlobals[target + *(ip + i)] = skvx::if_then_else(
                            *mask, POP().fFloat, fGlobals[target + *(ip + i)].fFloat);
                }
                ip += count;
                break;
            }

            case ByteCodeInstruction::kStoreSwizzleIndirect: {
                int count = READ8();
                int4 target = POP().fSigned;
                int4 m = *mask;
                for (int i = count - 1; i >= 0; --i) {
                    int4 v = POP().fSigned;
                    if (m[0]) { stack[target[0] + *(ip + i)].fSigned[0] = v[0]; }
                    if (m[1]) { stack[target[1] + *(ip + i)].fSigned[1] = v[1]; }
                    if (m[2]) { stack[target[2] + *(ip + i)].fSigned[2] = v[2]; }
                    if (m[3]) { stack[target[3] + *(ip + i)].fSigned[3] = v[3]; }
                }
                ip += count;
                break;
            }

            case ByteCodeInstruction::kStoreSwizzleIndirectGlobal: {
                int count = READ8();
                int4 target = POP().fSigned;
                int4 m = *mask;
                for (int i = count - 1; i >= 0; --i) {
                    int4 v = POP().fSigned;
                    if (m[0]) { fGlobals[target[0] + *(ip + i)].fSigned[0] = v[0]; }
                    if (m[1]) { fGlobals[target[1] + *(ip + i)].fSigned[1] = v[1]; }
                    if (m[2]) { fGlobals[target[2] + *(ip + i)].fSigned[2] = v[2]; }
                    if (m[3]) { fGlobals[target[3] + *(ip + i)].fSigned[3] = v[3]; }
                }
                ip += count;
                break;
            }

            VECTOR_BINARY_OP(kSubtractI, fSigned, -)
            VECTOR_MATRIX_BINARY_OP(kSubtractF, fFloat, -)

            case ByteCodeInstruction::kSwizzle: {
                VValue tmp[4];
                for (int i = READ8() - 1; i >= 0; --i) {
                    tmp[i] = POP();
                }
                for (int i = READ8() - 1; i >= 0; --i) {
                    PUSH(tmp[READ8()]);
                }
                break;
            }

            VECTOR_UNARY_FN_VEC(kTan, tanf, fFloat)

            case ByteCodeInstruction::kWriteExternal:
            case ByteCodeInstruction::kWriteExternal2:
            case ByteCodeInstruction::kWriteExternal3:
            case ByteCodeInstruction::kWriteExternal4:
                SkDEBUGFAIL("Not implemented"); break;

            case ByteCodeInstruction::kMaskPush:
                mask[1] = mask[0] & POP().fSigned;
                ++mask;
                break;
            case ByteCodeInstruction::kMaskPop:
                --mask;
                break;
            case ByteCodeInstruction::kMaskBlend: {
                int count = READ8();
                int4 m = POP().fSigned;
                for (int i = 0; i < count; ++i) {
                    sp[-count] = skvx::if_then_else(m, sp[-count].fFloat, sp[0].fFloat);
                    --sp;
                }
                break;
            }

            default:
                SkDEBUGFAILF("unsupported instruction %d\n", (int) inst);
        }
#ifdef TRACE
        int stackSize = (int) (sp - stack + 1);
        printf("STACK(%d):", stackSize);
        for (int i = 0; i < stackSize; ++i) {
            printf(" %d(%g)", stack[i].fSigned, stack[i].fFloat);
        }
        printf("\n");
#endif
    }
}

} // namespace

#endif
