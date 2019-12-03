/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BYTECODE
#define SKSL_BYTECODE

#include "include/private/SkOnce.h"
#include "include/private/SkVx.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

#include <memory>
#include <vector>

namespace SkSL {

class ByteCode;

class ByteCodeFunction {
public:
    // all counts are of 32-bit values, so a float4 counts as 4 parameter or return slots
    struct Parameter {
        int fSlotCount;
        bool fIsOutParameter;
    };

    /**
     * Note that this is the actual number of parameters, not the number of parameter slots.
     */
    int getParameterCount() const { return fParameters.size(); }

    Parameter getParameter(int idx) const { return fParameters[idx]; }

    int getParameterSlotCount() const { return fParameterSlotCount; }

    int getReturnSlotCount() const { return fReturnSlotCount; }

    void disassemble() const { }

private:
    ByteCodeFunction(const FunctionDeclaration* declaration)
        : fName(declaration->fName) {}

    String fName;

    std::vector<Parameter> fParameters;

    int fParameterSlotCount;

    int fReturnSlotCount;

    int fStackSlotCount;

    std::vector<uint8_t> fCode;

    friend class ByteCode;
    friend class ByteCodeGenerator;
    template<int width>
    friend class Interpreter;
};

class SK_API ByteCode {
public:
    template<int width>
    union Vector {
        skvx::Vec<width, float> fFloat;
        skvx::Vec<width, int32_t> fInt;
        skvx::Vec<width, uint32_t> fUInt;

        Vector() = default;

        Vector(skvx::Vec<width, float> f)
            : fFloat(f) {}

        Vector(skvx::Vec<width, int32_t> i)
            : fInt(i) {}

        Vector(skvx::Vec<width, uint32_t> u)
            : fUInt(u) {}
    };

    enum class Instruction : uint8_t {
        // no parameters
        kNop,
        // Register target, Register src1, Register src2
        kAddF,
        // Register target, Register src1, Register src2
        kAddI,
        // Register target, Register src1, Register src2
        kAnd,
        // Pointer target
        kBranch,
        // Pointer target
        kBranchIfAllFalse,
        // no parameters
        kBreak,
        // uint8_t functionIndex, Register target, [Register param1, ...]
        kCall,
        // Register target, Register src1, Register src2
        kCompareEQF,
        // Register target, Register src1, Register src2
        kCompareEQI,
        // Register target, Register src1, Register src2
        kCompareNEQF,
        // Register target, Register src1, Register src2
        kCompareNEQI,
        // Register target, Register src1, Register src2
        kCompareGTF,
        // Register target, Register src1, Register src2
        kCompareGTS,
        // Register target, Register src1, Register src2
        kCompareGTU,
        // Register target, Register src1, Register src2
        kCompareGTEQF,
        // Register target, Register src1, Register src2
        kCompareGTEQS,
        // Register target, Register src1, Register src2
        kCompareGTEQU,
        // Register target, Register src1, Register src2
        kCompareLTF,
        // Register target, Register src1, Register src2
        kCompareLTS,
        // Register target, Register src1, Register src2
        kCompareLTU,
        // Register target, Register src1, Register src2
        kCompareLTEQF,
        // Register target, Register src1, Register src2
        kCompareLTEQS,
        // Register target, Register src1, Register src2
        kCompareLTEQU,
        // no parameters
        kContinue,
        // Register target, Register src
        kCopy,
        // Register target, Register src,
        kCos,
        // Register target, Register src1, Register src2
        kDivideF,
        // Register target, Register src1, Register src2
        kDivideS,
        // Register target, Register src1, Register src2
        kDivideU,
        // Register target, Register src
        kFloatToSigned,
        // Register target, Register src
        kFloatToUnsigned,
        // Load a constant into a register
        // Register target, Immediate value
        kImmediate,
        // Load the memory cell pointed to by srcPtr into a register
        // Register target, Register srcPtr
        kLoad,
        // Load the memory cell pointed to by src into a register
        // Register target, Pointer src
        kLoadDirect,
        // Load the parameter slot pointed to by srcPtr into a register
        // Register target, Register srcPtr
        kLoadParameter,
        // Load the parameter slot pointed to by src into a register
        // Register target, Pointer src
        kLoadParameterDirect,
        // Load the stack cell pointed to by srcPtr + sp into a register
        // Register target, Register srcPtr
        kLoadStack,
        // Load the stack cell pointed to by src + sp into a register
        // Register target, Pointer src
        kLoadStackDirect,
        // Pushes a new loop onto the loop and continue stacks
        // no parameters
        kLoopBegin,
        // Pops the loop and continue stacks
        // no parameters
        kLoopEnd,
        // Register mask
        kLoopMask,
        // Register mask
        kLoopNext,
        // no parameters
        kMaskNegate,
        // no parameters
        kMaskPop,
        // Register mask
        kMaskPush,
        // Register target, Register src1, Register src2
        kMultiplyF,
        // Register target, Register src1, Register src2
        kMultiplyI,
        // Register target, Register src
        kNegateF,
        // Register target, Register src
        kNegateS,
        // Register target, Register src
        kNot,
        // Register target, Register src1, Register src2
        kOr,
        // Register target, Register src1, Register src2
        kRemainderF,
        // Register target, Register src1, Register src2
        kRemainderS,
        // Register target, Register src1, Register src2
        kRemainderU,
        // no parameters
        kReturn,
        // Returns a single value. To return more than one slot, the caller allocates storage space
        // for the return value and passes a pointer to it as an extra parameter to the function,
        // then uses kReturn instead of kReturnValue.
        // Register value
        kReturnValue,
        // Register target, Register test, Register ifTrue, Register ifFalse
        kSelect,
        // Register target, Register src, uint8_t count
        kShiftLeft,
        // Register target, Register src, uint8_t count
        kShiftRightS,
        // Register target, Register src, uint8_t count
        kShiftRightU,
        // Register target, Register src
        kSignedToFloat,
        // Register target, Register src,
        kSin,
        // Register target, Register src,
        kSqrt,
        // Store to the memory cell pointed to by dstPtr
        // Register dstPtr, Register src
        kStore,
        // Store to the memory cell pointed to by dst
        // Pointer dst, Register src
        kStoreDirect,
        // Store to the parameter slot pointed to by dstPtr
        // Register dstPtr, Register src
        kStoreParameter,
        // Store to the parameter slot pointed to by dst
        // Pointer dst, Register src
        kStoreParameterDirect,
        // Stores a register into the stack cell pointed to by dst + sp
        // Register dst, Register src
        kStoreStack,
        // Stores a register into the stack cell pointed to by dstPtr + sp
        // Pointer dst, Register src
        kStoreStackDirect,
        // Register target, Register src1, Register src2
        kSubtractF,
        // Register target, Register src1, Register src2
        kSubtractI,
        // Register target, Register src,
        kTan,
        // Register target, Register src,
        kUnsignedToFloat,
        // Register target, Register src1, Register src2
        kXor,
    };


    // Compound values like vectors span multiple Registers or Pointer addresses. We always refer to
    // them by the address of their first slot, so for instance if you add two float4's together,
    // the resulting Register contains the first channel of the result, with the other three
    // channels following in the next three Registers.

    struct Register {
        uint16_t fIndex;

        Register operator+(uint16_t offset) const {
            return Register{(uint16_t) (fIndex + offset)};
        }
    };

    struct Pointer {
        uint16_t fAddress;

        Pointer operator+(uint16_t offset) const {
            return Pointer{(uint16_t) (fAddress + offset)};
        }
    };

    union Immediate {
        float fFloat;
        int32_t fInt;
        uint32_t fUInt;

        Immediate() {}

        Immediate(float f)
            : fFloat(f) {}

        Immediate(int32_t i)
            : fInt(i) {}

        Immediate(uint32_t u)
            : fUInt(u) {}
    };

    static constexpr int kPointerMax = 65535;
    static constexpr int kRegisterMax = 65535;
    static constexpr int kVecWidth = 16;

    using VectorF = skvx::Vec<kVecWidth, float>;
    using VectorS = skvx::Vec<kVecWidth, int32_t>;
    using VectorU = skvx::Vec<kVecWidth, uint32_t>;

    const ByteCodeFunction* getFunction(const char* name) const {
        for (const auto& f : fFunctions) {
            if (f->fName == name) {
                return f.get();
            }
        }
        return nullptr;
    }

    int getGlobalCount() const {
        return fGlobalCount;
    }

    int getUniformCount() const {
        return fUniformCount;
    }

private:
    std::vector<std::unique_ptr<ByteCodeFunction>> fFunctions;

    int fGlobalCount;

    int fUniformCount;

    friend class ByteCodeGenerator;
    template<int width>
    friend class Interpreter;
};

} // namespace

#endif
