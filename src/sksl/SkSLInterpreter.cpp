/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STANDALONE

#include "src/core/SkRasterPipeline.h"
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

Interpreter::Interpreter(std::unique_ptr<Program> program, std::unique_ptr<ByteCode> byteCode,
                         Interpreter::Value inputs[])
    : fProgram(std::move(program))
    , fByteCode(std::move(byteCode)) {
    for (int i = 0; i < fByteCode->fGlobalCount; ++i) {
        fGlobals.push_back(Value((int) UNINITIALIZED));
    }
    this->setInputs(inputs);
}

void Interpreter::setInputs(Interpreter::Value inputs[]) {
    for (int i = fByteCode->fInputSlots.size() - 1; i >= 0; --i) {
        fGlobals[fByteCode->fInputSlots[i]] = inputs[i];
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
    run(f, stack, args, outReturn);
    int offset = 0;
    for (const auto& p : f.fDeclaration.fParameters) {
        if (p->fModifiers.fFlags & Modifiers::kOut_Flag) {
            for (int i = p->fType.columns() * p->fType.rows() - 1; i >= 0; --i) {
                args[offset] = stack[offset];
                ++offset;
            }
        } else {
            offset += p->fType.columns() * p->fType.rows();
        }
    }
}

struct CallbackCtx : public SkRasterPipeline_CallbackCtx {
    Interpreter* fInterpreter;
    const FunctionDefinition* fFunction;
};

#define STACK_SIZE() (int) (sp - stack + 1)

#define TOP() (*sp)

#define POP() (*(sp--))

#define PUSH(v) (*(++sp) = v)

#define READ8() (*(ip++))

#define READ16()                                                  \
    (SkASSERT((intptr_t) ip % 2 == 0),                            \
     ip += 2,                                                     \
     *(uint16_t*) (ip - 2))

#define READ32()                                                  \
    (SkASSERT((intptr_t) ip % 4 == 0),                            \
     ip += 4,                                                     \
     *(uint32_t*) (ip - 4))

static String value_string(uint32_t v) {
    union { uint32_t u; float f; } pun = { v };
    return to_string(v) + "(" + to_string(pun.f) + ")";
}

void Interpreter::disassemble(const ByteCodeFunction& f) {
    const uint8_t* ip = f.fCode.data();
    while (ip < f.fCode.data() + f.fCode.size()) {
        printf("%d: ", (int) (ip - f.fCode.data()));
        switch ((ByteCodeInstruction) READ8()) {
            case ByteCodeInstruction::kAddF: printf("addf"); break;
            case ByteCodeInstruction::kAddI: printf("addi"); break;
            case ByteCodeInstruction::kAndB: printf("andb"); break;
            case ByteCodeInstruction::kAndI: printf("andi"); break;
            case ByteCodeInstruction::kBranch: printf("branch %d", READ16()); break;
            case ByteCodeInstruction::kCall: printf("call %d", READ8()); break;
            case ByteCodeInstruction::kCompareIEQ: printf("comparei eq"); break;
            case ByteCodeInstruction::kCompareINEQ: printf("comparei neq"); break;
            case ByteCodeInstruction::kCompareFEQ: printf("comparef eq"); break;
            case ByteCodeInstruction::kCompareFGT: printf("comparef gt"); break;
            case ByteCodeInstruction::kCompareFGTEQ: printf("comparef gteq"); break;
            case ByteCodeInstruction::kCompareFLT: printf("comparef lt"); break;
            case ByteCodeInstruction::kCompareFLTEQ: printf("comparef lteq"); break;
            case ByteCodeInstruction::kCompareFNEQ: printf("comparef neq"); break;
            case ByteCodeInstruction::kCompareSGT: printf("compares sgt"); break;
            case ByteCodeInstruction::kCompareSGTEQ: printf("compares sgteq"); break;
            case ByteCodeInstruction::kCompareSLT: printf("compares lt"); break;
            case ByteCodeInstruction::kCompareSLTEQ: printf("compares lteq"); break;
            case ByteCodeInstruction::kCompareUGT: printf("compareu gt"); break;
            case ByteCodeInstruction::kCompareUGTEQ: printf("compareu gteq"); break;
            case ByteCodeInstruction::kCompareULT: printf("compareu lt"); break;
            case ByteCodeInstruction::kCompareULTEQ: printf("compareu lteq"); break;
            case ByteCodeInstruction::kConditionalBranch:
                printf("conditionalbranch %d", READ16());
                break;
            case ByteCodeInstruction::kDebugPrint: printf("debugprint"); break;
            case ByteCodeInstruction::kDivideF: printf("dividef"); break;
            case ByteCodeInstruction::kDivideS: printf("divides"); break;
            case ByteCodeInstruction::kDivideU: printf("divideu"); break;
            case ByteCodeInstruction::kDup: printf("dup"); break;
            case ByteCodeInstruction::kDupDown: printf("dupdown %d", READ8()); break;
            case ByteCodeInstruction::kFloatToInt: printf("floattoint"); break;
            case ByteCodeInstruction::kLoad: printf("load"); break;
            case ByteCodeInstruction::kLoadGlobal: printf("loadglobal %d", READ8()); break;
            case ByteCodeInstruction::kLoadSwizzle: {
                int count = READ8();
                printf("loadswizzle %d", count);
                for (int i = 0; i < count; ++i) {
                    printf(", %d", READ8());
                }
                break;
            }
            case ByteCodeInstruction::kMultiplyF: printf("multiplyf"); break;
            case ByteCodeInstruction::kMultiplyS: printf("multiplys"); break;
            case ByteCodeInstruction::kMultiplyU: printf("multiplyu"); break;
            case ByteCodeInstruction::kNegateF: printf("negatef"); break;
            case ByteCodeInstruction::kNegateS: printf("negates"); break;
            case ByteCodeInstruction::kNop1: printf("nop1"); break;
            case ByteCodeInstruction::kNop2: printf("nop2"); break;
            case ByteCodeInstruction::kNop3: printf("nop3"); break;
            case ByteCodeInstruction::kNot: printf("not"); break;
            case ByteCodeInstruction::kOrB: printf("orb"); break;
            case ByteCodeInstruction::kOrI: printf("ori"); break;
            case ByteCodeInstruction::kParameter: printf("parameter"); break;
            case ByteCodeInstruction::kPop: printf("pop %d", READ8()); break;
            case ByteCodeInstruction::kPushImmediate:
                printf("pushimmediate %s", value_string(READ32()).c_str());
                break;
            case ByteCodeInstruction::kRemainderF: printf("remainderf"); break;
            case ByteCodeInstruction::kRemainderS: printf("remainders"); break;
            case ByteCodeInstruction::kRemainderU: printf("remainderu"); break;
            case ByteCodeInstruction::kReturn: printf("return %d", READ8()); break;
            case ByteCodeInstruction::kSignedToFloat: printf("signedtofloat"); break;
            case ByteCodeInstruction::kStore: printf("store"); break;
            case ByteCodeInstruction::kStoreGlobal: printf("storeglobal"); break;
            case ByteCodeInstruction::kStoreSwizzle: {
                int count = READ8();
                printf("storeswizzle %d", count);
                for (int i = 0; i < count; ++i) {
                    printf(", %d", READ8());
                }
                break;
            }
            case ByteCodeInstruction::kSubtractF: printf("subtractf"); break;
            case ByteCodeInstruction::kSubtractI: printf("subtracti"); break;
            case ByteCodeInstruction::kSwizzle: {
                printf("swizzle %d, ", READ8());
                int count = READ8();
                printf("%d", count);
                for (int i = 0; i < count; ++i) {
                    printf(", %d", READ8());
                }
                break;
            }
            case ByteCodeInstruction::kUnsignedToFloat: printf("unsignedtofloat"); break;
            case ByteCodeInstruction::kVector: printf("vector%d", READ8()); break;
            default: printf("unknown(%d)\n", *(ip - 1)); SkASSERT(false);
        }
        printf("\n");
    }
}

#define BINARY_OP(inst, type, field, op) \
    case ByteCodeInstruction::inst: {    \
        type b = POP().field;      \
        Value* a = &TOP();       \
        *a = Value(a->field op b);       \
        break;                           \
    }

static constexpr int VECTOR_MAX = 16;

#define VECTOR_BINARY_OP(inst, type, field, op)               \
    case ByteCodeInstruction::inst: {                         \
        Value result[VECTOR_MAX];                             \
        for (int i = count - 1; i >= 0; --i) {                \
            result[i] = POP();                                \
        }                                                     \
        for (int i = count - 1; i >= 0; --i) {                \
            result[i] = POP().field op result[i].field;       \
        }                                                     \
        for (int i = 0; i < count; ++i) {                     \
            PUSH(result[i]);                                  \
        }                                                     \
        break;                                                \
    }

struct StackFrame {
    const uint8_t* fCode;
    const uint8_t* fIP;
    Interpreter::Value* fStack;
};

void Interpreter::run(const ByteCodeFunction& f, Value* stack, Value args[], Value* outReturn) {
    const uint8_t* code = f.fCode.data();
    const uint8_t* ip = code;
    memcpy(stack, args, f.fParameterCount * sizeof(Value));
    Value* sp = stack + f.fParameterCount + f.fLocalCount - 1;
    std::vector<StackFrame> frames;

    for (;;) {
        ByteCodeInstruction inst = (ByteCodeInstruction) READ8();
#ifdef TRACE
        printf("at %d\n", (int) (ip - fCurrentFunction->fCode.data() - 1));
#endif
        switch (inst) {
            BINARY_OP(kAddI, int32_t, fSigned, +)
            BINARY_OP(kAddF, float, fFloat, +)
            case ByteCodeInstruction::kBranch: {
                ip = code + READ16();
                break;
            }
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
            BINARY_OP(kCompareIEQ, int32_t, fSigned, ==)
            BINARY_OP(kCompareFEQ, float, fFloat, ==)
            BINARY_OP(kCompareINEQ, int32_t, fSigned, !=)
            BINARY_OP(kCompareFNEQ, float, fFloat, !=)
            BINARY_OP(kCompareSGT, int32_t, fSigned, >)
            BINARY_OP(kCompareUGT, uint32_t, fUnsigned, >)
            BINARY_OP(kCompareFGT, float, fFloat, >)
            BINARY_OP(kCompareSGTEQ, int32_t, fSigned, >=)
            BINARY_OP(kCompareUGTEQ, uint32_t, fUnsigned, >=)
            BINARY_OP(kCompareFGTEQ, float, fFloat, >=)
            BINARY_OP(kCompareSLT, int32_t, fSigned, <)
            BINARY_OP(kCompareULT, uint32_t, fUnsigned, <)
            BINARY_OP(kCompareFLT, float, fFloat, <)
            BINARY_OP(kCompareSLTEQ, int32_t, fSigned, <=)
            BINARY_OP(kCompareULTEQ, uint32_t, fUnsigned, <=)
            BINARY_OP(kCompareFLTEQ, float, fFloat, <=)
            case ByteCodeInstruction::kConditionalBranch: {
                int target = READ16();
                if (POP().fBool) {
                    ip = code + target;
                }
                break;
            }
            case ByteCodeInstruction::kDebugPrint: {
                Value v = POP();
                printf("Debug: %d(int), %d(uint), %f(float)\n", v.fSigned, v.fUnsigned, v.fFloat);
                break;
            }
            BINARY_OP(kDivideS, int32_t, fSigned, /)
            BINARY_OP(kDivideU, uint32_t, fUnsigned, /)
            BINARY_OP(kDivideF, float, fFloat, /)
            case ByteCodeInstruction::kDup: {
                Value& top = TOP();
                PUSH(top);
                break;
            }
            case ByteCodeInstruction::kDupDown: {
                int count = READ8();
                // before dupdown 4: X A B C D
                // after dupdown 4:  A B C D X A B C D
                memmove(sp, sp - count, sizeof(Value) * (count + 1));
                sp += count;
                memcpy(sp - count * 2, sp - count + 1, sizeof(Value) * count);
                break;
            }
            case ByteCodeInstruction::kFloatToInt: {
                Value& top = TOP();
                top.fSigned = (int) top.fFloat;
                break;
            }
            case ByteCodeInstruction::kSignedToFloat: {
                Value& top = TOP();
                top.fFloat = (float) top.fSigned;
                break;
            }
            case ByteCodeInstruction::kUnsignedToFloat: {
                Value& top = TOP();
                top.fFloat = (float) top.fUnsigned;
                break;
            }
            case ByteCodeInstruction::kLoad: {
                int target = POP().fSigned;
                SkASSERT(target < STACK_SIZE());
                PUSH(stack[target]);
                break;
            }
            case ByteCodeInstruction::kLoadGlobal: {
                int target = READ8();
                SkASSERT(target < (int) fGlobals.size());
                PUSH(fGlobals[target]);
                break;
            }
            case ByteCodeInstruction::kLoadSwizzle: {
                Value target = POP();
                int count = READ8();
                for (int i = 0; i < count; ++i) {
                    PUSH(stack[target.fSigned + *(ip + i)]);
                }
                ip += count;
                break;
            }
            BINARY_OP(kMultiplyS, int32_t, fSigned, *)
            BINARY_OP(kMultiplyU, uint32_t, fUnsigned, *)
            BINARY_OP(kMultiplyF, float, fFloat, *)
            case ByteCodeInstruction::kNot: {
                Value& top = TOP();
                top.fBool = !top.fBool;
                break;
            }
            case ByteCodeInstruction::kNegateF: {
                Value& top = TOP();
                top.fFloat = -top.fFloat;
                break;
            }
            case ByteCodeInstruction::kNegateS: {
                Value& top = TOP();
                top.fSigned = -top.fSigned;
                break;
            }
            case ByteCodeInstruction::kNop1:
                continue;
            case ByteCodeInstruction::kNop2:
                ++ip;
                continue;
            case ByteCodeInstruction::kNop3:
                ip += 2;
                continue;
            case ByteCodeInstruction::kPop:
                for (int i = READ8(); i > 0; --i) {
                    POP();
                }
                break;
            case ByteCodeInstruction::kPushImmediate:
                PUSH(Value((int) READ32()));
                break;
            case ByteCodeInstruction::kRemainderF: {
                float b = POP().fFloat;
                Value* a = &TOP();
                *a = Value(fmodf(a->fFloat, b));
                break;
            }
            BINARY_OP(kRemainderS, int32_t, fSigned, %)
            BINARY_OP(kRemainderU, uint32_t, fUnsigned, %)
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
            case ByteCodeInstruction::kStore: {
                Value value = POP();
                int target = POP().fSigned;
                SkASSERT(target < STACK_SIZE());
                stack[target] = value;
                break;
            }
            case ByteCodeInstruction::kStoreGlobal: {
                Value value = POP();
                int target = POP().fSigned;
                SkASSERT(target < (int) fGlobals.size());
                fGlobals[target] = value;
                break;
            }
            case ByteCodeInstruction::kStoreSwizzle: {
                int count = READ8();
                int target = (sp - count)->fSigned;
                for (int i = count - 1; i >= 0; --i) {
                    stack[target + *(ip + i)] = POP();
                }
                POP();
                ip += count;
                break;
            }
            BINARY_OP(kSubtractI, int32_t, fSigned, -)
            BINARY_OP(kSubtractF, float, fFloat, -)
            case ByteCodeInstruction::kSwizzle: {
                Value vec[4];
                for (int i = READ8() - 1; i >= 0; --i) {
                    vec[i] = POP();
                }
                for (int i = READ8() - 1; i >= 0; --i) {
                    PUSH(vec[READ8()]);
                }
                break;
            }
            case ByteCodeInstruction::kVector: {
                uint8_t count = READ8();
                ByteCodeInstruction inst = (ByteCodeInstruction) READ8();
                switch (inst) {
                    VECTOR_BINARY_OP(kAddI, int32_t, fSigned, +)
                    VECTOR_BINARY_OP(kAddF, float, fFloat, +)
                    case ByteCodeInstruction::kBranch: {
                        ip = code + READ16();
                        break;
                    }
                    VECTOR_BINARY_OP(kCompareIEQ, int32_t, fSigned, ==)
                    VECTOR_BINARY_OP(kCompareFEQ, float, fFloat, ==)
                    VECTOR_BINARY_OP(kCompareINEQ, int32_t, fSigned, !=)
                    VECTOR_BINARY_OP(kCompareFNEQ, float, fFloat, !=)
                    VECTOR_BINARY_OP(kCompareSGT, int32_t, fSigned, >)
                    VECTOR_BINARY_OP(kCompareUGT, uint32_t, fUnsigned, >)
                    VECTOR_BINARY_OP(kCompareFGT, float, fFloat, >)
                    VECTOR_BINARY_OP(kCompareSGTEQ, int32_t, fSigned, >=)
                    VECTOR_BINARY_OP(kCompareUGTEQ, uint32_t, fUnsigned, >=)
                    VECTOR_BINARY_OP(kCompareFGTEQ, float, fFloat, >=)
                    VECTOR_BINARY_OP(kCompareSLT, int32_t, fSigned, <)
                    VECTOR_BINARY_OP(kCompareULT, uint32_t, fUnsigned, <)
                    VECTOR_BINARY_OP(kCompareFLT, float, fFloat, <)
                    VECTOR_BINARY_OP(kCompareSLTEQ, int32_t, fSigned, <=)
                    VECTOR_BINARY_OP(kCompareULTEQ, uint32_t, fUnsigned, <=)
                    VECTOR_BINARY_OP(kCompareFLTEQ, float, fFloat, <=)
                    case ByteCodeInstruction::kConditionalBranch: {
                        uint16_t target = READ16();
                        if (POP().fBool) {
                            ip = code + target;
                        }
                        break;
                    }
                    VECTOR_BINARY_OP(kDivideS, int32_t, fSigned, /)
                    VECTOR_BINARY_OP(kDivideU, uint32_t, fUnsigned, /)
                    VECTOR_BINARY_OP(kDivideF, float, fFloat, /)
                    case ByteCodeInstruction::kFloatToInt: {
                        for (int i = 0; i < count; ++i) {
                            Value& v = sp[-i];
                            v.fSigned = (int) v.fFloat;
                        }
                        break;
                    }
                    case ByteCodeInstruction::kSignedToFloat: {
                        for (int i = 0; i < count; ++i) {
                            Value& v = sp[-i];
                            v.fFloat = (float) v.fSigned;
                        }
                        break;
                    }
                    case ByteCodeInstruction::kUnsignedToFloat: {
                        for (int i = 0; i < count; ++i) {
                            Value& v = sp[-i];
                            v.fFloat = (float) v.fUnsigned;
                        }
                        break;
                    }
                    case ByteCodeInstruction::kLoad: {
                        int src = POP().fSigned;
                        memcpy(sp + 1, &stack[src], count * sizeof(Value));
                        sp += count;
                        break;
                    }
                    case ByteCodeInstruction::kLoadGlobal: {
                        int target = READ8();
                        SkASSERT(target < (int) fGlobals.size());
                        PUSH(fGlobals[target]);
                        break;
                    }
                    case ByteCodeInstruction::kNegateS: {
                        for (int i = 0; i < count; ++i) {
                            Value& v = sp[-i];
                            v.fSigned = -v.fSigned;
                        }
                        break;
                    }
                    case ByteCodeInstruction::kNegateF: {
                        for (int i = 0; i < count; ++i) {
                            Value& v = sp[-i];
                            v.fFloat = -v.fFloat;
                        }
                        break;
                    }
                    VECTOR_BINARY_OP(kMultiplyS, int32_t, fSigned, *)
                    VECTOR_BINARY_OP(kMultiplyU, uint32_t, fUnsigned, *)
                    VECTOR_BINARY_OP(kMultiplyF, float, fFloat, *)
                    case ByteCodeInstruction::kRemainderF: {
                        Value result[VECTOR_MAX];
                        for (int i = count - 1; i >= 0; --i) {
                            result[i] = POP();
                        }
                        for (int i = count - 1; i >= 0; --i) {
                            result[i] = fmodf(POP().fFloat, result[i].fFloat);
                        }
                        for (int i = 0; i < count; ++i) {
                            PUSH(result[i]);
                        }
                        break;
                    }
                    VECTOR_BINARY_OP(kRemainderS, int32_t, fSigned, %)
                    VECTOR_BINARY_OP(kRemainderU, uint32_t, fUnsigned, %)
                    case ByteCodeInstruction::kStore: {
                        memcpy(&stack[(sp - count)->fSigned], sp - count + 1,
                               count * sizeof(Value));
                        sp -= count;
                        break;
                    }
                    VECTOR_BINARY_OP(kSubtractI, int32_t, fSigned, -)
                    VECTOR_BINARY_OP(kSubtractF, float, fFloat, -)
                    default:
                        printf("unsupported instruction %d\n", (int) inst);
                        SkASSERT(false);
                }
                break;
            }
            default:
                printf("unsupported instruction %d\n", (int) inst);
                SkASSERT(false);
        }
#ifdef TRACE
        printf("STACK:");
        for (int i = 0; i < STACK_SIZE(); ++i) {
            printf(" %d(%f)", stack[i].fSigned, stack[i].fFloat);
        }
        printf("\n");
#endif
    }
}

} // namespace

#endif
