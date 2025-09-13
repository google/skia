/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanSpirvTransforms.h"

#include "include/private/base/SkAssert.h"
#include "src/sksl/codegen/SkSLCodeGenTypes.h"
#include "src/sksl/spirv.h"

#ifdef SK_DEBUG
#include "src/gpu/graphite/Log.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLSPIRVValidator.h"
#endif

#include <cstdint>
#include <vector>

namespace skgpu::graphite {

namespace {

/*
 * The SPIR-V transformer is implemented as one main class, SpirvTransformer, and a number of
 * logically separate transformations (currently one), such as SpirvMultisampleTransformer.
 *
 * SpirvTransformer is a simple loop over the SPIR-V instructions. The instructions of interest are
 * parsed (as much as the transformations need), and passed to individual transformers.
 *
 * The transformers themselves may change an instruction, by generating new one(s) and dropping the
 * original instruction, or may add instructions triggered when encountering a specific instruction.
 *
 * This design lets multiple transformations be done in a single pass, avoiding the need to walk
 * over the SPIR-V again and again, most of which is uninteresting to the transformers.
 */

using SpirvBlob = std::vector<uint32_t>;

/*
 * SPIR-V's first five words are:
 *
 *     Index 0: Magic number
 *     Index 1: Version number
 *     Index 2: Generator's magic number
 *     Index 3: ID bound; all ids are smaller than this
 *     Index 4: 0 (reserved)
 *
 * The rest of the instructions start at index 5 in sections (see 2.4. Logical Layout of a Module in
 * the SPIR-V spec):
 *
 *     OpCapability instructions
 *     OpExtInst instructions
 *     OpExtInstImport instructions
 *     OpMemoryModel instructions
 *     OpEntryPoint instructions
 *     OpExecutionMode instructions
 *     Debug instructions
 *     Types, constants and global variable instructions
 *     Function declarations
 *
 * Most instructions are uninteresting to the transformer and are skipped over.
 */
constexpr uint32_t kIDBoundIndex = 3;
constexpr uint32_t kInstructionsStartIndex = 5;

SpvId GetNewId(SpirvBlob* blob) {
    // Return a new ID and increase the ID bound.
    return (*blob)[kIDBoundIndex]++;
}

SpvOp GetInstructionOp(const uint32_t* instruction) {
    return static_cast<SpvOp>(instruction[0] & SpvOpCodeMask);
}
uint32_t GetInstructionWordCount(const uint32_t* instruction) {
    return instruction[0] >> SpvWordCountShift;
}
uint32_t MakeInstructionOp(SpvOp opCode, uint32_t wordCount) {
    return opCode | wordCount << SpvWordCountShift;
}
void CopyInstruction(const uint32_t* instruction, size_t wordCount, SpirvBlob* spirvOut) {
    spirvOut->insert(spirvOut->end(), instruction, instruction + wordCount);
}

enum class TransformationState {
    Transformed,
    Unchanged,
};

/*
 * A transformer that at a high level changes subpassLoad(input) to subpassLoad(inputMS, sample),
 * such that shaders with input attachment are usable with multisampling. It unconditionally adds a
 * built-in SampleId variable and the corresponding SampleRateShading capability with the assumption
 * that the SPIR-V generator does not already produce them.
 */
class SpirvMultisampleTransformer {
public:
    SpirvMultisampleTransformer(uint32_t maxId) : fIsInputAttachmentImage(maxId + 1, false) {}

    void transformOpCapability(SpvCapability capability, SpirvBlob* spirvOut);
    TransformationState transformOpEntryPoint(const uint32_t* instruction, SpirvBlob* spirvOut);
    void transformOpDecorate(SpirvBlob* spirvOut);
    TransformationState transformOpTypePointer(const uint32_t* instruction, SpirvBlob* spirvOut);
    TransformationState transformOpTypeImage(SpvId resultId,
                                             SpvId sampledTypeId,
                                             SpirvBlob* spirvOut);
    void transformOpLoad(SpvId resultTypeId, SpvId resultId, SpvId pointerId, SpirvBlob* spirvOut);
    TransformationState transformOpImageRead(SpvId resultTypeId,
                                             SpvId resultId,
                                             SpvId imageId,
                                             SpvId coordinateId,
                                             SpirvBlob* spirvOut);

private:
    // Map OpLoad result to whether it was from the input attachment variable.
    std::vector<bool> fIsInputAttachmentImage;
    bool fHasAddedDecorations = false;
    SpvId fSampleIdVarId = 0;
};

void SpirvMultisampleTransformer::transformOpCapability(SpvCapability capability,
                                                        SpirvBlob* spirvOut) {
    // Add the SampleRateShading capability, once when the InputAttachment capability is encountered
    if (capability == SpvCapabilityInputAttachment) {
        // Generate:
        //     OpCapability SampleRateShading
        spirvOut->push_back(MakeInstructionOp(SpvOpCapability, 2));
        spirvOut->push_back(SpvCapabilitySampleRateShading);
    }
}

TransformationState SpirvMultisampleTransformer::transformOpEntryPoint(const uint32_t* instruction,
                                                                       SpirvBlob* spirvOut) {
    // Keep the entry point instruction as is, just append the SampleId variable to it.
    fSampleIdVarId = GetNewId(spirvOut);

    const uint32_t originalWordCount = GetInstructionWordCount(instruction);

    // The length of the instruction is increased by one
    spirvOut->push_back(MakeInstructionOp(SpvOpEntryPoint, originalWordCount + 1));
    // The rest of the instruction is copied verbatim
    CopyInstruction(instruction + 1, originalWordCount - 1, spirvOut);
    // The last word is the new variable
    spirvOut->push_back(fSampleIdVarId);

    return TransformationState::Transformed;
}

void SpirvMultisampleTransformer::transformOpDecorate(SpirvBlob* spirvOut) {
    // Add decorations for the new SampleId variable as soon as the decorations section is visited.
    // Note that there is always at least one decoration, because the shaders have _some_ input or
    // output.
    if (fHasAddedDecorations) {
        return;
    }

    // Generate:
    //     OpDecorate %SampleIdVar RelaxedPrecision
    //     OpDecorate %SampleIdVar Flat
    //     OpDecorate %SampleIdVar BuiltIn SampleId
    spirvOut->push_back(MakeInstructionOp(SpvOpDecorate, 3));
    spirvOut->push_back(fSampleIdVarId);
    spirvOut->push_back(SpvDecorationRelaxedPrecision);

    spirvOut->push_back(MakeInstructionOp(SpvOpDecorate, 3));
    spirvOut->push_back(fSampleIdVarId);
    spirvOut->push_back(SpvDecorationFlat);

    spirvOut->push_back(MakeInstructionOp(SpvOpDecorate, 4));
    spirvOut->push_back(fSampleIdVarId);
    spirvOut->push_back(SpvDecorationBuiltIn);
    spirvOut->push_back(SpvBuiltInSampleId);

    fHasAddedDecorations = true;
}

TransformationState SpirvMultisampleTransformer::transformOpTypePointer(const uint32_t* instruction,
                                                                        SpirvBlob* spirvOut) {
    // When the int pointer type declaration is encountered, declare the SampleId variable right
    // away.
    const SpvId resultId = instruction[1];
    if (resultId != SkSL::spirv::kIdTypePointerInputInt) {
        return TransformationState::Unchanged;
    }

    // Keep the int pointer declaration.
    CopyInstruction(instruction, GetInstructionWordCount(instruction), spirvOut);
    // Generate:
    //     %SampleIdVar = OpVariable %kIdTypePointerInputInt Input
    spirvOut->push_back(MakeInstructionOp(SpvOpVariable, 4));
    spirvOut->push_back(SkSL::spirv::kIdTypePointerInputInt);
    spirvOut->push_back(fSampleIdVarId);
    spirvOut->push_back(SpvStorageClassInput);

    return TransformationState::Transformed;
}

TransformationState SpirvMultisampleTransformer::transformOpTypeImage(SpvId resultId,
                                                                      SpvId sampledTypeId,
                                                                      SpirvBlob* spirvOut) {
    // Change the OpTypeImage instruction for the input attachment to be multisampled. The result id
    // is untouched, so the OpTypePointer and OpVariable instructions remain as-is!
    if (resultId != SkSL::spirv::kIdTypeImageSubpassData) {
        return TransformationState::Unchanged;
    }

    // The instruction must have been:
    //
    //     %resultId = OpTypeImage %sampledTypeId SubpassData 0 0 0 2 Unknown
    //
    // It is transformed to:
    //
    //     %resultId = OpTypeImage %sampledTypeId SubpassData 0 0 1 2 Unknown
    spirvOut->push_back(MakeInstructionOp(SpvOpTypeImage, 9));
    spirvOut->push_back(resultId);
    spirvOut->push_back(sampledTypeId);
    spirvOut->push_back(SpvDimSubpassData);
    spirvOut->push_back(0);
    spirvOut->push_back(0);
    spirvOut->push_back(1);
    spirvOut->push_back(2);
    spirvOut->push_back(SpvImageFormatUnknown);

    return TransformationState::Transformed;
}

void SpirvMultisampleTransformer::transformOpLoad(SpvId resultTypeId,
                                                  SpvId resultId,
                                                  SpvId pointerId,
                                                  SpirvBlob* spirvOut) {
    // If the input attachment variable is loaded, remember the result id. This is needed to know
    // which OpImageRead instructions to transform.
    if (pointerId == SkSL::spirv::kIdVariableImageSubpassData &&
        resultId < fIsInputAttachmentImage.size()) {
        fIsInputAttachmentImage[resultId] = true;
    }
}

TransformationState SpirvMultisampleTransformer::transformOpImageRead(SpvId resultTypeId,
                                                                      SpvId resultId,
                                                                      SpvId imageId,
                                                                      SpvId coordinateId,
                                                                      SpirvBlob* spirvOut) {
    // If reading from the input attachment variable, load from the SampleId variable and add a
    // `Sample` operand to the image read operation. The result id is kept as is, so the rest of the
    // SPIR-V is unaffected.
    if (imageId >= fIsInputAttachmentImage.size() || !fIsInputAttachmentImage[imageId]) {
        return TransformationState::Unchanged;
    }

    // The instruction is:
    //
    //     %resultId = OpImageRead %resultTypeId %imageId %coordinateId
    //
    // It is transformed into:
    //
    //     %sample = OpLoad %kIdTypeInt %SampleIdVar
    //     %resultId = OpImageRead %resultTypeId %imageId %coordinateId Sample %sample

    const SpvId sample = GetNewId(spirvOut);

    spirvOut->push_back(MakeInstructionOp(SpvOpLoad, 4));
    spirvOut->push_back(SkSL::spirv::kIdTypeInt);
    spirvOut->push_back(sample);
    spirvOut->push_back(fSampleIdVarId);

    spirvOut->push_back(MakeInstructionOp(SpvOpImageRead, 7));
    spirvOut->push_back(resultTypeId);
    spirvOut->push_back(resultId);
    spirvOut->push_back(imageId);
    spirvOut->push_back(coordinateId);
    spirvOut->push_back(SpvImageOperandsSampleMask);
    spirvOut->push_back(sample);

    return TransformationState::Transformed;
}

// A SPIR-V transformer.  It walks the instructions and modifies them as necessary, for example to
// modify input attachment load when multisampled.
class SpirvTransformer {
public:
    SpirvTransformer(const SpirvBlob& spirvIn,
                     const SPIRVTransformOptions& options,
                     SpirvBlob* spirvOut)
            : fSpirvIn(spirvIn)
            , fSpirvOut(spirvOut)
            , fOptions(options)
            , fMultisampleTransformer(spirvIn[kIDBoundIndex]) {
        // Preallocate memory for the result, with some room for additions by transformations.
        fSpirvOut->reserve(fSpirvIn.size() + 64);
    }

    void transform();

private:
    void onTransformBegin();

    const uint32_t* getCurrentInstruction(SpvOp* opCodeOut, uint32_t* wordCountOut) const;

    // Transform instructions:
    void transformInstruction();

    // SPIR-V to transform:
    const SpirvBlob& fSpirvIn;

    // Transformed SPIR-V:
    SpirvBlob* fSpirvOut;

    // Traversal state:
    size_t fCurrentWord = 0;
    bool fIsInFunctionSection = false;

    // Transformation state:
    SPIRVTransformOptions fOptions;
    SpirvMultisampleTransformer fMultisampleTransformer;

    // Instructions that potentially need transformation. They return Transformed if the instruction
    // is transformed. If Unchanged is returned, the instruction should be copied as-is.
    TransformationState transformOpCapability(const uint32_t* instruction);
    TransformationState transformOpEntryPoint(const uint32_t* instruction);
    TransformationState transformOpDecorate(const uint32_t* instruction);
    TransformationState transformOpTypePointer(const uint32_t* instruction);
    TransformationState transformOpTypeImage(const uint32_t* instruction);
    TransformationState transformOpLoad(const uint32_t* instruction);
    TransformationState transformOpImageRead(const uint32_t* instruction);
};

void SpirvTransformer::onTransformBegin() {
    // SPIR-V is expected to be valid.
    SkASSERT(fSpirvIn.size() >= kInstructionsStartIndex);
    // Make sure the transformer is not reused.
    SkASSERT(fCurrentWord == 0);
    SkASSERT(fIsInFunctionSection == false);
    // Make sure the output SPIR-V storage is not reused.
    SkASSERT(fSpirvOut->empty());

    // Copy the SPIR-V header to the output right away. This is needed for GetNewId() to work.
    fSpirvOut->assign(fSpirvIn.begin(), fSpirvIn.begin() + kInstructionsStartIndex);

    fCurrentWord = kInstructionsStartIndex;
}

const uint32_t* SpirvTransformer::getCurrentInstruction(SpvOp* opCodeOut,
                                                        uint32_t* wordCountOut) const {
    SkASSERT(fCurrentWord < fSpirvIn.size());
    const uint32_t* instruction = &fSpirvIn[fCurrentWord];

    *opCodeOut = GetInstructionOp(instruction);
    *wordCountOut = GetInstructionWordCount(instruction);

    // Basic validity check, instruction cound must not exceed SPIR-V size.
    SkASSERT(fCurrentWord + *wordCountOut <= fSpirvIn.size());

    return instruction;
}

void SpirvTransformer::transform() {
    this->onTransformBegin();

    while (fCurrentWord < fSpirvIn.size()) {
        this->transformInstruction();
    }
}

void SpirvTransformer::transformInstruction() {
    uint32_t wordCount;
    SpvOp opCode;
    const uint32_t* instruction = this->getCurrentInstruction(&opCode, &wordCount);

    if (opCode == SpvOpFunction) {
        // SPIR-V is structured in sections. Function declarations come last. Only a few
        // instructions inside functions need to be inspected.
        fIsInFunctionSection = true;
    }

    // Only look at interesting instructions.
    TransformationState transformationState = TransformationState::Unchanged;

    if (fIsInFunctionSection) {
        // Look at in-function opcodes.
        switch (opCode) {
            case SpvOpLoad:
                transformationState = this->transformOpLoad(instruction);
                break;
            case SpvOpImageRead:
                transformationState = this->transformOpImageRead(instruction);
                break;
            default:
                break;
        }
    } else {
        // Look at global declaration opcodes.
        switch (opCode) {
            case SpvOpCapability:
                transformationState = this->transformOpCapability(instruction);
                break;
            case SpvOpEntryPoint:
                transformationState = this->transformOpEntryPoint(instruction);
                break;
            case SpvOpDecorate:
                transformationState = this->transformOpDecorate(instruction);
                break;
            case SpvOpTypePointer:
                transformationState = this->transformOpTypePointer(instruction);
                break;
            case SpvOpTypeImage:
                transformationState = this->transformOpTypeImage(instruction);
                break;
            default:
                break;
        }
    }

    // If the instruction was not transformed, copy it to output as is.
    if (transformationState == TransformationState::Unchanged) {
        CopyInstruction(instruction, wordCount, fSpirvOut);
    }

    // Advance to next instruction.
    fCurrentWord += wordCount;
}

TransformationState SpirvTransformer::transformOpCapability(const uint32_t* instruction) {
    const SpvCapability capability = static_cast<SpvCapability>(instruction[1]);
    if (fOptions.fMultisampleInputLoad) {
        fMultisampleTransformer.transformOpCapability(capability, fSpirvOut);
    }
    return TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformOpEntryPoint(const uint32_t* instruction) {
    // Note: currently there is only one transformer, so control can be given to that. If more
    // transformers are added in the future, this function should instead take the list of interface
    // ids out, pass them to each transformer for a chance to modify it, then rewrite the entry
    // point once with the result.
    if (fOptions.fMultisampleInputLoad) {
        return fMultisampleTransformer.transformOpEntryPoint(instruction, fSpirvOut);
    }
    return TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformOpDecorate(const uint32_t* instruction) {
    if (fOptions.fMultisampleInputLoad) {
        fMultisampleTransformer.transformOpDecorate(fSpirvOut);
    }
    return TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformOpTypePointer(const uint32_t* instruction) {
    if (fOptions.fMultisampleInputLoad) {
        if (fMultisampleTransformer.transformOpTypePointer(instruction, fSpirvOut) ==
            TransformationState::Transformed) {
            return TransformationState::Transformed;
        }
    }
    return TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformOpTypeImage(const uint32_t* instruction) {
    const SpvId resultId = instruction[1];
    const SpvId sampledTypeId = instruction[2];
    if (fOptions.fMultisampleInputLoad) {
        if (fMultisampleTransformer.transformOpTypeImage(resultId, sampledTypeId, fSpirvOut) ==
            TransformationState::Transformed) {
            return TransformationState::Transformed;
        }
    }
    return TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformOpLoad(const uint32_t* instruction) {
    const SpvId resultTypeId = instruction[1];
    const SpvId resultId = instruction[2];
    const SpvId pointerId = instruction[3];
    if (fOptions.fMultisampleInputLoad) {
        fMultisampleTransformer.transformOpLoad(resultTypeId, resultId, pointerId, fSpirvOut);
    }
    return TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformOpImageRead(const uint32_t* instruction) {
    const SpvId resultTypeId = instruction[1];
    const SpvId resultId = instruction[2];
    const SpvId imageId = instruction[3];
    const SpvId coordinateId = instruction[4];
    if (fOptions.fMultisampleInputLoad) {
        if (fMultisampleTransformer.transformOpImageRead(
                    resultTypeId, resultId, imageId, coordinateId, fSpirvOut) ==
            TransformationState::Transformed) {
            return TransformationState::Transformed;
        }
    }
    return TransformationState::Unchanged;
}

}  // anonymous namespace

SkSL::NativeShader TransformSPIRV(const SkSL::NativeShader& spirv,
                                  const SPIRVTransformOptions& options) {
    SkSL::NativeShader result;

    SpirvTransformer transformer(spirv.fBinary, options, &result.fBinary);
    transformer.transform();

#ifdef SK_DEBUG
    // Validate the SPIR-V after performing any transformations. This is rather costly, so only
    // do this on debug builds.
    static SkSL::Compiler compiler;
    if (!SkSL::ValidateSPIRV(compiler.errorReporter(), {spirv.fBinary})) {
        SKGPU_LOG_E("SPIR-V transformations yielded invalid SPIR-V.");
    }
#endif

    return result;
}

}  // namespace skgpu::graphite
