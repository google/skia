/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrSkSLFP.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/GrContext_Base.h"
#include "include/private/SkSLString.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLPipelineStageCodeGenerator.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

class GrSkSLFP::Impl : public ProgramImpl {
public:
    void emitCode(EmitArgs& args) override {
        const GrSkSLFP& fp            = args.fFp.cast<GrSkSLFP>();
        const SkSL::Program& program  = *fp.fEffect->fBaseProgram;

        class FPCallbacks : public SkSL::PipelineStage::Callbacks {
        public:
            FPCallbacks(Impl* self,
                        EmitArgs& args,
                        const char* inputColor,
                        const SkSL::Context& context,
                        const uint8_t* uniformData,
                        const GrSkSLFP::UniformFlags* uniformFlags)
                    : fSelf(self)
                    , fArgs(args)
                    , fInputColor(inputColor)
                    , fContext(context)
                    , fUniformData(uniformData)
                    , fUniformFlags(uniformFlags) {}

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
                size_t sizeInBytes = type->slotCount() * sizeof(float);
                const float* floatData = reinterpret_cast<const float*>(fUniformData);
                const int* intData = reinterpret_cast<const int*>(fUniformData);
                fUniformData += sizeInBytes;

                bool isArray = false;
                if (type->isArray()) {
                    type = &type->componentType();
                    isArray = true;
                }

                GrSLType gpuType;
                SkAssertResult(SkSL::type_to_grsltype(fContext, *type, &gpuType));

                if (*fUniformFlags++ & GrSkSLFP::kSpecialize_Flag) {
                    SkASSERTF(!isArray, "specializing array uniforms is not allowed");
                    String value = GrGLSLTypeString(gpuType);
                    value.append("(");

                    bool isFloat = GrSLTypeIsFloatType(gpuType);
                    size_t slots = type->slotCount();
                    for (size_t i = 0; i < slots; ++i) {
                        value.append(isFloat ? SkSL::to_string(floatData[i])
                                             : SkSL::to_string(intData[i]));
                        value.append(",");
                    }
                    value.back() = ')';
                    return value;
                }

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

            String sampleShader(int index, String coords) override {
                // If the child was sampled using the coords passed to main (and they are never
                // modified), then we will have marked the child as PassThrough. The code generator
                // doesn't know that, and still supplies coords. Inside invokeChild, we assert that
                // any coords passed for a PassThrough child match args.fSampleCoords exactly.
                //
                // Normally, this is valid. Here, we *copied* the sample coords to a local variable
                // (so that they're mutable in the runtime effect SkSL). Thus, the coords string we
                // get here is the name of the local copy, and fSampleCoords still points to the
                // unmodified original (which might be a varying, for example).
                // To prevent the assert, we pass the empty string in this case. Note that for
                // children sampled like this, invokeChild doesn't even use the coords parameter,
                // except for that assert.
                const GrFragmentProcessor* child = fArgs.fFp.childProcessor(index);
                if (child && child->sampleUsage().isPassThrough()) {
                    coords.clear();
                }
                return String(fSelf->invokeChild(index, fInputColor, fArgs, coords).c_str());
            }

            String sampleColorFilter(int index, String color) override {
                return String(fSelf->invokeChild(index,
                                                 color.empty() ? fInputColor : color.c_str(),
                                                 fArgs)
                                      .c_str());
            }

            String sampleBlender(int index, String src, String dst) override {
                if (!fSelf->childProcessor(index)) {
                    return String::printf("blend_src_over(%s, %s)", src.c_str(), dst.c_str());
                }
                return String(fSelf->invokeChild(index, src.c_str(), dst.c_str(), fArgs).c_str());
            }

            Impl*                         fSelf;
            EmitArgs&                     fArgs;
            const char*                   fInputColor;
            const SkSL::Context&          fContext;
            const uint8_t*                fUniformData;
            const GrSkSLFP::UniformFlags* fUniformFlags;
            int                           fUniformIndex = 0;
        };

        // If we have an input child, we invoke it now, and make the result of that be the "input
        // color" for all other purposes later (eg, the default passed via sample calls, etc.)
        if (fp.fInputChildIndex >= 0) {
            args.fFragBuilder->codeAppendf("%s = %s;\n",
                                           args.fInputColor,
                                           this->invokeChild(fp.fInputChildIndex, args).c_str());
        }

        if (fp.fEffect->allowBlender()) {
            // If we have an dest-color child, we invoke it now, and make the result of that be the
            // "dest color" for all other purposes later.
            if (fp.fDestColorChildIndex >= 0) {
                args.fFragBuilder->codeAppendf(
                        "%s = %s;\n",
                        args.fDestColor,
                        this->invokeChild(fp.fDestColorChildIndex, args.fDestColor, args).c_str());
            }
        } else {
            // We're not making a blender, so we don't expect a dest-color child FP to exist.
            SkASSERT(fp.fDestColorChildIndex < 0);
        }

        // Snap off a global copy of the input color at the start of main. We need this when
        // we call child processors (particularly from helper functions, which can't "see" the
        // parameter to main). Even from within main, if the code mutates the parameter, calls to
        // sample should still be passing the original color (by default).
        SkString inputColorName;
        if (fp.fEffect->samplesOutsideMain()) {
            GrShaderVar inputColorCopy(args.fFragBuilder->getMangledFunctionName("inColor"),
                                       kHalf4_GrSLType);
            args.fFragBuilder->declareGlobal(inputColorCopy);
            inputColorName = inputColorCopy.getName();
            args.fFragBuilder->codeAppendf("%s = %s;\n", inputColorName.c_str(), args.fInputColor);
        } else {
            inputColorName = args.fFragBuilder->newTmpVarName("inColor");
            args.fFragBuilder->codeAppendf(
                    "half4 %s = %s;\n", inputColorName.c_str(), args.fInputColor);
        }

        // Copy the incoming coords to a local variable. Code in main might modify the coords
        // parameter. fSampleCoord could be a varying, so writes to it would be illegal.
        const char* coords = "float2(0)";
        SkString coordsVarName;
        if (fp.usesSampleCoordsDirectly()) {
            coordsVarName = args.fFragBuilder->newTmpVarName("coords");
            coords = coordsVarName.c_str();
            args.fFragBuilder->codeAppendf("float2 %s = %s;\n", coords, args.fSampleCoord);
        }

        FPCallbacks callbacks(this,
                              args,
                              inputColorName.c_str(),
                              *program.fContext,
                              fp.uniformData(),
                              fp.uniformFlags());
        SkSL::PipelineStage::ConvertProgram(
                program, coords, args.fInputColor, args.fDestColor, &callbacks);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        using Type = SkRuntimeEffect::Uniform::Type;
        size_t uniIndex = 0;
        const GrSkSLFP& outer = _proc.cast<GrSkSLFP>();
        const uint8_t* uniformData = outer.uniformData();
        const GrSkSLFP::UniformFlags* uniformFlags = outer.uniformFlags();
        for (const auto& v : outer.fEffect->uniforms()) {
            if (*uniformFlags++ & GrSkSLFP::kSpecialize_Flag) {
                continue;
            }
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

std::unique_ptr<GrSkSLFP> GrSkSLFP::MakeWithData(
        sk_sp<SkRuntimeEffect> effect,
        const char* name,
        std::unique_ptr<GrFragmentProcessor> inputFP,
        std::unique_ptr<GrFragmentProcessor> destColorFP,
        sk_sp<SkData> uniforms,
        SkSpan<std::unique_ptr<GrFragmentProcessor>> childFPs) {
    if (uniforms->size() != effect->uniformSize()) {
        return nullptr;
    }
    size_t uniformSize = uniforms->size();
    size_t uniformFlagSize = effect->uniforms().count() * sizeof(UniformFlags);
    std::unique_ptr<GrSkSLFP> fp(new (uniformSize + uniformFlagSize)
                                         GrSkSLFP(std::move(effect), name, OptFlags::kNone));
    sk_careful_memcpy(fp->uniformData(), uniforms->data(), uniformSize);
    for (auto& childFP : childFPs) {
        fp->addChild(std::move(childFP), /*mergeOptFlags=*/true);
    }
    if (inputFP) {
        fp->setInput(std::move(inputFP));
    }
    if (destColorFP) {
        fp->setDestColorFP(std::move(destColorFP));
    }
    return fp;
}

GrSkSLFP::GrSkSLFP(sk_sp<SkRuntimeEffect> effect, const char* name, OptFlags optFlags)
        : INHERITED(kGrSkSLFP_ClassID,
                    static_cast<OptimizationFlags>(optFlags) |
                            (effect->getFilterColorProgram()
                                     ? kConstantOutputForConstantInput_OptimizationFlag
                                     : kNone_OptimizationFlags))
        , fEffect(std::move(effect))
        , fName(name)
        , fUniformSize(SkToU32(fEffect->uniformSize())) {
    memset(this->uniformFlags(), 0, fEffect->uniforms().count() * sizeof(UniformFlags));
    if (fEffect->usesSampleCoords()) {
        this->setUsesSampleCoordsDirectly();
    }
    if (fEffect->allowBlender()) {
        this->setIsBlendFunction();
    }
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(other)
        , fEffect(other.fEffect)
        , fName(other.fName)
        , fUniformSize(other.fUniformSize)
        , fInputChildIndex(other.fInputChildIndex) {
    sk_careful_memcpy(this->uniformFlags(),
                      other.uniformFlags(),
                      fEffect->uniforms().count() * sizeof(UniformFlags));
    sk_careful_memcpy(this->uniformData(), other.uniformData(), fUniformSize);
}

void GrSkSLFP::addChild(std::unique_ptr<GrFragmentProcessor> child, bool mergeOptFlags) {
    SkASSERTF(fInputChildIndex == -1, "all addChild calls must happen before setInput");
    SkASSERTF(fDestColorChildIndex == -1, "all addChild calls must happen before setDestColorFP");
    int childIndex = this->numChildProcessors();
    SkASSERT((size_t)childIndex < fEffect->fSampleUsages.size());
    if (mergeOptFlags) {
        this->mergeOptimizationFlags(ProcessorOptimizationFlags(child.get()));
    }
    this->registerChild(std::move(child), fEffect->fSampleUsages[childIndex]);
}

void GrSkSLFP::setInput(std::unique_ptr<GrFragmentProcessor> input) {
    SkASSERTF(fInputChildIndex == -1, "setInput should not be called more than once");
    fInputChildIndex = this->numChildProcessors();
    SkASSERT((size_t)fInputChildIndex >= fEffect->fSampleUsages.size());
    this->mergeOptimizationFlags(ProcessorOptimizationFlags(input.get()));
    this->registerChild(std::move(input), SkSL::SampleUsage::PassThrough());
}

void GrSkSLFP::setDestColorFP(std::unique_ptr<GrFragmentProcessor> destColorFP) {
    SkASSERTF(fEffect->allowBlender(), "dest colors are only used by blend effects");
    SkASSERTF(fDestColorChildIndex == -1, "setDestColorFP should not be called more than once");
    fDestColorChildIndex = this->numChildProcessors();
    SkASSERT((size_t)fDestColorChildIndex >= fEffect->fSampleUsages.size());
    this->mergeOptimizationFlags(ProcessorOptimizationFlags(destColorFP.get()));
    this->registerChild(std::move(destColorFP), SkSL::SampleUsage::PassThrough());
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> GrSkSLFP::onMakeProgramImpl() const {
    return std::make_unique<Impl>();
}

void GrSkSLFP::onAddToKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
    // In the unlikely event of a hash collision, we also include the uniform size in the key.
    // That ensures that we will (at worst) use the wrong program, but one that expects the same
    // amount of uniform data.
    b->add32(fEffect->hash());
    b->add32(fUniformSize);

    const UniformFlags* flags = this->uniformFlags();
    const uint8_t* uniformData = this->uniformData();
    size_t uniformCount = fEffect->uniforms().count();
    auto iter = fEffect->uniforms().begin();

    for (size_t i = 0; i < uniformCount; ++i, ++iter) {
        bool specialize = flags[i] & kSpecialize_Flag;
        b->addBool(specialize, "specialize");
        if (specialize) {
            b->addBytes(iter->sizeInBytes(), uniformData + iter->offset, iter->name.c_str());
        }
    }
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSkSLFP& sk = other.cast<GrSkSLFP>();
    const size_t uniformFlagSize = fEffect->uniforms().count() * sizeof(UniformFlags);
    return fEffect->hash() == sk.fEffect->hash() &&
           fEffect->uniforms().count() == sk.fEffect->uniforms().count() &&
           fUniformSize == sk.fUniformSize &&
           !sk_careful_memcmp(
                   this->uniformData(), sk.uniformData(), fUniformSize + uniformFlagSize);
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new (UniformPayloadSize(fEffect.get()))
                                                        GrSkSLFP(*this));
}

SkPMColor4f GrSkSLFP::constantOutputForConstantInput(const SkPMColor4f& inputColor) const {
    const SkFilterColorProgram* program = fEffect->getFilterColorProgram();
    SkASSERT(program);

    auto evalChild = [&](int index, SkPMColor4f color) {
        return ConstantOutputForConstantInput(this->childProcessor(index), color);
    };

    SkPMColor4f color = (fInputChildIndex >= 0)
                                ? ConstantOutputForConstantInput(
                                          this->childProcessor(fInputChildIndex), inputColor)
                                : inputColor;
    return program->eval(color, this->uniformData(), evalChild);
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
