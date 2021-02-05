/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrSkSLFP.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/GrContext_Base.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrTexture.h"
#include "src/sksl/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

#include <map>

class GrGLSLSkSLFP : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrSkSLFP& fp            = args.fFp.cast<GrSkSLFP>();
        const SkRuntimeEffect& effect = *fp.fEffect;
        const SkSL::Program& program  = *effect.fBaseProgram;

        // Declare all uniform variables, and remember their mangled names
        std::map<const SkSL::Variable*, SkSL::String> uniformNames;
        for (const SkSL::ProgramElement* elem : program.elements()) {
            if (!elem->is<SkSL::GlobalVarDeclaration>()) {
                continue;
            }
            const SkSL::GlobalVarDeclaration& global = elem->as<SkSL::GlobalVarDeclaration>();
            const SkSL::VarDeclaration& decl = global.declaration()->as<SkSL::VarDeclaration>();
            const SkSL::Variable& var = decl.var();
            if (var.type() == *program.fContext->fTypes.fFragmentProcessor) {
                continue;
            }
            if (var.modifiers().fFlags & SkSL::Modifiers::kUniform_Flag) {
                const SkSL::Type* type = &var.type();
                bool isArray = false;
                if (type->isArray()) {
                    type = &type->componentType();
                    isArray = true;
                }

                GrSLType gpuType;
                SkAssertResult(SkSL::type_to_grsltype(*program.fContext, *type, &gpuType));
                const char* uniformName = nullptr;
                auto handle =
                        args.fUniformHandler->addUniformArray(&fp,
                                                              kFragment_GrShaderFlag,
                                                              gpuType,
                                                              SkString(var.name()).c_str(),
                                                              isArray ? var.type().columns() : 0,
                                                              &uniformName);
                fUniformHandles.push_back(handle);
                uniformNames.insert({&var, uniformName});
            }
        }

        // Callback to get the name of a uniform variable
        auto uniformNameFn = [&](const SkSL::Variable* v) {
            auto it = uniformNames.find(v);
            SkASSERT(it != uniformNames.end());
            return it->second;
        };

        // We need to ensure that we emit each child's helper function at least once.
        // Any child FP that isn't sampled won't trigger a call otherwise, leading to asserts later.
        for (int i = 0; i < this->numChildProcessors(); ++i) {
            if (this->childProcessor(i)) {
                this->emitChildFunction(i, args);
            }
        }

        // Callbacks to sample a child
        auto sampleChildFn = [&](int childIndex, SkSL::String coords) {
            return SkSL::String(this->invokeChild(childIndex, args, coords).c_str());
        };
        auto sampleChildWithMatrixFn = [&](int childIndex, SkSL::String matrix) {
            // TODO: Do we still need the trick to intentionally pass "" if we previously decided
            // the sample usage is uniform matrix? Otherwise, description() *has* to match matrix.
            return SkSL::String(this->invokeChildWithMatrix(childIndex, args, matrix).c_str());
        };

        // Callback to get the (mangled) name of a function
        std::map<const SkSL::FunctionDeclaration*, SkSL::String> functionNames;
        auto functionNameFn = [&](const SkSL::FunctionDeclaration* fn) {
            auto it = functionNames.find(fn);
            SkASSERT(it != functionNames.end());
            return it->second;
        };

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString coordsVarName = fragBuilder->newTmpVarName("coords");
        const char* coords = nullptr;
        if (fp.referencesSampleCoords()) {
            coords = coordsVarName.c_str();
            fragBuilder->codeAppendf("float2 %s = %s;\n", coords, args.fSampleCoord);
        }

        for (const SkSL::ProgramElement* elem : program.elements()) {
            if (!elem->is<SkSL::FunctionDefinition>()) {
                continue;
            }

            const SkSL::FunctionDefinition& def = elem->as<SkSL::FunctionDefinition>();
            const SkSL::FunctionDeclaration& decl = def.declaration();

            SkSL::String body = SkSL::PipelineStage::ConvertFunction(
                    program, def, coords, uniformNameFn, functionNameFn, sampleChildFn,
                    sampleChildWithMatrixFn);

            if (decl.name() == "main") {
                fragBuilder->codeAppend(body.c_str());
            } else {
                SkString mangledName(
                        fragBuilder->getMangledFunctionName(SkString(decl.name()).c_str()).c_str());
                fragBuilder->emitFunction(&decl, mangledName.c_str(), body.c_str());
                functionNames.insert({&decl, SkSL::String(mangledName.c_str())});
            }
        }
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        size_t uniIndex = 0;
        const GrSkSLFP& outer = _proc.cast<GrSkSLFP>();
        const uint8_t* uniformData = outer.fUniforms->bytes();
        for (const auto& v : outer.fEffect->uniforms()) {
            const float* data = reinterpret_cast<const float*>(uniformData + v.offset);
            switch (v.type) {
                case SkRuntimeEffect::Uniform::Type::kFloat:
                    pdman.set1fv(fUniformHandles[uniIndex++], v.count, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat2:
                    pdman.set2fv(fUniformHandles[uniIndex++], v.count, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat3:
                    pdman.set3fv(fUniformHandles[uniIndex++], v.count, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat4:
                    pdman.set4fv(fUniformHandles[uniIndex++], v.count, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat2x2:
                    pdman.setMatrix2fv(fUniformHandles[uniIndex++], v.count, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat3x3:
                    pdman.setMatrix3fv(fUniformHandles[uniIndex++], v.count, data);
                    break;
                case SkRuntimeEffect::Uniform::Type::kFloat4x4:
                    pdman.setMatrix4fv(fUniformHandles[uniIndex++], v.count, data);
                    break;
                default:
                    SkDEBUGFAIL("Unsupported uniform type");
                    break;
            }
        }
    }

    std::vector<UniformHandle> fUniformHandles;
};

std::unique_ptr<GrSkSLFP> GrSkSLFP::Make(GrContext_Base* context, sk_sp<SkRuntimeEffect> effect,
                                         const char* name, sk_sp<SkData> uniforms) {
    if (uniforms->size() != effect->uniformSize()) {
        return nullptr;
    }
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(context->priv().getShaderErrorHandler(),
                                                  std::move(effect), name, std::move(uniforms)));
}

GrSkSLFP::GrSkSLFP(ShaderErrorHandler* shaderErrorHandler, sk_sp<SkRuntimeEffect> effect,
                   const char* name, sk_sp<SkData> uniforms)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fShaderErrorHandler(shaderErrorHandler)
        , fEffect(std::move(effect))
        , fName(name)
        , fUniforms(std::move(uniforms)) {
    if (fEffect->usesSampleCoords()) {
        this->setUsesSampleCoordsDirectly();
    }
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fShaderErrorHandler(other.fShaderErrorHandler)
        , fEffect(other.fEffect)
        , fName(other.fName)
        , fUniforms(other.fUniforms) {
    if (fEffect->usesSampleCoords()) {
        this->setUsesSampleCoordsDirectly();
    }

    this->cloneAndRegisterAllChildProcessors(other);
}

const char* GrSkSLFP::name() const {
    return fName;
}

void GrSkSLFP::addChild(std::unique_ptr<GrFragmentProcessor> child) {
    int childIndex = this->numChildProcessors();
    SkASSERT((size_t)childIndex < fEffect->fSampleUsages.size());
    this->registerChild(std::move(child), fEffect->fSampleUsages[childIndex]);
}

GrGLSLFragmentProcessor* GrSkSLFP::onCreateGLSLInstance() const {
    return new GrGLSLSkSLFP();
}

void GrSkSLFP::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    // In the unlikely event of a hash collision, we also include the uniform size in the key.
    // That ensures that we will (at worst) use the wrong program, but one that expects the same
    // amount of uniform data.
    b->add32(fEffect->hash());
    b->add32(SkToU32(fUniforms->size()));
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSkSLFP& sk = other.cast<GrSkSLFP>();
    return fEffect->hash() == sk.fEffect->hash() && fUniforms->equals(sk.fUniforms.get());
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrSkSLFP(*this));
}

/**************************************************************************************************/

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSkSLFP);

#if GR_TEST_UTILS

#include "include/effects/SkOverdrawColorFilter.h"
#include "src/core/SkColorFilterBase.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"

extern const char* SKSL_OVERDRAW_SRC;

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::TestCreate(GrProcessorTestData* d) {
    SkColor colors[SkOverdrawColorFilter::kNumColors];
    for (SkColor& c : colors) {
        c = d->fRandom->nextU();
    }
    auto filter = SkOverdrawColorFilter::MakeWithSkColors(colors);
    auto [success, fp] = as_CFB(filter)->asFragmentProcessor(/*inputFP=*/nullptr, d->context(),
                                                             GrColorInfo{});
    SkASSERT(success);
    return std::move(fp);
}

#endif
