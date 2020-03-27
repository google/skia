/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrTypesPriv.h" // GrAlignTo
#include "src/core/SkUtils.h" // sk_unaligned_load
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLExternalValue.h"

#include <stack>

#ifndef SKSL_INTERPRETER
#define SKSL_INTERPRETER

namespace SkSL {

// GCC and Clang support the "labels as values" extension which we need to implement the interpreter
// using threaded code. Otherwise, we fall back to using a switch statement in a for loop.
#if defined(__GNUC__) || defined(__clang__)
    #define SKSL_THREADED_CODE
#endif

#ifdef SKSL_THREADED_CODE
    #define LABEL(name) name:
    #ifdef TRACE
        #define NEXT()                                   \
            {                                            \
                const uint8_t* trace_ip = ip;            \
                printf("%d: ", (int) (trace_ip - code)); \
                disassemble(&trace_ip);                  \
            }                                            \
            goto *labels[(int) read<ByteCode::Instruction>(&ip)]
    #else
        #define NEXT() goto *labels[(int) read<ByteCode::Instruction>(&ip)]
    #endif
#else
    #define LABEL(name) case ByteCode::Instruction::name:
    #define NEXT() continue
#endif

// If you trip this assert, it means that the order of the opcodes listed in ByteCodeInstruction
// does not match the order of the opcodes listed in the 'labels' array in innerRun().
#define CHECK_LABEL(name) \
    SkASSERT(labels[(int) ByteCode::Instruction::name] == &&name)

template<typename T>
static T read(const uint8_t** ip) {
    *ip += sizeof(T);
    return sk_unaligned_load<T>(*ip - sizeof(T));
}

#define BINARY_OP(inst, src, result, op)                                  \
    LABEL(inst) {                                                         \
        ByteCode::Register target = read<ByteCode::Register>(&ip);        \
        ByteCode::Register src1 = read<ByteCode::Register>(&ip);          \
        ByteCode::Register src2 = read<ByteCode::Register>(&ip);          \
        fRegisters[target.fIndex].result = fRegisters[src1.fIndex].src op \
                                           fRegisters[src2.fIndex].src;   \
        NEXT();                                                           \
    }

#define MASKED_BINARY_OP(inst, src, result, op)                                         \
    LABEL(inst) {                                                                       \
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
        NEXT();                                                                         \
    }

#define MASKED_VECTOR_BINARY_OP(inst, src, result, op)                                             \
    LABEL(inst) {                                                                                  \
        ByteCode::Register target = read<ByteCode::Register>(&ip);                                 \
        ByteCode::Register src1 = read<ByteCode::Register>(&ip);                                   \
        ByteCode::Register src2 = read<ByteCode::Register>(&ip);                                   \
        auto m = mask();                                                                           \
        for (int i = 0; i < width; ++i) {                                                          \
            if (m[i]) {                                                                            \
                fRegisters[target.fIndex].result[i] = fRegisters[src1.fIndex].src[i] op            \
                                                      fRegisters[src2.fIndex].src[i];              \
            }                                                                                      \
        }                                                                                          \
        NEXT();                                                                                    \
    }                                                                                              \
    LABEL(inst ## N) {                                                                             \
        uint8_t count = read<uint8_t>(&ip);                                                        \
        ByteCode::Register target = read<ByteCode::Register>(&ip);                                 \
        ByteCode::Register src1 = read<ByteCode::Register>(&ip);                                   \
        ByteCode::Register src2 = read<ByteCode::Register>(&ip);                                   \
        auto m = mask();                                                                           \
        for (int i = 0; i < width; ++i) {                                                          \
            if (m[i]) {                                                                            \
                for (int j = 0; j < count; ++j) {                                                  \
                    fRegisters[target.fIndex + j].result[i] = fRegisters[src1.fIndex + j].src[i]   \
                                                            op fRegisters[src2.fIndex + j].src[i]; \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
        NEXT();                                                                                    \
    }

#define VECTOR_BINARY_OP(inst, src, result, op)                                       \
    LABEL(inst) {                                                                     \
        ByteCode::Register target = read<ByteCode::Register>(&ip);                    \
        ByteCode::Register src1 = read<ByteCode::Register>(&ip);                      \
        ByteCode::Register src2 = read<ByteCode::Register>(&ip);                      \
        fRegisters[target.fIndex].result = fRegisters[src1.fIndex].src op             \
                                               fRegisters[src2.fIndex].src;           \
        NEXT();                                                                       \
    }                                                                                 \
    LABEL(inst ## N) {                                                                \
        uint8_t count = read<uint8_t>(&ip);                                           \
        ByteCode::Register target = read<ByteCode::Register>(&ip);                    \
        ByteCode::Register src1 = read<ByteCode::Register>(&ip);                      \
        ByteCode::Register src2 = read<ByteCode::Register>(&ip);                      \
        for (int i = 0; i < count; ++i) {                                             \
            fRegisters[target.fIndex + i].result = fRegisters[src1.fIndex + i].src op \
                                                   fRegisters[src2.fIndex + i].src;   \
        }                                                                             \
        NEXT();                                                                       \
    }

#define VECTOR_UNARY_FN(inst, fn)                                                       \
    LABEL(inst) {                                                                       \
        ByteCode::Register target = read<ByteCode::Register>(&ip);                      \
        ByteCode::Register src = read<ByteCode::Register>(&ip);                         \
        for (int i = 0; i < width; ++ i) {                                              \
            fRegisters[target.fIndex].fFloat[i] = fn(fRegisters[src.fIndex].fFloat[i]); \
        }                                                                               \
        NEXT();                                                                         \
    }

#define DISASSEMBLE_0(inst, name) \
    case ByteCode::Instruction::inst: printf(name "\n"); break;

#define DISASSEMBLE_1(inst, name)                                   \
    case ByteCode::Instruction::inst:                               \
        printf(name " $%d\n", read<ByteCode::Register>(ip).fIndex); \
        break;

#define DISASSEMBLE_UNARY(inst, name)                             \
    case ByteCode::Instruction::inst: {                           \
        ByteCode::Register target = read<ByteCode::Register>(ip); \
        ByteCode::Register src = read<ByteCode::Register>(ip);    \
        printf(name " $%d -> $%d\n", src.fIndex, target.fIndex);  \
        break;                                                    \
    }

#define DISASSEMBLE_VECTOR_UNARY(inst, name)                              \
    case ByteCode::Instruction::inst: {                                   \
        ByteCode::Register target = read<ByteCode::Register>(ip);         \
        ByteCode::Register src = read<ByteCode::Register>(ip);            \
        printf(name " $%d -> $%d\n", src.fIndex, target.fIndex);          \
        break;                                                            \
    }                                                                     \
    case ByteCode::Instruction::inst ## N: {                              \
        uint8_t count = read<uint8_t>(ip);                                \
        ByteCode::Register target = read<ByteCode::Register>(ip);         \
        ByteCode::Register src = read<ByteCode::Register>(ip);            \
        printf(name "%d $%d -> $%d\n", count, src.fIndex, target.fIndex); \
        break;                                                            \
    }

#define DISASSEMBLE_BINARY(inst, name)                                              \
    case ByteCode::Instruction::inst: {                                             \
        ByteCode::Register target = read<ByteCode::Register>(ip);                   \
        ByteCode::Register src1 = read<ByteCode::Register>(ip);                     \
        ByteCode::Register src2 = read<ByteCode::Register>(ip);                     \
        printf(name " $%d, $%d -> $%d\n", src1.fIndex, src2.fIndex, target.fIndex); \
        break;                                                                      \
    }

#define DISASSEMBLE_VECTOR_BINARY(inst, name)                                                \
    case ByteCode::Instruction::inst: {                                                      \
        ByteCode::Register target = read<ByteCode::Register>(ip);                            \
        ByteCode::Register src1 = read<ByteCode::Register>(ip);                              \
        ByteCode::Register src2 = read<ByteCode::Register>(ip);                              \
        printf(name " $%d, $%d -> $%d\n", src1.fIndex, src2.fIndex, target.fIndex);          \
        break;                                                                               \
    }                                                                                        \
    case ByteCode::Instruction::inst ## N: {                                                 \
        uint8_t count = read<uint8_t>(ip);                                                   \
        ByteCode::Register target = read<ByteCode::Register>(ip);                            \
        ByteCode::Register src1 = read<ByteCode::Register>(ip);                              \
        ByteCode::Register src2 = read<ByteCode::Register>(ip);                              \
        printf(name "%d $%d, $%d -> $%d\n", count, src1.fIndex, src2.fIndex, target.fIndex); \
        break;                                                                               \
    }

/**
 * Operates on vectors of the specified width, so creating an Interpreter<16> means that all inputs,
 * outputs, and internal calculations will be 16-wide vectors.
 */
template<int width>
class Interpreter {
public:
    using Vector = ByteCode::Vector<width>;
    using VectorI = skvx::Vec<width, int32_t>;
    using VectorF = skvx::Vec<width, float>;

    Interpreter(std::unique_ptr<ByteCode> code)
        : fCode(std::move(code)) {
        // C++ doesn't guarantee proper alignment of naively-allocated vectors, so we can't have the
        // registers and memory directly as fields of this object without jumping through some hoops
        // during Interpreter allocation and deallocation. We simplify this by having the backing
        // store be a separate allocation, jumping through the hoops ourselves rather than require
        // Interpreter's clients to be aware of alignment.
        // Ideally, we could use std::aligned_alloc here, but as of this writing it is not available
        // on some compilers despite claiming to support C++17.
        fBackingStore = calloc(sizeof(Vector), MEMORY_SIZE + REGISTER_COUNT + 1);
        fMemory = (Vector*) GrAlignTo((size_t) fBackingStore, alignof(Vector));
        fRegisters = fMemory + MEMORY_SIZE;
    }

    ~Interpreter() {
        free(fBackingStore);
    }

    void setUniforms(const float uniforms[]) {
        for (int i = 0; i < fCode->getUniformSlotCount(); ++i) {
            fMemory[fCode->getGlobalSlotCount() + i].fFloat = VectorF(uniforms[i]);
        }
    }

    /**
     * Returns true on success and stores a pointer to the first slot of the result into outResult.
     * This pointer is only guaranteed to be valid until the next run() call.
     */
     bool run(const ByteCodeFunction* f, Vector args[], Vector** outResult) {
        SkASSERT(f);
        VectorI condStack[MASK_STACK_SIZE];
        memset(&condStack[0], 255, sizeof(condStack[0]));
        VectorI maskStack[MASK_STACK_SIZE];
        memset(&maskStack[0], 255, sizeof(maskStack[0]));
        VectorI loopStack[LOOP_STACK_SIZE];
        memset(&loopStack[0], 255, sizeof(loopStack[0]));
        VectorI continueStack[LOOP_STACK_SIZE];
        memset(&continueStack[0], 0, sizeof(continueStack[0]));
        Vector* stack = fMemory + MEMORY_SIZE;
        int stackCount = f->fStackSlotCount + f->fParameterSlotCount;
        stack -= stackCount;
        if (f->fParameterSlotCount) {
            memcpy(stack, args, f->fParameterSlotCount * sizeof(Vector));
        }
        Context context(fMemory, stack, condStack, maskStack, loopStack, continueStack);
        if (this->innerRun(f, context, 0, outResult)) {
            int slot = 0;
            for (const auto& p : f->fParameters) {
                if (p.fIsOutParameter) {
                    memcpy(&args[slot], &stack[slot], p.fSlotCount * sizeof(Vector));
                }
                slot += p.fSlotCount;
            }
            return true;
        }
        return false;
    }

    /**
     * Invokes the specified function with the given arguments, 'count' times. 'args' and
     * 'outResult' are accepted and returned in structure-of-arrays form:
     *   args[0] points to an array of N values, the first argument for each invocation
     *   ...
     *   args[argCount - 1] points to an array of N values, the last argument for each invocation
     *
     * All values in 'args', 'outResult', and 'uniforms' are 32-bit values (typically floats,
     * but possibly int32_t or uint32_t, depending on the types used in the SkSL).
     * Any 'out' or 'inout' parameters will result in the 'args' array being modified.
     */
    bool runStriped(const ByteCodeFunction* f, int count, float* args[],
                    float* outResult[] = nullptr) {
        SkASSERT(f);
        Vector* stack = fMemory + MEMORY_SIZE;
        int stackCount = f->fStackSlotCount + f->fParameterSlotCount;
        stack -= stackCount;
        VectorI condStack[MASK_STACK_SIZE];
        VectorI maskStack[MASK_STACK_SIZE];
        VectorI loopStack[LOOP_STACK_SIZE];
        VectorI continueStack[LOOP_STACK_SIZE];
        Vector* innerResult = nullptr;
        Context context(fMemory, stack, condStack, maskStack, loopStack, continueStack);
        for (int i = 0; i < count; i += width) {
            int lanes = std::min(width, count - i);
            size_t size = lanes * sizeof(float);
            memset(&maskStack[0], 255, sizeof(maskStack[0]));
            memset(&loopStack[0], 255, sizeof(loopStack[0]));
            for (int j = lanes; j < width; ++j) {
                maskStack[0][j] = 0;
                loopStack[0][j] = 0;
            }
            memset(&continueStack[0], 0, sizeof(continueStack[0]));
            for (int j = 0; j < f->fParameterSlotCount; ++j) {
                memcpy(stack + j, &args[j][i], size);
            }
            if (!this->innerRun(f, context, i, &innerResult)) {
                return false;
            }
            int slot = 0;
            for (const auto& p : f->fParameters) {
                if (p.fIsOutParameter) {
                    for (int j = 0; j < p.fSlotCount; ++j) {
                        memcpy(&args[slot + j][i], stack + slot + j, size);
                    }
                }
                slot += p.fSlotCount;
            }
            if (outResult) {
                for (int j = 0; j < f->fReturnSlotCount; ++j) {
                    memcpy(&outResult[j][i], &innerResult[j], size);
                }
            }
        }
        return true;
    }

    const ByteCode& getCode() {
        return *fCode;
    }

private:
    static constexpr size_t REGISTER_COUNT = 1024;

    static constexpr size_t MEMORY_SIZE = 1024;

    static constexpr size_t MASK_STACK_SIZE = 64;

    static constexpr size_t LOOP_STACK_SIZE = 16;

    struct StackFrame {
        StackFrame(const ByteCodeFunction* function, const uint8_t* ip, const int stackSlotCount,
                   Vector* parameters, Vector* returnValue)
            : fFunction(function)
            , fIP(ip)
            , fStackSlotCount(stackSlotCount)
            , fParameters(parameters)
            , fReturnValue(returnValue) {}

        const ByteCodeFunction* fFunction;
        const uint8_t* fIP;
        const int fStackSlotCount;
        Vector* fParameters;
        Vector* fReturnValue;
    };

    struct Context {
        Context(Vector* memory, Vector* stack, VectorI* condStack, VectorI* maskStack,
                VectorI* loopStack,VectorI* continueStack)
            : fMemory(memory)
            , fStack(stack)
            , fCondStack(condStack)
            , fMaskStack(maskStack)
            , fLoopStack(loopStack)
            , fContinueStack(continueStack) {}

        Vector* fMemory;
        Vector* fStack;
        VectorI* fCondStack;
        VectorI* fMaskStack;
        VectorI* fLoopStack;
        VectorI* fContinueStack;
        std::stack<StackFrame> fCallStack;
    };

    // $x = register
    // @x = memory cell
    // &x = parameter
    void disassemble(const uint8_t** ip) {
        ByteCode::Instruction inst = read<ByteCode::Instruction>(ip);
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
            case ByteCode::Instruction::kBoundsCheck: {
                ByteCode::Register r = read<ByteCode::Register>(ip);
                int length = read<int>(ip);
                printf("boundsCheck 0 <= $%d < %d\n", r.fIndex, length);
                break;
            }
            case ByteCode::Instruction::kBranch:
                printf("branch %d\n", read<ByteCode::Pointer>(ip).fAddress);
                break;
            case ByteCode::Instruction::kBranchIfAllFalse:
                printf("branchIfAllFalse %d\n", read<ByteCode::Pointer>(ip).fAddress);
                break;
            DISASSEMBLE_0(kBreak, "break")
            case ByteCode::Instruction::kCall: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                uint8_t idx = read<uint8_t>(ip);
                ByteCode::Register args = read<ByteCode::Register>(ip);
                ByteCodeFunction* f = fCode->fFunctions[idx].get();
                printf("call %s($%d...) -> $%d", f->fName.c_str(), args.fIndex, target.fIndex);
                printf("\n");
                break;
            }
            case ByteCode::Instruction::kCallExternal: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                uint8_t idx = read<uint8_t>(ip);
                uint8_t targetCount = read<uint8_t>(ip);
                ByteCode::Register args = read<ByteCode::Register>(ip);
                uint8_t argCount = read<uint8_t>(ip);
                ExternalValue* ev = fCode->fExternalValues[idx];
                printf("callExternal %s($%d(%d)...) -> $%d(%d)", String(ev->fName).c_str(),
                        args.fIndex, argCount, target.fIndex, targetCount);
                printf("\n");
                break;
            }
            DISASSEMBLE_0(kContinue, "continue")
            DISASSEMBLE_UNARY(kCopy, "copy")
            DISASSEMBLE_UNARY(kCos, "cos")
            DISASSEMBLE_UNARY(kFloatToSigned, "FtoS")
            DISASSEMBLE_UNARY(kFloatToUnsigned, "FtoU")
            case ByteCode::Instruction::kImmediate: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Immediate src = read<ByteCode::Immediate>(ip);
                printf("immediate (%d | %f) -> $%d\n", src.fInt, src.fFloat, target.fIndex);
                break;
            }
            DISASSEMBLE_UNARY(kInverse2x2, "inverse2x2")
            DISASSEMBLE_UNARY(kInverse3x3, "inverse3x3")
            DISASSEMBLE_UNARY(kInverse4x4, "inverse4x4")
            DISASSEMBLE_VECTOR_UNARY(kLoad, "load")
            case ByteCode::Instruction::kLoadDirect: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Pointer src = read<ByteCode::Pointer>(ip);
                printf("loadDirect @%d -> $%d\n", src.fAddress, target.fIndex);
                break;
            }
            case ByteCode::Instruction::kLoadDirectN: {
                uint8_t count = read<uint8_t>(ip);
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Pointer src = read<ByteCode::Pointer>(ip);
                printf("loadDirect%d @%d -> $%d\n", count, src.fAddress, target.fIndex);
                break;
            }
            DISASSEMBLE_VECTOR_UNARY(kLoadParameter, "loadParameter")
            case ByteCode::Instruction::kLoadParameterDirect: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Pointer src = read<ByteCode::Pointer>(ip);
                printf("loadParameterDirect &%d -> $%d\n", src.fAddress, target.fIndex);
                break;
            }
            case ByteCode::Instruction::kLoadParameterDirectN: {
                uint8_t count = read<uint8_t>(ip);
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Pointer src = read<ByteCode::Pointer>(ip);
                printf("loadParameterDirect%d &%d -> $%d\n", count, src.fAddress, target.fIndex);
                break;
            }
            DISASSEMBLE_VECTOR_UNARY(kLoadStack, "loadStack")
            case ByteCode::Instruction::kLoadStackDirect: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Pointer src = read<ByteCode::Pointer>(ip);
                printf("loadStackDirect @%d -> $%d\n", src.fAddress, target.fIndex);
                break;
            }
            case ByteCode::Instruction::kLoadStackDirectN: {
                uint8_t count = read<uint8_t>(ip);
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Pointer src = read<ByteCode::Pointer>(ip);
                printf("loadStackDirect%d @%d -> $%d\n", count, src.fAddress, target.fIndex);
                break;
            }
            DISASSEMBLE_0(kLoopBegin, "loopBegin")
            DISASSEMBLE_0(kLoopEnd, "loopEnd")
            DISASSEMBLE_1(kLoopMask, "loopMask")
            DISASSEMBLE_0(kLoopNext, "loopNext")
            DISASSEMBLE_0(kMaskNegate, "maskNegate")
            DISASSEMBLE_0(kMaskPop, "maskPop")
            DISASSEMBLE_1(kMaskPush, "maskPush")
            case ByteCode::Instruction::kMatrixMultiply: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Register left = read<ByteCode::Register>(ip);
                ByteCode::Register right = read<ByteCode::Register>(ip);
                uint8_t leftColsAndRightRows = read<uint8_t>(ip);
                uint8_t leftRows = read<uint8_t>(ip);
                uint8_t rightColumns = read<uint8_t>(ip);
                printf("matrixMultiply $%d, $%d, %d, %d, %d -> $%d\n", left.fIndex, right.fIndex,
                       leftColsAndRightRows, leftRows, rightColumns, target.fIndex);
                break;
            }
            case ByteCode::Instruction::kMatrixToMatrix: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                uint8_t srcColumns = read<uint8_t>(ip);
                uint8_t srcRows = read<uint8_t>(ip);
                uint8_t dstColumns = read<uint8_t>(ip);
                uint8_t dstRows = read<uint8_t>(ip);
                printf("matrixToMatrix $%d, %dx%d to %dx%d -> $%d\n", src.fIndex, srcColumns,
                       srcRows, dstColumns, dstRows, target.fIndex);
                break;
            }
            DISASSEMBLE_UNARY(kNegateF, "negateF")
            DISASSEMBLE_UNARY(kNegateS, "negateS")
            DISASSEMBLE_UNARY(kNot, "not")
            case ByteCode::Instruction::kReadExternal: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                uint8_t count = read<uint8_t>(ip);
                uint8_t index = read<uint8_t>(ip);
                printf("readExternal %d, %d -> $%d\n", count, index, target.fIndex);
                break;
            }
            DISASSEMBLE_1(kPrint, "print")
            DISASSEMBLE_0(kReturn, "return")
            DISASSEMBLE_1(kReturnValue, "returnValue")
            case ByteCode::Instruction::kScalarToMatrix: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                uint8_t columns = read<uint8_t>(ip);
                uint8_t rows = read<uint8_t>(ip);
                printf("scalarToMatrix $%d, %dx%d -> $%d\n", src.fIndex, columns, rows,
                       target.fIndex);
                break;
            }
            case ByteCode::Instruction::kSelect: {
                ByteCode::Register target = read<ByteCode::Register>(ip);
                ByteCode::Register test = read<ByteCode::Register>(ip);
                ByteCode::Register src1 = read<ByteCode::Register>(ip);
                ByteCode::Register src2 = read<ByteCode::Register>(ip);
                printf("select $%d, $%d, $%d -> %d\n", test.fIndex, src1.fIndex, src2.fIndex,
                       target.fIndex);
                break;
            }
            DISASSEMBLE_BINARY(kShiftLeft, "shiftLeft")
            DISASSEMBLE_BINARY(kShiftRightS, "shiftRightS")
            DISASSEMBLE_BINARY(kShiftRightU, "shiftRightU")
            DISASSEMBLE_UNARY(kSignedToFloat, "signedToFloat")
            DISASSEMBLE_UNARY(kSin, "sin")
            case ByteCode::Instruction::kSplat: {
                uint8_t count = read<uint8_t>(ip);
                ByteCode::Pointer target = read<ByteCode::Pointer>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                printf("splat%d $%d -> @%d\n", count, src.fIndex, target.fAddress);
                break;
            }
            DISASSEMBLE_UNARY(kSqrt, "sqrt")
            DISASSEMBLE_VECTOR_UNARY(kStore, "store")
            case ByteCode::Instruction::kStoreDirect: {
                ByteCode::Pointer target = read<ByteCode::Pointer>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                printf("store $%d -> @%d\n", src.fIndex, target.fAddress);
                break;
            }
            case ByteCode::Instruction::kStoreDirectN: {
                uint8_t count = read<uint8_t>(ip);
                ByteCode::Pointer target = read<ByteCode::Pointer>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                printf("store%d $%d -> @%d\n", count, src.fIndex, target.fAddress);
                break;
            }
            DISASSEMBLE_VECTOR_UNARY(kStoreParameter, "storeParameter")
            case ByteCode::Instruction::kStoreParameterDirect: {
                ByteCode::Pointer target = read<ByteCode::Pointer>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                printf("storeParameterDirect $%d -> &%d\n", src.fIndex, target.fAddress);
                break;
            }
            case ByteCode::Instruction::kStoreParameterDirectN: {
                uint8_t count = read<uint8_t>(ip);
                ByteCode::Pointer target = read<ByteCode::Pointer>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                printf("storeParameterDirect%d $%d -> &%d\n", count, src.fIndex, target.fAddress);
                break;
            }
            DISASSEMBLE_VECTOR_UNARY(kStoreStack, "storeStack")
            case ByteCode::Instruction::kStoreStackDirect: {
                ByteCode::Pointer target = read<ByteCode::Pointer>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                printf("storeStackDirect $%d -> @%d\n", src.fIndex, target.fAddress);
                break;
            }
            case ByteCode::Instruction::kStoreStackDirectN: {
                uint8_t count = read<uint8_t>(ip);
                ByteCode::Pointer target = read<ByteCode::Pointer>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                printf("storeStackDirect%d $%d -> @%d\n", count, src.fIndex, target.fAddress);
                break;
            }
            DISASSEMBLE_UNARY(kTan, "tan")
            DISASSEMBLE_UNARY(kUnsignedToFloat, "unsignedToFloat")
            case ByteCode::Instruction::kWriteExternal: {
                uint8_t index = read<uint8_t>(ip);
                uint8_t count = read<uint8_t>(ip);
                ByteCode::Register src = read<ByteCode::Register>(ip);
                printf("writeExternal $%d, %d -> %d\n", src.fIndex, count, index);
                break;
            }
            default:
                printf("unsupported: %d\n", (int) inst);
                SkASSERT(false);
        }
    }

    static Vector VecMod(Vector x, Vector y) {
        return Vector(x.fFloat - skvx::trunc(x.fFloat / y.fFloat) * y.fFloat);
    }

    #define CHECK_STACK_BOUNDS(address)                              \
        SkASSERT(context.fStack + address >= fMemory &&              \
                 context.fStack + address <= fMemory + MEMORY_SIZE)

    static void Inverse2x2(Vector* in, Vector* out) {
        VectorF a = in[0].fFloat,
                b = in[1].fFloat,
                c = in[2].fFloat,
                d = in[3].fFloat;
        VectorF idet = VectorF(1) / (a*d - b*c);
        out[0].fFloat = d * idet;
        out[1].fFloat = -b * idet;
        out[2].fFloat = -c * idet;
        out[3].fFloat = a * idet;
    }

    static void Inverse3x3(Vector* in, Vector* out) {
        VectorF a11 = in[0].fFloat, a12 = in[3].fFloat, a13 = in[6].fFloat,
                a21 = in[1].fFloat, a22 = in[4].fFloat, a23 = in[7].fFloat,
                a31 = in[2].fFloat, a32 = in[5].fFloat, a33 = in[8].fFloat;
        VectorF idet = VectorF(1) / (a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 -
                                     a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31);
        out[0].fFloat = (a22 * a33 - a23 * a32) * idet;
        out[1].fFloat = (a23 * a31 - a21 * a33) * idet;
        out[2].fFloat = (a21 * a32 - a22 * a31) * idet;
        out[3].fFloat = (a13 * a32 - a12 * a33) * idet;
        out[4].fFloat = (a11 * a33 - a13 * a31) * idet;
        out[5].fFloat = (a12 * a31 - a11 * a32) * idet;
        out[6].fFloat = (a12 * a23 - a13 * a22) * idet;
        out[7].fFloat = (a13 * a21 - a11 * a23) * idet;
        out[8].fFloat = (a11 * a22 - a12 * a21) * idet;
    }


    static void Inverse4x4(Vector* in, Vector* out) {
        #define inf(index)  in[index].fFloat
        #define outf(index) out[index].fFloat
        VectorF a00 = inf(0), a10 = inf(4), a20 = inf( 8), a30 = inf(12),
                a01 = inf(1), a11 = inf(5), a21 = inf( 9), a31 = inf(13),
                a02 = inf(2), a12 = inf(6), a22 = inf(10), a32 = inf(14),
                a03 = inf(3), a13 = inf(7), a23 = inf(11), a33 = inf(15);

        VectorF b00 = a00 * a11 - a01 * a10,
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

        VectorF idet = VectorF(1) /
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

        outf( 0) = a11 * b11 - a12 * b10 + a13 * b09;
        outf( 1) = a02 * b10 - a01 * b11 - a03 * b09;
        outf( 2) = a31 * b05 - a32 * b04 + a33 * b03;
        outf( 3) = a22 * b04 - a21 * b05 - a23 * b03;
        outf( 4) = a12 * b08 - a10 * b11 - a13 * b07;
        outf( 5) = a00 * b11 - a02 * b08 + a03 * b07;
        outf( 6) = a32 * b02 - a30 * b05 - a33 * b01;
        outf( 7) = a20 * b05 - a22 * b02 + a23 * b01;
        outf( 8) = a10 * b10 - a11 * b08 + a13 * b06;
        outf( 9) = a01 * b08 - a00 * b10 - a03 * b06;
        outf(10) = a30 * b04 - a31 * b02 + a33 * b00;
        outf(11) = a21 * b02 - a20 * b04 - a23 * b00;
        outf(12) = a11 * b07 - a10 * b09 - a12 * b06;
        outf(13) = a00 * b09 - a01 * b07 + a02 * b06;
        outf(14) = a31 * b01 - a30 * b03 - a32 * b00;
        outf(15) = a20 * b03 - a21 * b01 + a22 * b00;
        #undef inf
        #undef outf
    }

    bool innerRun(const ByteCodeFunction* f, Context context, int baseIndex, Vector** outResult) {
#ifdef SKSL_THREADED_CODE
        static const void* labels[] = {
            // If you aren't familiar with it, the &&label syntax is the GCC / Clang "labels as
            // values" extension. If you add anything to this array, be sure to add the
            // corresponding CHECK_LABEL() assert below.
            &&kNop,
            &&kAbort,
            &&kAddF,
            &&kAddFN,
            &&kAddI,
            &&kAddIN,
            &&kAnd,
            &&kBoundsCheck,
            &&kBranch,
            &&kBranchIfAllFalse,
            &&kBreak,
            &&kCall,
            &&kCallExternal,
            &&kCompareEQF,
            &&kCompareEQI,
            &&kCompareNEQF,
            &&kCompareNEQI,
            &&kCompareGTF,
            &&kCompareGTS,
            &&kCompareGTU,
            &&kCompareGTEQF,
            &&kCompareGTEQS,
            &&kCompareGTEQU,
            &&kCompareLTF,
            &&kCompareLTS,
            &&kCompareLTU,
            &&kCompareLTEQF,
            &&kCompareLTEQS,
            &&kCompareLTEQU,
            &&kContinue,
            &&kCopy,
            &&kCos,
            &&kDivideF,
            &&kDivideFN,
            &&kDivideS,
            &&kDivideSN,
            &&kDivideU,
            &&kDivideUN,
            &&kFloatToSigned,
            &&kFloatToUnsigned,
            &&kImmediate,
            &&kInverse2x2,
            &&kInverse3x3,
            &&kInverse4x4,
            &&kLoad,
            &&kLoadN,
            &&kLoadDirect,
            &&kLoadDirectN,
            &&kLoadParameter,
            &&kLoadParameterN,
            &&kLoadParameterDirect,
            &&kLoadParameterDirectN,
            &&kLoadStack,
            &&kLoadStackN,
            &&kLoadStackDirect,
            &&kLoadStackDirectN,
            &&kLoopBegin,
            &&kLoopEnd,
            &&kLoopMask,
            &&kLoopNext,
            &&kMaskNegate,
            &&kMaskPop,
            &&kMaskPush,
            &&kMatrixMultiply,
            &&kMatrixToMatrix,
            &&kMultiplyF,
            &&kMultiplyFN,
            &&kMultiplyI,
            &&kMultiplyIN,
            &&kNegateF,
            &&kNegateS,
            &&kNot,
            &&kOr,
            &&kPrint,
            &&kReadExternal,
            &&kRemainderF,
            &&kRemainderFN,
            &&kRemainderS,
            &&kRemainderSN,
            &&kRemainderU,
            &&kRemainderUN,
            &&kReturn,
            &&kReturnValue,
            &&kScalarToMatrix,
            &&kSelect,
            &&kShiftLeft,
            &&kShiftRightS,
            &&kShiftRightU,
            &&kSignedToFloat,
            &&kSin,
            &&kSplat,
            &&kSqrt,
            &&kStore,
            &&kStoreN,
            &&kStoreDirect,
            &&kStoreDirectN,
            &&kStoreParameter,
            &&kStoreParameterN,
            &&kStoreParameterDirect,
            &&kStoreParameterDirectN,
            &&kStoreStack,
            &&kStoreStackN,
            &&kStoreStackDirect,
            &&kStoreStackDirectN,
            &&kSubtractF,
            &&kSubtractFN,
            &&kSubtractI,
            &&kSubtractIN,
            &&kTan,
            &&kUnsignedToFloat,
            &&kWriteExternal,
            &&kXor
        };
        CHECK_LABEL(kNop);
        CHECK_LABEL(kAbort);
        CHECK_LABEL(kAddF);
        CHECK_LABEL(kAddI);
        CHECK_LABEL(kAnd);
        CHECK_LABEL(kBoundsCheck);
        CHECK_LABEL(kBranch);
        CHECK_LABEL(kBranchIfAllFalse);
        CHECK_LABEL(kBreak);
        CHECK_LABEL(kCall);
        CHECK_LABEL(kCallExternal);
        CHECK_LABEL(kCompareEQF);
        CHECK_LABEL(kCompareEQI);
        CHECK_LABEL(kCompareNEQF);
        CHECK_LABEL(kCompareNEQI);
        CHECK_LABEL(kCompareGTF);
        CHECK_LABEL(kCompareGTS);
        CHECK_LABEL(kCompareGTU);
        CHECK_LABEL(kCompareGTEQF);
        CHECK_LABEL(kCompareGTEQS);
        CHECK_LABEL(kCompareGTEQU);
        CHECK_LABEL(kCompareLTF);
        CHECK_LABEL(kCompareLTS);
        CHECK_LABEL(kCompareLTU);
        CHECK_LABEL(kCompareLTEQF);
        CHECK_LABEL(kCompareLTEQS);
        CHECK_LABEL(kCompareLTEQU);
        CHECK_LABEL(kContinue);
        CHECK_LABEL(kCopy);
        CHECK_LABEL(kCos);
        CHECK_LABEL(kDivideF);
        CHECK_LABEL(kDivideFN);
        CHECK_LABEL(kDivideS);
        CHECK_LABEL(kDivideSN);
        CHECK_LABEL(kDivideU);
        CHECK_LABEL(kDivideUN);
        CHECK_LABEL(kFloatToSigned);
        CHECK_LABEL(kFloatToUnsigned);
        CHECK_LABEL(kImmediate);
        CHECK_LABEL(kInverse2x2);
        CHECK_LABEL(kInverse3x3);
        CHECK_LABEL(kInverse4x4);
        CHECK_LABEL(kLoad);
        CHECK_LABEL(kLoadN);
        CHECK_LABEL(kLoadDirect);
        CHECK_LABEL(kLoadDirectN);
        CHECK_LABEL(kLoadParameter);
        CHECK_LABEL(kLoadParameterN);
        CHECK_LABEL(kLoadParameterDirect);
        CHECK_LABEL(kLoadParameterDirectN);
        CHECK_LABEL(kLoadStack);
        CHECK_LABEL(kLoadStackN);
        CHECK_LABEL(kLoadStackDirect);
        CHECK_LABEL(kLoadStackDirectN);
        CHECK_LABEL(kLoopBegin);
        CHECK_LABEL(kLoopEnd);
        CHECK_LABEL(kLoopMask);
        CHECK_LABEL(kLoopNext);
        CHECK_LABEL(kMaskNegate);
        CHECK_LABEL(kMaskPop);
        CHECK_LABEL(kMaskPush);
        CHECK_LABEL(kMatrixMultiply);
        CHECK_LABEL(kMatrixToMatrix);
        CHECK_LABEL(kMultiplyF);
        CHECK_LABEL(kMultiplyFN);
        CHECK_LABEL(kMultiplyI);
        CHECK_LABEL(kMultiplyIN);
        CHECK_LABEL(kNegateF);
        CHECK_LABEL(kNegateS);
        CHECK_LABEL(kNot);
        CHECK_LABEL(kOr);
        CHECK_LABEL(kPrint);
        CHECK_LABEL(kReadExternal);
        CHECK_LABEL(kRemainderF);
        CHECK_LABEL(kRemainderFN);
        CHECK_LABEL(kRemainderS);
        CHECK_LABEL(kRemainderSN);
        CHECK_LABEL(kRemainderU);
        CHECK_LABEL(kRemainderUN);
        CHECK_LABEL(kReturn);
        CHECK_LABEL(kReturnValue);
        CHECK_LABEL(kScalarToMatrix);
        CHECK_LABEL(kSelect);
        CHECK_LABEL(kShiftLeft);
        CHECK_LABEL(kShiftRightS);
        CHECK_LABEL(kShiftRightU);
        CHECK_LABEL(kSignedToFloat);
        CHECK_LABEL(kSin);
        CHECK_LABEL(kSplat);
        CHECK_LABEL(kSqrt);
        CHECK_LABEL(kStore);
        CHECK_LABEL(kStoreN);
        CHECK_LABEL(kStoreDirect);
        CHECK_LABEL(kStoreDirectN);
        CHECK_LABEL(kStoreParameter);
        CHECK_LABEL(kStoreParameterN);
        CHECK_LABEL(kStoreParameterDirect);
        CHECK_LABEL(kStoreParameterDirectN);
        CHECK_LABEL(kStoreStack);
        CHECK_LABEL(kStoreStackN);
        CHECK_LABEL(kStoreStackDirect);
        CHECK_LABEL(kStoreStackDirectN);
        CHECK_LABEL(kSubtractF);
        CHECK_LABEL(kSubtractFN);
        CHECK_LABEL(kSubtractI);
        CHECK_LABEL(kSubtractIN);
        CHECK_LABEL(kTan);
        CHECK_LABEL(kUnsignedToFloat);
        CHECK_LABEL(kWriteExternal);
        CHECK_LABEL(kXor);
#endif
        auto mask = [&]() { return *context.fMaskStack & *context.fLoopStack; };
        auto parameterBase = [&]() {
            return context.fCallStack.empty() ? context.fStack
                                              : context.fCallStack.top().fParameters;
        };
        const uint8_t* code = f->fCode.data();
        const uint8_t* ip = code;
#ifdef SKSL_THREADED_CODE
        #ifdef TRACE
            const uint8_t* trace_ip = ip;
            printf("0: ");
            disassemble(&trace_ip);
        #endif
        goto *labels[(int) read<ByteCode::Instruction>(&ip)];
#else
        for (;;) {
            #ifdef TRACE
                const uint8_t* trace_ip = ip;
                disassemble(&trace_ip);
            #endif
            ByteCode::Instruction inst = read<ByteCode::Instruction>(&ip);
            switch (inst) {
#endif
                VECTOR_BINARY_OP(kAddF, fFloat, fFloat, +)
                VECTOR_BINARY_OP(kAddI, fInt, fInt, +)
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
                VECTOR_BINARY_OP(kSubtractF, fFloat, fFloat, -)
                VECTOR_BINARY_OP(kSubtractI, fInt, fInt, -)
                VECTOR_BINARY_OP(kDivideF, fFloat, fFloat, /)
                MASKED_VECTOR_BINARY_OP(kDivideS, fInt, fInt, /)
                MASKED_VECTOR_BINARY_OP(kDivideU, fUInt, fUInt, /)
                MASKED_VECTOR_BINARY_OP(kRemainderS, fInt, fInt, %)
                MASKED_VECTOR_BINARY_OP(kRemainderU, fUInt, fUInt, %)
                VECTOR_BINARY_OP(kMultiplyF, fFloat, fFloat, *)
                VECTOR_BINARY_OP(kMultiplyI, fInt, fInt, *)
                BINARY_OP(kOr, fInt, fInt, |)
                BINARY_OP(kXor, fInt, fInt, ^)
                LABEL(kAbort)
                    SkASSERT(false);
                    return false;
                LABEL(kBoundsCheck) {
                    ByteCode::Register r = read<ByteCode::Register>(&ip);
                    int length = read<int>(&ip);
                    if (skvx::any(mask() & ((fRegisters[r.fIndex].fInt < 0) |
                                            (fRegisters[r.fIndex].fInt >= length)))) {
                        return false;
                    }
                    NEXT();
                }
                LABEL(kBranch) {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    ip = code + target.fAddress;
                    NEXT();
                }
                LABEL(kBranchIfAllFalse) {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    if (!skvx::any(mask())) {
                        ip = code + target.fAddress;
                    }
                    NEXT();
                }
                LABEL(kBreak)
                    *context.fLoopStack &= ~mask();
                    NEXT();
                LABEL(kCall) {
                    ByteCode::Register returnValue = read<ByteCode::Register>(&ip);
                    uint8_t idx = read<uint8_t>(&ip);
                    ByteCode::Register args = read<ByteCode::Register>(&ip);
                    const ByteCodeFunction* target = fCode->fFunctions[idx].get();
                    int stackSlotCount = target->fStackSlotCount + target->fParameterSlotCount;
                    context.fCallStack.push(StackFrame(f, ip, stackSlotCount,
                                                       &fRegisters[args.fIndex],
                                                       &fRegisters[returnValue.fIndex]));
                    f = target;
                    code = f->fCode.data();
                    ip = code;
                    context.fStack -= stackSlotCount;
                    memcpy(context.fStack, &fRegisters[args.fIndex],
                           f->fParameterSlotCount * sizeof(Vector));
                    NEXT();
                }
                LABEL(kCallExternal) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    uint8_t index = read<uint8_t>(&ip);
                    uint8_t targetSize = read<uint8_t>(&ip);
                    ByteCode::Register arguments = read<ByteCode::Register>(&ip);
                    uint8_t argumentSize = read<uint8_t>(&ip);
                    ExternalValue* v = fCode->fExternalValues[index];
                    float tmpReturn[64];
                    SkASSERT(targetSize < 64);
                    float tmpArgs[64];
                    SkASSERT(argumentSize < 64);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            for (int j = 0; j < argumentSize; j++) {
                                tmpArgs[j] = fRegisters[arguments.fIndex + j].fFloat[i];
                            }
                            v->call(baseIndex + i, tmpArgs, tmpReturn);
                            for (int j = 0; j < targetSize; j++) {
                                fRegisters[target.fIndex + j].fFloat[i] = tmpReturn[j];
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kContinue) {
                    VectorI m = mask();
                    *context.fContinueStack |= m;
                    *context.fLoopStack &= ~m;
                    NEXT();
                }
                LABEL(kCopy) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fInt = fRegisters[src.fIndex].fInt;
                    NEXT();
                }
                VECTOR_UNARY_FN(kCos, cosf)
                LABEL(kFloatToSigned) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = Vector(skvx::cast<int32_t>(
                                                       fRegisters[src.fIndex].fFloat));
                    NEXT();
                }
                LABEL(kFloatToUnsigned) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = Vector(skvx::cast<uint32_t>(
                                                       fRegisters[src.fIndex].fFloat));
                    NEXT();
                }
                LABEL(kImmediate) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Immediate src = read<ByteCode::Immediate>(&ip);
                    fRegisters[target.fIndex].fInt = src.fInt;
                    NEXT();
                }
                LABEL(kInverse2x2) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Inverse2x2(&fRegisters[src.fIndex], &fRegisters[target.fIndex]);
                    NEXT();
                }
                LABEL(kInverse3x3) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Inverse3x3(&fRegisters[src.fIndex], &fRegisters[target.fIndex]);
                    NEXT();
                }
                LABEL(kInverse4x4) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Inverse4x4(&fRegisters[src.fIndex], &fRegisters[target.fIndex]);
                    NEXT();
                }
                LABEL(kLoad) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            fRegisters[target.fIndex].fInt[i] =
                                                    fMemory[fRegisters[src.fIndex].fInt[i]].fInt[i];
                        }
                    }
                    NEXT();
                }
                LABEL(kLoadN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            for (int j = 0; j < count; ++j) {
                                fRegisters[target.fIndex + j].fInt[i] =
                                                fMemory[fRegisters[src.fIndex].fInt[i] + j].fInt[i];
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kLoadDirect) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Pointer src = read<ByteCode::Pointer>(&ip);
                    fRegisters[target.fIndex].fInt = fMemory[src.fAddress].fInt;
                    NEXT();
                }
                LABEL(kLoadDirectN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Pointer src = read<ByteCode::Pointer>(&ip);
                    for (int i = 0; i < count; ++i) {
                        fRegisters[target.fIndex + i].fInt = fMemory[src.fAddress + i].fInt;
                    }
                    NEXT();
                }
                LABEL(kLoadParameter) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Vector* base = parameterBase();
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            fRegisters[target.fIndex].fInt[i] =
                                                       base[fRegisters[src.fIndex].fInt[i]].fInt[i];
                        }
                    }
                    NEXT();
                }
                LABEL(kLoadParameterN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Vector* base = parameterBase();
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            for (int j = 0; j < count; ++j) {
                                fRegisters[target.fIndex + j].fInt[i] =
                                                   base[fRegisters[src.fIndex].fInt[i] + j].fInt[i];
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kLoadParameterDirect) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Pointer src = read<ByteCode::Pointer>(&ip);
                    Vector* base = parameterBase();
                    fRegisters[target.fIndex].fInt = base[src.fAddress].fInt;
                    NEXT();
                }
                LABEL(kLoadParameterDirectN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Pointer src = read<ByteCode::Pointer>(&ip);
                    Vector* base = parameterBase();
                    for (int i = 0; i < count; ++i) {
                        fRegisters[target.fIndex + i].fInt = base[src.fAddress + i].fInt;
                    }
                    NEXT();
                }
                LABEL(kLoadStack) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            fRegisters[target.fIndex].fInt[i] =
                                             context.fStack[fRegisters[src.fIndex].fInt[i]].fInt[i];
                        }
                    }
                    NEXT();
                }
                LABEL(kLoadStackN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            for (int j = 0; j < count; ++j) {
                                fRegisters[target.fIndex + j].fInt[i] =
                                         context.fStack[fRegisters[src.fIndex].fInt[i] + j].fInt[i];
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kLoadStackDirect) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Pointer src = read<ByteCode::Pointer>(&ip);
                    CHECK_STACK_BOUNDS(src.fAddress);
                    fRegisters[target.fIndex].fInt = context.fStack[src.fAddress].fInt;
                    NEXT();
                }
                LABEL(kLoadStackDirectN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Pointer src = read<ByteCode::Pointer>(&ip);
                    CHECK_STACK_BOUNDS(src.fAddress);
                    for (int i = 0; i < count; ++i) {
                        fRegisters[target.fIndex + i].fInt = context.fStack[src.fAddress + i].fInt;
                    }
                    NEXT();
                }
                LABEL(kLoopBegin) {
                    context.fLoopStack[1] = context.fLoopStack[0];
                    ++context.fLoopStack;
                    context.fContinueStack[1] = 0;
                    ++context.fContinueStack;
                    NEXT();
                }
                LABEL(kLoopEnd) {
                    --context.fLoopStack;
                    --context.fContinueStack;
                    NEXT();
                }
                LABEL(kLoopMask) {
                    ByteCode::Register value = read<ByteCode::Register>(&ip);
                    *context.fLoopStack &= fRegisters[value.fIndex].fInt;
                    NEXT();
                }
                LABEL(kLoopNext) {
                    *context.fLoopStack |= *context.fContinueStack;
                    *context.fContinueStack = 0;
                    NEXT();
                }
                LABEL(kMaskNegate) {
                    *context.fMaskStack = context.fMaskStack[-1] & ~context.fCondStack[0];
                    NEXT();
                }
                LABEL(kMaskPop) {
                    --context.fMaskStack;
                    --context.fCondStack;
                    NEXT();
                }
                LABEL(kMaskPush) {
                    ByteCode::Register value = read<ByteCode::Register>(&ip);
                    context.fCondStack[1] = fRegisters[value.fIndex].fInt;
                    context.fMaskStack[1] = context.fMaskStack[0] & context.fCondStack[1];
                    ++context.fCondStack;
                    ++context.fMaskStack;
                    NEXT();
                }
                LABEL(kMatrixMultiply) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register left = read<ByteCode::Register>(&ip);
                    ByteCode::Register right = read<ByteCode::Register>(&ip);
                    uint8_t lCols = read<uint8_t>(&ip);
                    uint8_t lRows = read<uint8_t>(&ip);
                    uint8_t rCols = read<uint8_t>(&ip);
                    uint8_t rRows = lCols;
                    memset(&fRegisters[target.fIndex], 0, sizeof(Vector) * rCols * lRows);
                    for (int c = 0; c < rCols; ++c) {
                        for (int r = 0; r < lRows; ++r) {
                            for (int j = 0; j < lCols; ++j) {
                                fRegisters[target.fIndex + c * lRows + r].fFloat +=
                                        fRegisters[left.fIndex + j * lRows + r].fFloat *
                                        fRegisters[right.fIndex + c * rRows + j].fFloat;
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kMatrixToMatrix) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    uint8_t srcColumns = read<uint8_t>(&ip);
                    uint8_t srcRows = read<uint8_t>(&ip);
                    uint8_t dstColumns = read<uint8_t>(&ip);
                    uint8_t dstRows = read<uint8_t>(&ip);
                    int offset = 0;
                    for (int i = 0; i < dstColumns; ++i) {
                        for (int j = 0; j < dstRows; ++j) {
                            if (i < srcColumns && j < srcRows) {
                                fRegisters[target.fIndex + offset] =
                                                         fRegisters[src.fIndex + (srcRows * i) + j];
                            } else {
                                if (i == j) {
                                    fRegisters[target.fIndex + offset].fFloat = 1;
                                } else {
                                    fRegisters[target.fIndex + offset].fFloat = 0;
                                }
                            }
                            ++offset;
                        }
                    }
                    NEXT();
                }
                LABEL(kNegateF) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fFloat = -fRegisters[src.fIndex].fFloat;
                    NEXT();
                }
                LABEL(kNegateS) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fInt = -fRegisters[src.fIndex].fInt;
                    NEXT();
                }
                LABEL(kNop)
                    NEXT();
                LABEL(kNot) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fInt = ~fRegisters[src.fIndex].fInt;
                    NEXT();
                }
                LABEL(kPrint) {
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    if (skvx::any(mask())) {
                        printf("[");
                        const char* separator = "";
                        for (int i = 0; i < width; ++i) {
                            if (mask()[i]) {
                                printf("%s%f", separator, fRegisters[src.fIndex].fFloat[i]);
                            }
                            else {
                                printf("%s-", separator);
                            }
                            separator = ", ";
                        }
                        printf("]\n");
                    }
                    NEXT();
                }
                LABEL(kReadExternal) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    uint8_t count = read<uint8_t>(&ip);
                    uint8_t index = read<uint8_t>(&ip);
                    SkASSERT(count <= 4);
                    SkASSERT(fCode->fExternalValues.size() > index);
                    float tmp[4];
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            fCode->fExternalValues[index]->read(baseIndex + i, tmp);
                            for (int j = 0; j < count; ++j) {
                                fRegisters[target.fIndex + j].fFloat[i] = tmp[j];
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kRemainderF) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src1 = read<ByteCode::Register>(&ip);
                    ByteCode::Register src2 = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = VecMod(fRegisters[src1.fIndex],
                                                       fRegisters[src2.fIndex]);
                    NEXT();
                }
                LABEL(kRemainderFN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src1 = read<ByteCode::Register>(&ip);
                    ByteCode::Register src2 = read<ByteCode::Register>(&ip);
                    for (int i = 0; i < count; ++i) {
                        fRegisters[target.fIndex + i] = VecMod(fRegisters[src1.fIndex + i],
                                                               fRegisters[src2.fIndex + i]);
                    }
                    NEXT();
                }
                LABEL(kReturn) {
                    if (context.fCallStack.empty()) {
                        return true;
                    }
                    StackFrame frame = context.fCallStack.top();
                    f = frame.fFunction;
                    code = f->fCode.data();
                    ip = frame.fIP;
                    context.fStack += frame.fStackSlotCount;
                    context.fCallStack.pop();
                    NEXT();
                }
                LABEL(kReturnValue) {
                    ByteCode::Register returnValue = read<ByteCode::Register>(&ip);
                    if (context.fCallStack.empty()) {
                        if (outResult) {
                            *outResult = &fRegisters[returnValue.fIndex];
                        }
                        return true;
                    }
                    StackFrame frame = context.fCallStack.top();
                    ip = frame.fIP;
                    context.fStack += frame.fStackSlotCount;
                    memcpy(frame.fReturnValue, &fRegisters[returnValue.fIndex],
                           sizeof(Vector) * f->fReturnSlotCount);
                    f = frame.fFunction;
                    code = f->fCode.data();
                    context.fCallStack.pop();
                    NEXT();
                }
                LABEL(kScalarToMatrix) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    uint8_t columns = read<uint8_t>(&ip);
                    uint8_t rows = read<uint8_t>(&ip);
                    int offset = 0;
                    for (int i = 0; i < columns; ++i) {
                        for (int j = 0; j < rows; ++j) {
                            if (i == j) {
                                fRegisters[target.fIndex + offset] = fRegisters[src.fIndex];
                            } else {
                                fRegisters[target.fIndex + offset].fFloat = 0;
                            }
                            ++offset;
                        }
                    }
                    NEXT();
                }
                LABEL(kSelect) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register test = read<ByteCode::Register>(&ip);
                    ByteCode::Register src1 = read<ByteCode::Register>(&ip);
                    ByteCode::Register src2 = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = skvx::if_then_else(fRegisters[test.fIndex].fInt,
                                                                   fRegisters[src1.fIndex].fFloat,
                                                                   fRegisters[src2.fIndex].fFloat);
                    NEXT();
                }
                LABEL(kShiftLeft) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    uint8_t count = read<uint8_t>(&ip);
                    fRegisters[target.fIndex].fInt = fRegisters[src.fIndex].fInt << count;
                    NEXT();
                }
                LABEL(kShiftRightS) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    int8_t count = read<int8_t>(&ip);
                    fRegisters[target.fIndex].fInt = fRegisters[src.fIndex].fInt >> count;
                    NEXT();
                }
                LABEL(kShiftRightU) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    uint8_t count = read<uint8_t>(&ip);
                    fRegisters[target.fIndex].fUInt = fRegisters[src.fIndex].fUInt >> count;
                    NEXT();
                }
                LABEL(kSignedToFloat) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = Vector(skvx::cast<float>(
                                                                      fRegisters[src.fIndex].fInt));
                    NEXT();
                }
                VECTOR_UNARY_FN(kSin, sinf)
                LABEL(kSplat) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    for (int i = 0; i < count; ++i) {
                        fRegisters[target.fIndex + i] = fRegisters[src.fIndex];
                    }
                    NEXT();
                }
                LABEL(kSqrt) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex].fFloat = skvx::sqrt(fRegisters[src.fIndex].fFloat);
                    NEXT();
                }
                LABEL(kStore) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            fMemory[fRegisters[target.fIndex].fInt[i]].fInt[i] =
                                                                     fRegisters[src.fIndex].fInt[i];
                        }
                    }
                    NEXT();
                }
                LABEL(kStoreN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            for (int j = 0; j < count; ++j) {
                                fMemory[fRegisters[target.fIndex].fInt[i] + j].fInt[i] =
                                                                 fRegisters[src.fIndex + j].fInt[i];
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kStoreDirect) {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fMemory[target.fAddress] = skvx::if_then_else(mask(),
                                                                  fRegisters[src.fIndex].fFloat,
                                                                  fMemory[target.fAddress].fFloat);
                    NEXT();
                }
                LABEL(kStoreDirectN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    for (int i = 0; i < count; ++i) {
                        fMemory[target.fAddress + i] = skvx::if_then_else(
                                                               mask(),
                                                               fRegisters[src.fIndex + i].fFloat,
                                                               fMemory[target.fAddress + i].fFloat);
                    }
                    NEXT();
                }
                LABEL(kStoreParameter) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Vector* base = parameterBase();
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            base[fRegisters[target.fIndex].fInt[i]].fInt[i] =
                                                                     fRegisters[src.fIndex].fInt[i];
                        }
                    }
                    NEXT();
                }
                LABEL(kStoreParameterN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Vector* base = parameterBase();
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            for (int j = 0; j < count; ++j) {
                                base[fRegisters[target.fIndex].fInt[i] + j].fInt[i] =
                                                                 fRegisters[src.fIndex + j].fInt[i];
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kStoreParameterDirect) {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Vector* base = parameterBase();
                    base[target.fAddress].fFloat = skvx::if_then_else(mask(),
                                                                      fRegisters[src.fIndex].fFloat,
                                                                      base[target.fAddress].fFloat);
                    NEXT();
                }
                LABEL(kStoreParameterDirectN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    Vector* base = parameterBase();
                    for (int i = 0; i < count; ++i) {
                        base[target.fAddress + i].fFloat = skvx::if_then_else(
                                                                  mask(),
                                                                  fRegisters[src.fIndex + i].fFloat,
                                                                  base[target.fAddress + i].fFloat);
                    }
                    NEXT();
                }
                LABEL(kStoreStack) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            context.fStack[fRegisters[target.fIndex].fInt[i]].fInt[i] =
                                                                     fRegisters[src.fIndex].fInt[i];
                        }
                    }
                    NEXT();
                }
                LABEL(kStoreStackN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            for (int j = 0; j < count; ++j) {
                                context.fStack[fRegisters[target.fIndex].fInt[i] + j].fInt[i] =
                                                                 fRegisters[src.fIndex + j].fInt[i];
                            }
                        }
                    }
                    NEXT();
                }
                LABEL(kStoreStackDirect) {
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    CHECK_STACK_BOUNDS(target.fAddress);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    context.fStack[target.fAddress] = skvx::if_then_else(
                                                            mask(),
                                                            fRegisters[src.fIndex].fFloat,
                                                            context.fStack[target.fAddress].fFloat);
                    NEXT();
                }
                LABEL(kStoreStackDirectN) {
                    uint8_t count = read<uint8_t>(&ip);
                    ByteCode::Pointer target = read<ByteCode::Pointer>(&ip);
                    CHECK_STACK_BOUNDS(target.fAddress);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    for (int i = 0; i < count; ++i) {
                        context.fStack[target.fAddress + i] = skvx::if_then_else(
                                                        mask(),
                                                        fRegisters[src.fIndex + i].fFloat,
                                                        context.fStack[target.fAddress + i].fFloat);
                    }
                    NEXT();
                }
                VECTOR_UNARY_FN(kTan, tanf)
                LABEL(kUnsignedToFloat) {
                    ByteCode::Register target = read<ByteCode::Register>(&ip);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    fRegisters[target.fIndex] = Vector(skvx::cast<float>(
                                                                     fRegisters[src.fIndex].fUInt));
                    NEXT();
                }
                LABEL(kWriteExternal) {
                    uint8_t index = read<uint8_t>(&ip);
                    uint8_t count = read<uint8_t>(&ip);
                    SkASSERT(count <= 4);
                    SkASSERT(fCode->fExternalValues.size() > index);
                    ByteCode::Register src = read<ByteCode::Register>(&ip);
                    float tmp[4];
                    VectorI m = mask();
                    for (int i = 0; i < width; ++i) {
                        if (m[i]) {
                            for (int j = 0; j < count; ++j) {
                                tmp[j] = fRegisters[src.fIndex + j].fFloat[i];
                            }
                            fCode->fExternalValues[index]->write(baseIndex + i, tmp);
                        }
                    }
                    NEXT();
                }
#ifndef SKSL_THREADED_CODE
            }
        }
#endif
    }

    const std::unique_ptr<ByteCode> fCode;

    void* fBackingStore;

    Vector* fRegisters;

    Vector* fMemory;

    friend class ByteCode;

    friend class ByteCodeGenerator;
};

#undef BINARY_OP
#undef CHECK_STACK_BOUNDS

} // namespace

#endif
