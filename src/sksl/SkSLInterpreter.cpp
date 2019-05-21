/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STANDALONE

#include "src/core/SkRasterPipeline.h"
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
        const int nvalues = p->fType.columns()
                          * p->fType.rows();
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

#define VECTOR_DISASSEMBLE(op, text)                               \
    case ByteCodeInstruction::op: printf(text); break;        \
    case ByteCodeInstruction::op##2: printf(text "2"); break; \
    case ByteCodeInstruction::op##3: printf(text "3"); break; \
    case ByteCodeInstruction::op##4: printf(text "4"); break;

void Interpreter::disassemble(const ByteCodeFunction& f) {
    const uint8_t* ip = f.fCode.data();
    while (ip < f.fCode.data() + f.fCode.size()) {
        printf("%d: ", (int) (ip - f.fCode.data()));
        switch ((ByteCodeInstruction) READ16()) {
            VECTOR_DISASSEMBLE(kAddF, "addf")
            VECTOR_DISASSEMBLE(kAddI, "addi")
            case ByteCodeInstruction::kAndB: printf("andb"); break;
            case ByteCodeInstruction::kAndI: printf("andi"); break;
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
            VECTOR_DISASSEMBLE(kCompareFEQ, "comparefeq")
            VECTOR_DISASSEMBLE(kCompareFNEQ, "comparefneq")
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
            VECTOR_DISASSEMBLE(kDivideF, "dividef")
            VECTOR_DISASSEMBLE(kDivideS, "divideS")
            VECTOR_DISASSEMBLE(kDivideU, "divideu")
            VECTOR_DISASSEMBLE(kDup, "dup")
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
            VECTOR_DISASSEMBLE(kMultiplyF, "multiplyf")
            VECTOR_DISASSEMBLE(kMultiplyI, "multiplyi")
            VECTOR_DISASSEMBLE(kNegateF, "negatef")
            VECTOR_DISASSEMBLE(kNegateI, "negatei")
            VECTOR_DISASSEMBLE(kNot, "not")
            VECTOR_DISASSEMBLE(kOrB, "orb")
            VECTOR_DISASSEMBLE(kOrI, "ori")
            VECTOR_DISASSEMBLE(kPop, "pop")
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
            VECTOR_DISASSEMBLE(kSubtractF, "subtractf")
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
            default: printf("unknown(%d)\n", *(ip - 1)); SkASSERT(false);
        }
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

#define VECTOR_BINARY_FN(base, field, fn)                             \
    case ByteCodeInstruction::base ## 4:                              \
        sp[-4] = fn(sp[-4].field, sp[0].field);                       \
        POP();                                                        \
        /* fall through */                                            \
    case ByteCodeInstruction::base ## 3: {                            \
        int count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count] = fn(sp[count].field, sp[0].field);                 \
        POP();                                                        \
    }   /* fall through */                                            \
    case ByteCodeInstruction::base ## 2: {                            \
        int count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count] = fn(sp[count].field, sp[0].field);                 \
        POP();                                                        \
    }   /* fall through */                                            \
    case ByteCodeInstruction::base: {                                 \
        int count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count] = fn(sp[count].field, sp[0].field);                 \
        POP();                                                        \
        break;                                                        \
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

void Interpreter::innerRun(const ByteCodeFunction& f, Value* stack, Value* outReturn) {
    Value* sp = stack + f.fParameterCount + f.fLocalCount - 1;

    auto POP =  [&]          { SkASSERT(sp     >= stack); return *(sp--); };
    auto PUSH = [&](Value v) { SkASSERT(sp + 1 >= stack); *(++sp) = v;    };

    const uint8_t* code = f.fCode.data();
    const uint8_t* ip = code;
    std::vector<StackFrame> frames;

    for (;;) {
        ByteCodeInstruction inst = (ByteCodeInstruction) READ16();
#ifdef TRACE
        printf("at %d\n", (int) (ip - code - 1));
#endif
        switch (inst) {
            VECTOR_BINARY_OP(kAddI, fSigned, +)
            VECTOR_BINARY_OP(kAddF, fFloat, +)

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
            VECTOR_BINARY_OP(kCompareFEQ, fFloat, ==)
            VECTOR_BINARY_OP(kCompareINEQ, fSigned, !=)
            VECTOR_BINARY_OP(kCompareFNEQ, fFloat, !=)
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

            case ByteCodeInstruction::kDebugPrint: {
                Value v = POP();
                printf("Debug: %d(int), %d(uint), %f(float)\n", v.fSigned, v.fUnsigned, v.fFloat);
                break;
            }

            VECTOR_BINARY_OP(kDivideS, fSigned, /)
            VECTOR_BINARY_OP(kDivideU, fUnsigned, /)
            VECTOR_BINARY_OP(kDivideF, fFloat, /)

            case ByteCodeInstruction::kDup4: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup3: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup2: PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
            case ByteCodeInstruction::kDup : PUSH(sp[(int)ByteCodeInstruction::kDup - (int)inst]);
                                             break;

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
                                                    sp += (int)inst - (int)ByteCodeInstruction::kLoadGlobal + 1;
                                                    break;

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

            VECTOR_BINARY_OP(kMultiplyI, fSigned, *)
            VECTOR_BINARY_OP(kMultiplyF, fFloat, *)

            case ByteCodeInstruction::kNot:
                sp[0].fBool = !sp[0].fBool;
                break;

            case ByteCodeInstruction::kNegateF4: sp[-3] = -sp[-3].fFloat;
            case ByteCodeInstruction::kNegateF3: sp[-2] = -sp[-2].fFloat;
            case ByteCodeInstruction::kNegateF2: sp[-1] = -sp[-1].fFloat;
            case ByteCodeInstruction::kNegateF : sp[ 0] = -sp[ 0].fFloat;
                                                 break;

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
            VECTOR_BINARY_OP(kRemainderU,  fUnsigned, %)

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

            VECTOR_BINARY_OP(kSubtractI, fSigned, -)
            VECTOR_BINARY_OP(kSubtractF, fFloat, -)

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
            printf(" %d(%f)", stack[i].fSigned, stack[i].fFloat);
        }
        printf("\n");
#endif
    }
}

} // namespace

#endif
