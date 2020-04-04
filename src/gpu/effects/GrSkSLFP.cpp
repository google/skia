/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrSkSLFP.h"

#include "include/gpu/GrTexture.h"
#include "include/private/GrContext_Base.h"
#include "src/gpu/GrBaseContextPriv.h"
#include "src/gpu/GrSkSLFPFactoryCache.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

GrSkSLFPFactory::GrSkSLFPFactory(const char* name, const GrShaderCaps* shaderCaps, const char* sksl)
        : fName(name) {
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    fBaseProgram = fCompiler.convertProgram(SkSL::Program::kPipelineStage_Kind,
                                            SkSL::String(sksl), settings);
    if (fCompiler.errorCount()) {
        SkDebugf("%s\n", fCompiler.errorText().c_str());
    }
    SkASSERT(fBaseProgram);
    SkASSERT(!fCompiler.errorCount());
    for (const auto& e : *fBaseProgram) {
        if (e.fKind == SkSL::ProgramElement::kVar_Kind) {
            SkSL::VarDeclarations& v = (SkSL::VarDeclarations&) e;
            for (const auto& varStatement : v.fVars) {
                const SkSL::Variable& var = *((SkSL::VarDeclaration&) *varStatement).fVar;
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

const SkSL::Program* GrSkSLFPFactory::getSpecialization(const SkSL::String& key, const void* inputs,
                                                        size_t inputSize) {
    const auto& found = fSpecializations.find(key);
    if (found != fSpecializations.end()) {
        return found->second.get();
    }

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
    const SkSL::Program* result = specialized.get();
    fSpecializations.insert(std::make_pair(key, std::move(specialized)));
    return result;
}

static std::tuple<SkSL::Layout::CType, int> get_ctype(const SkSL::Context& context,
                                                      const SkSL::Variable& v) {
    auto [type, arrayCount] = strip_array(&v.fType);
    SkSL::Layout::CType result = v.fModifiers.fLayout.fCType;
    if (result == SkSL::Layout::CType::kDefault) {
        if (type == context.fFloat_Type.get() || type == context.fHalf_Type.get()) {
            result = SkSL::Layout::CType::kFloat;
        } else if (type == context.fFloat2_Type.get() || type == context.fHalf2_Type.get()) {
            result = SkSL::Layout::CType::kFloat2;
        } else if (type == context.fFloat3_Type.get() || type == context.fHalf3_Type.get()) {
            result = SkSL::Layout::CType::kFloat3;
        } else if (type == context.fFloat4_Type.get() || type == context.fHalf4_Type.get()) {
            result = SkSL::Layout::CType::kSkRect;
        } else if (type == context.fInt_Type.get()) {
            result = SkSL::Layout::CType::kInt32;
        } else if (type == context.fBool_Type.get()) {
            result = SkSL::Layout::CType::kBool;
        } else {
            return std::make_tuple(SkSL::Layout::CType::kDefault, arrayCount);
        }
    }
    return std::make_tuple(result, arrayCount);
}

class GrGLSLSkSLFP : public GrGLSLFragmentProcessor {
public:
    GrGLSLSkSLFP(const SkSL::Context* context,
                 const std::vector<const SkSL::Variable*>* inAndUniformVars,
                 SkSL::String glsl, std::vector<SkSL::Compiler::FormatArg> formatArgs,
                 std::vector<SkSL::Compiler::GLSLFunction> functions)
            : fContext(*context)
            , fInAndUniformVars(*inAndUniformVars)
            , fGLSL(glsl)
            , fFormatArgs(std::move(formatArgs))
            , fFunctions(std::move(functions)) {}

    GrSLType uniformType(const SkSL::Type& type) {
        if (type == *fContext.fFloat_Type) {
            return kFloat_GrSLType;
        } else if (type == *fContext.fHalf_Type) {
            return kHalf_GrSLType;
        } else if (type == *fContext.fFloat2_Type) {
            return kFloat2_GrSLType;
        } else if (type == *fContext.fHalf2_Type) {
            return kHalf2_GrSLType;
        } else if (type == *fContext.fFloat3_Type) {
            return kFloat3_GrSLType;
        } else if (type == *fContext.fHalf3_Type) {
            return kHalf3_GrSLType;
        } else if (type == *fContext.fFloat4_Type) {
            return kFloat4_GrSLType;
        } else if (type == *fContext.fHalf4_Type) {
            return kHalf4_GrSLType;
        } else if (type == *fContext.fFloat2x2_Type) {
            return kFloat2x2_GrSLType;
        } else if (type == *fContext.fHalf2x2_Type) {
            return kHalf2x2_GrSLType;
        } else if (type == *fContext.fFloat3x3_Type) {
            return kFloat3x3_GrSLType;
        } else if (type == *fContext.fHalf3x3_Type) {
            return kHalf3x3_GrSLType;
        } else if (type == *fContext.fFloat4x4_Type) {
            return kFloat4x4_GrSLType;
        } else if (type == *fContext.fHalf4x4_Type) {
            return kHalf4x4_GrSLType;
        } else if (type == *fContext.fBool_Type) {
            return kBool_GrSLType;
        } else if (type == *fContext.fInt_Type) {
            return kInt_GrSLType;
        }
        printf("%s\n", SkSL::String(type.fName).c_str());
        SK_ABORT("unsupported uniform type");
    }

    SkSL::String expandFormatArgs(const SkSL::String& raw,
                                  const EmitArgs& args,
                                  const std::vector<SkSL::Compiler::FormatArg> formatArgs,
                                  const char* coordsName,
                                  const std::vector<SkString>& childNames) {
        SkSL::String result;
        int substringStartIndex = 0;
        int formatArgIndex = 0;
        for (size_t i = 0; i < raw.length(); ++i) {
            char c = raw[i];
            if (c == '%') {
                result += SkSL::StringFragment(raw.c_str() + substringStartIndex,
                                               i - substringStartIndex);
                ++i;
                c = raw[i];
                switch (c) {
                    case 's': {
                        const SkSL::Compiler::FormatArg& arg = formatArgs[formatArgIndex++];
                        switch (arg.fKind) {
                            case SkSL::Compiler::FormatArg::Kind::kInput:
                                result += args.fInputColor;
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kOutput:
                                result += args.fOutputColor;
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kCoordX:
                                result += coordsName;
                                result += ".x";
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kCoordY:
                                result += coordsName;
                                result += ".y";
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kUniform:
                                result += args.fUniformHandler->getUniformCStr(
                                                                       fUniformHandles[arg.fIndex]);
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kChildProcessor:
                                result += childNames[arg.fIndex].c_str();
                                break;
                            case SkSL::Compiler::FormatArg::Kind::kFunctionName:
                                SkASSERT((int) fFunctionNames.size() > arg.fIndex);
                                result += fFunctionNames[arg.fIndex].c_str();
                                break;
                        }
                        break;
                    }
                    default:
                        result += c;
                }
                substringStartIndex = i + 1;
            }
        }
        result += SkSL::StringFragment(raw.c_str() + substringStartIndex,
                                       raw.length() - substringStartIndex);
        return result;
    }

    void emitCode(EmitArgs& args) override {
        for (const auto& v : fInAndUniformVars) {
            if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag && v->fType !=
                                                                *fContext.fFragmentProcessor_Type) {
                auto [type, arrayCount] = strip_array(&v->fType);
                fUniformHandles.push_back(args.fUniformHandler->addUniformArray(
                                                                   kFragment_GrShaderFlag,
                                                                   this->uniformType(*type),
                                                                   SkSL::String(v->fName).c_str(),
                                                                   arrayCount));
            }
        }
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString coords = args.fTransformedCoords.count()
            ? fragBuilder->ensureCoords2D(args.fTransformedCoords[0].fVaryingPoint)
            : SkString("sk_FragCoord");
        std::vector<SkString> childNames;
        for (int i = 0; i < this->numChildProcessors(); ++i) {
            childNames.push_back(SkStringPrintf("_child%d", i));
            this->invokeChild(i, &childNames[i], args);
        }
        for (const auto& f : fFunctions) {
            fFunctionNames.emplace_back();
            SkSL::String body = this->expandFormatArgs(f.fBody.c_str(), args, f.fFormatArgs,
                                                       coords.c_str(), childNames);
            fragBuilder->emitFunction(f.fReturnType,
                                      f.fName.c_str(),
                                      f.fParameters.size(),
                                      f.fParameters.data(),
                                      body.c_str(),
                                      &fFunctionNames.back());
        }
        fragBuilder->codeAppend(this->expandFormatArgs(fGLSL.c_str(), args, fFormatArgs,
                                                       coords.c_str(), childNames).c_str());
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        size_t uniformIndex = 0;
        size_t offset = 0;
        const GrSkSLFP& outer = _proc.cast<GrSkSLFP>();
        char* inputs = (char*) outer.fInputs.get();
        for (const auto& v : outer.fFactory->fInAndUniformVars) {
            auto [ctype, arrayCount] = get_ctype(fContext, *v);
            arrayCount = SkTMax(1, arrayCount);
            switch (ctype) {
                case SkSL::Layout::CType::kSkPMColor: {
                    offset = SkAlign4(offset);
                    SkSTArray<4, float, true> f;
                    for (int i = 0; i < arrayCount * 4; ++i) {
                        f.push_back(((uint8_t*)inputs)[offset++] / 255.0);
                    }
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                        pdman.set4fv(fUniformHandles[uniformIndex++], arrayCount, f.begin());
                    }
                    break;
                }
                case SkSL::Layout::CType::kFloat2: {
                    offset = SkAlign4(offset);
                    const float* f = (float*)(inputs + offset);
                    offset += sizeof(float) * 2 * arrayCount;
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                        pdman.set2fv(fUniformHandles[uniformIndex++], arrayCount, f);
                    }
                    break;
                }
                case SkSL::Layout::CType::kFloat3: {
                    offset = SkAlign4(offset);
                    const float* f = (float*)(inputs + offset);
                    offset += sizeof(float) * 3 * arrayCount;
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                        pdman.set3fv(fUniformHandles[uniformIndex++], arrayCount, f);
                    }
                    break;
                }
                case SkSL::Layout::CType::kSkPMColor4f:
                case SkSL::Layout::CType::kSkRect: {
                    offset = SkAlign4(offset);
                    const float* f = (float*)(inputs + offset);
                    offset += sizeof(float) * 4 * arrayCount;
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                        pdman.set4fv(fUniformHandles[uniformIndex++], arrayCount, f);
                    }
                    break;
                }
                case SkSL::Layout::CType::kInt32: {
                    offset = SkAlign4(offset);
                    int32_t* i = (int32_t*)(inputs + offset);
                    offset += sizeof(int32_t) * arrayCount;
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                        pdman.set1iv(fUniformHandles[uniformIndex++], arrayCount, i);
                    }
                    break;
                }
                case SkSL::Layout::CType::kFloat: {
                    offset = SkAlign4(offset);
                    float* f = (float*)(inputs + offset);
                    offset += sizeof(float) * arrayCount;
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag) {
                        pdman.set1fv(fUniformHandles[uniformIndex++], arrayCount, f);
                    }
                    break;
                }
                case SkSL::Layout::CType::kBool:
                    SkASSERT(!(v->fModifiers.fFlags & SkSL::Modifiers::kUniform_Flag));
                    ++offset;
                    break;
                default:
                    SkASSERT(&v->fType == fContext.fFragmentProcessor_Type.get());
            }
        }
    }

    const SkSL::Context& fContext;
    const std::vector<const SkSL::Variable*>& fInAndUniformVars;
    // nearly-finished GLSL; still contains printf-style "%s" format tokens
    const SkSL::String fGLSL;
    std::vector<SkSL::Compiler::FormatArg> fFormatArgs;
    std::vector<SkSL::Compiler::GLSLFunction> fFunctions;
    std::vector<UniformHandle> fUniformHandles;
    std::vector<SkString> fFunctionNames;
};

std::unique_ptr<GrSkSLFP> GrSkSLFP::Make(GrContext_Base* context, int index, const char* name,
                                         const char* sksl, const void* inputs, size_t inputSize,
                                         const SkMatrix* matrix) {
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(context->priv().fpFactoryCache(),
                                                  index, name, sksl, SkString(),
                                                  inputs, inputSize, matrix));
}

std::unique_ptr<GrSkSLFP> GrSkSLFP::Make(GrContext_Base* context, int index, const char* name,
                                         SkString sksl, const void* inputs, size_t inputSize,
                                         const SkMatrix* matrix) {
    return std::unique_ptr<GrSkSLFP>(new GrSkSLFP(context->priv().fpFactoryCache(),
                                                  index, name, nullptr, std::move(sksl),
                                                  inputs, inputSize, matrix));
}

GrSkSLFP::GrSkSLFP(sk_sp<GrSkSLFPFactoryCache> factoryCache,
                   int index, const char* name, const char* sksl,
                   SkString skslString, const void* inputs, size_t inputSize,
                   const SkMatrix* matrix)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fFactoryCache(factoryCache)
        , fIndex(index)
        , fName(name)
        , fSkSLString(skslString)
        , fSkSL(sksl ? sksl : fSkSLString.c_str())
        , fInputs(new int8_t[inputSize])
        , fInputSize(inputSize) {
    if (fInputSize) {
        memcpy(fInputs.get(), inputs, inputSize);
    }
    if (matrix) {
        fCoordTransform = GrCoordTransform(*matrix);
        this->addCoordTransform(&fCoordTransform);
    }
}

GrSkSLFP::GrSkSLFP(const GrSkSLFP& other)
        : INHERITED(kGrSkSLFP_ClassID, kNone_OptimizationFlags)
        , fFactoryCache(other.fFactoryCache)
        , fFactory(other.fFactory)
        , fIndex(other.fIndex)
        , fName(other.fName)
        , fSkSLString(other.fSkSLString)
        , fSkSL(other.fSkSL)
        , fInputs(new int8_t[other.fInputSize])
        , fInputSize(other.fInputSize) {
    if (fInputSize) {
        memcpy(fInputs.get(), other.fInputs.get(), fInputSize);
    }
    if (other.numCoordTransforms()) {
        fCoordTransform = other.fCoordTransform;
        this->addCoordTransform(&fCoordTransform);
    }
}

const char* GrSkSLFP::name() const {
    return fName;
}

void GrSkSLFP::createFactory() const {
    if (!fFactory) {
        fFactory = fFactoryCache->findOrCreate(fIndex, fName, fSkSL);
    }
}

void GrSkSLFP::addChild(std::unique_ptr<GrFragmentProcessor> child) {
    this->registerChildProcessor(std::move(child));
}

GrGLSLFragmentProcessor* GrSkSLFP::onCreateGLSLInstance() const {
    this->createFactory();
    const SkSL::Program* specialized = fFactory->getSpecialization(fKey, fInputs.get(), fInputSize);
    SkSL::String glsl;
    std::vector<SkSL::Compiler::FormatArg> formatArgs;
    std::vector<SkSL::Compiler::GLSLFunction> functions;
    if (!fFactory->fCompiler.toPipelineStage(*specialized, &glsl, &formatArgs, &functions)) {
        printf("%s\n", fFactory->fCompiler.errorText().c_str());
        SkASSERT(false);
    }
    return new GrGLSLSkSLFP(specialized->fContext.get(), &fFactory->fInAndUniformVars, glsl,
                            formatArgs, functions);
}

static void copy_floats_key(char* inputs, GrProcessorKeyBuilder* b, bool isIn, int count,
                            size_t* offset, SkSL::String* key) {
    if (isIn) {
        for (size_t i = 0; i < sizeof(float) * count; ++i) {
            (*key) += inputs[*offset + i];
            b->add32(*(int32_t*) (inputs + *offset));
            (*offset) += sizeof(float);
        }
    } else {
        (*offset) += sizeof(float) * count;
    }
}

void GrSkSLFP::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                     GrProcessorKeyBuilder* b) const {
    this->createFactory();
    b->add32(fIndex);
    size_t offset = 0;
    char* inputs = (char*) fInputs.get();
    const SkSL::Context& context = fFactory->fCompiler.context();
    for (const auto& v : fFactory->fInAndUniformVars) {
        if (&v->fType == context.fFragmentProcessor_Type.get()) {
            continue;
        }
        auto [ctype, arrayCount] = get_ctype(context, *v);
        for (int idx = 0; idx < SkTMax(1, arrayCount); ++idx) {
            switch (ctype) {
                case SkSL::Layout::CType::kBool:
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                        fKey += inputs[offset];
                        b->add32(inputs[offset]);
                    }
                    ++offset;
                    break;
                case SkSL::Layout::CType::kInt32: {
                    offset = SkAlign4(offset);
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                        fKey += inputs[offset + 0];
                        fKey += inputs[offset + 1];
                        fKey += inputs[offset + 2];
                        fKey += inputs[offset + 3];
                        b->add32(*(int32_t*)(inputs + offset));
                    }
                    offset += sizeof(int32_t);
                    break;
                }
                case SkSL::Layout::CType::kFloat: {
                    offset = SkAlign4(offset);
                    if (v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag) {
                        fKey += inputs[offset + 0];
                        fKey += inputs[offset + 1];
                        fKey += inputs[offset + 2];
                        fKey += inputs[offset + 3];
                        b->add32(*(float*)(inputs + offset));
                    }
                    offset += sizeof(float);
                    break;
                }
                case SkSL::Layout::CType::kFloat2:
                    copy_floats_key(inputs, b, v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag, 2,
                                    &offset, &fKey);
                    break;
                case SkSL::Layout::CType::kFloat3:
                    copy_floats_key(inputs, b, v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag, 3,
                                    &offset, &fKey);
                    break;
                case SkSL::Layout::CType::kSkPMColor:
                case SkSL::Layout::CType::kSkPMColor4f:
                case SkSL::Layout::CType::kSkRect:
                    copy_floats_key(inputs, b, v->fModifiers.fFlags & SkSL::Modifiers::kIn_Flag, 4,
                                    &offset, &fKey);
                    break;
                default:
                    // unsupported input var type
                    printf("%s\n", SkSL::String(v->fType.fName).c_str());
                    SkASSERT(false);
            }
        }
    }
}

bool GrSkSLFP::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSkSLFP& sk = other.cast<GrSkSLFP>();
    SkASSERT(fIndex != sk.fIndex || fInputSize == sk.fInputSize);
    return fIndex == sk.fIndex &&
            !memcmp(fInputs.get(), sk.fInputs.get(), fInputSize);
}

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::clone() const {
    std::unique_ptr<GrSkSLFP> result(new GrSkSLFP(*this));
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        result->registerChildProcessor(this->childProcessor(i).clone());
    }
    return std::unique_ptr<GrFragmentProcessor>(result.release());
}

/**************************************************************************************************/
GrSkSLFPFactoryCache::~GrSkSLFPFactoryCache() {}

sk_sp<GrSkSLFPFactory> GrSkSLFPFactoryCache::findOrCreate(int index, const char* name,
                                                          const char* skSL) {
    // acquire lock for checking/adding to cache
    SkAutoMutexExclusive ame(fCacheMutex);

    sk_sp<GrSkSLFPFactory> factory = this->get(index);
    if (!factory) {
        factory = sk_sp<GrSkSLFPFactory>(new GrSkSLFPFactory(name, fShaderCaps.get(), skSL));
        this->set(index, factory);
    }

    return factory;
}

sk_sp<GrSkSLFPFactory> GrSkSLFPFactoryCache::get(int index) {
    if (index >= (int) fFactories.size()) {
        return nullptr;
    }

    return fFactories[index];
}

void GrSkSLFPFactoryCache::set(int index, sk_sp<GrSkSLFPFactory> factory) {
    while (index >= (int) fFactories.size()) {
        fFactories.emplace_back();
    }
    SkASSERT(!fFactories[index]);
    fFactories[index] = std::move(factory);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSkSLFP);

#if GR_TEST_UTILS

#include "include/effects/SkArithmeticImageFilter.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"

extern const char* SKSL_ARITHMETIC_SRC;
extern const char* SKSL_DITHER_SRC;
extern const char* SKSL_OVERDRAW_SRC;

using Value = SkSL::Program::Settings::Value;

std::unique_ptr<GrFragmentProcessor> GrSkSLFP::TestCreate(GrProcessorTestData* d) {
    int type = d->fRandom->nextULessThan(3);
    switch (type) {
        case 0: {
            static int ditherIndex = NewIndex();
            int rangeType = d->fRandom->nextULessThan(3);
            std::unique_ptr<GrSkSLFP> result = GrSkSLFP::Make(d->context(), ditherIndex, "Dither",
                                                              SKSL_DITHER_SRC, &rangeType,
                                                              sizeof(rangeType));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
        case 1: {
            static int arithmeticIndex = NewIndex();
            ArithmeticFPInputs inputs;
            inputs.k[0] = d->fRandom->nextF();
            inputs.k[1] = d->fRandom->nextF();
            inputs.k[2] = d->fRandom->nextF();
            inputs.k[3] = d->fRandom->nextF();
            inputs.enforcePMColor = d->fRandom->nextBool();
            std::unique_ptr<GrSkSLFP> result = GrSkSLFP::Make(d->context(), arithmeticIndex,
                                                              "Arithmetic", SKSL_ARITHMETIC_SRC,
                                                              &inputs, sizeof(inputs));
            result->addChild(GrConstColorProcessor::Make(
                                                        SK_PMColor4fWHITE,
                                                        GrConstColorProcessor::InputMode::kIgnore));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
        case 2: {
            static int overdrawIndex = NewIndex();
            SkColor4f inputs[6];
            for (int i = 0; i < 6; ++i) {
                inputs[i] = SkColor4f::FromBytes_RGBA(d->fRandom->nextU());
            }
            std::unique_ptr<GrSkSLFP> result = GrSkSLFP::Make(d->context(), overdrawIndex,
                                                              "Overdraw", SKSL_OVERDRAW_SRC,
                                                              &inputs, sizeof(inputs));
            return std::unique_ptr<GrFragmentProcessor>(result.release());
        }
    }
    SK_ABORT("unreachable");
}

#endif
