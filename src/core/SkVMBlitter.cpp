/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkImageInfoPriv.h"
#include "include/private/SkMacros.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkOpts.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkVMBlitter.h"
#include "src/shaders/SkColorFilterShader.h"

#include <cinttypes>

namespace {

    // Uniforms set by the Blitter itself,
    // rather than by the Shader, which follow this struct in the skvm::Uniforms buffer.
    struct BlitterUniforms {
        int       right;  // First device x + blit run length n, used to get device x coordinate.
        int       y;      // Device y coordinate.
    };
    static_assert(SkIsAlign4(sizeof(BlitterUniforms)), "");
    inline static constexpr int kBlitterUniformsCount = sizeof(BlitterUniforms) / 4;

    static skvm::Coord device_coord(skvm::Builder* p, skvm::Uniforms* uniforms) {
        skvm::I32 dx = p->uniform32(uniforms->base, offsetof(BlitterUniforms, right))
                     - p->index(),
                  dy = p->uniform32(uniforms->base, offsetof(BlitterUniforms, y));
        return {
            to_F32(dx) + 0.5f,
            to_F32(dy) + 0.5f,
        };
    }

    struct NoopColorFilter : public SkColorFilterBase {
        skvm::Color onProgram(skvm::Builder*, skvm::Color c,
                              const SkColorInfo&, skvm::Uniforms*, SkArenaAlloc*) const override {
            return c;
        }

        bool onAppendStages(const SkStageRec&, bool) const override { return true; }

        // Only created here, should never be flattened / unflattened.
        Factory getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return "NoopColorFilter"; }
    };

    struct SpriteShader : public SkShaderBase {
        explicit SpriteShader(SkPixmap sprite) : fSprite(sprite) {}

        SkPixmap fSprite;

        // Only created here temporarily... never serialized.
        Factory      getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return "SpriteShader"; }

        bool isOpaque() const override { return fSprite.isOpaque(); }

        skvm::Color onProgram(skvm::Builder* p,
                              skvm::Coord /*device*/, skvm::Coord /*local*/, skvm::Color /*paint*/,
                              const SkMatrixProvider&, const SkMatrix* /*localM*/,
                              const SkColorInfo& dst,
                              skvm::Uniforms* uniforms, SkArenaAlloc*) const override {
            const SkColorType ct = fSprite.colorType();

            skvm::PixelFormat fmt = skvm::SkColorType_to_PixelFormat(ct);

            skvm::Color c = p->load(fmt, p->varying(SkColorTypeBytesPerPixel(ct)));

            return SkColorSpaceXformSteps{fSprite, dst}.program(p, uniforms, c);
        }
    };

    struct DitherShader : public SkShaderBase {
        explicit DitherShader(sk_sp<SkShader> shader) : fShader(std::move(shader)) {}

        sk_sp<SkShader> fShader;

        // Only created here temporarily... never serialized.
        Factory      getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return "DitherShader"; }

        bool isOpaque() const override { return fShader->isOpaque(); }

        skvm::Color onProgram(skvm::Builder* p,
                              skvm::Coord device, skvm::Coord local, skvm::Color paint,
                              const SkMatrixProvider& matrices, const SkMatrix* localM,
                              const SkColorInfo& dst,
                              skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
            // Run our wrapped shader.
            skvm::Color c = as_SB(fShader)->program(p, device,local, paint,
                                                    matrices,localM, dst, uniforms,alloc);
            if (!c) {
                return {};
            }

            float rate = 0.0f;
            switch (dst.colorType()) {
                case kARGB_4444_SkColorType:    rate =   1/15.0f; break;
                case   kRGB_565_SkColorType:    rate =   1/63.0f; break;
                case    kGray_8_SkColorType:
                case  kRGB_888x_SkColorType:
                case kRGBA_8888_SkColorType:
                case kBGRA_8888_SkColorType:
                case kSRGBA_8888_SkColorType:   rate =  1/255.0f; break;
                case kRGB_101010x_SkColorType:
                case kRGBA_1010102_SkColorType:
                case kBGR_101010x_SkColorType:
                case kBGRA_1010102_SkColorType: rate = 1/1023.0f; break;

                case kUnknown_SkColorType:
                case kAlpha_8_SkColorType:
                case kRGBA_F16_SkColorType:
                case kRGBA_F16Norm_SkColorType:
                case kRGBA_F32_SkColorType:
                case kR8G8_unorm_SkColorType:
                case kA16_float_SkColorType:
                case kA16_unorm_SkColorType:
                case kR16G16_float_SkColorType:
                case kR16G16_unorm_SkColorType:
                case kR16G16B16A16_unorm_SkColorType: return c;
            }

            // See SkRasterPipeline dither stage.
            // This is 8x8 ordered dithering.  From here we'll only need dx and dx^dy.
            SkASSERT(local.x.id == device.x.id);
            SkASSERT(local.y.id == device.y.id);
            skvm::I32 X =     trunc(device.x - 0.5f),
                      Y = X ^ trunc(device.y - 0.5f);

            // If X's low bits are abc and Y's def, M is fcebda,
            // 6 bits producing all values [0,63] shuffled over an 8x8 grid.
            skvm::I32 M = shl(Y & 1, 5)
                        | shl(X & 1, 4)
                        | shl(Y & 2, 2)
                        | shl(X & 2, 1)
                        | shr(Y & 4, 1)
                        | shr(X & 4, 2);

            // Scale to [0,1) by /64, then to (-0.5,0.5) using 63/128 (~0.492) as 0.5-ε,
            // and finally scale all that by rate.  We keep dither strength strictly
            // within ±0.5 to not change exact values like 0 or 1.

            // rate could be a uniform, but since it's based on the destination SkColorType,
            // we can bake it in without hurting the cache hit rate.
            float scale = rate * (  2/128.0f),
                  bias  = rate * (-63/128.0f);
            skvm::F32 dither = to_F32(M) * scale + bias;
            c.r += dither;
            c.g += dither;
            c.b += dither;

            c.r = clamp(c.r, 0.0f, c.a);
            c.g = clamp(c.g, 0.0f, c.a);
            c.b = clamp(c.b, 0.0f, c.a);
            return c;
        }
    };

    // This is similar to using SkShaders::Color(paint.getColor4f(), nullptr),
    // but uses the blitter-provided paint color uniforms instead of pushing its own.
    struct PaintColorShader : public SkShaderBase {
        explicit PaintColorShader(bool isOpaque) : fIsOpaque(isOpaque) {}

        const bool fIsOpaque;

        // Only created here temporarily... never serialized.
        Factory      getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return "PaintColorShader"; }

        bool isOpaque() const override { return fIsOpaque; }

        skvm::Color onProgram(skvm::Builder*,
                              skvm::Coord, skvm::Coord, skvm::Color paint,
                              const SkMatrixProvider&, const SkMatrix*, const SkColorInfo&,
                              skvm::Uniforms*, SkArenaAlloc*) const override {
            // Incoming `paint` is unpremul in the destination color space,
            // so we just need to premul it.
            return premul(paint);
        }
    };
}  // namespace

bool SkVMBlitter::Key::operator==(const Key& that) const {
    return this->shader      == that.shader
        && this->clip        == that.clip
        && this->blender     == that.blender
        && this->colorSpace  == that.colorSpace
        && this->colorType   == that.colorType
        && this->alphaType   == that.alphaType
        && this->coverage    == that.coverage;
}

SkVMBlitter::Key SkVMBlitter::Key::withCoverage(Coverage c) const {
    Key k = *this;
    k.coverage = SkToU8(c);
    return k;
}

SkVMBlitter::Params SkVMBlitter::Params::withCoverage(Coverage c) const {
    Params p = *this;
    p.coverage = c;
    return p;
}

SkVMBlitter::Params SkVMBlitter::EffectiveParams(const SkPixmap& device,
                                                 const SkPixmap* sprite,
                                                 SkPaint paint,
                                                 const SkMatrixProvider& matrices,
                                                 sk_sp<SkShader> clip) {
    // Sprites take priority over any shader.  (There's rarely one set, and it's meaningless.)
    if (sprite) {
        paint.setShader(sk_make_sp<SpriteShader>(*sprite));
    }

    // Normal blitters will have already folded color filters into their shader,
    // but we may still need to do that here for SpriteShaders.
    if (paint.getColorFilter()) {
        SkPaintPriv::RemoveColorFilter(&paint, device.colorSpace());
    }
    SkASSERT(!paint.getColorFilter());

    // If there's no explicit shader, the paint color is the shader,
    // but if there is a shader, it's modulated by the paint alpha.
    sk_sp<SkShader> shader = paint.refShader();
    if (!shader) {
        shader = sk_make_sp<PaintColorShader>(paint.getColor4f().isOpaque());
    } else if (paint.getAlphaf() < 1.0f) {
        shader = sk_make_sp<SkColorFilterShader>(std::move(shader),
                                                 paint.getAlphaf(),
                                                 sk_make_sp<NoopColorFilter>());
    }

    // Add dither to the end of the shader pipeline if requested and needed.
    if (paint.isDither() && !as_SB(shader)->isConstant()) {
        shader = sk_make_sp<DitherShader>(std::move(shader));
    }

    // Add the blender.
    sk_sp<SkBlender> blender = paint.refBlender();
    if (!blender) {
        blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    }

    // The most common blend mode is SrcOver, and it can be strength-reduced
    // _greatly_ to Src mode when the shader is opaque.
    //
    // In general all the information we use to make decisions here need to
    // be reflected in Params and Key to make program caching sound, and it
    // might appear that shader->isOpaque() is a property of the shader's
    // uniforms than its fundamental program structure and so unsafe to use.
    //
    // Opacity is such a powerful property that SkShaderBase::program()
    // forces opacity for any shader subclass that claims isOpaque(), so
    // the opaque bit is strongly guaranteed to be part of the program and
    // not just a property of the uniforms.  The shader program hash includes
    // this information, making it safe to use anywhere in the blitter codegen.
    if (as_BB(blender)->asBlendMode() == SkBlendMode::kSrcOver && shader->isOpaque()) {
        blender = SkBlender::Mode(SkBlendMode::kSrc);
    }

    SkColor4f paintColor = paint.getColor4f();
    SkColorSpaceXformSteps{sk_srgb_singleton(), kUnpremul_SkAlphaType,
                           device.colorSpace(), kUnpremul_SkAlphaType}
            .apply(paintColor.vec());

    return {
        std::move(shader),
        std::move(clip),
        std::move(blender),
        { device.colorType(), device.alphaType(), device.refColorSpace() },
        Coverage::Full,  // Placeholder... withCoverage() will change as needed.
        paintColor,
        matrices,
    };
}

skvm::Color SkVMBlitter::DstColor(skvm::Builder* p, const Params& params) {
    skvm::PixelFormat dstFormat = skvm::SkColorType_to_PixelFormat(params.dst.colorType());
    skvm::Ptr dst_ptr = p->varying(SkColorTypeBytesPerPixel(params.dst.colorType()));
    return p->load(dstFormat, dst_ptr);
}

void SkVMBlitter::BuildProgram(skvm::Builder* p, const Params& params,
                               skvm::Uniforms* uniforms, SkArenaAlloc* alloc) {
    // First two arguments are always uniforms and the destination buffer.
    uniforms->base    = p->uniform();
    skvm::Ptr dst_ptr = p->varying(SkColorTypeBytesPerPixel(params.dst.colorType()));
    // A SpriteShader (in this file) may next use one argument as its varying source.
    // Subsequent arguments depend on params.coverage:
    //    - Full:      (no more arguments)
    //    - Mask3D:    mul varying, add varying, 8-bit coverage varying
    //    - MaskA8:    8-bit coverage varying
    //    - MaskLCD16: 565 coverage varying
    //    - UniformF:  float coverage uniform

    skvm::Coord device = device_coord(p, uniforms);
    skvm::Color paint = p->uniformColor(params.paint, uniforms);

    // See note about arguments above: a SpriteShader will call p->arg() once during program().
    skvm::Color src = as_SB(params.shader)->program(p, device, /*local=*/device, paint,
                                                    params.matrices, /*localM=*/nullptr,
                                                    params.dst, uniforms, alloc);
    SkASSERT(src);
    if (params.coverage == Coverage::Mask3D) {
        skvm::F32 M = from_unorm(8, p->load8(p->varying<uint8_t>())),
                  A = from_unorm(8, p->load8(p->varying<uint8_t>()));

        src.r = min(src.r * M + A, src.a);
        src.g = min(src.g * M + A, src.a);
        src.b = min(src.b * M + A, src.a);
    }

    // GL clamps all its color channels to limits of the format just before the blend step (~here).
    // TODO: Below, we also clamp after the blend step. If we can prove that none of the work here
    // (especially blending, for built-in blend modes) will produce colors outside [0, 1] we may be
    // able to skip the second clamp. For now, we clamp twice.
    if (SkColorTypeIsNormalized(params.dst.colorType())) {
        src = clamp01(src);
    }

    // Load the destination color.
    skvm::PixelFormat dstFormat = skvm::SkColorType_to_PixelFormat(params.dst.colorType());
    skvm::Color dst = p->load(dstFormat, dst_ptr);
    if (params.dst.isOpaque()) {
        // When a destination is known opaque, we may assume it both starts and stays fully
        // opaque, ignoring any math that disagrees.  This sometimes trims a little work.
        dst.a = p->splat(1.0f);
    } else if (params.dst.alphaType() == kUnpremul_SkAlphaType) {
        // All our blending works in terms of premul.
        dst = premul(dst);
    }

    // Load coverage.
    skvm::Color cov;
    switch (params.coverage) {
        case Coverage::Full:
            cov.r = cov.g = cov.b = cov.a = p->splat(1.0f);
            break;

        case Coverage::UniformF:
            cov.r = cov.g = cov.b = cov.a = p->uniformF(p->uniform(), 0);
            break;

        case Coverage::Mask3D:
        case Coverage::MaskA8:
            cov.r = cov.g = cov.b = cov.a = from_unorm(8, p->load8(p->varying<uint8_t>()));
            break;

        case Coverage::MaskLCD16: {
            skvm::PixelFormat fmt = skvm::SkColorType_to_PixelFormat(kRGB_565_SkColorType);
            cov = p->load(fmt, p->varying<uint16_t>());
            cov.a = select(src.a < dst.a, min(cov.r, min(cov.g, cov.b)),
                           max(cov.r, max(cov.g, cov.b)));
        } break;
    }
    if (params.clip) {
        skvm::Color clip = as_SB(params.clip)->program(p, device, /*local=*/device, paint,
                                                       params.matrices, /*localM=*/nullptr,
                                                       params.dst, uniforms, alloc);
        SkAssertResult(clip);
        cov.r *= clip.a;  // We use the alpha channel of clip for all four.
        cov.g *= clip.a;
        cov.b *= clip.a;
        cov.a *= clip.a;
    }

    const SkBlenderBase* blender = as_BB(params.blender);
    const auto as_blendmode = blender->asBlendMode();

    // The math for some blend modes lets us fold coverage into src before the blend, which is
    // simpler than the canonical post-blend lerp().
    bool applyPostBlendCoverage = true;
    if (as_blendmode &&
        SkBlendMode_ShouldPreScaleCoverage(as_blendmode.value(),
                                           params.coverage == Coverage::MaskLCD16)) {
        applyPostBlendCoverage = false;
        src.r *= cov.r;
        src.g *= cov.g;
        src.b *= cov.b;
        src.a *= cov.a;
    }

    // Apply our blend function to the computed color.
    src = blender->program(p, src, dst, params.dst, uniforms, alloc);

    if (applyPostBlendCoverage) {
        src.r = lerp(dst.r, src.r, cov.r);
        src.g = lerp(dst.g, src.g, cov.g);
        src.b = lerp(dst.b, src.b, cov.b);
        src.a = lerp(dst.a, src.a, cov.a);
    }

    if (params.dst.isOpaque()) {
        // (See the note above when loading the destination color.)
        src.a = p->splat(1.0f);
    } else if (params.dst.alphaType() == kUnpremul_SkAlphaType) {
        src = unpremul(src);
    }

    // Clamp to fit destination color format if needed.
    if (SkColorTypeIsNormalized(params.dst.colorType())) {
        src = clamp01(src);
    }

    // Write it out!
    store(dstFormat, dst_ptr, src);
}

// If BuildProgram() can't build this program, CacheKey() sets *ok to false.
SkVMBlitter::Key SkVMBlitter::CacheKey(
        const Params& params, skvm::Uniforms* uniforms, SkArenaAlloc* alloc, bool* ok) {
    // Take care to match buildProgram()'s reuse of the paint color uniforms.
    skvm::Uniform r = uniforms->pushF(params.paint.fR),
                  g = uniforms->pushF(params.paint.fG),
                  b = uniforms->pushF(params.paint.fB),
                  a = uniforms->pushF(params.paint.fA);

    auto hash_shader = [&](skvm::Builder& p, const sk_sp<SkShader>& shader,
                           skvm::Color* outColor) {
        const SkShaderBase* sb = as_SB(shader);

        skvm::Coord device = device_coord(&p, uniforms);
        skvm::Color paint = {
            p.uniformF(r),
            p.uniformF(g),
            p.uniformF(b),
            p.uniformF(a),
        };

        uint64_t hash = 0;
        *outColor = sb->program(&p, device, /*local=*/device, paint, params.matrices,
                /*localM=*/nullptr, params.dst, uniforms, alloc);
        if (*outColor) {
            hash = p.hash();
            // p.hash() folds in all instructions to produce r,g,b,a but does not know
            // precisely which value we'll treat as which channel.  Imagine the shader
            // called std::swap(*r,*b)... it draws differently, but p.hash() is unchanged.
            // We'll fold the hash of their IDs in order to disambiguate.
            const skvm::Val outputs[] = {
                outColor->r.id,
                outColor->g.id,
                outColor->b.id,
                outColor->a.id
            };
            hash ^= SkOpts::hash(outputs, sizeof(outputs));
        } else {
            *ok = false;
        }
        return hash;
    };

    // Use this builder for shader, clip and blender, so that color objects that pass
    // from one to the other all 'make sense' -- i.e. have the same builder and/or have
    // meaningful values for the hash.
    //
    // Question: better if we just pass in mock uniform colors, so we don't need to
    //           explicitly use the output color from one stage as input to another?
    //
    skvm::Builder p;

    // Calculate a hash for the color shader.
    SkASSERT(params.shader);
    skvm::Color src;
    uint64_t shaderHash = hash_shader(p, params.shader, &src);

    // Calculate a hash for the clip shader, if one exists.
    uint64_t clipHash = 0;
    if (params.clip) {
        skvm::Color cov;
        clipHash = hash_shader(p, params.clip, &cov);
        if (clipHash == 0) {
            clipHash = 1;
        }
    }

    // Calculate a hash for the blender.
    uint64_t blendHash = 0;
    if (auto bm = as_BB(params.blender)->asBlendMode()) {
        blendHash = static_cast<uint8_t>(bm.value());
    } else if (*ok) {
        const SkBlenderBase* blender = as_BB(params.blender);

        skvm::Color dst = DstColor(&p, params);
        skvm::Color outColor = blender->program(&p, src, dst, params.dst, uniforms, alloc);
        if (outColor) {
            blendHash = p.hash();
            // Like in `hash_shader` above, we must fold the color component IDs into our hash.
            const skvm::Val outputs[] = {
                outColor.r.id,
                outColor.g.id,
                outColor.b.id,
                outColor.a.id
            };
            blendHash ^= SkOpts::hash(outputs, sizeof(outputs));
        } else {
            *ok = false;
        }
        if (blendHash == 0) {
            blendHash = 1;
        }
    }

    return {
        shaderHash,
        clipHash,
        blendHash,
        params.dst.colorSpace() ? params.dst.colorSpace()->hash() : 0,
        SkToU8(params.dst.colorType()),
        SkToU8(params.dst.alphaType()),
        SkToU8(params.coverage),
    };
}

SkVMBlitter::SkVMBlitter(const SkPixmap& device,
                         const SkPaint& paint,
                         const SkPixmap* sprite,
                         SkIPoint spriteOffset,
                         const SkMatrixProvider& matrices,
                         sk_sp<SkShader> clip,
                         bool* ok)
        : fDevice(device)
        , fSprite(sprite ? *sprite : SkPixmap{})
        , fSpriteOffset(spriteOffset)
        , fUniforms(skvm::UPtr{0}, kBlitterUniformsCount)
        , fParams(EffectiveParams(device, sprite, paint, matrices, std::move(clip)))
        , fKey(CacheKey(fParams, &fUniforms, &fAlloc, ok)) {}

SkVMBlitter::~SkVMBlitter() {
    if (SkLRUCache<Key, skvm::Program>* cache = TryAcquireProgramCache()) {
        auto cache_program = [&](skvm::Program&& program, Coverage coverage) {
            if (!program.empty()) {
                cache->insert_or_update(fKey.withCoverage(coverage), std::move(program));
            }
        };
        cache_program(std::move(fBlitH),         Coverage::Full);
        cache_program(std::move(fBlitAntiH),     Coverage::UniformF);
        cache_program(std::move(fBlitMaskA8),    Coverage::MaskA8);
        cache_program(std::move(fBlitMask3D),    Coverage::Mask3D);
        cache_program(std::move(fBlitMaskLCD16), Coverage::MaskLCD16);

        ReleaseProgramCache();
    }
}

SkLRUCache<SkVMBlitter::Key, skvm::Program>* SkVMBlitter::TryAcquireProgramCache() {
#if defined(SKVM_JIT)
    thread_local static SkLRUCache<Key, skvm::Program> cache{64};
    return &cache;
#else
    // iOS now supports thread_local since iOS 9.
    // On the other hand, we'll never be able to JIT there anyway.
    // It's probably fine to not cache any interpreted programs, anywhere.
    return nullptr;
#endif
}

SkString SkVMBlitter::DebugName(const Key& key) {
    return SkStringPrintf("Shader-%" PRIx64 "_Clip-%" PRIx64 "_Blender-%" PRIx64
                          "_CS-%" PRIx64 "_CT-%d_AT-%d_Cov-%d",
                          key.shader,
                          key.clip,
                          key.blender,
                          key.colorSpace,
                          key.colorType,
                          key.alphaType,
                          key.coverage);
}

void SkVMBlitter::ReleaseProgramCache() {}

skvm::Program SkVMBlitter::buildProgram(Coverage coverage) {
    Key key = fKey.withCoverage(coverage);
    {
        skvm::Program p;
        if (SkLRUCache<Key, skvm::Program>* cache = TryAcquireProgramCache()) {
            if (skvm::Program* found = cache->find(key)) {
                p = std::move(*found);
            }
            ReleaseProgramCache();
        }
        if (!p.empty()) {
            return p;
        }
    }
    // We don't really _need_ to rebuild fUniforms here.
    // It's just more natural to have effects unconditionally emit them,
    // and more natural to rebuild fUniforms than to emit them into a temporary buffer.
    // fUniforms should reuse the exact same memory, so this is very cheap.
    SkDEBUGCODE(size_t prev = fUniforms.buf.size();)
    fUniforms.buf.resize(kBlitterUniformsCount);
    skvm::Builder builder;
    BuildProgram(&builder, fParams.withCoverage(coverage), &fUniforms, &fAlloc);
    SkASSERTF(fUniforms.buf.size() == prev,
              "%zu, prev was %zu", fUniforms.buf.size(), prev);

    skvm::Program program = builder.done(DebugName(key).c_str());
    if (false) {
        static std::atomic<int> missed{0},
                total{0};
        if (!program.hasJIT()) {
            SkDebugf("\ncouldn't JIT %s\n", DebugName(key).c_str());
            builder.dump();
            program.dump();

            missed++;
        }
        if (0 == total++) {
            atexit([]{ SkDebugf("SkVMBlitter compiled %d programs, %d without JIT.\n",
                                total.load(), missed.load()); });
        }
    }
    return program;
}

void SkVMBlitter::updateUniforms(int right, int y) {
    BlitterUniforms uniforms{right, y};
    memcpy(fUniforms.buf.data(), &uniforms, sizeof(BlitterUniforms));
}

const void* SkVMBlitter::isSprite(int x, int y) const {
    if (fSprite.colorType() != kUnknown_SkColorType) {
        return fSprite.addr(x - fSpriteOffset.x(),
                            y - fSpriteOffset.y());
    }
    return nullptr;
}

void SkVMBlitter::blitH(int x, int y, int w) {
    if (fBlitH.empty()) {
        fBlitH = this->buildProgram(Coverage::Full);
    }
    this->updateUniforms(x+w, y);
    if (const void* sprite = this->isSprite(x,y)) {
        fBlitH.eval(w, fUniforms.buf.data(), fDevice.addr(x,y), sprite);
    } else {
        fBlitH.eval(w, fUniforms.buf.data(), fDevice.addr(x,y));
    }
}

void SkVMBlitter::blitAntiH(int x, int y, const SkAlpha cov[], const int16_t runs[]) {
    if (fBlitAntiH.empty()) {
        fBlitAntiH = this->buildProgram(Coverage::UniformF);
    }
    for (int16_t run = *runs; run > 0; run = *runs) {
        this->updateUniforms(x+run, y);
        const float covF = *cov * (1/255.0f);
        if (const void* sprite = this->isSprite(x,y)) {
            fBlitAntiH.eval(run, fUniforms.buf.data(), fDevice.addr(x,y), sprite, &covF);
        } else {
            fBlitAntiH.eval(run, fUniforms.buf.data(), fDevice.addr(x,y), &covF);
        }
        x    += run;
        runs += run;
        cov  += run;
    }
}

void SkVMBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    if (mask.fFormat == SkMask::kBW_Format) {
        return SkBlitter::blitMask(mask, clip);
    }

    const skvm::Program* program = nullptr;
    switch (mask.fFormat) {
        default: SkUNREACHABLE;     // ARGB and SDF masks shouldn't make it here.

        case SkMask::k3D_Format:
            if (fBlitMask3D.empty()) {
                fBlitMask3D = this->buildProgram(Coverage::Mask3D);
            }
            program = &fBlitMask3D;
            break;

        case SkMask::kA8_Format:
            if (fBlitMaskA8.empty()) {
                fBlitMaskA8 = this->buildProgram(Coverage::MaskA8);
            }
            program = &fBlitMaskA8;
            break;

        case SkMask::kLCD16_Format:
            if (fBlitMaskLCD16.empty()) {
                fBlitMaskLCD16 = this->buildProgram(Coverage::MaskLCD16);
            }
            program = &fBlitMaskLCD16;
            break;
    }

    SkASSERT(program);
    if (program) {
        for (int y = clip.top(); y < clip.bottom(); y++) {
             int x = clip.left(),
                 w = clip.width();
            void* dptr =        fDevice.writable_addr(x,y);
            auto  mptr = (const uint8_t*)mask.getAddr(x,y);
            this->updateUniforms(x+w,y);

            if (program == &fBlitMask3D) {
                size_t plane = mask.computeImageSize();
                if (const void* sprite = this->isSprite(x,y)) {
                    program->eval(w, fUniforms.buf.data(), dptr, sprite, mptr + 1*plane
                                                                       , mptr + 2*plane
                                                                       , mptr + 0*plane);
                } else {
                    program->eval(w, fUniforms.buf.data(), dptr, mptr + 1*plane
                                                               , mptr + 2*plane
                                                               , mptr + 0*plane);
                }
            } else {
                if (const void* sprite = this->isSprite(x,y)) {
                    program->eval(w, fUniforms.buf.data(), dptr, sprite, mptr);
                } else {
                    program->eval(w, fUniforms.buf.data(), dptr, mptr);
                }
            }
        }
    }
}

SkVMBlitter* SkVMBlitter::Make(const SkPixmap& device,
                               const SkPaint& paint,
                               const SkMatrixProvider& matrices,
                               SkArenaAlloc* alloc,
                               sk_sp<SkShader> clip) {
    bool ok = true;
    SkVMBlitter* blitter = alloc->make<SkVMBlitter>(
            device, paint, /*sprite=*/nullptr, SkIPoint{0,0}, matrices, std::move(clip), &ok);
    return ok ? blitter : nullptr;
}

SkVMBlitter* SkVMBlitter::Make(const SkPixmap& device,
                               const SkPaint& paint,
                               const SkPixmap& sprite,
                               int left, int top,
                               SkArenaAlloc* alloc,
                               sk_sp<SkShader> clip) {
    if (paint.getMaskFilter()) {
        // TODO: SkVM support for mask filters?  definitely possible!
        return nullptr;
    }
    bool ok = true;
    auto blitter = alloc->make<SkVMBlitter>(
            device, paint, &sprite, SkIPoint{left,top},
            SkSimpleMatrixProvider{SkMatrix{}}, std::move(clip), &ok);
    return ok ? blitter : nullptr;
}
