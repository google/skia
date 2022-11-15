/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/codegen/SkSLRasterPipelineCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <string>
#include <vector>

namespace SkSL {

class IRNode;

namespace RP {

class Generator {
public:
    Generator(const SkSL::Program& program) : fProgram(program) {}

    /** Converts the SkSL main() function into a set of Instructions. */
    void writeProgram(const FunctionDefinition& function);

    /** Converts an SkSL function into a set of Instructions. */
    SlotRange writeFunction(const IRNode& callSite,
                            const FunctionDefinition& function,
                            SkSpan<const SlotRange> args);

    /** Implements low-level slot creation; slots will not be known to the debugger. */
    SlotRange createSlots(int numSlots);

    /** Creates slots associated with an SkSL variable or return value. */
    SlotRange createSlots(std::string name,
                          const Type& type,
                          int extraSlots,
                          Position pos,
                          bool isFunctionReturnValue);

    /** Looks up the slots associated with an SkSL variable; creates the slot if necessary. */
    SlotRange getSlots(const Variable& v);

    /**
     * Looks up the slots associated with an SkSL function's return value and returned-mask; creates
     * the range if necessary. Note that recursion is never supported, so we don't need to maintain
     * return values in a stack; we can just statically allocate one slot per function call-site.
     */
    struct FunctionSlots {
        SlotRange fReturnSlots;
        Slot      fReturnedMask;
    };
    FunctionSlots getFunctionSlots(const IRNode& callSite, const FunctionDeclaration& f);

    /** The Builder stitches our instructions together into Raster Pipeline code. */
    Builder* builder() { return &fBuilder; }

private:
    static FunctionSlots FunctionSlotsFromRange(SlotRange fs);

    [[maybe_unused]] const SkSL::Program& fProgram;
    Builder fBuilder;

    SkTHashMap<const IRNode*, SlotRange> fSlotMap;
    int fSlotCount = 0;

    SkTArray<FunctionSlots> fFunctionStack;
};

SlotRange Generator::createSlots(int numSlots) {
    SlotRange range = {fSlotCount, numSlots};
    fSlotCount += numSlots;
    return range;
}

SlotRange Generator::createSlots(std::string name,
                                 const Type& type,
                                 int extraSlots,
                                 Position pos,
                                 bool isFunctionReturnValue) {
    // TODO(skia:13676): `name`, `pos` and `isFunctionReturnValue` will be used by the debugger.
    // For now, ignore these and just create the raw slots.
    return this->createSlots(type.slotCount() + extraSlots);
}

SlotRange Generator::getSlots(const Variable& v) {
    SlotRange* entry = fSlotMap.find(&v);
    if (entry != nullptr) {
        return *entry;
    }
    SlotRange range = this->createSlots(std::string(v.name()),
                                        v.type(),
                                        /*extraSlots=*/0,
                                        v.fPosition,
                                        /*isFunctionReturnValue=*/false);
    fSlotMap.set(&v, range);
    return range;
}

Generator::FunctionSlots Generator::FunctionSlotsFromRange(SlotRange range) {
    // We allocate one extra slot after a function result so that we can hold the returned-mask.
    SkASSERT(range.count >= 1);

    FunctionSlots fs;
    fs.fReturnSlots = SlotRange{range.index, range.count - 1};
    fs.fReturnedMask = range.index + fs.fReturnSlots.count;
    return fs;
}

Generator::FunctionSlots Generator::getFunctionSlots(const IRNode& callSite,
                                                     const FunctionDeclaration& f) {
    SlotRange* entry = fSlotMap.find(&callSite);
    if (entry != nullptr) {
        return FunctionSlotsFromRange(*entry);
    }
    SlotRange range = this->createSlots("[" + std::string(f.name()) + "].result",
                                        f.returnType(),
                                        /*extraSlots=*/1,
                                        f.fPosition,
                                        /*isFunctionReturnValue=*/true);
    fSlotMap.set(&callSite, range);
    return FunctionSlotsFromRange(range);
}

SlotRange Generator::writeFunction(const IRNode& callSite,
                                   const FunctionDefinition& function,
                                   SkSpan<const SlotRange> args) {
    fFunctionStack.push_back(this->getFunctionSlots(callSite, function.declaration()));

    // TODO(skia:13676): support all return types
    // For now, assert that the function returns a vec4.
    SkASSERT(function.declaration().returnType().isVector() &&
             function.declaration().returnType().slotCount() == 4 &&
             function.declaration().returnType().componentType().isFloat());

    SlotRange functionResult = fFunctionStack.back().fReturnSlots;
    SkASSERT(functionResult.count == 4);

    // TODO(skia:13676): emit a function body
    // For now, ignore the program and always return magenta (1 1 0 1).
    fBuilder.immediate_f(1.0f);
    fBuilder.store_unmasked(functionResult.index + 0);
    fBuilder.store_unmasked(functionResult.index + 1);
    fBuilder.store_unmasked(functionResult.index + 3);
    fBuilder.immediate_f(0.0f);
    fBuilder.store_unmasked(functionResult.index + 2);

    fFunctionStack.pop_back();

    return functionResult;
}

void Generator::writeProgram(const FunctionDefinition& function) {
    // Assign slots to the parameters of main; copy src and dst into those slots as appropriate.
    SkSTArray<2, SlotRange> args;
    for (const SkSL::Variable* param : function.declaration().parameters()) {
        switch (param->modifiers().fLayout.fBuiltin) {
            case SK_MAIN_COORDS_BUILTIN: {
                // Coordinates are passed via RG.
                SlotRange fragCoord = this->getSlots(*param);
                SkASSERT(fragCoord.count == 2);
                fBuilder.store_src_rg(fragCoord);
                args.push_back(fragCoord);
                break;
            }
            case SK_INPUT_COLOR_BUILTIN: {
                // Input colors are passed via RGBA.
                SlotRange srcColor = this->getSlots(*param);
                SkASSERT(srcColor.count == 4);
                fBuilder.store_src(srcColor);
                args.push_back(srcColor);
                break;
            }
            case SK_DEST_COLOR_BUILTIN: {
                // Dest colors are passed via dRGBA.
                SlotRange destColor = this->getSlots(*param);
                SkASSERT(destColor.count == 4);
                fBuilder.store_dst(destColor);
                args.push_back(destColor);
                break;
            }
            default: {
                SkDEBUGFAIL("Invalid parameter to main()");
                return;
            }
        }
    }

    // Invoke main().
    SlotRange mainResult = this->writeFunction(function, function, args);

    // Move the result of main() from slots into RGBA. Allow dRGBA to remain in a trashed state.
    SkASSERT(mainResult.count == 4);
    fBuilder.load_src(mainResult);
}

}  // namespace RP

std::unique_ptr<RP::Program> MakeRasterPipelineProgram(const SkSL::Program& program,
                                                       const FunctionDefinition& function) {
    // TODO(skia:13676): add mechanism for uniform passing
    RP::Generator generator(program);
    generator.writeProgram(function);
    return generator.builder()->finish();
}

}  // namespace SkSL
