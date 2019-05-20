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

#define TOP() (SkASSERT(sp >= stack), *sp)

#define POP() (SkASSERT(sp >= stack), *(sp--))

#define PUSH(v) (SkASSERT(sp + 1 >= stack), *(++sp) = v)

#define READ8() (*(ip++))

template <typename T>
static T unaligned_load(const void* ptr) {
    T val;
    memcpy(&val, ptr, sizeof(val));
    return val;
}

#define READ16() (ip += 2, unaligned_load<uint16_t>(ip - 2))
#define READ32() (ip += 4, unaligned_load<uint32_t>(ip - 4))

static String value_string(uint32_t v) {
    union { uint32_t u; float f; } pun = { v };
    return to_string(v) + "(" + to_string(pun.f) + ")";
}

#define VECTOR_DISASSEMBLE(op, text)                               \
    case ByteCodeInstruction::op: printf(text); break;        \
    case ByteCodeInstruction::op##2: printf(text "2"); break; \
    case ByteCodeInstruction::op##3: printf(text "3"); break; \
    case ByteCodeInstruction::op##4: printf(text "4"); break;

void Interpreter::disassemble(const ByteCodeFunction& f) {
    const uint8_t* ip = f.fCode.data();
    while (ip < f.fCode.data() + f.fCode.size()) {
        printf("%d: ", (int) (ip - f.fCode.data()));
        switch ((ByteCodeInstruction) READ8()) {
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
            case ByteCodeInstruction::kDebugPrint: printf("debugprint"); break;
            VECTOR_DISASSEMBLE(kDivideF, "dividef")
            VECTOR_DISASSEMBLE(kDivideS, "divideS")
            VECTOR_DISASSEMBLE(kDivideU, "divideu")
            VECTOR_DISASSEMBLE(kDup, "dup")
            VECTOR_DISASSEMBLE(kFloatToInt, "floattoint")
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
            VECTOR_DISASSEMBLE(kNegateS, "negates")
            VECTOR_DISASSEMBLE(kNot, "not")
            VECTOR_DISASSEMBLE(kOrB, "orb")
            VECTOR_DISASSEMBLE(kOrI, "ori")
            VECTOR_DISASSEMBLE(kPop, "pop")
            case ByteCodeInstruction::kPushImmediate:
                printf("pushimmediate %s", value_string(READ32()).c_str());
                break;
            case ByteCodeInstruction::kReadExternal: printf("readexternal %d", READ8()); break;
            case ByteCodeInstruction::kReadExternal2: printf("readexternal2 %d", READ8()); break;
            case ByteCodeInstruction::kReadExternal3: printf("readexternal3 %d", READ8()); break;
            case ByteCodeInstruction::kReadExternal4: printf("readexternal4 %d", READ8()); break;
            VECTOR_DISASSEMBLE(kRemainderF, "remainderf")
            VECTOR_DISASSEMBLE(kRemainderS, "remainders")
            VECTOR_DISASSEMBLE(kRemainderU, "remainderu")
            case ByteCodeInstruction::kReturn: printf("return %d", READ8()); break;
            VECTOR_DISASSEMBLE(kSignedToFloat, "signedtofloat")
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
            VECTOR_DISASSEMBLE(kUnsignedToFloat, "unsignedtofloat")
            case ByteCodeInstruction::kWriteExternal: printf("writeexternal %d", READ8()); break;
            case ByteCodeInstruction::kWriteExternal2: printf("writeexternal2 %d", READ8()); break;
            case ByteCodeInstruction::kWriteExternal3: printf("writeexternal3 %d", READ8()); break;
            case ByteCodeInstruction::kWriteExternal4: printf("writeexternal4 %d", READ8()); break;
            default: printf("unknown(%d)\n", *(ip - 1)); SkASSERT(false);
        }
        printf("\n");
    }
}

#define VECTOR_BINARY_OP(base, type, src, target, op)             \
    case ByteCodeInstruction::base ## 4:                          \
        sp[-4].target = sp[-4].src op sp[0].src;                  \
        POP();                                                    \
        /* fall through */                                        \
    case ByteCodeInstruction::base ## 3:                          \
        count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count].target = sp[count].src op sp[0].src;            \
        POP();                                                    \
        /* fall through */                                        \
    case ByteCodeInstruction::base ## 2:                          \
        count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count].target = sp[count].src op sp[0].src;            \
        POP();                                                    \
        /* fall through */                                        \
    case ByteCodeInstruction::base:                               \
        count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count].target = sp[count].src op sp[0].src;            \
        POP();                                                    \
        break;

#define VECTOR_BINARY_FN(base, type, src, target, fn)             \
    case ByteCodeInstruction::base ## 4:                          \
        sp[-4].target = fn(sp[-4].src, sp[0].src);                \
        POP();                                                    \
        /* fall through */                                        \
    case ByteCodeInstruction::base ## 3:                          \
        count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count].target = fn(sp[count].src, sp[0].src);          \
        POP();                                                    \
        /* fall through */                                        \
    case ByteCodeInstruction::base ## 2:                          \
        count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count].target = fn(sp[count].src, sp[0].src);          \
        POP();                                                    \
        /* fall through */                                        \
    case ByteCodeInstruction::base:                               \
        count = (int) ByteCodeInstruction::base - (int) inst - 1; \
        sp[count].target = fn(sp[count].src, sp[0].src);          \
        POP();                                                    \
        break;

struct StackFrame {
    const uint8_t* fCode;
    const uint8_t* fIP;
    Interpreter::Value* fStack;
};

void Interpreter::run(const ByteCodeFunction& f, Value* stack, Value args[], Value* outReturn) {
    const uint8_t* code = f.fCode.data();
    const uint8_t* ip = code;
    static constexpr int TMP_SLOTS = 4;
    Value tmp[TMP_SLOTS];
    int src;
    int target;
    int count;
    if (f.fParameterCount) {
        memcpy(stack, args, f.fParameterCount * sizeof(Value));
    }
    Value* sp = stack + f.fParameterCount + f.fLocalCount - 1;
    std::vector<StackFrame> frames;

    for (;;) {
        ByteCodeInstruction inst = (ByteCodeInstruction) READ8();
#ifdef TRACE
        printf("at %d\n", (int) (ip - code - 1));
#endif
        switch (inst) {
            VECTOR_BINARY_OP(kAddI, int32_t, fSigned, fSigned, +)
            VECTOR_BINARY_OP(kAddF, float, fFloat, fFloat, +)
            case ByteCodeInstruction::kBranch:
                ip = code + READ16();
                break;
            case ByteCodeInstruction::kCall: {
                // Precursor code has pushed all parameters to the stack. Update our bottom of
                // stack to point at the first parameter, and our sp to point past those parameters
                // (plus space for locals).
                target = READ8();
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
                SkASSERT(returnCount <= TMP_SLOTS);
                target = READ8();
                ExternalValue* v = fByteCode->fExternalValues[target];
                sp -= argumentCount - 1;
                v->call(sp, tmp);
                memcpy(sp, tmp, returnCount * sizeof(Value));
                sp += returnCount - 1;
                break;
            }
            VECTOR_BINARY_OP(kCompareIEQ, int32_t, fSigned, fBool, ==)
            VECTOR_BINARY_OP(kCompareFEQ, float, fFloat, fBool, ==)
            VECTOR_BINARY_OP(kCompareINEQ, int32_t, fSigned, fBool, !=)
            VECTOR_BINARY_OP(kCompareFNEQ, float, fFloat, fBool, !=)
            VECTOR_BINARY_OP(kCompareSGT, int32_t, fSigned, fBool, >)
            VECTOR_BINARY_OP(kCompareUGT, uint32_t, fUnsigned, fBool, >)
            VECTOR_BINARY_OP(kCompareFGT, float, fFloat, fBool, >)
            VECTOR_BINARY_OP(kCompareSGTEQ, int32_t, fSigned, fBool, >=)
            VECTOR_BINARY_OP(kCompareUGTEQ, uint32_t, fUnsigned, fBool, >=)
            VECTOR_BINARY_OP(kCompareFGTEQ, float, fFloat, fBool, >=)
            VECTOR_BINARY_OP(kCompareSLT, int32_t, fSigned, fBool, <)
            VECTOR_BINARY_OP(kCompareULT, uint32_t, fUnsigned, fBool, <)
            VECTOR_BINARY_OP(kCompareFLT, float, fFloat, fBool, <)
            VECTOR_BINARY_OP(kCompareSLTEQ, int32_t, fSigned, fBool, <=)
            VECTOR_BINARY_OP(kCompareULTEQ, uint32_t, fUnsigned, fBool, <=)
            VECTOR_BINARY_OP(kCompareFLTEQ, float, fFloat, fBool, <=)
            case ByteCodeInstruction::kConditionalBranch:
                tmp[0].fSigned = READ16(); // target
                if (POP().fBool) {
                    ip = code + tmp[0].fSigned;
                }
                break;
            case ByteCodeInstruction::kDebugPrint:
                tmp[0] = POP();
                printf("Debug: %d(int), %d(uint), %f(float)\n", tmp[0].fSigned, tmp[0].fUnsigned,
                       tmp[0].fFloat);
                break;
            VECTOR_BINARY_OP(kDivideS, int32_t, fSigned, fSigned, /)
            VECTOR_BINARY_OP(kDivideU, uint32_t, fUnsigned, fUnsigned, /)
            VECTOR_BINARY_OP(kDivideF, float, fFloat, fFloat, /)
            case ByteCodeInstruction::kDup4:
                sp[1] = sp[-3];
                ++sp;
                // fall through
            case ByteCodeInstruction::kDup3:
                count = (int) ByteCodeInstruction::kDup - (int) inst;
                sp[1] = sp[count];
                ++sp;
                // fall through
            case ByteCodeInstruction::kDup2:
                count = (int) ByteCodeInstruction::kDup - (int) inst;
                sp[1] = sp[count];
                ++sp;
                // fall through
            case ByteCodeInstruction::kDup:
                count = (int) ByteCodeInstruction::kDup - (int) inst;
                sp[1] = sp[count];
                ++sp;
                break;
            case ByteCodeInstruction::kFloatToInt4:
                sp[-3].fSigned = (int) sp[-3].fFloat;
                // fall through
            case ByteCodeInstruction::kFloatToInt3:
                sp[-2].fSigned = (int) sp[-2].fFloat;
                // fall through
            case ByteCodeInstruction::kFloatToInt2:
                sp[-1].fSigned = (int) sp[-1].fFloat;
                // fall through
            case ByteCodeInstruction::kFloatToInt:
                sp[0].fSigned = (int) sp[0].fFloat;
                break;
            case ByteCodeInstruction::kSignedToFloat4:
                sp[-3].fFloat = (int) sp[-3].fSigned;
                // fall through
            case ByteCodeInstruction::kSignedToFloat3:
                sp[-2].fFloat = (int) sp[-2].fSigned;
                // fall through
            case ByteCodeInstruction::kSignedToFloat2:
                sp[-1].fFloat = (int) sp[-1].fSigned;
                // fall through
            case ByteCodeInstruction::kSignedToFloat:
                sp[0].fFloat = (int) sp[0].fSigned;
                break;
            case ByteCodeInstruction::kUnsignedToFloat4:
                sp[-3].fFloat = (int) sp[-3].fUnsigned;
                // fall through
            case ByteCodeInstruction::kUnsignedToFloat3:
                sp[-2].fFloat = (int) sp[-2].fUnsigned;
                // fall through
            case ByteCodeInstruction::kUnsignedToFloat2:
                sp[-1].fFloat = (int) sp[-1].fUnsigned;
                // fall through
            case ByteCodeInstruction::kUnsignedToFloat:
                sp[0].fFloat = (int) sp[0].fUnsigned;
                break;
            case ByteCodeInstruction::kLoad4:
                sp[4] = stack[*ip + 3];
                // fall through
            case ByteCodeInstruction::kLoad3:
                sp[3] = stack[*ip + 2];
                // fall through
            case ByteCodeInstruction::kLoad2:
                sp[2] = stack[*ip + 1];
                // fall through
            case ByteCodeInstruction::kLoad:
                sp[1] = stack[*ip];
                ++ip;
                sp += (int) inst - (int) ByteCodeInstruction::kLoad + 1;
                break;
            case ByteCodeInstruction::kLoadGlobal4:
                sp[4] = fGlobals[*ip + 3];
                // fall through
            case ByteCodeInstruction::kLoadGlobal3:
                sp[3] = fGlobals[*ip + 2];
                // fall through
            case ByteCodeInstruction::kLoadGlobal2:
                sp[2] = fGlobals[*ip + 1];
                // fall through
            case ByteCodeInstruction::kLoadGlobal:
                sp[1] = fGlobals[*ip];
                ++ip;
                sp += (int) inst - (int) ByteCodeInstruction::kLoadGlobal + 1;
                break;
            case ByteCodeInstruction::kLoadSwizzle:
                src = READ8();
                count = READ8();
                for (int i = 0; i < count; ++i) {
                    PUSH(stack[src + *(ip + i)]);
                }
                ip += count;
                break;
            case ByteCodeInstruction::kLoadSwizzleGlobal:
                src = READ8();
                SkASSERT(src < (int) fGlobals.size());
                count = READ8();
                for (int i = 0; i < count; ++i) {
                    PUSH(fGlobals[src + *(ip + i)]);
                }
                ip += count;
                break;
            VECTOR_BINARY_OP(kMultiplyI, int32_t, fSigned, fSigned, *)
            VECTOR_BINARY_OP(kMultiplyF, float, fFloat, fFloat, *)
            case ByteCodeInstruction::kNot:
                TOP().fBool = !TOP().fBool;
                break;
            case ByteCodeInstruction::kNegateF4:
                sp[-3].fFloat = -sp[-3].fFloat;
                // fall through
            case ByteCodeInstruction::kNegateF3:
                sp[-2].fFloat = -sp[-2].fFloat;
                // fall through
            case ByteCodeInstruction::kNegateF2:
                sp[-1].fFloat = -sp[-1].fFloat;
                // fall through
            case ByteCodeInstruction::kNegateF:
                sp[0].fFloat = -sp[0].fFloat;
                break;
            case ByteCodeInstruction::kNegateS4:
                sp[-3].fSigned = -sp[-3].fSigned;
                // fall through
            case ByteCodeInstruction::kNegateS3:
                sp[-2].fSigned = -sp[-2].fSigned;
                // fall through
            case ByteCodeInstruction::kNegateS2:
                sp[-1].fSigned = -sp[-1].fSigned;
                // fall through
            case ByteCodeInstruction::kNegateS:
                sp[0].fSigned = -sp[0].fSigned;
                break;
            case ByteCodeInstruction::kPop:
                POP();
                break;
            case ByteCodeInstruction::kPop2:
                sp -= 2;
                break;
            case ByteCodeInstruction::kPop3:
                sp -= 3;
                break;
            case ByteCodeInstruction::kPop4:
                sp -= 4;
                break;
            case ByteCodeInstruction::kPushImmediate:
                PUSH(Value((int) READ32()));
                break;
            case ByteCodeInstruction::kReadExternal:  // fall through
            case ByteCodeInstruction::kReadExternal2: // fall through
            case ByteCodeInstruction::kReadExternal3: // fall through
            case ByteCodeInstruction::kReadExternal4:
                count = (int) inst - (int) ByteCodeInstruction::kReadExternal + 1;
                src = READ8();
                fByteCode->fExternalValues[src]->read(sp + 1);
                sp += count;
                break;
            VECTOR_BINARY_FN(kRemainderF, int32_t, fFloat, fFloat, fmodf)
            VECTOR_BINARY_OP(kRemainderS, int32_t, fSigned, fSigned, %)
            VECTOR_BINARY_OP(kRemainderU, uint32_t, fUnsigned, fUnsigned, %)
            case ByteCodeInstruction::kReturn:
                count = READ8();
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
            case ByteCodeInstruction::kStore4:
                stack[*ip + 3] = sp[0];
                POP();
                // fall through
            case ByteCodeInstruction::kStore3:
                stack[*ip + 2] = sp[0];
                POP();
                // fall through
            case ByteCodeInstruction::kStore2:
                stack[*ip + 1] = sp[0];
                POP();
            case ByteCodeInstruction::kStore:
                stack[*ip] = sp[0];
                POP();
                ++ip;
                break;
            case ByteCodeInstruction::kStoreGlobal4:
                fGlobals[*ip + 3] = sp[0];
                POP();
                // fall through
            case ByteCodeInstruction::kStoreGlobal3:
                fGlobals[*ip + 2] = sp[0];
                POP();
                // fall through
            case ByteCodeInstruction::kStoreGlobal2:
                fGlobals[*ip + 1] = sp[0];
                POP();
            case ByteCodeInstruction::kStoreGlobal:
                fGlobals[*ip] = sp[0];
                POP();
                ++ip;
                break;
            case ByteCodeInstruction::kStoreSwizzle:
                target = READ8();
                count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    stack[target + *(ip + i)] = POP();
                }
                ip += count;
                break;
            case ByteCodeInstruction::kStoreSwizzleGlobal:
                target = READ8();
                count = READ8();
                for (int i = count - 1; i >= 0; --i) {
                    fGlobals[target + *(ip + i)] = POP();
                }
                ip += count;
                break;
            VECTOR_BINARY_OP(kSubtractI, int32_t, fSigned, fSigned, -)
            VECTOR_BINARY_OP(kSubtractF, float, fFloat, fFloat, -)
            case ByteCodeInstruction::kSwizzle:
                for (int i = READ8() - 1; i >= 0; --i) {
                    tmp[i] = POP();
                }
                for (int i = READ8() - 1; i >= 0; --i) {
                    PUSH(tmp[READ8()]);
                }
                break;
            case ByteCodeInstruction::kWriteExternal:  // fall through
            case ByteCodeInstruction::kWriteExternal2: // fall through
            case ByteCodeInstruction::kWriteExternal3: // fall through
            case ByteCodeInstruction::kWriteExternal4:
                count = (int) inst - (int) ByteCodeInstruction::kWriteExternal + 1;
                target = READ8();
                fByteCode->fExternalValues[target]->write(sp - count + 1);
                sp -= count;
                break;
            default:
                printf("unsupported instruction %d\n", (int) inst);
                SkASSERT(false);
        }
#ifdef TRACE
        printf("STACK(%d):", STACK_SIZE());
        for (int i = 0; i < STACK_SIZE(); ++i) {
            printf(" %d(%f)", stack[i].fSigned, stack[i].fFloat);
        }
        printf("\n");
#endif
    }
}

} // namespace

#endif
