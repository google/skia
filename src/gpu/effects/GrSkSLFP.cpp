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

class GrGLSLSkSLFP : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrSkSLFP& fp            = args.fFp.cast<GrSkSLFP>();
        const SkSL::Program& program  = *fp.fEffect->fBaseProgram;

        // We need to ensure that we emit each child's helper function at least once.
        // Any child FP that isn't sampled won't trigger a call otherwise, leading to asserts later.
        for (int i = 0; i < this->numChildProcessors(); ++i) {
            if (this->childProcessor(i)) {
                this->emitChildFunction(i, args);
            }
        }

        class FPCallbacks : public SkSL::PipelineStage::Callbacks {
        public:
            FPCallbacks(GrGLSLSkSLFP* self, EmitArgs& args, const SkSL::Context& context)
                    : fSelf(self), fArgs(args), fContext(context) {}

            using String = SkSL::String;

            String declareUniform(const SkSL::VarDeclaration* decl) override {
                const SkSL::Variable& var = decl->var();
                if (var.type().isOpaque()) {
                    // Nothing to do. The only opaque type we should see is fragmentProcessor, and
                    // those (children) are handled specially, above.
                    SkASSERT(var.type() == *fContext.fTypes.fFragmentProcessor);
                    return String(var.name());
                }

                const SkSL::Type* type = &var.type();
                bool isArray = false;
                if (type->isArray()) {
                    type = &type->componentType();
                    isArray = true;
                }

                GrSLType gpuType;
                SkAssertResult(SkSL::type_to_grsltype(fContext, *type, &gpuType));
                const char* uniformName = nullptr;
                auto handle =
                        fArgs.fUniformHandler->addUniformArray(&fArgs.fFp.cast<GrSkSLFP>(),
                                                               kFragment_GrShaderFlag,
                                                               gpuType,
                                                               SkString(var.name()).c_str(),
                                                               isArray ? var.type().columns() : 0,
                                                               &uniformName);
                fSelf->fUniformHandles.push_back(handle);
                return String(uniformName);
            }

            String getMangledName(const char* name) override {
                return String(fArgs.fFragBuilder->getMangledFunctionName(name).c_str());
            }

            void defineFunction(const char* decl, const char* body, bool isMain) override {
                if (isMain) {
                    fArgs.fFragBuilder->codeAppend(body);
                } else {
                    fArgs.fFragBuilder->emitFunction(decl, body);
                }
            }

            void defineStruct(const char* definition) override {
                fArgs.fFragBuilder->definitionAppend(definition);
            }

            void declareGlobal(const char* declaration) override {
                fArgs.fFragBuilder->definitionAppend(declaration);
            }

            String sampleChild(int index, String coords) override {
                return String(fSelf->invokeChild(index, fArgs, coords).c_str());
            }

            String sampleChildWithMatrix(int index, String matrix) override {
                // If the child is sampled with a uniform matrix, we need to pass the empty string.
                // 'invokeChildWithMatrix' will assert that the passed-in matrix matches the one
                // extracted from the SkSL when the sample usages were determined. We've mangled
                // the uniform names, though, so it won't match.
                const GrFragmentProcessor* child = fArgs.fFp.childProcessor(index);
                const bool hasUniformMatrix = child && child->sampleUsage().hasUniformMatrix();
                return String(
                        fSelf->invokeChildWithMatrix(index, fArgs, hasUniformMatrix ? "" : matrix)
                                .c_str());
            }

            GrGLSLSkSLFP*        fSelf;
            EmitArgs&            fArgs;
            const SkSL::Context& fContext;
        };

        FPCallbacks callbacks(this, args, *program.fContext);

        // Callback to define a function (and return its mangled name)
        SkString coordsVarName = args.fFragBuilder->newTmpVarName("coords");
        const char* coords = nullptr;
        if (fp.referencesSampleCoords()) {
            coords = coordsVarName.c_str();
            args.fFragBuilder->codeAppendf("float2 %s = %s;\n", coords, args.fSampleCoord);
        }

        SkSL::PipelineStage::ConvertProgram(program, coords, &callbacks);
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

std::unique_ptr<GrGLSLFragmentProcessor> GrSkSLFP::onMakeProgramImpl() const {
    return std::make_unique<GrGLSLSkSLFP>();
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
