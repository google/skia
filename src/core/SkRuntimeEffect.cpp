/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkRuntimeEffect.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkSurface.h"
#include "include/private/SkMutex.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkUtils.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLSharedCompiler.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/codegen/SkSLVMCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/tracing/SkVMDebugTrace.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrFPArgs.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SurfaceFillContext.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/image/SkImage_Gpu.h"
#endif

#include <algorithm>

using ChildType = SkRuntimeEffect::ChildType;

#ifdef SK_ENABLE_SKSL

static sk_sp<SkSL::SkVMDebugTrace> make_skvm_debug_trace(SkRuntimeEffect* effect,
                                                         const SkIPoint& coord) {
    auto debugTrace = sk_make_sp<SkSL::SkVMDebugTrace>();
    debugTrace->setSource(effect->source());
    debugTrace->setTraceCoord(coord);
    return debugTrace;
}

static bool init_uniform_type(const SkSL::Context& ctx,
                              const SkSL::Type* type,
                              SkRuntimeEffect::Uniform* v) {
    using Type = SkRuntimeEffect::Uniform::Type;
    if (type->matches(*ctx.fTypes.fFloat))    { v->type = Type::kFloat;    return true; }
    if (type->matches(*ctx.fTypes.fHalf))     { v->type = Type::kFloat;    return true; }
    if (type->matches(*ctx.fTypes.fFloat2))   { v->type = Type::kFloat2;   return true; }
    if (type->matches(*ctx.fTypes.fHalf2))    { v->type = Type::kFloat2;   return true; }
    if (type->matches(*ctx.fTypes.fFloat3))   { v->type = Type::kFloat3;   return true; }
    if (type->matches(*ctx.fTypes.fHalf3))    { v->type = Type::kFloat3;   return true; }
    if (type->matches(*ctx.fTypes.fFloat4))   { v->type = Type::kFloat4;   return true; }
    if (type->matches(*ctx.fTypes.fHalf4))    { v->type = Type::kFloat4;   return true; }
    if (type->matches(*ctx.fTypes.fFloat2x2)) { v->type = Type::kFloat2x2; return true; }
    if (type->matches(*ctx.fTypes.fHalf2x2))  { v->type = Type::kFloat2x2; return true; }
    if (type->matches(*ctx.fTypes.fFloat3x3)) { v->type = Type::kFloat3x3; return true; }
    if (type->matches(*ctx.fTypes.fHalf3x3))  { v->type = Type::kFloat3x3; return true; }
    if (type->matches(*ctx.fTypes.fFloat4x4)) { v->type = Type::kFloat4x4; return true; }
    if (type->matches(*ctx.fTypes.fHalf4x4))  { v->type = Type::kFloat4x4; return true; }

    if (type->matches(*ctx.fTypes.fInt))  { v->type = Type::kInt;  return true; }
    if (type->matches(*ctx.fTypes.fInt2)) { v->type = Type::kInt2; return true; }
    if (type->matches(*ctx.fTypes.fInt3)) { v->type = Type::kInt3; return true; }
    if (type->matches(*ctx.fTypes.fInt4)) { v->type = Type::kInt4; return true; }

    return false;
}

static ChildType child_type(const SkSL::Type& type) {
    switch (type.typeKind()) {
        case SkSL::Type::TypeKind::kBlender:     return ChildType::kBlender;
        case SkSL::Type::TypeKind::kColorFilter: return ChildType::kColorFilter;
        case SkSL::Type::TypeKind::kShader:      return ChildType::kShader;
        default: SkUNREACHABLE;
    }
}

static bool verify_child_effects(const std::vector<SkRuntimeEffect::Child>& reflected,
                                 SkSpan<SkRuntimeEffect::ChildPtr> effectPtrs) {
    // Verify that the number of passed-in child-effect pointers matches the SkSL code.
    if (reflected.size() != effectPtrs.size()) {
        return false;
    }

    // Verify that each child object's type matches its declared type in the SkSL.
    for (size_t i = 0; i < effectPtrs.size(); ++i) {
        skstd::optional<ChildType> effectType = effectPtrs[i].type();
        if (effectType && effectType != reflected[i].type) {
            return false;
        }
    }
    return true;
}

static bool read_child_effects(SkReadBuffer& buffer,
                               const SkRuntimeEffect* effect,
                               SkTArray<SkRuntimeEffect::ChildPtr>* children) {
    size_t childCount = buffer.read32();
    if (!buffer.validate(childCount == effect->children().size())) {
        return false;
    }

    children->reset();
    children->reserve_back(childCount);

    for (const auto& child : effect->children()) {
        if (child.type == ChildType::kShader) {
            children->emplace_back(buffer.readShader());
        } else if (child.type == ChildType::kColorFilter) {
            children->emplace_back(buffer.readColorFilter());
        } else if (child.type == ChildType::kBlender) {
            children->emplace_back(buffer.readBlender());
        } else {
            return false;
        }
    }

    return buffer.isValid();
}

static void write_child_effects(SkWriteBuffer& buffer,
                                const std::vector<SkRuntimeEffect::ChildPtr>& children) {
    buffer.write32(children.size());
    for (const auto& child : children) {
        buffer.writeFlattenable(child.flattenable());
    }
}

static std::vector<skvm::Val> make_skvm_uniforms(skvm::Builder* p,
                                                 skvm::Uniforms* uniforms,
                                                 size_t inputSize,
                                                 const SkData& inputs) {
    SkASSERTF(!(inputSize & 3), "inputSize was %zu, expected a multiple of 4", inputSize);

    const int32_t* data = reinterpret_cast<const int32_t*>(inputs.data());
    const size_t uniformCount = inputSize / sizeof(int32_t);
    std::vector<skvm::Val> uniform;
    uniform.reserve(uniformCount);
    for (size_t index = 0; index < uniformCount; ++index) {
        int32_t bits;
        memcpy(&bits, data + index, sizeof(int32_t));
        uniform.push_back(p->uniform32(uniforms->push(bits)).id);
    }

    return uniform;
}

SkSL::ProgramSettings SkRuntimeEffect::MakeSettings(const Options& options, bool optimize) {
    SkSL::ProgramSettings settings;
    settings.fInlineThreshold = 0;
    settings.fForceNoInline = options.forceNoInline;
    settings.fEnforceES2Restrictions = options.enforceES2Restrictions;
    settings.fOptimize = optimize;
    return settings;
}

// TODO: Many errors aren't caught until we process the generated Program here. Catching those
// in the IR generator would provide better errors messages (with locations).
#define RETURN_FAILURE(...) return Result{nullptr, SkStringPrintf(__VA_ARGS__)}

SkRuntimeEffect::Result SkRuntimeEffect::MakeFromSource(SkString sksl,
                                                        const Options& options,
                                                        SkSL::ProgramKind kind) {
    std::unique_ptr<SkSL::Program> program;
    {
        // We keep this SharedCompiler in a separate scope to make sure it's destroyed before
        // calling the Make overload at the end, which creates its own (non-reentrant)
        // SharedCompiler instance
        SkSL::SharedCompiler compiler;
        SkSL::Program::Settings settings = MakeSettings(options, /*optimize=*/true);
        program = compiler->convertProgram(kind, SkSL::String(sksl.c_str(), sksl.size()), settings);

        if (!program) {
            RETURN_FAILURE("%s", compiler->errorText().c_str());
        }
    }
    return MakeInternal(std::move(program), options, kind);
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeFromDSL(std::unique_ptr<SkSL::Program> program,
                                                     const Options& options,
                                                     SkSL::ProgramKind kind) {
    // This factory is used for all DSL runtime effects, which don't have anything stored in the
    // program's source. Populate it so that we can compute fHash, and serialize these effects.
    program->fSource = std::make_unique<SkSL::String>(program->description());
    return MakeInternal(std::move(program), options, kind);
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeInternal(std::unique_ptr<SkSL::Program> program,
                                                      const Options& options,
                                                      SkSL::ProgramKind kind) {
    SkSL::SharedCompiler compiler;

    // Find 'main', then locate the sample coords parameter. (It might not be present.)
    const SkSL::FunctionDefinition* main = SkSL::Program_GetFunction(*program, "main");
    if (!main) {
        RETURN_FAILURE("missing 'main' function");
    }
    const auto& mainParams = main->declaration().parameters();
    auto iter = std::find_if(mainParams.begin(), mainParams.end(), [](const SkSL::Variable* p) {
        return p->modifiers().fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN;
    });
    const SkSL::ProgramUsage::VariableCounts sampleCoordsUsage =
            iter != mainParams.end() ? program->usage()->get(**iter)
                                     : SkSL::ProgramUsage::VariableCounts{};

    uint32_t flags = 0;
    switch (kind) {
        case SkSL::ProgramKind::kRuntimeColorFilter: flags |= kAllowColorFilter_Flag; break;
        case SkSL::ProgramKind::kRuntimeShader:      flags |= kAllowShader_Flag;      break;
        case SkSL::ProgramKind::kRuntimeBlender:     flags |= kAllowBlender_Flag;     break;
        default: SkUNREACHABLE;
    }

    if (sampleCoordsUsage.fRead || sampleCoordsUsage.fWrite) {
        flags |= kUsesSampleCoords_Flag;
    }

    // TODO(skia:12202): When we can layer modules, implement this restriction by moving the
    // declaration of sk_FragCoord to a private module.
    if (!options.allowFragCoord && SkSL::Analysis::ReferencesFragCoords(*program)) {
        RETURN_FAILURE("unknown identifier 'sk_FragCoord'");
    }

    // Color filters and blends are not allowed to depend on position (local or device) in any way.
    // The signature of main, and the declarations in sksl_rt_colorfilter/sksl_rt_blend should
    // guarantee this.
    if (flags & (kAllowColorFilter_Flag | kAllowBlender_Flag)) {
        SkASSERT(!(flags & kUsesSampleCoords_Flag));
        SkASSERT(!SkSL::Analysis::ReferencesFragCoords(*program));
    }

    if (SkSL::Analysis::CallsSampleOutsideMain(*program)) {
        flags |= kSamplesOutsideMain_Flag;
    }

    // Determine if this effect uses of the color transform intrinsics. Effects need to know this
    // so they can allocate color transform objects, etc.
    if (SkSL::Analysis::CallsColorTransformIntrinsics(*program)) {
        flags |= kUsesColorTransform_Flag;
    }

    size_t offset = 0;
    std::vector<Uniform> uniforms;
    std::vector<Child> children;
    std::vector<SkSL::SampleUsage> sampleUsages;
    int elidedSampleCoords = 0;
    const SkSL::Context& ctx(compiler->context());

    // Go through program elements, pulling out information that we need
    for (const SkSL::ProgramElement* elem : program->elements()) {
        // Variables (uniform, etc.)
        if (elem->is<SkSL::GlobalVarDeclaration>()) {
            const SkSL::GlobalVarDeclaration& global = elem->as<SkSL::GlobalVarDeclaration>();
            const SkSL::VarDeclaration& varDecl = global.declaration()->as<SkSL::VarDeclaration>();

            const SkSL::Variable& var = varDecl.var();
            const SkSL::Type& varType = var.type();

            // Child effects that can be sampled ('shader', 'colorFilter', 'blender')
            if (varType.isEffectChild()) {
                Child c;
                c.name  = SkString(var.name());
                c.type  = child_type(varType);
                c.index = children.size();
                children.push_back(c);
                auto usage = SkSL::Analysis::GetSampleUsage(
                        *program, var, sampleCoordsUsage.fWrite != 0, &elidedSampleCoords);
                // If the child is never sampled, we pretend that it's actually in PassThrough mode.
                // Otherwise, the GP code for collecting transforms and emitting transform code gets
                // very confused, leading to asserts and bad (backend) shaders. There's an implicit
                // assumption that every FP is used by its parent. (skbug.com/12429)
                sampleUsages.push_back(usage.isSampled() ? usage
                                                         : SkSL::SampleUsage::PassThrough());
            }
            // 'uniform' variables
            else if (var.modifiers().fFlags & SkSL::Modifiers::kUniform_Flag) {
                Uniform uni;
                uni.name = SkString(var.name());
                uni.flags = 0;
                uni.count = 1;

                const SkSL::Type* type = &var.type();
                if (type->isArray()) {
                    uni.flags |= Uniform::kArray_Flag;
                    uni.count = type->columns();
                    type = &type->componentType();
                }

                if (!init_uniform_type(ctx, type, &uni)) {
                    RETURN_FAILURE("Invalid uniform type: '%s'", type->displayName().c_str());
                }

                if (var.modifiers().fLayout.fFlags & SkSL::Layout::Flag::kColor_Flag) {
                    uni.flags |= Uniform::kColor_Flag;
                }

                uni.offset = offset;
                offset += uni.sizeInBytes();
                SkASSERT(SkIsAlign4(offset));

                uniforms.push_back(uni);
            }
        }
    }

    // If the sample coords are never written to, then we will have converted sample calls that use
    // them unmodified into "passthrough" sampling. If all references to the sample coords were of
    // that form, then we don't actually "use" sample coords. We unset the flag to prevent creating
    // an extra (unused) varying holding the coords.
    if (elidedSampleCoords == sampleCoordsUsage.fRead && sampleCoordsUsage.fWrite == 0) {
        flags &= ~kUsesSampleCoords_Flag;
    }

#undef RETURN_FAILURE

    sk_sp<SkRuntimeEffect> effect(new SkRuntimeEffect(std::move(program),
                                                      options,
                                                      *main,
                                                      std::move(uniforms),
                                                      std::move(children),
                                                      std::move(sampleUsages),
                                                      flags));
    return Result{std::move(effect), SkString()};
}

sk_sp<SkRuntimeEffect> SkRuntimeEffect::makeUnoptimizedClone() {
    // Compile with maximally-permissive options; any restrictions we need to enforce were already
    // handled when the original SkRuntimeEffect was made. We don't keep around the Options struct
    // from when it was initially made so we don't know what was originally requested.
    Options options;
    options.forceNoInline = true;
    options.enforceES2Restrictions = false;
    options.allowFragCoord = true;

    // We do know the original ProgramKind, so we don't need to re-derive it.
    SkSL::ProgramKind kind = fBaseProgram->fConfig->fKind;

    // Attempt to recompile the program's source with optimizations off. This ensures that the
    // Debugger shows results on every line, even for things that could be optimized away (static
    // branches, unused variables, etc). If recompilation fails, we fall back to the original code.
    std::unique_ptr<SkSL::Program> program;
    {
        // We keep this SharedCompiler in a separate scope to make sure it's destroyed before
        // calling MakeInternal at the end, which creates its own (non-reentrant) SharedCompiler
        // instance.
        SkSL::SharedCompiler compiler;
        SkSL::Program::Settings settings = MakeSettings(options, /*optimize=*/false);
        program = compiler->convertProgram(kind, *fBaseProgram->fSource, settings);

        if (!program) {
            // Turning off compiler optimizations can theoretically expose a program error that
            // had been optimized away (e.g. "all control paths return a value" might appear if
            // optimizing a program simplifies its control flow).
            // If this happens, the debugger will just have to show the optimized code.
            return sk_ref_sp(this);
        }
    }

    SkRuntimeEffect::Result result = MakeInternal(std::move(program), options, kind);
    if (!result.effect) {
        // Nothing in MakeInternal should change as a result of optimizations being toggled.
        SkDEBUGFAILF("makeUnoptimizedClone: MakeInternal failed\n%s",
                     result.errorText.c_str());
        return sk_ref_sp(this);
    }

    return result.effect;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForColorFilter(SkString sksl, const Options& options) {
    auto result = MakeFromSource(std::move(sksl), options, SkSL::ProgramKind::kRuntimeColorFilter);
    SkASSERT(!result.effect || result.effect->allowColorFilter());
    return result;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForShader(SkString sksl, const Options& options) {
    auto result = MakeFromSource(std::move(sksl), options, SkSL::ProgramKind::kRuntimeShader);
    SkASSERT(!result.effect || result.effect->allowShader());
    return result;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForBlender(SkString sksl, const Options& options) {
    auto result = MakeFromSource(std::move(sksl), options, SkSL::ProgramKind::kRuntimeBlender);
    SkASSERT(!result.effect || result.effect->allowBlender());
    return result;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForColorFilter(std::unique_ptr<SkSL::Program> program,
                                                            const Options& options) {
    auto result = MakeFromDSL(std::move(program), options, SkSL::ProgramKind::kRuntimeColorFilter);
    SkASSERT(!result.effect || result.effect->allowColorFilter());
    return result;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForShader(std::unique_ptr<SkSL::Program> program,
                                                       const Options& options) {
    auto result = MakeFromDSL(std::move(program), options, SkSL::ProgramKind::kRuntimeShader);
    SkASSERT(!result.effect || result.effect->allowShader());
    return result;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForBlender(std::unique_ptr<SkSL::Program> program,
                                                        const Options& options) {
    auto result = MakeFromDSL(std::move(program), options, SkSL::ProgramKind::kRuntimeBlender);
    SkASSERT(!result.effect || result.effect->allowBlender());
    return result;
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForColorFilter(
        std::unique_ptr<SkSL::Program> program) {
    return MakeForColorFilter(std::move(program), Options{});
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForShader(std::unique_ptr<SkSL::Program> program) {
    return MakeForShader(std::move(program), Options{});
}

SkRuntimeEffect::Result SkRuntimeEffect::MakeForBlender(std::unique_ptr<SkSL::Program> program) {
    return MakeForBlender(std::move(program), Options{});
}

sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(SkRuntimeEffect::Result (*make)(SkString sksl),
                                                 SkString sksl) {
    SK_BEGIN_REQUIRE_DENSE
    struct Key {
        uint32_t skslHashA;
        uint32_t skslHashB;

        bool operator==(const Key& that) const {
            return this->skslHashA == that.skslHashA
                && this->skslHashB == that.skslHashB;
        }

        explicit Key(const SkString& sksl)
            : skslHashA(SkOpts::hash(sksl.c_str(), sksl.size(), 0))
            , skslHashB(SkOpts::hash(sksl.c_str(), sksl.size(), 1)) {}
    };
    SK_END_REQUIRE_DENSE

    static auto* mutex = new SkMutex;
    static auto* cache = new SkLRUCache<Key, sk_sp<SkRuntimeEffect>>(11/*totally arbitrary*/);

    Key key(sksl);
    {
        SkAutoMutexExclusive _(*mutex);
        if (sk_sp<SkRuntimeEffect>* found = cache->find(key)) {
            return *found;
        }
    }

    auto [effect, err] = make(std::move(sksl));
    if (!effect) {
        return nullptr;
    }
    SkASSERT(err.isEmpty());

    {
        SkAutoMutexExclusive _(*mutex);
        cache->insert_or_update(key, effect);
    }
    return effect;
}

static size_t uniform_element_size(SkRuntimeEffect::Uniform::Type type) {
    switch (type) {
        case SkRuntimeEffect::Uniform::Type::kFloat:  return sizeof(float);
        case SkRuntimeEffect::Uniform::Type::kFloat2: return sizeof(float) * 2;
        case SkRuntimeEffect::Uniform::Type::kFloat3: return sizeof(float) * 3;
        case SkRuntimeEffect::Uniform::Type::kFloat4: return sizeof(float) * 4;

        case SkRuntimeEffect::Uniform::Type::kFloat2x2: return sizeof(float) * 4;
        case SkRuntimeEffect::Uniform::Type::kFloat3x3: return sizeof(float) * 9;
        case SkRuntimeEffect::Uniform::Type::kFloat4x4: return sizeof(float) * 16;

        case SkRuntimeEffect::Uniform::Type::kInt:  return sizeof(int);
        case SkRuntimeEffect::Uniform::Type::kInt2: return sizeof(int) * 2;
        case SkRuntimeEffect::Uniform::Type::kInt3: return sizeof(int) * 3;
        case SkRuntimeEffect::Uniform::Type::kInt4: return sizeof(int) * 4;
        default: SkUNREACHABLE;
    }
}

size_t SkRuntimeEffect::Uniform::sizeInBytes() const {
    static_assert(sizeof(int) == sizeof(float));
    return uniform_element_size(this->type) * this->count;
}

SkRuntimeEffect::SkRuntimeEffect(std::unique_ptr<SkSL::Program> baseProgram,
                                 const Options& options,
                                 const SkSL::FunctionDefinition& main,
                                 std::vector<Uniform>&& uniforms,
                                 std::vector<Child>&& children,
                                 std::vector<SkSL::SampleUsage>&& sampleUsages,
                                 uint32_t flags)
        : fHash(SkOpts::hash_fn(baseProgram->fSource->c_str(), baseProgram->fSource->size(), 0))
        , fBaseProgram(std::move(baseProgram))
        , fMain(main)
        , fUniforms(std::move(uniforms))
        , fChildren(std::move(children))
        , fSampleUsages(std::move(sampleUsages))
        , fFlags(flags) {
    SkASSERT(fBaseProgram);
    SkASSERT(fChildren.size() == fSampleUsages.size());

    // Everything from SkRuntimeEffect::Options which could influence the compiled result needs to
    // be accounted for in `fHash`. If you've added a new field to Options and caused the static-
    // assert below to trigger, please incorporate your field into `fHash` and update KnownOptions
    // to match the layout of Options.
    struct KnownOptions { bool forceNoInline, enforceES2Restrictions, allowFragCoord; };
    static_assert(sizeof(Options) == sizeof(KnownOptions));
    fHash = SkOpts::hash_fn(&options.forceNoInline,
                      sizeof(options.forceNoInline), fHash);
    fHash = SkOpts::hash_fn(&options.enforceES2Restrictions,
                      sizeof(options.enforceES2Restrictions), fHash);
    fHash = SkOpts::hash_fn(&options.allowFragCoord,
                      sizeof(options.allowFragCoord), fHash);

    fFilterColorProgram = SkFilterColorProgram::Make(this);
}

SkRuntimeEffect::~SkRuntimeEffect() = default;

const std::string& SkRuntimeEffect::source() const {
    return *fBaseProgram->fSource;
}

size_t SkRuntimeEffect::uniformSize() const {
    return fUniforms.empty() ? 0
                             : SkAlign4(fUniforms.back().offset + fUniforms.back().sizeInBytes());
}

const SkRuntimeEffect::Uniform* SkRuntimeEffect::findUniform(const char* name) const {
    SkASSERT(name);
    size_t len = strlen(name);
    auto iter = std::find_if(fUniforms.begin(), fUniforms.end(), [name, len](const Uniform& u) {
        return u.name.equals(name, len);
    });
    return iter == fUniforms.end() ? nullptr : &(*iter);
}

const SkRuntimeEffect::Child* SkRuntimeEffect::findChild(const char* name) const {
    SkASSERT(name);
    size_t len = strlen(name);
    auto iter = std::find_if(fChildren.begin(), fChildren.end(), [name, len](const Child& c) {
        return c.name.equals(name, len);
    });
    return iter == fChildren.end() ? nullptr : &(*iter);
}

std::unique_ptr<SkFilterColorProgram> SkFilterColorProgram::Make(const SkRuntimeEffect* effect) {
    // Our per-effect program technique is only possible (and necessary) for color filters
    if (!effect->allowColorFilter()) {
        return nullptr;
    }

    // TODO(skia:10479): Can we support this? When the color filter is invoked like this, there
    // may not be a real working space? If there is, we'd need to add it as a parameter to eval,
    // and then coordinate where the relevant uniforms go. For now, just fall back to the slow
    // path if we see these intrinsics being called.
    if (effect->usesColorTransform()) {
        return nullptr;
    }

    // We require that any children are color filters (not shaders or blenders). In theory, we could
    // detect the coords being passed to shader children, and replicate those calls, but that's very
    // complicated, and has diminishing returns. (eg, for table lookup color filters).
    if (!std::all_of(effect->fChildren.begin(),
                     effect->fChildren.end(),
                     [](const SkRuntimeEffect::Child& c) {
                         return c.type == ChildType::kColorFilter;
                     })) {
        return nullptr;
    }

    skvm::Builder p;

    // For SkSL uniforms, we reserve space and allocate skvm Uniform ids for each one. When we run
    // the program, these ids will be loads from the *first* arg ptr, the uniform data of the
    // specific color filter instance.
    skvm::Uniforms skslUniforms{p.uniform(), 0};
    const size_t uniformCount = effect->uniformSize() / 4;
    std::vector<skvm::Val> uniform;
    uniform.reserve(uniformCount);
    for (size_t i = 0; i < uniformCount; i++) {
        uniform.push_back(p.uniform32(skslUniforms.push(/*placeholder*/ 0)).id);
    }

    // We reserve a uniform color for each child invocation. While processing the SkSL, we record
    // the index of the child, and the color being filtered (in a SampleCall struct).
    // When we run this program later, we use the SampleCall to evaluate the correct child, and
    // populate these uniform values. These Uniform ids are loads from the *second* arg ptr.
    // If the color being passed is too complex for us to describe and re-create using SampleCall,
    // we are unable to use this per-effect program, and callers will need to fall back to another
    // (slower) implementation.
    skvm::Uniforms childColorUniforms{p.uniform(), 0};
    skvm::Color inputColor = p.uniformColor(/*placeholder*/ SkColors::kWhite, &childColorUniforms);
    std::vector<SkFilterColorProgram::SampleCall> sampleCalls;

    class Callbacks : public SkSL::SkVMCallbacks {
    public:
        Callbacks(skvm::Builder* builder,
                  const skvm::Uniforms* skslUniforms,
                  skvm::Uniforms* childColorUniforms,
                  skvm::Color inputColor,
                  std::vector<SkFilterColorProgram::SampleCall>* sampleCalls)
                : fBuilder(builder)
                , fSkslUniforms(skslUniforms)
                , fChildColorUniforms(childColorUniforms)
                , fInputColor(inputColor)
                , fSampleCalls(sampleCalls) {}

        bool isSimpleUniform(skvm::Color c, int* baseOffset) {
            skvm::Uniform ur, ug, ub, ua;
            if (!fBuilder->allUniform(c.r.id, &ur, c.g.id, &ug, c.b.id, &ub, c.a.id, &ua)) {
                return false;
            }
            skvm::Ptr uniPtr = fSkslUniforms->base;
            if (ur.ptr != uniPtr || ug.ptr != uniPtr || ub.ptr != uniPtr || ua.ptr != uniPtr) {
                return false;
            }
            *baseOffset = ur.offset;
            return ug.offset == ur.offset + 4 &&
                   ub.offset == ur.offset + 8 &&
                   ua.offset == ur.offset + 12;
        }

        static bool IDsEqual(skvm::Color x, skvm::Color y) {
            return x.r.id == y.r.id && x.g.id == y.g.id && x.b.id == y.b.id && x.a.id == y.a.id;
        }

        skvm::Color sampleColorFilter(int ix, skvm::Color c) override {
            skvm::Color result =
                    fBuilder->uniformColor(/*placeholder*/ SkColors::kWhite, fChildColorUniforms);
            SkFilterColorProgram::SampleCall call;
            call.fChild = ix;
            if (IDsEqual(c, fInputColor)) {
                call.fKind = SkFilterColorProgram::SampleCall::Kind::kInputColor;
            } else if (fBuilder->allImm(c.r.id, &call.fImm.fR,
                                        c.g.id, &call.fImm.fG,
                                        c.b.id, &call.fImm.fB,
                                        c.a.id, &call.fImm.fA)) {
                call.fKind = SkFilterColorProgram::SampleCall::Kind::kImmediate;
            } else if (auto it = std::find_if(fChildColors.begin(),
                                              fChildColors.end(),
                                              [&](skvm::Color x) { return IDsEqual(x, c); });
                       it != fChildColors.end()) {
                call.fKind = SkFilterColorProgram::SampleCall::Kind::kPrevious;
                call.fPrevious = SkTo<int>(it - fChildColors.begin());
            } else if (isSimpleUniform(c, &call.fOffset)) {
                call.fKind = SkFilterColorProgram::SampleCall::Kind::kUniform;
            } else {
                fAllSampleCallsSupported = false;
            }
            fSampleCalls->push_back(call);
            fChildColors.push_back(result);
            return result;
        }

        // We did an early return from this function if we saw any child that wasn't a shader, so
        // it should be impossible for either of these callbacks to occur:
        skvm::Color sampleShader(int, skvm::Coord) override {
            SkDEBUGFAIL("Unexpected child type");
            return {};
        }
        skvm::Color sampleBlender(int, skvm::Color, skvm::Color) override {
            SkDEBUGFAIL("Unexpected child type");
            return {};
        }

        // We did an early return from this function if we saw any call to these intrinsics, so it
        // should be impossible for either of these callbacks to occur:
        skvm::Color toLinearSrgb(skvm::Color color) override {
            SkDEBUGFAIL("Unexpected color transform intrinsic");
            return {};
        }
        skvm::Color fromLinearSrgb(skvm::Color color) override {
            SkDEBUGFAIL("Unexpected color transform intrinsic");
            return {};
        }

        skvm::Builder* fBuilder;
        const skvm::Uniforms* fSkslUniforms;
        skvm::Uniforms* fChildColorUniforms;
        skvm::Color fInputColor;
        std::vector<SkFilterColorProgram::SampleCall>* fSampleCalls;

        std::vector<skvm::Color> fChildColors;
        bool fAllSampleCallsSupported = true;
    };
    Callbacks callbacks(&p, &skslUniforms, &childColorUniforms, inputColor, &sampleCalls);

    // Emit the skvm instructions for the SkSL
    skvm::Coord zeroCoord = {p.splat(0.0f), p.splat(0.0f)};
    skvm::Color result = SkSL::ProgramToSkVM(*effect->fBaseProgram,
                                             effect->fMain,
                                             &p,
                                             /*debugTrace=*/nullptr,
                                             SkMakeSpan(uniform),
                                             /*device=*/zeroCoord,
                                             /*local=*/zeroCoord,
                                             inputColor,
                                             inputColor,
                                             &callbacks);

    // Then store the result to the *third* arg ptr
    p.store({skvm::PixelFormat::FLOAT, 32, 32, 32, 32, 0, 32, 64, 96},
            p.varying<skvm::F32>(), result);

    if (!callbacks.fAllSampleCallsSupported) {
        return nullptr;
    }

    // This is conservative. If a filter gets the input color by sampling a null child, we'll
    // return an (acceptable) false negative. All internal runtime color filters should work.
    bool alphaUnchanged = (inputColor.a.id == result.a.id);

    // We'll use this program to filter one color at a time, don't bother with jit
    return std::unique_ptr<SkFilterColorProgram>(
            new SkFilterColorProgram(p.done(/*debug_name=*/nullptr, /*allow_jit=*/false),
                                     std::move(sampleCalls),
                                     alphaUnchanged));
}

SkFilterColorProgram::SkFilterColorProgram(skvm::Program program,
                                           std::vector<SampleCall> sampleCalls,
                                           bool alphaUnchanged)
        : fProgram(std::move(program))
        , fSampleCalls(std::move(sampleCalls))
        , fAlphaUnchanged(alphaUnchanged) {}

SkPMColor4f SkFilterColorProgram::eval(
        const SkPMColor4f& inColor,
        const void* uniformData,
        std::function<SkPMColor4f(int, SkPMColor4f)> evalChild) const {
    // Our program defines sampling any child as returning a uniform color. Assemble a buffer
    // containing those colors. The first entry is always the input color. Subsequent entries
    // are for each sample call, based on the information in fSampleCalls. For any null children,
    // the sample result is just the passed-in color.
    SkSTArray<4, SkPMColor4f, true> childColors;
    childColors.push_back(inColor);
    for (const auto& s : fSampleCalls) {
        SkPMColor4f passedColor = inColor;
        switch (s.fKind) {
            case SampleCall::Kind::kInputColor:                                             break;
            case SampleCall::Kind::kImmediate:  passedColor = s.fImm;                       break;
            case SampleCall::Kind::kPrevious:   passedColor = childColors[s.fPrevious + 1]; break;
            case SampleCall::Kind::kUniform:
                passedColor = *SkTAddOffset<const SkPMColor4f>(uniformData, s.fOffset);
                break;
        }
        childColors.push_back(evalChild(s.fChild, passedColor));
    }

    SkPMColor4f result;
    fProgram.eval(1, uniformData, childColors.begin(), result.vec());
    return result;
}

const SkFilterColorProgram* SkRuntimeEffect::getFilterColorProgram() {
    return fFilterColorProgram.get();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkData> get_xformed_uniforms(const SkRuntimeEffect* effect,
                                          sk_sp<SkData> baseUniforms,
                                          const SkColorSpace* dstCS) {
    using Flags = SkRuntimeEffect::Uniform::Flags;
    using Type = SkRuntimeEffect::Uniform::Type;
    SkColorSpaceXformSteps steps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                 dstCS,               kUnpremul_SkAlphaType);

    sk_sp<SkData> uniforms = nullptr;
    auto writableData = [&]() {
        if (!uniforms) {
            uniforms = SkData::MakeWithCopy(baseUniforms->data(), baseUniforms->size());
        }
        return uniforms->writable_data();
    };

    for (const auto& v : effect->uniforms()) {
        if (v.flags & Flags::kColor_Flag) {
            SkASSERT(v.type == Type::kFloat3 || v.type == Type::kFloat4);
            if (steps.flags.mask()) {
                float* color = SkTAddOffset<float>(writableData(), v.offset);
                if (v.type == Type::kFloat4) {
                    // RGBA, easy case
                    for (int i = 0; i < v.count; ++i) {
                        steps.apply(color);
                        color += 4;
                    }
                } else {
                    // RGB, need to pad out to include alpha. Technically, this isn't necessary,
                    // because steps shouldn't include unpremul or premul, and thus shouldn't
                    // read or write the fourth element. But let's be safe.
                    float rgba[4];
                    for (int i = 0; i < v.count; ++i) {
                        memcpy(rgba, color, 3 * sizeof(float));
                        rgba[3] = 1.0f;
                        steps.apply(rgba);
                        memcpy(color, rgba, 3 * sizeof(float));
                        color += 3;
                    }
                }
            }
        }
    }
    return uniforms ? uniforms : baseUniforms;
}

#if SK_SUPPORT_GPU
static GrFPResult make_effect_fp(sk_sp<SkRuntimeEffect> effect,
                                 const char* name,
                                 sk_sp<SkData> uniforms,
                                 std::unique_ptr<GrFragmentProcessor> inputFP,
                                 std::unique_ptr<GrFragmentProcessor> destColorFP,
                                 SkSpan<const SkRuntimeEffect::ChildPtr> children,
                                 const GrFPArgs& childArgs) {
    SkSTArray<8, std::unique_ptr<GrFragmentProcessor>> childFPs;
    for (const auto& child : children) {
        skstd::optional<ChildType> type = child.type();
        if (type == ChildType::kShader) {
            // Convert a SkShader into a child FP.
            auto childFP = as_SB(child.shader())->asFragmentProcessor(childArgs);
            if (!childFP) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else if (type == ChildType::kColorFilter) {
            // Convert a SkColorFilter into a child FP.
            auto [success, childFP] = as_CFB(child.colorFilter())
                                              ->asFragmentProcessor(/*inputFP=*/nullptr,
                                                                    childArgs.fContext,
                                                                    *childArgs.fDstColorInfo);
            if (!success) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else if (type == ChildType::kBlender) {
            // Convert a SkBlender into a child FP.
            auto childFP = as_BB(child.blender())->asFragmentProcessor(
                    /*srcFP=*/nullptr,
                    GrFragmentProcessor::UseDestColorAsInput(/*dstFP=*/nullptr),
                    childArgs);
            if (!childFP) {
                return GrFPFailure(std::move(inputFP));
            }
            childFPs.push_back(std::move(childFP));
        } else {
            // We have a null child effect.
            childFPs.push_back(nullptr);
        }
    }
    auto fp = GrSkSLFP::MakeWithData(std::move(effect),
                                     name,
                                     childArgs.fDstColorInfo->refColorSpace(),
                                     std::move(inputFP),
                                     std::move(destColorFP),
                                     std::move(uniforms),
                                     SkMakeSpan(childFPs));
    SkASSERT(fp);
    return GrFPSuccess(std::move(fp));
}
#endif

class RuntimeEffectVMCallbacks : public SkSL::SkVMCallbacks {
public:
    RuntimeEffectVMCallbacks(skvm::Builder* builder,
                             skvm::Uniforms* uniforms,
                             SkArenaAlloc* alloc,
                             const std::vector<SkRuntimeEffect::ChildPtr>& children,
                             skvm::Color inColor,
                             const SkColorInfo& colorInfo)
            : fBuilder(builder)
            , fUniforms(uniforms)
            , fAlloc(alloc)
            , fChildren(children)
            , fInColor(inColor)
            , fColorInfo(colorInfo) {}

    skvm::Color sampleShader(int ix, skvm::Coord coord) override {
        if (SkShader* shader = fChildren[ix].shader()) {
            SkOverrideDeviceMatrixProvider matrixProvider(SkMatrix::I());
            return as_SB(shader)->program(fBuilder, coord, coord, fInColor, matrixProvider,
                                          /*localM=*/nullptr, fColorInfo, fUniforms, fAlloc);
        }
        return fInColor;
    }

    skvm::Color sampleColorFilter(int ix, skvm::Color color) override {
        if (SkColorFilter* colorFilter = fChildren[ix].colorFilter()) {
            return as_CFB(colorFilter)->program(fBuilder, color, fColorInfo, fUniforms, fAlloc);
        }
        return color;
    }

    skvm::Color sampleBlender(int ix, skvm::Color src, skvm::Color dst) override {
        if (SkBlender* blender = fChildren[ix].blender()) {
            return as_BB(blender)->program(fBuilder, src, dst, fColorInfo, fUniforms, fAlloc);
        }
        return blend(SkBlendMode::kSrcOver, src, dst);
    }

    skvm::Color toLinearSrgb(skvm::Color color) override {
        if (!fColorInfo.colorSpace()) {
            // These intrinsics do nothing when color management is disabled
            return color;
        }
        return SkColorSpaceXformSteps{fColorInfo.colorSpace(),    kUnpremul_SkAlphaType,
                                      sk_srgb_linear_singleton(), kUnpremul_SkAlphaType}
                .program(fBuilder, fUniforms, color);
    }

    skvm::Color fromLinearSrgb(skvm::Color color) override {
        if (!fColorInfo.colorSpace()) {
            // These intrinsics do nothing when color management is disabled
            return color;
        }
        return SkColorSpaceXformSteps{sk_srgb_linear_singleton(), kUnpremul_SkAlphaType,
                                      fColorInfo.colorSpace(),    kUnpremul_SkAlphaType}
                .program(fBuilder, fUniforms, color);
    }

    skvm::Builder* fBuilder;
    skvm::Uniforms* fUniforms;
    SkArenaAlloc* fAlloc;
    const std::vector<SkRuntimeEffect::ChildPtr>& fChildren;
    const skvm::Color fInColor;
    const SkColorInfo& fColorInfo;
};

class SkRuntimeColorFilter : public SkColorFilterBase {
public:
    SkRuntimeColorFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<SkData> uniforms,
                         SkSpan<SkRuntimeEffect::ChildPtr> children)
            : fEffect(std::move(effect))
            , fUniforms(std::move(uniforms))
            , fChildren(children.begin(), children.end()) {}

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& colorInfo) const override {
        sk_sp<SkData> uniforms =
                get_xformed_uniforms(fEffect.get(), fUniforms, colorInfo.colorSpace());
        SkASSERT(uniforms);

        SkOverrideDeviceMatrixProvider matrixProvider(SkMatrix::I());
        GrFPArgs childArgs(context, matrixProvider, &colorInfo);
        return make_effect_fp(fEffect,
                              "runtime_color_filter",
                              std::move(uniforms),
                              std::move(inputFP),
                              /*destColorFP=*/nullptr,
                              SkMakeSpan(fChildren),
                              childArgs);
    }
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        return false;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          const SkColorInfo& colorInfo,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        sk_sp<SkData> inputs =
                get_xformed_uniforms(fEffect.get(), fUniforms, colorInfo.colorSpace());
        SkASSERT(inputs);

        RuntimeEffectVMCallbacks callbacks(p, uniforms, alloc, fChildren, c, colorInfo);
        std::vector<skvm::Val> uniform = make_skvm_uniforms(p, uniforms, fEffect->uniformSize(),
                                                            *inputs);

        // There should be no way for the color filter to use device coords, but we need to supply
        // something. (Uninitialized values can trigger asserts in skvm::Builder).
        skvm::Coord zeroCoord = { p->splat(0.0f), p->splat(0.0f) };
        return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p,/*debugTrace=*/nullptr,
                                   SkMakeSpan(uniform), /*device=*/zeroCoord, /*local=*/zeroCoord,
                                   c, c, &callbacks);
    }

    SkPMColor4f onFilterColor4f(const SkPMColor4f& color, SkColorSpace* dstCS) const override {
        // Get the generic program for filtering a single color
        const SkFilterColorProgram* program = fEffect->getFilterColorProgram();
        if (!program) {
            // We were unable to build a cached (per-effect) program. Use the base-class fallback,
            // which builds a program for the specific filter instance.
            return SkColorFilterBase::onFilterColor4f(color, dstCS);
        }

        // Get our specific uniform values
        sk_sp<SkData> inputs = get_xformed_uniforms(fEffect.get(), fUniforms, dstCS);
        SkASSERT(inputs);

        auto evalChild = [&](int index, SkPMColor4f inColor) {
            const auto& child = fChildren[index];

            // SkFilterColorProgram::Make has guaranteed that any children will be color filters.
            SkASSERT(!child.shader());
            SkASSERT(!child.blender());
            if (SkColorFilter* colorFilter = child.colorFilter()) {
                return as_CFB(colorFilter)->onFilterColor4f(inColor, dstCS);
            }
            return inColor;
        };

        return program->eval(color, inputs->data(), evalChild);
    }

    bool onIsAlphaUnchanged() const override {
        return fEffect->getFilterColorProgram() &&
               fEffect->getFilterColorProgram()->isAlphaUnchanged();
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeString(fEffect->source().c_str());
        buffer.writeDataAsByteArray(fUniforms.get());
        write_child_effects(buffer, fChildren);
    }

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    SK_FLATTENABLE_HOOKS(SkRuntimeColorFilter)

private:
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkData> fUniforms;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

sk_sp<SkFlattenable> SkRuntimeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForColorFilter, std::move(sksl));
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    SkSTArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!read_child_effects(buffer, effect.get(), &children)) {
        return nullptr;
    }

    return effect->makeColorFilter(std::move(uniforms), SkMakeSpan(children));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(sk_sp<SkRuntimeEffect> effect,
               sk_sp<SkSL::SkVMDebugTrace> debugTrace,
               sk_sp<SkData> uniforms,
               const SkMatrix* localMatrix,
               SkSpan<SkRuntimeEffect::ChildPtr> children,
               bool isOpaque)
            : SkShaderBase(localMatrix)
            , fEffect(std::move(effect))
            , fDebugTrace(std::move(debugTrace))
            , fIsOpaque(isOpaque)
            , fUniforms(std::move(uniforms))
            , fChildren(children.begin(), children.end()) {}

    SkRuntimeEffect::TracedShader makeTracedClone(const SkIPoint& coord) {
        sk_sp<SkRuntimeEffect> unoptimized = fEffect->makeUnoptimizedClone();
        sk_sp<SkSL::SkVMDebugTrace> debugTrace = make_skvm_debug_trace(unoptimized.get(), coord);
        auto debugShader = sk_make_sp<SkRTShader>(unoptimized, debugTrace, fUniforms,
                                                  &this->getLocalMatrix(), SkMakeSpan(fChildren),
                                                  fIsOpaque);

        return SkRuntimeEffect::TracedShader{std::move(debugShader), std::move(debugTrace)};
    }

    bool isOpaque() const override { return fIsOpaque; }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs& args) const override {
        SkMatrix matrix;
        if (!this->totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }

        sk_sp<SkData> uniforms =
                get_xformed_uniforms(fEffect.get(), fUniforms, args.fDstColorInfo->colorSpace());
        SkASSERT(uniforms);

        auto [success, fp] = make_effect_fp(fEffect,
                                            "runtime_shader",
                                            std::move(uniforms),
                                            /*inputFP=*/nullptr,
                                            /*destColorFP=*/nullptr,
                                            SkMakeSpan(fChildren),
                                            args);
        if (!success) {
            return nullptr;
        }

        // If the shader was created with isOpaque = true, we *force* that result here.
        // CPU does the same thing (in SkShaderBase::program).
        if (fIsOpaque) {
            fp = GrFragmentProcessor::SwizzleOutput(std::move(fp), GrSwizzle::RGB1());
        }
        return GrMatrixEffect::Make(matrix, std::move(fp));
    }
#endif

    bool onAppendStages(const SkStageRec& rec) const override {
        return false;
    }

    skvm::Color onProgram(skvm::Builder* p,
                          skvm::Coord device, skvm::Coord local, skvm::Color paint,
                          const SkMatrixProvider& matrices, const SkMatrix* localM,
                          const SkColorInfo& colorInfo,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        sk_sp<SkData> inputs =
                get_xformed_uniforms(fEffect.get(), fUniforms, colorInfo.colorSpace());
        SkASSERT(inputs);

        SkMatrix inv;
        if (!this->computeTotalInverse(matrices.localToDevice(), localM, &inv)) {
            return {};
        }
        local = SkShaderBase::ApplyMatrix(p,inv,local,uniforms);

        RuntimeEffectVMCallbacks callbacks(p, uniforms, alloc, fChildren, paint, colorInfo);
        std::vector<skvm::Val> uniform = make_skvm_uniforms(p, uniforms, fEffect->uniformSize(),
                                                            *inputs);

        return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p, fDebugTrace.get(),
                                   SkMakeSpan(uniform), device, local, paint, paint, &callbacks);
    }

    void flatten(SkWriteBuffer& buffer) const override {
        uint32_t flags = 0;
        if (fIsOpaque) {
            flags |= kIsOpaque_Flag;
        }
        if (!this->getLocalMatrix().isIdentity()) {
            flags |= kHasLocalMatrix_Flag;
        }

        buffer.writeString(fEffect->source().c_str());
        buffer.writeDataAsByteArray(fUniforms.get());
        buffer.write32(flags);
        if (flags & kHasLocalMatrix_Flag) {
            buffer.writeMatrix(this->getLocalMatrix());
        }
        write_child_effects(buffer, fChildren);
    }

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    SK_FLATTENABLE_HOOKS(SkRTShader)

private:
    enum Flags {
        kIsOpaque_Flag          = 1 << 0,
        kHasLocalMatrix_Flag    = 1 << 1,
    };

    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkSL::SkVMDebugTrace> fDebugTrace;
    bool fIsOpaque;

    sk_sp<SkData> fUniforms;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();
    uint32_t flags = buffer.read32();

    bool isOpaque = SkToBool(flags & kIsOpaque_Flag);
    SkMatrix localM, *localMPtr = nullptr;
    if (flags & kHasLocalMatrix_Flag) {
        buffer.readMatrix(&localM);
        localMPtr = &localM;
    }

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    SkSTArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!read_child_effects(buffer, effect.get(), &children)) {
        return nullptr;
    }

    return effect->makeShader(std::move(uniforms), SkMakeSpan(children), localMPtr, isOpaque);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkRuntimeBlender : public SkBlenderBase {
public:
    SkRuntimeBlender(sk_sp<SkRuntimeEffect> effect,
                     sk_sp<SkData> uniforms,
                     SkSpan<SkRuntimeEffect::ChildPtr> children)
            : fEffect(std::move(effect))
            , fUniforms(std::move(uniforms))
            , fChildren(children.begin(), children.end()) {}

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                          const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                          SkArenaAlloc* alloc) const override {
        sk_sp<SkData> inputs = get_xformed_uniforms(fEffect.get(), fUniforms,
                                                    colorInfo.colorSpace());
        SkASSERT(inputs);

        RuntimeEffectVMCallbacks callbacks(p, uniforms, alloc, fChildren, src, colorInfo);
        std::vector<skvm::Val> uniform = make_skvm_uniforms(p, uniforms, fEffect->uniformSize(),
                                                            *inputs);

        // Emit the blend function as an SkVM program.
        skvm::Coord zeroCoord = {p->splat(0.0f), p->splat(0.0f)};
        return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p,/*debugTrace=*/nullptr,
                                   SkMakeSpan(uniform), /*device=*/zeroCoord, /*local=*/zeroCoord,
                                   src, dst, &callbacks);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            std::unique_ptr<GrFragmentProcessor> srcFP,
            std::unique_ptr<GrFragmentProcessor> dstFP,
            const GrFPArgs& args) const override {
        sk_sp<SkData> uniforms = get_xformed_uniforms(fEffect.get(), fUniforms,
                                                      args.fDstColorInfo->colorSpace());
        SkASSERT(uniforms);
        auto [success, fp] = make_effect_fp(fEffect,
                                            "runtime_blender",
                                            std::move(uniforms),
                                            std::move(srcFP),
                                            std::move(dstFP),
                                            SkMakeSpan(fChildren),
                                            args);

        return success ? std::move(fp) : nullptr;
    }
#endif

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeString(fEffect->source().c_str());
        buffer.writeDataAsByteArray(fUniforms.get());
        write_child_effects(buffer, fChildren);
    }

    SK_FLATTENABLE_HOOKS(SkRuntimeBlender)

private:
    using INHERITED = SkBlenderBase;

    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkData> fUniforms;
    std::vector<SkRuntimeEffect::ChildPtr> fChildren;
};

sk_sp<SkFlattenable> SkRuntimeBlender::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForBlender, std::move(sksl));
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    SkSTArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!read_child_effects(buffer, effect.get(), &children)) {
        return nullptr;
    }

    return effect->makeBlender(std::move(uniforms), SkMakeSpan(children));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<SkData> uniforms,
                                            sk_sp<SkShader> childShaders[],
                                            size_t childCount,
                                            const SkMatrix* localMatrix,
                                            bool isOpaque) const {
    SkSTArray<4, ChildPtr> children(childCount);
    for (size_t i = 0; i < childCount; ++i) {
        children.emplace_back(childShaders[i]);
    }
    return this->makeShader(std::move(uniforms), SkMakeSpan(children), localMatrix, isOpaque);
}

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<SkData> uniforms,
                                            SkSpan<ChildPtr> children,
                                            const SkMatrix* localMatrix,
                                            bool isOpaque) const {
    if (!this->allowShader()) {
        return nullptr;
    }
    if (!verify_child_effects(fChildren, children)) {
        return nullptr;
    }
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    if (uniforms->size() != this->uniformSize()) {
        return nullptr;
    }
    return sk_make_sp<SkRTShader>(sk_ref_sp(this), /*debugTrace=*/nullptr, std::move(uniforms),
                                  localMatrix, children, isOpaque);
}

sk_sp<SkImage> SkRuntimeEffect::makeImage(GrRecordingContext* rContext,
                                          sk_sp<SkData> uniforms,
                                          SkSpan<ChildPtr> children,
                                          const SkMatrix* localMatrix,
                                          SkImageInfo resultInfo,
                                          bool mipmapped) const {
    if (rContext) {
#if SK_SUPPORT_GPU
        if (!rContext->priv().caps()->mipmapSupport()) {
            mipmapped = false;
        }
        auto fillContext = rContext->priv().makeSFC(resultInfo,
                                                    SkBackingFit::kExact,
                                                    /*sample count*/ 1,
                                                    GrMipmapped(mipmapped));
        if (!fillContext) {
            return nullptr;
        }
        uniforms = get_xformed_uniforms(this, std::move(uniforms), resultInfo.colorSpace());
        SkASSERT(uniforms);

        SkOverrideDeviceMatrixProvider matrixProvider(SkMatrix::I());
        GrColorInfo colorInfo(resultInfo.colorInfo());
        GrFPArgs args(rContext, matrixProvider, &colorInfo);
        SkSTArray<8, std::unique_ptr<GrFragmentProcessor>> childFPs;
        for (size_t i = 0; i < children.size(); ++i) {
            // TODO: add support for other types of child effects
            if (SkShader* shader = children[i].shader()) {
                childFPs.push_back(as_SB(shader)->asFragmentProcessor(args));
            } else {
                return nullptr;
            }
        }
        auto fp = GrSkSLFP::MakeWithData(sk_ref_sp(this),
                                         "runtime_image",
                                         colorInfo.refColorSpace(),
                                         /*inputFP=*/nullptr,
                                         /*destColorFP=*/nullptr,
                                         std::move(uniforms),
                                         SkMakeSpan(childFPs));

        if (localMatrix) {
            SkMatrix invLM;
            if (!localMatrix->invert(&invLM)) {
                return nullptr;
            }
            fillContext->fillWithFP(invLM, std::move(fp));
        } else {
            fillContext->fillWithFP(std::move(fp));
        }
        return sk_sp<SkImage>(new SkImage_Gpu(sk_ref_sp(rContext),
                                              kNeedNewImageUniqueID,
                                              fillContext->readSurfaceView(),
                                              resultInfo.colorInfo()));
#else
        return nullptr;
#endif
    }
    if (resultInfo.alphaType() == kUnpremul_SkAlphaType) {
        // We don't have a good way of supporting this right now. In this case the runtime effect
        // will produce a unpremul value. The shader generated from it is assumed to produce
        // premul and RGB get pinned to A. Moreover, after the blend in premul the new dst is
        // unpremul'ed, producing a double unpremul result.
        return nullptr;
    }
    auto surf = SkSurface::MakeRaster(resultInfo);
    if (!surf) {
        return nullptr;
    }
    SkCanvas* canvas = surf->getCanvas();
    SkTLazy<SkCanvas> tempCanvas;
    auto shader = this->makeShader(std::move(uniforms), children, localMatrix, false);
    if (!shader) {
        return nullptr;
    }
    SkPaint paint;
    paint.setShader(std::move(shader));
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawPaint(paint);
    // TODO: Specify snapshot should have mip levels if mipmapped is true.
    return surf->makeImageSnapshot();
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> uniforms,
                                                      sk_sp<SkColorFilter> childColorFilters[],
                                                      size_t childCount) const {
    SkSTArray<4, ChildPtr> children(childCount);
    for (size_t i = 0; i < childCount; ++i) {
        children.emplace_back(childColorFilters[i]);
    }
    return this->makeColorFilter(std::move(uniforms), SkMakeSpan(children));
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> uniforms,
                                                      SkSpan<ChildPtr> children) const {
    if (!this->allowColorFilter()) {
        return nullptr;
    }
    if (!verify_child_effects(fChildren, children)) {
        return nullptr;
    }
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    if (uniforms->size() != this->uniformSize()) {
        return nullptr;
    }
    return sk_make_sp<SkRuntimeColorFilter>(sk_ref_sp(this), std::move(uniforms), children);
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> uniforms) const {
    return this->makeColorFilter(std::move(uniforms), /*children=*/{});
}

sk_sp<SkBlender> SkRuntimeEffect::makeBlender(sk_sp<SkData> uniforms,
                                              SkSpan<ChildPtr> children) const {
    if (!this->allowBlender()) {
        return nullptr;
    }
    if (!verify_child_effects(fChildren, children)) {
        return nullptr;
    }
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    if (uniforms->size() != this->uniformSize()) {
        return nullptr;
    }
    return sk_make_sp<SkRuntimeBlender>(sk_ref_sp(this), std::move(uniforms), children);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRuntimeEffect::TracedShader SkRuntimeEffect::MakeTraced(sk_sp<SkShader> shader,
                                                          const SkIPoint& traceCoord) {
    SkRuntimeEffect* effect = as_SB(shader)->asRuntimeEffect();
    if (!effect) {
        return TracedShader{nullptr, nullptr};
    }
    // An SkShader with an attached SkRuntimeEffect must be an SkRTShader.
    SkRTShader* rtShader = static_cast<SkRTShader*>(shader.get());
    return rtShader->makeTracedClone(traceCoord);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skstd::optional<ChildType> SkRuntimeEffect::ChildPtr::type() const {
    if (fChild) {
        switch (fChild->getFlattenableType()) {
            case SkFlattenable::kSkShader_Type:
                return ChildType::kShader;
            case SkFlattenable::kSkColorFilter_Type:
                return ChildType::kColorFilter;
            case SkFlattenable::kSkBlender_Type:
                return ChildType::kBlender;
            default:
                break;
        }
    }
    return skstd::nullopt;
}

SkShader* SkRuntimeEffect::ChildPtr::shader() const {
    return (fChild && fChild->getFlattenableType() == SkFlattenable::kSkShader_Type)
                   ? static_cast<SkShader*>(fChild.get())
                   : nullptr;
}

SkColorFilter* SkRuntimeEffect::ChildPtr::colorFilter() const {
    return (fChild && fChild->getFlattenableType() == SkFlattenable::kSkColorFilter_Type)
                   ? static_cast<SkColorFilter*>(fChild.get())
                   : nullptr;
}

SkBlender* SkRuntimeEffect::ChildPtr::blender() const {
    return (fChild && fChild->getFlattenableType() == SkFlattenable::kSkBlender_Type)
                   ? static_cast<SkBlender*>(fChild.get())
                   : nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkRuntimeEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkRuntimeColorFilter);
    SK_REGISTER_FLATTENABLE(SkRTShader);
    SK_REGISTER_FLATTENABLE(SkRuntimeBlender);
}

SkRuntimeShaderBuilder::SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect> effect)
        : INHERITED(std::move(effect)) {}

SkRuntimeShaderBuilder::~SkRuntimeShaderBuilder() = default;

sk_sp<SkImage> SkRuntimeShaderBuilder::makeImage(GrRecordingContext* recordingContext,
                                                 const SkMatrix* localMatrix,
                                                 SkImageInfo resultInfo,
                                                 bool mipmapped) {
    return this->effect()->makeImage(recordingContext,
                                     this->uniforms(),
                                     SkMakeSpan(this->children(), this->numChildren()),
                                     localMatrix,
                                     resultInfo,
                                     mipmapped);
}

sk_sp<SkShader> SkRuntimeShaderBuilder::makeShader(const SkMatrix* localMatrix, bool isOpaque) {
    return this->effect()->makeShader(this->uniforms(),
                                      SkMakeSpan(this->children(), this->numChildren()),
                                      localMatrix,
                                      isOpaque);
}

SkRuntimeBlendBuilder::SkRuntimeBlendBuilder(sk_sp<SkRuntimeEffect> effect)
        : INHERITED(std::move(effect)) {}

SkRuntimeBlendBuilder::~SkRuntimeBlendBuilder() = default;

sk_sp<SkBlender> SkRuntimeBlendBuilder::makeBlender() {
    return this->effect()->makeBlender(this->uniforms(),
                                       SkMakeSpan(this->children(), this->numChildren()));
}

#endif  // SK_ENABLE_SKSL
