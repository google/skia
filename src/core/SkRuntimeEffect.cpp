/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkMutex.h"
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
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/SkSLVMGenerator.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrFPArgs.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrSurfaceFillContext.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/image/SkImage_Gpu.h"
#endif

#include <algorithm>

namespace SkSL {
class SharedCompiler {
public:
    SharedCompiler() : fLock(compiler_mutex()) {
        if (!gImpl) {
            gImpl = new Impl();
        }
    }

    SkSL::Compiler* operator->() const { return gImpl->fCompiler; }

private:
    SkAutoMutexExclusive fLock;

    static SkMutex& compiler_mutex() {
        static SkMutex& mutex = *(new SkMutex);
        return mutex;
    }

    struct Impl {
        Impl() {
            // These caps are configured to apply *no* workarounds. This avoids changes that are
            // unnecessary (GLSL intrinsic rewrites), or possibly incorrect (adding do-while loops).
            // We may apply other "neutral" transformations to the user's SkSL, including inlining.
            // Anything determined by the device caps is deferred to the GPU backend. The processor
            // set produces the final program (including our re-emitted SkSL), and the backend's
            // compiler resolves any necessary workarounds.
            fCaps = ShaderCapsFactory::Standalone();
            fCaps->fBuiltinFMASupport = true;
            fCaps->fBuiltinDeterminantSupport = true;
            // Don't inline if it would require a do loop, some devices don't support them.
            fCaps->fCanUseDoLoops = false;

            fCompiler = new SkSL::Compiler(fCaps.get());
        }

        SkSL::ShaderCapsPointer fCaps;
        SkSL::Compiler*         fCompiler;
    };

    static Impl* gImpl;
};

SharedCompiler::Impl* SharedCompiler::gImpl = nullptr;

}  // namespace SkSL

// Accepts a valid marker, or "normals(<marker>)"
static bool parse_marker(const SkSL::StringFragment& marker, uint32_t* id, uint32_t* flags) {
    SkString s = marker;
    if (s.startsWith("normals(") && s.endsWith(')')) {
        *flags |= SkRuntimeEffect::Uniform::kMarkerNormals_Flag;
        s.set(marker.fChars + 8, marker.fLength - 9);
    }
    if (!SkCanvasPriv::ValidateMarker(s.c_str())) {
        return false;
    }
    *id = SkOpts::hash_fn(s.c_str(), s.size(), 0);
    return true;
}

static bool init_uniform_type(const SkSL::Context& ctx,
                              const SkSL::Type* type,
                              SkRuntimeEffect::Uniform* v) {
    using Type = SkRuntimeEffect::Uniform::Type;

    if (type == ctx.fTypes.fFloat.get())    { v->type = Type::kFloat;    return true; }
    if (type == ctx.fTypes.fHalf.get())     { v->type = Type::kFloat;    return true; }
    if (type == ctx.fTypes.fFloat2.get())   { v->type = Type::kFloat2;   return true; }
    if (type == ctx.fTypes.fHalf2.get())    { v->type = Type::kFloat2;   return true; }
    if (type == ctx.fTypes.fFloat3.get())   { v->type = Type::kFloat3;   return true; }
    if (type == ctx.fTypes.fHalf3.get())    { v->type = Type::kFloat3;   return true; }
    if (type == ctx.fTypes.fFloat4.get())   { v->type = Type::kFloat4;   return true; }
    if (type == ctx.fTypes.fHalf4.get())    { v->type = Type::kFloat4;   return true; }
    if (type == ctx.fTypes.fFloat2x2.get()) { v->type = Type::kFloat2x2; return true; }
    if (type == ctx.fTypes.fHalf2x2.get())  { v->type = Type::kFloat2x2; return true; }
    if (type == ctx.fTypes.fFloat3x3.get()) { v->type = Type::kFloat3x3; return true; }
    if (type == ctx.fTypes.fHalf3x3.get())  { v->type = Type::kFloat3x3; return true; }
    if (type == ctx.fTypes.fFloat4x4.get()) { v->type = Type::kFloat4x4; return true; }
    if (type == ctx.fTypes.fHalf4x4.get())  { v->type = Type::kFloat4x4; return true; }

    return false;
}

SkRuntimeEffect::Result SkRuntimeEffect::Make(SkString sksl, const Options& options) {
    SkSL::SharedCompiler compiler;
    SkSL::Program::Settings settings;
    settings.fInlineThreshold = options.inlineThreshold;
    settings.fAllowNarrowingConversions = true;
    auto program = compiler->convertProgram(SkSL::ProgramKind::kRuntimeEffect,
                                            SkSL::String(sksl.c_str(), sksl.size()),
                                            settings);
    // TODO: Many errors aren't caught until we process the generated Program here. Catching those
    // in the IR generator would provide better errors messages (with locations).
    #define RETURN_FAILURE(...) return Result{nullptr, SkStringPrintf(__VA_ARGS__)}

    if (!program) {
        RETURN_FAILURE("%s", compiler->errorText().c_str());
    }

    const SkSL::FunctionDefinition* main = nullptr;
    const bool usesSampleCoords = SkSL::Analysis::ReferencesSampleCoords(*program);
    const bool usesFragCoords   = SkSL::Analysis::ReferencesFragCoords(*program);

    // Color filters are not allowed to depend on position (local or device) in any way, but they
    // can sample children with matrices or explicit coords. Because the children are color filters,
    // we know (by induction) that they don't use those coords, so we keep the overall invariant.
    //
    // Further down, we also ensure that color filters can't use layout(marker), which would allow
    // them to change behavior based on the CTM.
    bool allowColorFilter = !usesSampleCoords && !usesFragCoords;

    size_t offset = 0;
    std::vector<Uniform> uniforms;
    std::vector<SkString> children;
    std::vector<SkSL::SampleUsage> sampleUsages;
    std::vector<Varying> varyings;
    const SkSL::Context& ctx(compiler->context());

    // Go through program elements, pulling out information that we need
    for (const SkSL::ProgramElement* elem : program->elements()) {
        // Variables (uniform, varying, etc.)
        if (elem->is<SkSL::GlobalVarDeclaration>()) {
            const SkSL::GlobalVarDeclaration& global = elem->as<SkSL::GlobalVarDeclaration>();
            const SkSL::VarDeclaration& varDecl = global.declaration()->as<SkSL::VarDeclaration>();

            const SkSL::Variable& var = varDecl.var();
            const SkSL::Type& varType = var.type();

            // Varyings (only used in conjunction with drawVertices)
            if (var.modifiers().fFlags & SkSL::Modifiers::kVarying_Flag) {
                varyings.push_back({var.name(),
                                    varType.typeKind() == SkSL::Type::TypeKind::kVector
                                            ? varType.columns()
                                            : 1});
            }
            // Fragment Processors (aka 'shader'): These are child effects
            else if (&varType == ctx.fTypes.fFragmentProcessor.get()) {
                children.push_back(var.name());
                sampleUsages.push_back(SkSL::Analysis::GetSampleUsage(*program, var));
            }
            // 'uniform' variables
            else if (var.modifiers().fFlags & SkSL::Modifiers::kUniform_Flag) {
                Uniform uni;
                uni.name = var.name();
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

                const SkSL::StringFragment& marker(var.modifiers().fLayout.fMarker);
                if (marker.fLength) {
                    uni.flags |= Uniform::kMarker_Flag;
                    allowColorFilter = false;
                    if (!parse_marker(marker, &uni.marker, &uni.flags)) {
                        RETURN_FAILURE("Invalid 'marker' string: '%.*s'", (int)marker.fLength,
                                        marker.fChars);
                    }
                }

                if (var.modifiers().fLayout.fFlags & SkSL::Layout::Flag::kSRGBUnpremul_Flag) {
                    uni.flags |= Uniform::kSRGBUnpremul_Flag;
                }

                uni.offset = offset;
                offset += uni.sizeInBytes();
                SkASSERT(SkIsAlign4(offset));

                uniforms.push_back(uni);
            }
        }
        // Functions
        else if (elem->is<SkSL::FunctionDefinition>()) {
            const auto& func = elem->as<SkSL::FunctionDefinition>();
            const SkSL::FunctionDeclaration& decl = func.declaration();
            if (decl.name() == "main") {
                main = &func;
            }
        }
    }

    if (!main) {
        RETURN_FAILURE("missing 'main' function");
    }

#undef RETURN_FAILURE

    sk_sp<SkRuntimeEffect> effect(new SkRuntimeEffect(std::move(sksl),
                                                      std::move(program),
                                                      *main,
                                                      std::move(uniforms),
                                                      std::move(children),
                                                      std::move(sampleUsages),
                                                      std::move(varyings),
                                                      usesSampleCoords,
                                                      allowColorFilter));
    return Result{std::move(effect), SkString()};
}

sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(SkString sksl) {
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

    auto [effect, err] = SkRuntimeEffect::Make(std::move(sksl));
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

size_t SkRuntimeEffect::Uniform::sizeInBytes() const {
    auto element_size = [](Type type) -> size_t {
        switch (type) {
            case Type::kFloat:  return sizeof(float);
            case Type::kFloat2: return sizeof(float) * 2;
            case Type::kFloat3: return sizeof(float) * 3;
            case Type::kFloat4: return sizeof(float) * 4;

            case Type::kFloat2x2: return sizeof(float) * 4;
            case Type::kFloat3x3: return sizeof(float) * 9;
            case Type::kFloat4x4: return sizeof(float) * 16;
            default: SkUNREACHABLE;
        }
    };
    return element_size(this->type) * this->count;
}

SkRuntimeEffect::SkRuntimeEffect(SkString sksl,
                                 std::unique_ptr<SkSL::Program> baseProgram,
                                 const SkSL::FunctionDefinition& main,
                                 std::vector<Uniform>&& uniforms,
                                 std::vector<SkString>&& children,
                                 std::vector<SkSL::SampleUsage>&& sampleUsages,
                                 std::vector<Varying>&& varyings,
                                 bool usesSampleCoords,
                                 bool allowColorFilter)
        : fHash(SkGoodHash()(sksl))
        , fSkSL(std::move(sksl))
        , fBaseProgram(std::move(baseProgram))
        , fMain(main)
        , fUniforms(std::move(uniforms))
        , fChildren(std::move(children))
        , fSampleUsages(std::move(sampleUsages))
        , fVaryings(std::move(varyings))
        , fUsesSampleCoords(usesSampleCoords)
        , fAllowColorFilter(allowColorFilter) {
    SkASSERT(fBaseProgram);
    SkASSERT(fChildren.size() == fSampleUsages.size());
}

SkRuntimeEffect::~SkRuntimeEffect() = default;

size_t SkRuntimeEffect::uniformSize() const {
    return fUniforms.empty() ? 0
                             : SkAlign4(fUniforms.back().offset + fUniforms.back().sizeInBytes());
}

const SkRuntimeEffect::Uniform* SkRuntimeEffect::findUniform(const char* name) const {
    auto iter = std::find_if(fUniforms.begin(), fUniforms.end(),
                             [name](const Uniform& u) { return u.name.equals(name); });
    return iter == fUniforms.end() ? nullptr : &(*iter);
}

int SkRuntimeEffect::findChild(const char* name) const {
    auto iter = std::find_if(fChildren.begin(), fChildren.end(),
                             [name](const SkString& s) { return s.equals(name); });
    return iter == fChildren.end() ? -1 : static_cast<int>(iter - fChildren.begin());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkData> get_xformed_uniforms(const SkRuntimeEffect* effect,
                                          sk_sp<SkData> baseUniforms,
                                          const SkMatrixProvider* matrixProvider,
                                          const SkColorSpace* dstCS) {
    using Flags = SkRuntimeEffect::Uniform::Flags;
    using Type = SkRuntimeEffect::Uniform::Type;
    SkColorSpaceXformSteps steps(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                 dstCS,               kUnpremul_SkAlphaType);

    sk_sp<SkData> uniforms = nullptr;
    auto writableData = [&]() {
        if (!uniforms) {
            uniforms =  SkData::MakeWithCopy(baseUniforms->data(), baseUniforms->size());
        }
        return uniforms->writable_data();
    };

    for (const auto& v : effect->uniforms()) {
        if (v.flags & Flags::kMarker_Flag) {
            SkASSERT(v.type == Type::kFloat4x4);
            // Color filters don't provide a matrix provider, but shouldn't be allowed to get here
            SkASSERT(matrixProvider);
            SkM44* localToMarker = SkTAddOffset<SkM44>(writableData(), v.offset);
            if (!matrixProvider->getLocalToMarker(v.marker, localToMarker)) {
                // We couldn't provide a matrix that was requested by the SkSL
                return nullptr;
            }
            if (v.flags & Flags::kMarkerNormals_Flag) {
                // Normals need to be transformed by the inverse-transpose of the upper-left
                // 3x3 portion (scale + rotate) of the matrix.
                localToMarker->setRow(3, {0, 0, 0, 1});
                localToMarker->setCol(3, {0, 0, 0, 1});
                if (!localToMarker->invert(localToMarker)) {
                    return nullptr;
                }
                *localToMarker = localToMarker->transpose();
            }
        } else if (v.flags & Flags::kSRGBUnpremul_Flag) {
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

class SkRuntimeColorFilter : public SkColorFilterBase {
public:
    SkRuntimeColorFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<SkData> uniforms,
                         sk_sp<SkColorFilter> children[],
                         size_t childCount)
            : fEffect(std::move(effect))
            , fUniforms(std::move(uniforms))
            , fChildren(children, children + childCount)
            , fIsAlphaUnchanged(this->computeIsAlphaUnchanged()) {}

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& colorInfo) const override {
        sk_sp<SkData> uniforms =
                get_xformed_uniforms(fEffect.get(), fUniforms, nullptr, colorInfo.colorSpace());
        if (!uniforms) {
            return GrFPFailure(nullptr);
        }

        auto fp = GrSkSLFP::Make(context, fEffect, "Runtime_Color_Filter", std::move(uniforms));
        for (const auto& child : fChildren) {
            std::unique_ptr<GrFragmentProcessor> childFP;
            if (child) {
                bool success;
                std::tie(success, childFP) = as_CFB(child)->asFragmentProcessor(
                        /*inputFP=*/nullptr, context, colorInfo);
                if (!success) {
                    return GrFPFailure(std::move(inputFP));
                }
            }
            fp->addChild(std::move(childFP));
        }

        // Runtime effect scripts are written to take an input color, not a fragment processor.
        // We need to pass the input to the runtime filter using Compose. This ensures that it will
        // be invoked exactly once, and the result will be returned when null children are sampled,
        // or as the (default) input color for non-null children.
        return GrFPSuccess(GrFragmentProcessor::Compose(std::move(fp), std::move(inputFP)));
    }
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        return false;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          SkColorSpace* dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        sk_sp<SkData> inputs = get_xformed_uniforms(fEffect.get(), fUniforms, nullptr, dstCS);
        if (!inputs) {
            return {};
        }

        // The color filter code might use sample-with-matrix (even though the matrix/coords are
        // ignored by the child). There should be no way for the color filter to use device coords.
        // Regardless, just to be extra-safe, we pass something valid (0, 0) as both coords, so
        // the builder isn't trying to do math on invalid values.
        skvm::Coord zeroCoord = { p->splat(0.0f), p->splat(0.0f) };

        auto sampleChild = [&](int ix, skvm::Coord /*coord*/) {
            if (fChildren[ix]) {
                return as_CFB(fChildren[ix])->program(p, c, dstCS, uniforms, alloc);
            } else {
                return c;
            }
        };

        std::vector<skvm::Val> uniform;
        for (int i = 0; i < (int)fEffect->uniformSize() / 4; i++) {
            int bits;
            memcpy(&bits, (const char*)inputs->data() + 4*i, 4);
            uniform.push_back(p->uniform32(uniforms->push(bits)).id);
        }

        return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p, uniform,
                                   /*device=*/zeroCoord, /*local=*/zeroCoord, sampleChild);
    }

    SkPMColor4f onFilterColor4f(const SkPMColor4f& color, SkColorSpace* dstCS) const override {
        fEffect->fColorFilterProgramOnce([&]{
            // While color filters are often transient, runtime effects are longer lived & cached.
            // So: We build and save a program on the *effect* that can filter a single color,
            // without baking in anything tied to a particular instance (uniforms or children).
            skvm::Builder p;

            // We allocate a uniform color for each child in the SkSL. When we run this program
            // later, these uniform values are replaced with either the results of the child,
            // or the input color (if the child is nullptr). These Uniform ids are loads from the
            // *first* arg ptr.
            skvm::Uniforms childColorUniforms{p.uniform(), 0};
            std::vector<skvm::Color> childColors;
            for (size_t i = 0; i < fChildren.size(); ++i) {
                childColors.push_back(
                        p.uniformColor(/*placeholder*/ SkColors::kWhite, &childColorUniforms));
            }
            auto sampleChild = [&](int ix, skvm::Coord /*coord*/) { return childColors[ix]; };

            // For SkSL uniforms, we reserve space and allocate skvm Uniform ids for each one.
            // When we run the program, these ids will be loads from the *second* arg ptr, the
            // uniform data of the specific color filter instance.
            skvm::Uniforms skslUniforms{p.uniform(), 0};
            std::vector<skvm::Val> uniform;
            for (int i = 0; i < (int)fEffect->uniformSize() / 4; i++) {
                uniform.push_back(p.uniform32(skslUniforms.push(/*placeholder*/ 0)).id);
            }

            // Emit the skvm instructions for the SkSL
            skvm::Coord zeroCoord = { p.splat(0.0f), p.splat(0.0f) };
            skvm::Color result = SkSL::ProgramToSkVM(*fEffect->fBaseProgram,
                                                     fEffect->fMain,
                                                     &p,
                                                     uniform,
                                                     /*device=*/zeroCoord,
                                                     /*local=*/zeroCoord,
                                                     sampleChild);

            // Then store the result to the *third* arg ptr
            p.store({skvm::PixelFormat::FLOAT, 32,32,32,32, 0,32,64,96}, p.arg(16), result);

            // We'll use this program to filter one color at a time, don't bother with jit
            fEffect->fColorFilterProgram =
                    std::make_unique<skvm::Program>(p.done(nullptr, /*allow_jit=*/false));
        });

        // At this point, we have a "generic" program for filtering a single color

        // Get our specific uniform values
        sk_sp<SkData> inputs = get_xformed_uniforms(fEffect.get(), fUniforms, nullptr, dstCS);

        // There should be no way for a color filter (which can't use "marker") to fail here
        SkASSERT(inputs && fEffect->fColorFilterProgram);

        // We defined sampling any child as returning a uniform color. Here, assemble a buffer
        // containing those colors. For any null children, the sample result is just the input
        // color. For non-null children, it's the result of that child filtering the input color.
        SkSTArray<1, SkPMColor4f, true> inputColors;
        for (const auto &child : fChildren) {
            inputColors.push_back(child ? as_CFB(child)->onFilterColor4f(color, dstCS) : color);
        }

        SkPMColor4f result;
        fEffect->fColorFilterProgram->eval(1, inputColors.begin(), inputs->data(), result.vec());
        return result;
    }

    bool onIsAlphaUnchanged() const override { return fIsAlphaUnchanged; }

    bool computeIsAlphaUnchanged() const {
        skvm::Builder  p;
        SkColorSpace*  dstCS = sk_srgb_singleton();  // This _shouldn't_ matter for alpha.
        skvm::Uniforms uniforms{p.uniform(), 0};
        SkArenaAlloc   alloc{16};

        skvm::Color in = p.load({skvm::PixelFormat::FLOAT, 32,32,32,32, 0,32,64,96}, p.arg(16)),
                   out = this->onProgram(&p,in,dstCS,&uniforms,&alloc);

        return out.a.id == in.a.id;
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeString(fEffect->source().c_str());
        if (fUniforms) {
            buffer.writeDataAsByteArray(fUniforms.get());
        } else {
            buffer.writeByteArray(nullptr, 0);
        }
        buffer.write32(fChildren.size());
        for (const auto& child : fChildren) {
            buffer.writeFlattenable(child.get());
        }
    }

    SK_FLATTENABLE_HOOKS(SkRuntimeColorFilter)

private:
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkData> fUniforms;
    std::vector<sk_sp<SkColorFilter>> fChildren;
    const bool fIsAlphaUnchanged;
};

sk_sp<SkFlattenable> SkRuntimeColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    auto effect = SkMakeCachedRuntimeEffect(std::move(sksl));
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    size_t childCount = buffer.read32();
    if (!buffer.validate(childCount == effect->children().count())) {
        return nullptr;
    }

    std::vector<sk_sp<SkColorFilter>> children(childCount);
    for (size_t i = 0; i < children.size(); ++i) {
        children[i] = buffer.readColorFilter();
    }

    return effect->makeColorFilter(std::move(uniforms), children.data(), children.size());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkRTShader : public SkShaderBase {
public:
    SkRTShader(sk_sp<SkRuntimeEffect> effect, sk_sp<SkData> uniforms, const SkMatrix* localMatrix,
               sk_sp<SkShader>* children, size_t childCount, bool isOpaque)
            : SkShaderBase(localMatrix)
            , fEffect(std::move(effect))
            , fIsOpaque(isOpaque)
            , fUniforms(std::move(uniforms))
            , fChildren(children, children + childCount) {}

    bool isOpaque() const override { return fIsOpaque; }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs& args) const override {
        SkMatrix matrix;
        if (!this->totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
            return nullptr;
        }

        sk_sp<SkData> uniforms = get_xformed_uniforms(
                fEffect.get(), fUniforms, &args.fMatrixProvider, args.fDstColorInfo->colorSpace());
        if (!uniforms) {
            return nullptr;
        }

        auto fp = GrSkSLFP::Make(args.fContext, fEffect, "runtime_shader", std::move(uniforms));
        for (const auto& child : fChildren) {
            auto childFP = child ? as_SB(child)->asFragmentProcessor(args) : nullptr;
            fp->addChild(std::move(childFP));
        }
        std::unique_ptr<GrFragmentProcessor> result = std::move(fp);
        // If the shader was created with isOpaque = true, we *force* that result here.
        // CPU does the same thing (in SkShaderBase::program).
        if (fIsOpaque) {
            result = GrFragmentProcessor::SwizzleOutput(std::move(result), GrSwizzle::RGB1());
        }
        result = GrMatrixEffect::Make(matrix, std::move(result));
        // Three cases of GrClampType to think about:
        //   kAuto   - Normalized fixed-point. If fIsOpaque, then A is 1 (above), and the format's
        //             range ensures RGB must be no larger. If !fIsOpaque, we clamp here.
        //   kManual - Normalized floating point. Whether or not we set A above, the format's range
        //             means we need to clamp RGB.
        //   kNone   - Unclamped floating point. No clamping is done, ever.
        GrClampType clampType = GrColorTypeClampType(args.fDstColorInfo->colorType());
        if (clampType == GrClampType::kManual || (clampType == GrClampType::kAuto && !fIsOpaque)) {
            return GrFragmentProcessor::ClampPremulOutput(std::move(result));
        } else {
            return result;
        }
    }
#endif

    bool onAppendStages(const SkStageRec& rec) const override {
        return false;
    }

    skvm::Color onProgram(skvm::Builder* p,
                          skvm::Coord device, skvm::Coord local, skvm::Color paint,
                          const SkMatrixProvider& matrices, const SkMatrix* localM,
                          SkFilterQuality quality, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        sk_sp<SkData> inputs =
                get_xformed_uniforms(fEffect.get(), fUniforms, &matrices, dst.colorSpace());
        if (!inputs) {
            return {};
        }

        SkMatrix inv;
        if (!this->computeTotalInverse(matrices.localToDevice(), localM, &inv)) {
            return {};
        }
        local = SkShaderBase::ApplyMatrix(p,inv,local,uniforms);

        auto sampleChild = [&](int ix, skvm::Coord coord) {
            if (fChildren[ix]) {
                SkOverrideDeviceMatrixProvider mats{matrices, SkMatrix::I()};
                return as_SB(fChildren[ix])->program(p, device, coord, paint,
                                                     mats, nullptr,
                                                     quality, dst,
                                                     uniforms, alloc);
            } else {
                return paint;
            }
        };

        std::vector<skvm::Val> uniform;
        for (int i = 0; i < (int)fEffect->uniformSize() / 4; i++) {
            int bits;
            memcpy(&bits, (const char*)inputs->data() + 4*i, 4);
            uniform.push_back(p->uniform32(uniforms->push(bits)).id);
        }

        return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p, uniform,
                                   device, local, sampleChild);
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
        if (fUniforms) {
            buffer.writeDataAsByteArray(fUniforms.get());
        } else {
            buffer.writeByteArray(nullptr, 0);
        }
        buffer.write32(flags);
        if (flags & kHasLocalMatrix_Flag) {
            buffer.writeMatrix(this->getLocalMatrix());
        }
        buffer.write32(fChildren.size());
        for (const auto& child : fChildren) {
            buffer.writeFlattenable(child.get());
        }
    }

    SkRuntimeEffect* asRuntimeEffect() const override { return fEffect.get(); }

    SK_FLATTENABLE_HOOKS(SkRTShader)

private:
    enum Flags {
        kIsOpaque_Flag          = 1 << 0,
        kHasLocalMatrix_Flag    = 1 << 1,
    };

    sk_sp<SkRuntimeEffect> fEffect;
    bool fIsOpaque;

    sk_sp<SkData> fUniforms;
    std::vector<sk_sp<SkShader>> fChildren;
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

    auto effect = SkMakeCachedRuntimeEffect(std::move(sksl));
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    size_t childCount = buffer.read32();
    if (!buffer.validate(childCount == effect->children().count())) {
        return nullptr;
    }

    std::vector<sk_sp<SkShader>> children(childCount);
    for (size_t i = 0; i < children.size(); ++i) {
        children[i] = buffer.readShader();
    }

    return effect->makeShader(std::move(uniforms), children.data(), children.size(), localMPtr,
                              isOpaque);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkRuntimeEffect::makeShader(sk_sp<SkData> uniforms,
                                            sk_sp<SkShader> children[], size_t childCount,
                                            const SkMatrix* localMatrix, bool isOpaque) {
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    return uniforms->size() == this->uniformSize() && childCount == fChildren.size()
        ? sk_sp<SkShader>(new SkRTShader(sk_ref_sp(this), std::move(uniforms), localMatrix,
                                         children, childCount, isOpaque))
        : nullptr;
}

sk_sp<SkImage> SkRuntimeEffect::makeImage(GrRecordingContext* recordingContext,
                                          sk_sp<SkData> uniforms,
                                          sk_sp<SkShader> children[],
                                          size_t childCount,
                                          const SkMatrix* localMatrix,
                                          SkImageInfo resultInfo,
                                          bool mipmapped) {
    if (recordingContext) {
#if SK_SUPPORT_GPU
        if (!recordingContext->priv().caps()->mipmapSupport()) {
            mipmapped = false;
        }
        auto fillContext = GrSurfaceFillContext::Make(recordingContext,
                                                      resultInfo,
                                                      SkBackingFit::kExact,
                                                      /*sample count*/ 1,
                                                      GrMipmapped(mipmapped));
        if (!fillContext) {
            return nullptr;
        }
        SkSimpleMatrixProvider matrixProvider(SkMatrix::I());
        uniforms = get_xformed_uniforms(this,
                                        std::move(uniforms),
                                        &matrixProvider,
                                        resultInfo.colorSpace());
        if (!uniforms) {
            return nullptr;
        }

        auto fp = GrSkSLFP::Make(recordingContext,
                                 sk_ref_sp(this),
                                 "runtime_image",
                                 std::move(uniforms));
        GrColorInfo colorInfo(resultInfo.colorInfo());
        GrFPArgs args(recordingContext,
                      matrixProvider,
                      SkSamplingOptions{},
                      &colorInfo);
        for (size_t i = 0; i < childCount; ++i) {
            if (!children[i]) {
                return nullptr;
            }
            auto childFP = as_SB(children[i])->asFragmentProcessor(args);
            fp->addChild(std::move(childFP));
        }
        if (localMatrix) {
            SkMatrix invLM;
            if (!localMatrix->invert(&invLM)) {
                return nullptr;
            }
            fillContext->fillWithFP(invLM, std::move(fp));
        } else {
            fillContext->fillWithFP(std::move(fp));
        }
        return sk_sp<SkImage>(new SkImage_Gpu(sk_ref_sp(recordingContext),
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
    auto shader = this->makeShader(std::move(uniforms), children, childCount, localMatrix, false);
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
                                                      sk_sp<SkColorFilter> children[],
                                                      size_t childCount) {
    if (!fAllowColorFilter) {
        return nullptr;
    }
    if (!uniforms) {
        uniforms = SkData::MakeEmpty();
    }
    return uniforms->size() == this->uniformSize() && childCount == fChildren.size()
        ? sk_sp<SkColorFilter>(new SkRuntimeColorFilter(sk_ref_sp(this), std::move(uniforms),
                                                        children, childCount))
        : nullptr;
}

sk_sp<SkColorFilter> SkRuntimeEffect::makeColorFilter(sk_sp<SkData> uniforms) {
    return this->makeColorFilter(std::move(uniforms), nullptr, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkRuntimeEffect::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkRuntimeColorFilter);
    SK_REGISTER_FLATTENABLE(SkRTShader);
}

SkRuntimeShaderBuilder::SkRuntimeShaderBuilder(sk_sp<SkRuntimeEffect> effect)
    : fEffect(std::move(effect))
    , fUniforms(SkData::MakeUninitialized(fEffect->uniformSize()))
    , fChildren(fEffect->children().count()) {}

SkRuntimeShaderBuilder::~SkRuntimeShaderBuilder() = default;

void* SkRuntimeShaderBuilder::writableUniformData() {
    if (!fUniforms->unique()) {
        fUniforms = SkData::MakeWithCopy(fUniforms->data(), fUniforms->size());
    }
    return fUniforms->writable_data();
}

sk_sp<SkShader> SkRuntimeShaderBuilder::makeShader(const SkMatrix* localMatrix, bool isOpaque) {
    return fEffect->makeShader(fUniforms, fChildren.data(), fChildren.size(), localMatrix, isOpaque);
}

sk_sp<SkImage> SkRuntimeShaderBuilder::makeImage(GrRecordingContext* recordingContext,
                                                 const SkMatrix* localMatrix,
                                                 SkImageInfo resultInfo,
                                                 bool mipmapped) {
    return fEffect->makeImage(recordingContext,
                              fUniforms,
                              fChildren.data(),
                              fChildren.size(),
                              localMatrix,
                              resultInfo,
                              mipmapped);
}

SkRuntimeShaderBuilder::BuilderChild&
SkRuntimeShaderBuilder::BuilderChild::operator=(const sk_sp<SkShader>& val) {
    if (fIndex < 0) {
        SkDEBUGFAIL("Assigning to missing child");
    } else {
        fOwner->fChildren[fIndex] = val;
    }
    return *this;
}
