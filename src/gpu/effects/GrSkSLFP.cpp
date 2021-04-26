/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrSkSLFP.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/GrContext_Base.h"
#include "src/core/SkVM.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrTexture.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
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
            FPCallbacks(GrGLSLSkSLFP* self,
                        EmitArgs& args,
                        const char* inputColor,
                        const SkSL::Context& context)
                    : fSelf(self), fArgs(args), fInputColor(inputColor), fContext(context) {}

            using String = SkSL::String;

            String declareUniform(const SkSL::VarDeclaration* decl) override {
                const SkSL::Variable& var = decl->var();
                if (var.type().isOpaque()) {
                    // Nothing to do. The only opaque types we should see are children, and those
                    // are handled specially, above.
                    SkASSERT(var.type().isEffectChild());
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
                return String(fSelf->invokeChild(index, fInputColor, fArgs, coords).c_str());
            }

            String sampleChildWithMatrix(int index, String matrix) override {
                // If the child is sampled with a uniform matrix, we need to pass the empty string.
                // 'invokeChildWithMatrix' will assert that the passed-in matrix matches the one
                // extracted from the SkSL when the sample usages were determined. We've mangled
                // the uniform names, though, so it won't match.
                const GrFragmentProcessor* child = fArgs.fFp.childProcessor(index);
                const bool hasUniformMatrix = child && child->sampleUsage().hasUniformMatrix();
                return String(
                        fSelf->invokeChildWithMatrix(
                                     index, fInputColor, fArgs, hasUniformMatrix ? "" : matrix)
                                .c_str());
            }

            GrGLSLSkSLFP*        fSelf;
            EmitArgs&            fArgs;
            const char*          fInputColor;
            const SkSL::Context& fContext;
        };

        // Snap off a global copy of the input color at the start of main. We need this when
        // we call child processors (particularly from helper functions, which can't "see" the
        // parameter to main). Even from within main, if the code mutates the parameter, calls to
        // sample should still be passing the original color (by default).
        GrShaderVar inputColorCopy(args.fFragBuilder->getMangledFunctionName("inColor"),
                                   kHalf4_GrSLType);
        args.fFragBuilder->declareGlobal(inputColorCopy);
        args.fFragBuilder->codeAppendf("%s = %s;\n", inputColorCopy.c_str(), args.fInputColor);

        // Callback to define a function (and return its mangled name)
        SkString coordsVarName = args.fFragBuilder->newTmpVarName("coords");
        const char* coords = nullptr;
        if (fp.referencesSampleCoords()) {
            coords = coordsVarName.c_str();
            args.fFragBuilder->codeAppendf("float2 %s = %s;\n", coords, args.fSampleCoord);
        }

        FPCallbacks callbacks(this, args, inputColorCopy.c_str(), *program.fContext);
        SkSL::PipelineStage::ConvertProgram(program, coords, args.fInputColor, &callbacks);
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        using Type = SkRuntimeEffect::Uniform::Type;
        size_t uniIndex = 0;
        const GrSkSLFP& outer = _proc.cast<GrSkSLFP>();
        const uint8_t* uniformData = outer.fUniforms->bytes();
        for (const auto& v : outer.fEffect->uniforms()) {
            const UniformHandle handle = fUniformHandles[uniIndex++];
            auto floatData = [=] { return SkTAddOffset<const float>(uniformData, v.offset); };
            auto intData = [=] { return SkTAddOffset<const int>(uniformData, v.offset); };
            switch (v.type) {
                case Type::kFloat:  pdman.set1fv(handle, v.count, floatData()); break;
                case Type::kFloat2: pdman.set2fv(handle, v.count, floatData()); break;
                case Type::kFloat3: pdman.set3fv(handle, v.count, floatData()); break;
                case Type::kFloat4: pdman.set4fv(handle, v.count, floatData()); break;

                case Type::kFloat2x2: pdman.setMatrix2fv(handle, v.count, floatData()); break;
                case Type::kFloat3x3: pdman.setMatrix3fv(handle, v.count, floatData()); break;
                case Type::kFloat4x4: pdman.setMatrix4fv(handle, v.count, floatData()); break;

                case Type::kInt:  pdman.set1iv(handle, v.count, intData()); break;
                case Type::kInt2: pdman.set2iv(handle, v.count, intData()); break;
                case Type::kInt3: pdman.set3iv(handle, v.count, intData()); break;
                case Type::kInt4: pdman.set4iv(handle, v.count, intData()); break;

                default:
                    SkDEBUGFAIL("Unsupported uniform type");
                    break;
            }
        }
    }

    std::vector<UniformHandle> fUniformHandles;
};

std::unique_ptr<GrSkSLFP> GrSkSLFP::Make(sk_sp<SkRuntimeEffect> effect,
                                         const char* name,
                                         sk_sp<SkData> uniforms) {
    if (uniforms->size() != effect->uniformSize()) {
        return nullptr;
    }
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(std::move(effect), name, std::move(uniforms)));
}

GrSkSLFP::GrSkSLFP(sk_sp<SkRuntimeEffect> effect,
                   const char* name,
                   sk_sp<SkData> uniforms)
        : INHERITED(kGrSkSLFP_ClassID,
                    effect->allowColorFilter() ? kConstantOutputForConstantInput_OptimizationFlag
                                               : kNone_OptimizationFlags)
        , fEffect(std::move(effect))
        , fName(name)
        , fUniforms(std::move(uniforms)) {
    if (fEffect->usesSampleCoords()) {
        this->setUsesSampleCoordsDirectly();
    }
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(kGrSkSLFP_ClassID, other.optimizationFlags())
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
    this->mergeOptimizationFlags(ProcessorOptimizationFlags(child.get()));
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

SkPMColor4f GrSkSLFP::constantOutputForConstantInput(const SkPMColor4f& inputColor) const {
    const skvm::Program& program = fEffect->getFilterColorInfo().program;

    SkSTArray<3, SkPMColor4f, true> childColors;
    childColors.push_back(inputColor);
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        childColors.push_back(ConstantOutputForConstantInput(this->childProcessor(i), inputColor));
    }

    SkPMColor4f result;
    program.eval(1, childColors.begin(), fUniforms->data(), result.vec());
    return result;
}

/**************************************************************************************************/

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSkSLFP);

#if GR_TEST_UTILS

#include "include/effects/SkOverdrawColorFilter.h"
#include "src/core/SkColorFilterBase.h"

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

/**************************************************************************************************/

GrRuntimeFPBuilder::GrRuntimeFPBuilder(sk_sp<SkRuntimeEffect> effect)
        : INHERITED(std::move(effect)) {}

GrRuntimeFPBuilder::~GrRuntimeFPBuilder() = default;

std::unique_ptr<GrFragmentProcessor> GrRuntimeFPBuilder::makeFP() {
    return this->effect()->makeFP(this->uniforms(),
                                  this->children(),
                                  this->numChildren());
}
