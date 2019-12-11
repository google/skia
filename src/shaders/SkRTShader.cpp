/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkRTShader.h"

#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"

#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/generated/GrMixerEffect.h"

static inline int new_sksl_index() {
    return GrSkSLFP::NewIndex();
}
#else
static inline int new_sksl_index() {
    return 0;   // not used w/o GPU
}
#endif

SkRTShader::SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> inputs,
                       const SkMatrix* localMatrix, bool isOpaque)
        : SkShaderBase(localMatrix)
        , fEffect(std::move(effect))
        , fIsOpaque(isOpaque)
        , fInputs(std::move(inputs)) {
}

bool SkRTShader::onAppendStages(const SkStageRec& rec) const {
    SkMatrix inverse;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &inverse)) {
        return false;
    }

    auto ctx = rec.fAlloc->make<SkRasterPipeline_InterpreterCtx>();
    ctx->paintColor = rec.fPaint.getColor4f();
    ctx->inputs = fInputs->data();
    ctx->ninputs = fInputs->size() / 4;
    ctx->shaderConvention = true;

    SkAutoMutexExclusive ama(fByteCodeMutex);
    if (!fByteCode) {
        // TODO: Need to make this safe and fast. I think we (currently) need to reuse the original
        // Compiler object, to ensure that type checks are against the same Context. A Context
        // singleton could get around that. Otherwise, we need to guard usage of the effect's
        // compiler, in case multiple threads are doing second-stage compiles.
        fByteCode = fEffect->fCompiler.toByteCode(*fEffect->fBaseProgram);
        if (fEffect->fCompiler.errorCount()) {
            SkDebugf("%s\n", fEffect->fCompiler.errorText().c_str());
            return false;
        }
        SkASSERT(fByteCode);
        if (!fByteCode->getFunction("main")) {
            return false;
        }
    }
    ctx->byteCode = fByteCode.get();
    ctx->fn = ctx->byteCode->getFunction("main");

    rec.fPipeline->append(SkRasterPipeline::seed_shader);
    rec.fPipeline->append_matrix(rec.fAlloc, inverse);
    rec.fPipeline->append(SkRasterPipeline::interpreter, ctx);
    return true;
}

enum Flags {
    kIsOpaque_Flag          = 1 << 0,
    kHasLocalMatrix_Flag    = 1 << 1,
};

void SkRTShader::flatten(SkWriteBuffer& buffer) const {
    uint32_t flags = 0;
    if (fIsOpaque) {
        flags |= kIsOpaque_Flag;
    }
    if (!this->getLocalMatrix().isIdentity()) {
        flags |= kHasLocalMatrix_Flag;
    }

    buffer.writeString(fEffect->fSkSL.c_str());
    if (fInputs) {
        buffer.writeDataAsByteArray(fInputs.get());
    } else {
        buffer.writeByteArray(nullptr, 0);
    }
    buffer.write32(flags);
    if (flags & kHasLocalMatrix_Flag) {
        buffer.writeMatrix(this->getLocalMatrix());
    }
}

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> inputs = buffer.readByteArrayAsData();
    uint32_t flags = buffer.read32();

    bool isOpaque = SkToBool(flags & kIsOpaque_Flag);
    SkMatrix localM, *localMPtr = nullptr;
    if (flags & kHasLocalMatrix_Flag) {
        buffer.readMatrix(&localM);
        localMPtr = &localM;
    }

    // We don't have a way to ensure that indices are consistent and correct when deserializing.
    // Perhaps we should have a hash table to map strings to indices? For now, all shaders get a
    // new unique ID after serialization.
    return sk_sp<SkFlattenable>(new SkRTShader(SkRuntimeEffect::Make(std::move(sksl)),
                                               std::move(inputs), localMPtr, isOpaque));
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkRTShader::asFragmentProcessor(const GrFPArgs& args) const {
    SkMatrix matrix;
    if (!this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    return GrSkSLFP::Make(args.fContext, fEffect, "runtime-shader",
                          fInputs->data(), fInputs->size(), &matrix);
}
#endif

sk_sp<SkRuntimeEffect> SkRuntimeEffect::Make(SkString sksl) {
    return sk_sp<SkRuntimeEffect>(new SkRuntimeEffect(std::move(sksl)));
}

SkRuntimeEffect::SkRuntimeEffect(SkString sksl)
        : fIndex(new_sksl_index())
        , fSkSL(std::move(sksl)) {
    fBaseProgram = fCompiler.convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(fSkSL.c_str(), fSkSL.size()),
                                            SkSL::Program::Settings());
    if (!fBaseProgram) {
        return;
    }
    SkASSERT(!fCompiler.errorCount());

    for (const auto& e : *fBaseProgram) {
        if (e.fKind == SkSL::ProgramElement::kVar_Kind) {
            SkSL::VarDeclarations& v = (SkSL::VarDeclarations&) e;
            for (const auto& varStatement : v.fVars) {
                const SkSL::Variable& var = *((SkSL::VarDeclaration&) * varStatement).fVar;
                if ((var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) ||
                    (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag)) {
                    fInAndUniformVars.push_back(&var);
                }
                // "in uniform" doesn't make sense outside of .fp files
                SkASSERT((var.fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) == 0 ||
                    (var.fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) == 0);
                // "layout(key)" doesn't make sense outside of .fp files; all 'in' variables are
                // part of the key
                SkASSERT(!var.fModifiers.fLayout.fKey);
            }
        }
    }
}

static std::tuple<const SkSL::Type*, int> strip_array(const SkSL::Type* type) {
    int arrayCount = 0;
    if (type->kind() == SkSL::Type::kArray_Kind) {
        arrayCount = type->columns();
        type = &type->componentType();
    }
    return std::make_tuple(type, arrayCount);
}

std::unique_ptr<SkSL::Program> SkRuntimeEffect::getSpecialization(const void* inputs,
                                                                  size_t inputSize) {
    std::unordered_map<SkSL::String, SkSL::Program::Settings::Value> inputMap;
    size_t offset = 0;
    for (const auto& v : fInAndUniformVars) {
        auto [type, arrayCount] = strip_array(&v->fType);
        arrayCount = SkTMax(1, arrayCount);
        SkSL::String name(v->fName);
        if (type == fCompiler.context().fInt_Type.get() ||
            type == fCompiler.context().fShort_Type.get()) {
            offset = SkAlign4(offset);
            if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                int32_t v = *(int32_t*)(((uint8_t*)inputs) + offset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            }
            offset += sizeof(int32_t) * arrayCount;
        } else if (type == fCompiler.context().fFloat_Type.get() ||
                   type == fCompiler.context().fHalf_Type.get()) {
            offset = SkAlign4(offset);
            if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                float v = *(float*)(((uint8_t*)inputs) + offset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            }
            offset += sizeof(float) * arrayCount;
        } else if (type == fCompiler.context().fBool_Type.get()) {
            if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                bool v = *(((bool*)inputs) + offset);
                inputMap.insert(std::make_pair(name, SkSL::Program::Settings::Value(v)));
            }
            offset += sizeof(bool) * arrayCount;
        } else if (type == fCompiler.context().fFloat2_Type.get() ||
                   type == fCompiler.context().fHalf2_Type.get()) {
            offset = SkAlign4(offset) + sizeof(float) * 2 * arrayCount;
        } else if (type == fCompiler.context().fFloat3_Type.get() ||
                   type == fCompiler.context().fHalf3_Type.get()) {
            offset = SkAlign4(offset) + sizeof(float) * 3 * arrayCount;
        } else if (type == fCompiler.context().fFloat4_Type.get() ||
                   type == fCompiler.context().fHalf4_Type.get()) {
            offset = SkAlign4(offset) + sizeof(float) * 4 * arrayCount;
        } else if (type == fCompiler.context().fFragmentProcessor_Type.get()) {
            // do nothing
        } else {
            printf("can't handle input var: %s\n", SkSL::String(v->fType.fName).c_str());
            SkASSERT(false);
        }
    }

    std::unique_ptr<SkSL::Program> specialized = fCompiler.specialize(*fBaseProgram, inputMap);
    bool optimized = fCompiler.optimize(*specialized);
    if (!optimized) {
        SkDebugf("%s\n", fCompiler.errorText().c_str());
        SkASSERT(false);
    }
    return specialized;
}

SkRuntimeShaderFactory::SkRuntimeShaderFactory(SkString sksl, bool isOpaque)
    : fEffect(SkRuntimeEffect::Make(std::move(sksl)))
    , fIsOpaque(isOpaque) {}

sk_sp<SkShader> SkRuntimeShaderFactory::make(sk_sp<SkData> inputs, const SkMatrix* localMatrix) {
    return fEffect && fEffect->isValid()
        ? sk_sp<SkShader>(new SkRTShader(fEffect, std::move(inputs), localMatrix, fIsOpaque))
        : nullptr;
}
