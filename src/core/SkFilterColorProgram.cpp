/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFilterColorProgram.h"

#include "include/core/SkColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkVM.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/codegen/SkSLVMCodeGenerator.h"
#include "src/sksl/ir/SkSLProgram.h"

using namespace skia_private;

#if defined(SK_ENABLE_SKSL) && defined(SK_ENABLE_SKVM)

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
                         return c.type == SkRuntimeEffect::ChildType::kColorFilter;
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
                                             SkSpan(uniform),
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

    // We'll use this program to filter one color at a time, don't bother with jit
    return std::unique_ptr<SkFilterColorProgram>(
            new SkFilterColorProgram(p.done(/*debug_name=*/nullptr, /*allow_jit=*/false),
                                     std::move(sampleCalls)));
}

SkFilterColorProgram::SkFilterColorProgram(skvm::Program program,
                                           std::vector<SampleCall> sampleCalls)
        : fProgram(std::move(program))
        , fSampleCalls(std::move(sampleCalls)) {}

SkPMColor4f SkFilterColorProgram::eval(
        const SkPMColor4f& inColor,
        const void* uniformData,
        std::function<SkPMColor4f(int, SkPMColor4f)> evalChild) const {
    // Our program defines sampling any child as returning a uniform color. Assemble a buffer
    // containing those colors. The first entry is always the input color. Subsequent entries
    // are for each sample call, based on the information in fSampleCalls. For any null children,
    // the sample result is just the passed-in color.
    STArray<4, SkPMColor4f, true> childColors;
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

#endif  // defined(SK_ENABLE_SKSL) && defined(SK_ENABLE_SKVM)
