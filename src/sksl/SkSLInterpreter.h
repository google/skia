/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkUtils.h" // sk_unaligned_load

#ifndef SKSL_INTERPRETER
#define SKSL_INTERPRETER

namespace SkSL {

template<typename T>
static T read(const uint8_t** ip) {
    *ip += sizeof(T);
    return sk_unaligned_load<T>(*ip - sizeof(T));
}

#define BINARY_OP(inst, src, result, op)                                  \
    case ByteCode::Instruction::inst: {                                   \
        ByteCode::Register target = read<ByteCode::Register>(&ip);        \
        ByteCode::Register src1 = read<ByteCode::Register>(&ip);          \
        ByteCode::Register src2 = read<ByteCode::Register>(&ip);          \
        fRegisters[target.fIndex].result = fRegisters[src1.fIndex].src op \
                                           fRegisters[src2.fIndex].src;   \
        printf("binary " #src " %d(%d | %g), %d(%d | %g) -> %d(%d | %g)\n", src1.fIndex, fRegisters[src1.fIndex].fInt[0], fRegisters[src1.fIndex].fFloat[0], src2.fIndex, fRegisters[src2.fIndex].fInt[0], fRegisters[src2.fIndex].fFloat[0], target.fIndex, fRegisters[target.fIndex].fInt[0], fRegisters[target.fIndex].fFloat[0]); \
        break;                                                            \
    }

#define MASKED_BINARY_OP(inst, src, result, op)                                         \
    case ByteCode::Instruction::inst: {                                                 \
        ByteCode::Register target = read<ByteCode::Register>(&ip);                      \
        ByteCode::Register src1 = read<ByteCode::Register>(&ip);                        \
        ByteCode::Register src2 = read<ByteCode::Register>(&ip);                        \
        auto m = mask();                                                                \
        for (int i = 0; i < width; ++i) {                                               \
            if (m[i]) {                                                                 \
                fRegisters[target.fIndex].result[i] = fRegisters[src1.fIndex].src[i] op \
                                                   fRegisters[src2.fIndex].src[i];      \
            }                                                                           \
        }                                                                               \
        printf("binary " #src " %d(%d | %g), %d(%d | %g) -> %d(%d | %g)\n", src1.fIndex, fRegisters[src1.fIndex].fInt[0], fRegisters[src1.fIndex].fFloat[0], src2.fIndex, fRegisters[src2.fIndex].fInt[0], fRegisters[src2.fIndex].fFloat[0], target.fIndex, fRegisters[target.fIndex].fInt[0], fRegisters[target.fIndex].fFloat[0]); \
        break;                                                                          \
    }

#define VECTOR_UNARY_FN(inst, fn)                                                       \
    case ByteCode::Instruction::inst: {                                                 \
        ByteCode::Register target = read<ByteCode::Register>(&ip);                      \
        ByteCode::Register src = read<ByteCode::Register>(&ip);                         \
        for (int i = 0; i < width; ++ i) {                                              \
            fRegisters[target.fIndex].fFloat[i] = fn(fRegisters[src.fIndex].fFloat[i]); \
        }                                                                               \
        printf(#fn " %d -> %d\n", src.fIndex, target.fIndex);                           \
        break;                                                                          \
    }

/**
 * Parameterized over SkSL::ByteCode::Scalar or SkSL::ByteCode::Vector. Operates on vectors of the
 * specified width, so creating an Interpreter<16> means that all inputs, outputs, and internal
 * calculations will be 16-wide vectors.
 */
template<int width>
class Interpreter {
public:
    using Vector = ByteCode::Vector<width>;
    using VectorI = skvx::Vec<width, int32_t>;

    Interpreter(std::unique_ptr<ByteCode> code)
        : fCode(std::move(code)) {}

    void setUniforms(const Vector uniforms[]) {
        memcpy(&fMemory[fCode->getGlobalCount()], uniforms,
               sizeof(Vector) * fCode->getUniformCount());
        printf("uniforms start at %d\n", fCode->getGlobalCount());
        for (int i = 0; i < 4; ++i) {
            printf("uniform %d: %d\n", i, uniforms[i].fInt[0]);
        }
        for (int i = 0; i < 32; ++i) {
            printf("%d: %d\n", i, fMemory[i].fInt[0]);
        }
    }

    /**
     * Returns a pointer to the first slot of the result. This pointer is only guaranteed to be
     * valid until the next run() call.
     */
    Vector* run(const ByteCodeFunction* f, Vector args[]) {
        SkASSERT(f);
        VectorI maskStack[MASK_STACK_SIZE];
        memset(maskStack, 255, sizeof(VectorI));
        VectorI loopStack[LOOP_STACK_SIZE];
        memset(loopStack, 255, sizeof(VectorI));
        VectorI continueStack[LOOP_STACK_SIZE];
        memset(continueStack, 0, sizeof(VectorI));
        Vector* stack = fMemory + MEMORY_SIZE;
        stack -= f->fStackSlotCount + f->fParameterSlotCount;
        memcpy(stack, args, f->fParameterSlotCount * sizeof(Vector));
        Vector* result = this->innerRun(f, Context{args, stack, maskStack, loopStack,
                                                  continueStack});
        int slot = 0;
        for (const auto& p : f->fParameters) {
            if (p.fIsOutParameter) {
                memcpy(&args[slot], &stack[slot], p.fSlotCount * sizeof(Vector));
            }
            slot += p.fSlotCount;
        }
        return result;
    }

    Vector getRegister(int index) {
        return fRegisters[index];
    }

    Vector getMemory(int address) {
        return fRegisters[address];
    }

private:
    static constexpr size_t REGISTER_COUNT = 1024;

    static constexpr size_t MEMORY_SIZE = 1024;

    static constexpr size_t MASK_STACK_SIZE = 64;

    static constexpr size_t LOOP_STACK_SIZE = 16;

    struct Context {
        Vector* fArgs;
        Vector* fStack;
        VectorI* fMaskStack;
        VectorI* fLoopStack;
        VectorI* fContinueStack;
    };

    static Vector VecMod(Vector x, Vector y) {
        return Vector(x.fFloat - skvx::trunc(x.fFloat / y.fFloat) * y.fFloat);
    }

    void handleCall(const uint8_t** ip, Context context) {
        uint8_t idx = read<uint8_t>(ip);
        ByteCodeFunction* f = fCode->fFunctions[idx].get();
        Vector* oldStack = context.fStack;
        context.fStack -= f->fStackSlotCount + f->fParameterSlotCount;
        ByteCode::Register result = read<ByteCode::Register>(ip);
        Vector* currentArg = context.fStack;
        for (int i = 0; i < f->getParameterCount(); ++i) {
            ByteCode::Register arg = read<ByteCode::Register>(ip);
            int count = f->getParameter(i).fSlotCount;
            memcpy(currentArg, &fRegisters[arg.fIndex], sizeof(Vector) * count);
            currentArg += count;
        }
        Vector* v = this->innerRun(f, context);
        memcpy(&fRegisters[result.fIndex], v, sizeof(Vector) * f->fReturnSlotCount);
        context.fStack = oldStack;
    }

    void handleCallVoid(const uint8_t** ip, Context context) {
        uint8_t idx = read<uint8_t>(ip);
        ByteCodeFunction* f = fCode->fFunctions[idx].get();
        Vector* oldStack = context.fStack;
        context.fStack -= f->fStackSlotCount + f->fParameterSlotCount;
        Vector* currentArg = context.fStack;
        for (int i = 0; i < f->getParameterCount(); ++i) {
            ByteCode::Register arg = read<ByteCode::Register>(ip);
            int count = f->getParameter(i).fSlotCount;
            memcpy(currentArg, &fRegisters[arg.fIndex], sizeof(Vector) * count);
            currentArg += count;
        }
        this->innerRun(f, context);
        context.fStack = oldStack;

    }

    #define CHECK_STACK_BOUNDS(address)                              \
        SkASSERT(context.fStack + address >= fMemory &&              \
                 context.fStack + address <= fMemory + MEMORY_SIZE);

    Vector* innerRun(const ByteCodeFunction* f, Context context) {
        auto mask = [&]() { return *context.fMaskStack & *context.fLoopStack; };
        printf("--- call %s\n", f->fName.c_str());
        const uint8_t* code = f->fCode.data();
        const uint8_t* ip = code;
        for (;;) {
            printf("ip:%d: ", (int) (ip - code));
            ByteCode::Instruction inst = read<ByteCode::Instruction>(&ip);
            switch (inst) {
                BINARY_OP(kAddF, fFloat, fFloat, +)
                BINARY_OP(kAddI, fInt, fInt, +)
                BINARY_OP(kAnd, fInt, fInt, &)
                BINARY_OP(kCompareEQF, fFloat, fInt, ==)
                BINARY_OP(kCompareEQI, fInt, fInt, ==)
                BINARY_OP(kCompareNEQF, fFloat, fInt, !=)
                BINARY_OP(kCompareNEQI, fInt, fInt, !=)
                BINARY_OP(kCompareGTF, fFloat, fInt, >)
                BINARY_OP(kCompareGTS, fInt, fInt, >)
                BINARY_OP(kCompareGTU, fUInt, fUInt, >)
                BINARY_OP(kCompareGTEQF, fFloat, fInt, >=)
                BINARY_OP(kCompareGTEQS, fInt, fInt, >=)
                BINARY_OP(kCompareGTEQU, fUInt, fUInt, >=)
                BINARY_OP(kCompareLTF, fFloat, fInt, <)
                BINARY_OP(kCompareLTS, fInt, fInt, <)
                BINARY_OP(kCompareLTU, fUInt, fUInt, <)
                BINARY_OP(kCompareLTEQF, fFloat, fInt, <=)
                BINARY_OP(kCompareLTEQS, fInt, fInt, <=)
                BINARY_OP(kCompareLTEQU, fUInt, fUInt, <=)
                BINARY_OP(kSubtractF, fFloat, fFloat, -)
                BINARY_OP(kSubtractI, fInt, fInt, -)
                BINARY_OP(kDivideF, fFloat, fFloat, /)
                MASKED_BINARY_OP(kDivideS, fInt, fInt, /)
                MASKED_BINARY_OP(kDivideU, fUInt, fUInt, /)
                MASKED_BINARY_OP(kRemainderS, fInt, fInt, %)
                MASKED_BINARY_OP(kRemainderU, fUInt, fUInt, %)
                BINARY_OP(kMultiplyF, fFloat, fFloat, *)
                BINARY_OP(kMultiplyI, fInt, fInt, *)
                BINARY_OP(kOr, fInt, fInt, |)
                BINARY_OP(kXor, fInt, fInt, ^)
                case ByteCode::Instruction::kComment:
                    printf("Comment %d\n", read<ByteCode::Immediate>(&ip).fInt);
                    break;
                case ByteCode::Instruction::kRemainderF: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src1 = read<ByteCode::Register>(&ip);
                    ByteCode::Register src2 = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = VecMod(fRegisters[src1.fIndex],
                                                       fRegisters[src2.fIndex]);
                    printf("RemainderF %d, %d -> %d\n", src1.fIndex, src2.fIndex, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kBranch: {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    ip = code + target.fAddress;
                    printf("branch\n");
                    break;
                }
                case ByteCode::Instruction::kBranchIfAllFalse: {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    if (!skvx::any(mask())) {
                        printf("branchIfAllFalse taken\n");
                        ip = code + target.fAddress;
                    } else {
                        printf("branchIfAllFalse skipped\n");
                    }
                    break;
                }
                case ByteCode::Instruction::kBreak:
                    *context.fLoopStack &= ~mask();
                    printf("break (%d -> %d)\n", (*context.fMaskStack)[0], (*context.fLoopStack)[0]);
                    break;
                case ByteCode::Instruction::kCall:
                    this->handleCall(&ip, context);
                    break;
                case ByteCode::Instruction::kCallVoid:
                    this->handleCallVoid(&ip, context);
                    break;
                case ByteCode::Instruction::kContinue: {
                    VectorI m = mask();
                    *context.fContinueStack |= m;
                    *context.fLoopStack &= ~m;
                    printf("continue (%d)\n", m[0]);
                    break;
                }
                case ByteCode::Instruction::kCopy: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fInt = fRegisters[src.fIndex].fInt;
                    printf("copy %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                VECTOR_UNARY_FN(kCos, cosf)
                case ByteCode::Instruction::kFloatToSigned: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = Vector(skvx::cast<int32_t>(
                                                       fRegisters[src.fIndex].fFloat));
                    printf("floatToSigned %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kFloatToUnsigned: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = Vector(skvx::cast<uint32_t>(
                                                       fRegisters[src.fIndex].fFloat));
                    printf("floatToUnsigned %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kImmediate: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Immediate src = read<ByteCode::Immediate>(&ip);
                    printf("immediate (%d | %g) -> %d\n", src.fInt, src.fFloat, target.fIndex);
                    fRegisters[target.fIndex].fInt = src.fInt;
                    break;
                }
                case ByteCode::Instruction::kLoad: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    for (int i = 0; i < width; ++i) {
                        fRegisters[target.fIndex].fInt[i] =
                                                    fMemory[fRegisters[src.fIndex].fInt[i]].fInt[0];
                    }
                    printf("load %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kLoadDirect: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Pointer src = read<ByteCode::Pointer>(&ip);
                    fRegisters[target.fIndex].fInt = fMemory[src.fAddress].fInt;
                    printf("loadDirect %d -> %d\n", src.fAddress, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kLoadStack: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    for (int i = 0; i < width; ++i) {
                        fRegisters[target.fIndex].fInt[i] =
                                             context.fStack[fRegisters[src.fIndex].fInt[i]].fInt[i];
                    }
                    printf("loadStack %d -> %d(%d | %g)\n", src.fIndex, target.fIndex, fRegisters[target.fIndex].fInt[0], fRegisters[target.fIndex].fFloat[0]);
                    break;
                }
                case ByteCode::Instruction::kLoadStackDirect: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Pointer src = read<ByteCode::Pointer>(&ip);
                    CHECK_STACK_BOUNDS(src.fAddress);
                    fRegisters[target.fIndex].fInt = context.fStack[src.fAddress].fInt;
                    printf("loadStackDirect %d -> %d(%d | %g)\n", src.fAddress, target.fIndex, fRegisters[target.fIndex].fInt[0], fRegisters[target.fIndex].fFloat[0]);
                    break;
                }
                case ByteCode::Instruction::kLoopBegin: {
                    context.fLoopStack[1] = context.fLoopStack[0];
                    ++context.fLoopStack;
                    context.fContinueStack[1] = 0;
                    ++context.fContinueStack;
                    printf("loopBegin\n");
                    break;
                }
                case ByteCode::Instruction::kLoopEnd: {
                    --context.fLoopStack;
                    --context.fContinueStack;
                    printf("loopEnd\n");
                    break;
                }
                case ByteCode::Instruction::kLoopMask: {
                    ByteCode::Register value = read<ByteCode::Register>(&ip);
                    *context.fLoopStack &= fRegisters[value.fIndex].fInt;
                    printf("loopMask %d(%d)\n", value.fIndex, (*context.fLoopStack)[0]);
                    break;
                }
                case ByteCode::Instruction::kLoopNext: {
                    *context.fLoopStack |= *context.fContinueStack;
                    *context.fContinueStack = 0;
                    printf("loopNext\n");
                    break;
                }
                case ByteCode::Instruction::kMaskNegate: {
                    printf("maskNegate\n");
                    *context.fMaskStack = ~*context.fMaskStack;
                    break;
                }
                case ByteCode::Instruction::kMaskPop: {
                    printf("maskPop\n");
                    --context.fMaskStack;
                    break;
                }
                case ByteCode::Instruction::kMaskPush: {
                    ByteCode::Register value = read<ByteCode::Register>(&ip);
                    ++context.fMaskStack;
                    *context.fMaskStack = fRegisters[value.fIndex].fInt;
                    printf("maskPush %d\n", value.fIndex);
                    break;
                }
                case ByteCode::Instruction::kNegateF: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fFloat = -fRegisters[src.fIndex].fFloat;
                    printf("negatef %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kNegateI: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fInt = -fRegisters[src.fIndex].fInt;
                    printf("negatei %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kNot: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fInt = ~fRegisters[src.fIndex].fInt;
                    printf("not %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kReturn:
                    printf("return\n");
                    printf("--- finished call %s\n", f->fName.c_str());
                    return nullptr;
                case ByteCode::Instruction::kReturnValue: {
                    ByteCode::Register value = read<ByteCode::Register>(&ip);
                    printf("returnValue %d\n", value.fIndex);
                    printf("--- finished call %s\n", f->fName.c_str());
                    return &fRegisters[value.fIndex];
                }
                case ByteCode::Instruction::kSelect: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register test = read<ByteCode::Register>(&ip);
                    ByteCode::Register src1 = read<ByteCode::Register>(&ip);
                    ByteCode::Register src2 = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = skvx::if_then_else(fRegisters[test.fIndex].fInt,
                                                                   fRegisters[src1.fIndex].fInt,
                                                                   fRegisters[src2.fIndex].fInt);
                    printf("select %d, %d, %d -> %d\n", test.fIndex, src1.fIndex, src2.fIndex,
                                                        target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kShiftLeft: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    uint8_t count = read<uint8_t>(&ip);
                    fRegisters[target.fIndex].fInt = fRegisters[src.fIndex].fInt << count;
                    printf("shiftLeft %d, %d -> %d\n", src.fIndex, count, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kShiftRightS: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    int8_t count = read<int8_t>(&ip);
                    fRegisters[target.fIndex].fInt = fRegisters[src.fIndex].fInt >> count;
                    printf("shiftRightS %d, %d -> %d\n", src.fIndex, count, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kShiftRightU: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    uint8_t count = read<uint8_t>(&ip);
                    fRegisters[target.fIndex].fUInt = fRegisters[src.fIndex].fUInt >> count;
                    printf("shiftRightU %d, %d -> %d\n", src.fIndex, count, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kSignedToFloat: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = Vector(skvx::cast<float>(
                                                                      fRegisters[src.fIndex].fInt));
                    printf("signedToFloat %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                VECTOR_UNARY_FN(kSin, sinf)
                case ByteCode::Instruction::kSqrt: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fFloat = skvx::sqrt(fRegisters[src.fIndex].fFloat);
                    printf("sqrt %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                case ByteCode::Instruction::kStore: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    for (int i = 0; i < width; ++i) {
                        fMemory[fRegisters[target.fIndex].fInt[i]].fInt[i] =
                                                                     fRegisters[src.fIndex].fInt[i];
                    }
                    printf("store %d -> %d (mask = %d)\n", src.fIndex, target.fIndex, (*context.fMaskStack)[0]);
                    break;
                }
                case ByteCode::Instruction::kStoreDirect: {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fMemory[target.fAddress] = skvx::if_then_else(mask(),
                                                                  fRegisters[src.fIndex].fInt,
                                                                  fMemory[target.fAddress].fInt);
                    printf("storeDirect %d -> %d (mask = %d)\n", src.fIndex, target.fAddress, (*context.fMaskStack)[0]);
                    break;
                }
                case ByteCode::Instruction::kStoreStack: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    printf("storeStack %d -> %d (mask = %d)\n", src.fIndex, target.fIndex, (*context.fMaskStack)[0]);
                    printf("reg %d = %d\n", src.fIndex, fRegisters[src.fIndex].fInt[0]);
                    for (int i = 0; i < width; ++i) {
                        printf("src: %d\n", fRegisters[src.fIndex].fInt[i]);
                        context.fStack[fRegisters[target.fIndex].fInt[i]].fInt[i] =
                                                                     fRegisters[src.fIndex].fInt[i];
                    }
                    printf("storeStack %d -> %d (mask = %d)\n", src.fIndex, target.fIndex, (*context.fMaskStack)[0]);
                    break;
                }
                case ByteCode::Instruction::kStoreStackDirect: {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    CHECK_STACK_BOUNDS(target.fAddress);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    context.fStack[target.fAddress] = skvx::if_then_else(
                                                              mask(),
                                                              fRegisters[src.fIndex].fInt,
                                                              context.fStack[target.fAddress].fInt);
                    printf("storeStackDirect %d -> %d (mask = %d)\n", src.fIndex, target.fAddress, (*context.fMaskStack)[0]);
                    break;
                }
                VECTOR_UNARY_FN(kTan, tanf)
                case ByteCode::Instruction::kUnsignedToFloat: {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = Vector(skvx::cast<float>(
                                                                     fRegisters[src.fIndex].fUInt));
                    printf("unsignedToFloat %d -> %d\n", src.fIndex, target.fIndex);
                    break;
                }
                default:
                    printf("unsupported instruction: %d\n", (int) inst);
                    SkASSERT(false);
                    return nullptr;
            }
        }
    }

    const std::unique_ptr<ByteCode> fCode;

    Vector fRegisters[REGISTER_COUNT];

    Vector fMemory[MEMORY_SIZE];

    friend class ByteCode;

    friend class ByteCodeGenerator;
};

#undef BINARY_OP
#undef CHECK_STACK_BOUNDS

} // namespace

#endif
