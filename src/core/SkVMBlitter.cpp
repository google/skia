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
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkOpts.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkColorFilterShader.h"

#include <cinttypes>

namespace {

    // Uniforms set by the Blitter itself,
    // rather than by the Shader, which follow this struct in the skvm::Uniforms buffer.
    struct BlitterUniforms {
        int       right;  // First device x + blit run length n, used to get device x coordinate.
        int       y;      // Device y coordinate.
        SkColor4f paint;  // In device color space.
    };
    static_assert(SkIsAlign4(sizeof(BlitterUniforms)), "");
    static constexpr int kBlitterUniformsCount = sizeof(BlitterUniforms) / 4;

    enum class Coverage { Full, UniformA8, MaskA8, MaskLCD16, Mask3D };

    struct Params {
        sk_sp<SkShader> shader;
        sk_sp<SkShader> clip;
        SkColorInfo     dst;
        SkBlendMode     blendMode;
        Coverage        coverage;
        SkFilterQuality quality;
        SkMatrix        ctm;

        Params withCoverage(Coverage c) const {
            Params p = *this;
            p.coverage = c;
            return p;
        }
    };

    SK_BEGIN_REQUIRE_DENSE;
    struct Key {
        uint64_t shader,
                 clip,
                 colorSpace;
        uint8_t  colorType,
                 alphaType,
                 blendMode,
                 coverage;
        uint32_t padding{0};
        // Params::quality and Params::ctm are only passed to {shader,clip}->program(),
        // not used here by the blitter itself.  No need to include them in the key;
        // they'll be folded into the shader key if used.

        bool operator==(const Key& that) const {
            return this->shader     == that.shader
                && this->clip       == that.clip
                && this->colorSpace == that.colorSpace
                && this->colorType  == that.colorType
                && this->alphaType  == that.alphaType
                && this->blendMode  == that.blendMode
                && this->coverage   == that.coverage;
        }

        Key withCoverage(Coverage c) const {
            Key k = *this;
            k.coverage = SkToU8(c);
            return k;
        }
    };
    SK_END_REQUIRE_DENSE;

    static SkString debug_name(const Key& key) {
        return SkStringPrintf(
            "Shader-%" PRIx64 "_Clip-%" PRIx64 "_CS-%" PRIx64 "_CT-%d_AT-%d_Blend-%d_Cov-%d",
            key.shader,
            key.clip,
            key.colorSpace,
            key.colorType,
            key.alphaType,
            key.blendMode,
            key.coverage);
    }

    static SkLRUCache<Key, skvm::Program>* try_acquire_program_cache() {
    #if 1 && defined(SKVM_JIT)
        thread_local static SkLRUCache<Key, skvm::Program> cache{64};
        return &cache;
    #else
        // iOS in particular does not support thread_local until iOS 9.0.
        // On the other hand, we'll never be able to JIT there anyway.
        // It's probably fine to not cache any interpreted programs, anywhere.
        return nullptr;
    #endif
    }

    static void release_program_cache() { }

    // If build_program() can't build this program, cache_key() sets *ok to false.
    static Key cache_key(const Params& params,
                         skvm::Uniforms* uniforms, SkArenaAlloc* alloc, bool* ok) {
        auto hash_shader = [&](const sk_sp<SkShader>& shader) {
            const SkShaderBase* sb = as_SB(shader);
            skvm::Builder p;

            skvm::I32 dx = p.uniform32(uniforms->base, offsetof(BlitterUniforms, right))
                         - p.index(),
                      dy = p.uniform32(uniforms->base, offsetof(BlitterUniforms, y));
            skvm::F32 x = to_f32(dx) + 0.5f,
                      y = to_f32(dy) + 0.5f;

            skvm::Color paint = {
                p.uniformF(uniforms->base, offsetof(BlitterUniforms, paint.fR)),
                p.uniformF(uniforms->base, offsetof(BlitterUniforms, paint.fG)),
                p.uniformF(uniforms->base, offsetof(BlitterUniforms, paint.fB)),
                p.uniformF(uniforms->base, offsetof(BlitterUniforms, paint.fA)),
            };

            uint64_t hash = 0;
            if (auto c = sb->program(&p,
                                     x,y, paint,
                                     params.ctm, /*localM=*/nullptr,
                                     params.quality, params.dst,
                                     uniforms,alloc)) {
                hash = p.hash();
                // p.hash() folds in all instructions to produce r,g,b,a but does not know
                // precisely which value we'll treat as which channel.  Imagine the shader
                // called std::swap(*r,*b)... it draws differently, but p.hash() is unchanged.
                // We'll fold the hash of their IDs in order to disambiguate.
                const skvm::Val outputs[] = { c.r.id, c.g.id, c.b.id, c.a.id };
                hash ^= SkOpts::hash(outputs, sizeof(outputs));
            } else {
                *ok = false;
            }
            return hash;
        };

        SkASSERT(params.shader);
        uint64_t shaderHash = hash_shader(params.shader);

        uint64_t clipHash = 0;
        if (params.clip) {
            clipHash = hash_shader(params.clip);
            if (clipHash == 0) {
                clipHash = 1;
            }
        }

        switch (params.dst.colorType()) {
            default: *ok = false;
                     break;

            case kRGB_565_SkColorType:
            case kRGB_888x_SkColorType:
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType:
            case kRGBA_1010102_SkColorType:
            case kBGRA_1010102_SkColorType:
            case kRGB_101010x_SkColorType:
            case kBGR_101010x_SkColorType:  break;
        }

        return {
            shaderHash,
              clipHash,
            params.dst.colorSpace() ? params.dst.colorSpace()->hash() : 0,
            SkToU8(params.dst.colorType()),
            SkToU8(params.dst.alphaType()),
            SkToU8(params.blendMode),
            SkToU8(params.coverage),
        };
    }

    static void build_program(skvm::Builder* p, const Params& params,
                              skvm::Uniforms* uniforms, SkArenaAlloc* alloc) {
        // First two arguments are always uniforms and the destination buffer.
        uniforms->base    = p->uniform();
        skvm::Arg dst_ptr = p->arg(SkColorTypeBytesPerPixel(params.dst.colorType()));
        // Other arguments depend on params.coverage:
        //    - Full:      (no more arguments)
        //    - Mask3D:    mul varying, add varying, 8-bit coverage varying
        //    - MaskA8:    8-bit coverage varying
        //    - MaskLCD16: 565 coverage varying
        //    - UniformA8: 8-bit coverage uniform

        skvm::I32 dx = p->uniform32(uniforms->base, offsetof(BlitterUniforms, right))
                     - p->index(),
                  dy = p->uniform32(uniforms->base, offsetof(BlitterUniforms, y));
        skvm::F32 x = to_f32(dx) + 0.5f,
                  y = to_f32(dy) + 0.5f;

        skvm::Color paint = {
            p->uniformF(uniforms->base, offsetof(BlitterUniforms, paint.fR)),
            p->uniformF(uniforms->base, offsetof(BlitterUniforms, paint.fG)),
            p->uniformF(uniforms->base, offsetof(BlitterUniforms, paint.fB)),
            p->uniformF(uniforms->base, offsetof(BlitterUniforms, paint.fA)),
        };

        skvm::Color src = as_SB(params.shader)->program(p, x,y, paint,
                                                        params.ctm, /*localM=*/nullptr,
                                                        params.quality, params.dst,
                                                        uniforms, alloc);
        SkASSERT(src);
        if (params.coverage == Coverage::Mask3D) {
            skvm::F32 M = from_unorm(8, p->load8(p->varying<uint8_t>())),
                      A = from_unorm(8, p->load8(p->varying<uint8_t>()));

            src.r = min(src.r * M + A, src.a);
            src.g = min(src.g * M + A, src.a);
            src.b = min(src.b * M + A, src.a);
        }

        // If we can determine this we can skip a fair bit of clamping!
        bool src_in_gamut = false;

        // Normalized premul formats can surprisingly represent some out-of-gamut
        // values (e.g. r=0xff, a=0xee fits in unorm8 but r = 1.07), but most code
        // working with normalized premul colors is not prepared to handle r,g,b > a.
        // So we clamp the shader to gamut here before blending and coverage.
        //
        // In addition, GL clamps all its color channels to limits of the format just
        // before the blend step (~here).  To match that auto-clamp, we clamp alpha to
        // [0,1] too, just in case someone gave us a crazy alpha.
        if (!src_in_gamut
                && params.dst.alphaType() == kPremul_SkAlphaType
                && SkColorTypeIsNormalized(params.dst.colorType())) {
            src.a = clamp(src.a, 0.0f,  1.0f);
            src.r = clamp(src.r, 0.0f, src.a);
            src.g = clamp(src.g, 0.0f, src.a);
            src.b = clamp(src.b, 0.0f, src.a);
            src_in_gamut = true;
        }

        // There are several orderings here of when we load dst and coverage
        // and how coverage is applied, and to complicate things, LCD coverage
        // needs to know dst.a.  We're careful to assert it's loaded in time.
        skvm::Color dst;
        SkDEBUGCODE(bool dst_loaded = false;)

        // load_coverage() returns false when there's no need to apply coverage.
        auto load_coverage = [&](skvm::Color* cov) {
            bool partial_coverage = true;
            switch (params.coverage) {
                case Coverage::Full: cov->r = cov->g = cov->b = cov->a = p->splat(1.0f);
                                     partial_coverage = false;
                                     break;

                case Coverage::UniformA8: cov->r = cov->g = cov->b = cov->a =
                                          from_unorm(8, p->uniform8(p->uniform(), 0));
                                          break;

                case Coverage::Mask3D:
                case Coverage::MaskA8: cov->r = cov->g = cov->b = cov->a =
                                       from_unorm(8, p->load8(p->varying<uint8_t>()));
                                       break;

                case Coverage::MaskLCD16:
                    SkASSERT(dst_loaded);
                    *cov = unpack_565(p->load16(p->varying<uint16_t>()));
                    cov->a = select(src.a < dst.a, min(cov->r, min(cov->g, cov->b))
                                                 , max(cov->r, max(cov->g, cov->b)));
                    break;
            }

            if (params.clip) {
                skvm::Color clip = as_SB(params.clip)->program(p, x,y, paint,
                                                               params.ctm, /*localM=*/nullptr,
                                                               params.quality, params.dst,
                                                               uniforms, alloc);
                SkAssertResult(clip);
                cov->r *= clip.a;  // We use the alpha channel of clip for all four.
                cov->g *= clip.a;
                cov->b *= clip.a;
                cov->a *= clip.a;
                return true;
            }

            return partial_coverage;
        };

        // The math for some blend modes lets us fold coverage into src before the blend,
        // obviating the need for the lerp afterwards. This early-coverage strategy tends
        // to be both faster and require fewer registers.
        bool lerp_coverage_post_blend = true;
        if (SkBlendMode_ShouldPreScaleCoverage(params.blendMode,
                                               params.coverage == Coverage::MaskLCD16)) {
            skvm::Color cov;
            if (load_coverage(&cov)) {
                src.r *= cov.r;
                src.g *= cov.g;
                src.b *= cov.b;
                src.a *= cov.a;
            }
            lerp_coverage_post_blend = false;
        }

        // Load up the destination color.
        SkDEBUGCODE(dst_loaded = true;)
        switch (params.dst.colorType()) {
            default: SkUNREACHABLE;
            case kRGB_565_SkColorType: dst = unpack_565(p->load16(dst_ptr));
                                       break;

            case  kRGB_888x_SkColorType: [[fallthrough]];
            case kRGBA_8888_SkColorType: dst = unpack_8888(p->load32(dst_ptr));
                                         break;

            case kBGRA_8888_SkColorType: dst = unpack_8888(p->load32(dst_ptr));
                                         std::swap(dst.r, dst.b);
                                         break;

            case  kRGB_101010x_SkColorType: [[fallthrough]];
            case kRGBA_1010102_SkColorType: dst = unpack_1010102(p->load32(dst_ptr));
                                            break;

            case  kBGR_101010x_SkColorType: [[fallthrough]];
            case kBGRA_1010102_SkColorType: dst = unpack_1010102(p->load32(dst_ptr));
                                            std::swap(dst.r, dst.b);
                                            break;
        }

        // When a destination is known opaque, we may assume it both starts and stays fully
        // opaque, ignoring any math that disagrees.  This sometimes trims a little work.
        if (params.dst.isOpaque()) {
            dst.a = p->splat(1.0f);
        } else if (params.dst.alphaType() == kUnpremul_SkAlphaType) {
            dst = premul(dst);
        }

        src = blend(params.blendMode, src, dst);

        // Lerp with coverage post-blend if needed.
        if (skvm::Color cov; lerp_coverage_post_blend && load_coverage(&cov)) {
            src.r = lerp(dst.r, src.r, cov.r);
            src.g = lerp(dst.g, src.g, cov.g);
            src.b = lerp(dst.b, src.b, cov.b);
            src.a = lerp(dst.a, src.a, cov.a);
        }

        if (params.dst.isOpaque()) {
            src.a = p->splat(1.0f);
        } else if (params.dst.alphaType() == kUnpremul_SkAlphaType) {
            src = unpremul(src);
        }

        // Clamp to fit destination color format if needed.
        if (src_in_gamut) {
            // An in-gamut src blended with an in-gamut dst should stay in gamut.
            // Being in-gamut implies all channels are in [0,1], so no need to clamp.
            // We allow one ulp error above 1.0f, and about that much (~1.2e-7) below 0.
            skvm::F32 lo = bit_cast(p->splat(0xb400'0000)),
                      hi = bit_cast(p->splat(0x3f80'0001));
            assert_true(src.r == clamp(src.r, lo, hi), src.r);
            assert_true(src.g == clamp(src.g, lo, hi), src.g);
            assert_true(src.b == clamp(src.b, lo, hi), src.b);
            assert_true(src.a == clamp(src.a, lo, hi), src.a);
        } else if (SkColorTypeIsNormalized(params.dst.colorType())) {
            src.r = clamp01(src.r);
            src.g = clamp01(src.g);
            src.b = clamp01(src.b);
            src.a = clamp01(src.a);
        }

        // Store back to the destination.
        switch (params.dst.colorType()) {
            default: SkUNREACHABLE;

            case kRGB_565_SkColorType:
                store16(dst_ptr, pack(pack(to_unorm(5,src.b),
                                           to_unorm(6,src.g), 5),
                                           to_unorm(5,src.r),11));
                break;

            case kBGRA_8888_SkColorType: std::swap(src.r, src.b);  [[fallthrough]];
            case  kRGB_888x_SkColorType:                           [[fallthrough]];
            case kRGBA_8888_SkColorType:
                 store32(dst_ptr, pack(pack(to_unorm(8, src.r),
                                            to_unorm(8, src.g), 8),
                                       pack(to_unorm(8, src.b),
                                            to_unorm(8, src.a), 8), 16));
                 break;

            case  kBGR_101010x_SkColorType:                          [[fallthrough]];
            case kBGRA_1010102_SkColorType: std::swap(src.r, src.b); [[fallthrough]];
            case  kRGB_101010x_SkColorType:                          [[fallthrough]];
            case kRGBA_1010102_SkColorType:
                 store32(dst_ptr, pack(pack(to_unorm(10, src.r),
                                            to_unorm(10, src.g), 10),
                                       pack(to_unorm(10, src.b),
                                            to_unorm( 2, src.a), 10), 20));
                 break;
        }
    }


    struct NoopColorFilter : public SkColorFilter {
        skvm::Color onProgram(skvm::Builder*, skvm::Color c,
                              SkColorSpace*, skvm::Uniforms*, SkArenaAlloc*) const override {
            return c;
        }

        bool onAppendStages(const SkStageRec&, bool) const override { return true; }

        // Only created here, should never be flattened / unflattened.
        Factory getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return "NoopColorFilter"; }
    };

    struct DitherShader : public SkShaderBase {
        explicit DitherShader(sk_sp<SkShader> shader) : fShader(std::move(shader)) {}

        sk_sp<SkShader> fShader;

        // Only created here temporarily... never serialized.
        Factory      getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return "DitherShader"; }

        bool isOpaque() const override { return fShader->isOpaque(); }

        skvm::Color onProgram(skvm::Builder* p, skvm::F32 x, skvm::F32 y, skvm::Color paint,
                              const SkMatrix& ctm, const SkMatrix* localM,
                              SkFilterQuality quality, const SkColorInfo& dst,
                              skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
            // Run our wrapped shader.
            skvm::Color c = as_SB(fShader)->program(p, x,y, paint,
                                                    ctm,localM, quality,dst, uniforms,alloc);
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
                case kBGRA_8888_SkColorType:    rate =  1/255.0f; break;
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
            skvm::I32 X =     trunc(x - 0.5f),
                      Y = X ^ trunc(y - 0.5f);

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
            skvm::F32 dither = to_f32(M) * scale + bias;
            c.r += dither;
            c.g += dither;
            c.b += dither;

            c.r = clamp(c.r, 0.0f, c.a);
            c.g = clamp(c.g, 0.0f, c.a);
            c.b = clamp(c.b, 0.0f, c.a);
            return c;
        }
    };

    static Params effective_params(const SkPixmap& device,
                                   const SkPaint& paint,
                                   const SkMatrix& ctm,
                                   sk_sp<SkShader> clip) {
        // Color filters have been handled for us by SkBlitter::Choose().
        SkASSERT(!paint.getColorFilter());

        // If there's no explicit shader, the paint color is the shader,
        // but if there is a shader, it's modulated by the paint alpha.
        sk_sp<SkShader> shader = paint.refShader();
        if (!shader) {
            shader = SkShaders::Color(paint.getColor4f(), nullptr);
        } else if (paint.getAlphaf() < 1.0f) {
            shader = sk_make_sp<SkColorFilterShader>(std::move(shader),
                                                     paint.getAlphaf(),
                                                     sk_make_sp<NoopColorFilter>());
        }

        // Add dither to the end of the shader pipeline if requested and needed.
        if (paint.isDither() && !as_SB(shader)->isConstant()) {
            shader = sk_make_sp<DitherShader>(std::move(shader));
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
        SkBlendMode blendMode = paint.getBlendMode();
        if (blendMode == SkBlendMode::kSrcOver && shader->isOpaque()) {
            blendMode =  SkBlendMode::kSrc;
        }

        return {
            std::move(shader),
            std::move(clip),
            { device.colorType(), device.alphaType(), device.refColorSpace() },
            blendMode,
            Coverage::Full,  // Placeholder... withCoverage() will change as needed.
            paint.getFilterQuality(),
            ctm,
        };
    }

    class Blitter final : public SkBlitter {
    public:
        Blitter(const SkPixmap& device,
                const SkPaint& paint,
                const SkMatrix& ctm,
                sk_sp<SkShader> clip,
                bool* ok)
            : fDevice(device)
            , fUniforms(kBlitterUniformsCount)
            , fParams(effective_params(device, paint, ctm, std::move(clip)))
            , fKey(cache_key(fParams, &fUniforms, &fAlloc, ok))
            , fPaint([&]{
                SkColor4f color = paint.getColor4f();
                SkColorSpaceXformSteps{sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                       device.colorSpace(), kUnpremul_SkAlphaType}
                    .apply(color.vec());
                return color;
            }()) {}

        ~Blitter() override {
            if (SkLRUCache<Key, skvm::Program>* cache = try_acquire_program_cache()) {
                auto cache_program = [&](skvm::Program&& program, Coverage coverage) {
                    if (!program.empty()) {
                        Key key = fKey.withCoverage(coverage);
                        if (skvm::Program* found = cache->find(key)) {
                            *found = std::move(program);
                        } else {
                            cache->insert(key, std::move(program));
                        }
                    }
                };
                cache_program(std::move(fBlitH),         Coverage::Full);
                cache_program(std::move(fBlitAntiH),     Coverage::UniformA8);
                cache_program(std::move(fBlitMaskA8),    Coverage::MaskA8);
                cache_program(std::move(fBlitMask3D),    Coverage::Mask3D);
                cache_program(std::move(fBlitMaskLCD16), Coverage::MaskLCD16);

                release_program_cache();
            }
        }

    private:
        SkPixmap        fDevice;
        skvm::Uniforms  fUniforms;                // Most data is copied directly into fUniforms,
        SkArenaAlloc    fAlloc{2*sizeof(void*)};  // but a few effects need to ref large content.
        const Params    fParams;
        const Key       fKey;
        const SkColor4f fPaint;
        skvm::Program   fBlitH,
                        fBlitAntiH,
                        fBlitMaskA8,
                        fBlitMask3D,
                        fBlitMaskLCD16;

        skvm::Program buildProgram(Coverage coverage) {
            Key key = fKey.withCoverage(coverage);
            {
                skvm::Program p;
                if (SkLRUCache<Key, skvm::Program>* cache = try_acquire_program_cache()) {
                    if (skvm::Program* found = cache->find(key)) {
                        p = std::move(*found);
                    }
                    release_program_cache();
                }
                if (!p.empty()) {
                    return p;
                }
            }
            // We don't really _need_ to rebuild fUniforms here.
            // It's just more natural to have effects unconditionally emit them,
            // and more natural to rebuild fUniforms than to emit them into a dummy buffer.
            // fUniforms should reuse the exact same memory, so this is very cheap.
            SkDEBUGCODE(size_t prev = fUniforms.buf.size();)
            fUniforms.buf.resize(kBlitterUniformsCount);
            skvm::Builder builder;
            build_program(&builder, fParams.withCoverage(coverage), &fUniforms, &fAlloc);
            SkASSERTF(fUniforms.buf.size() == prev,
                      "%zu, prev was %zu", fUniforms.buf.size(), prev);

            skvm::Program program = builder.done(debug_name(key).c_str());
            if (false) {
                static std::atomic<int> missed{0},
                                         total{0};
                if (!program.hasJIT()) {
                    SkDebugf("\ncouldn't JIT %s\n", debug_name(key).c_str());
                    builder.dump();
                    program.dump();

                    SkString path = SkStringPrintf("/tmp/%s.dot", debug_name(key).c_str());
                    SkFILEWStream tmp(path.c_str());
                    builder.dot(&tmp);

                    missed++;
                }
                if (0 == total++) {
                    atexit([]{ SkDebugf("SkVMBlitter compiled %d programs, %d without JIT.\n",
                                        total.load(), missed.load()); });
                }
            }
            return program;
        }

        void updateUniforms(int right, int y) {
            BlitterUniforms uniforms{right, y, fPaint};
            memcpy(fUniforms.buf.data(), &uniforms, sizeof(BlitterUniforms));
        }

        void blitH(int x, int y, int w) override {
            if (fBlitH.empty()) {
                fBlitH = this->buildProgram(Coverage::Full);
            }
            this->updateUniforms(x+w, y);
            fBlitH.eval(w, fUniforms.buf.data(), fDevice.addr(x,y));
        }

        void blitAntiH(int x, int y, const SkAlpha cov[], const int16_t runs[]) override {
            if (fBlitAntiH.empty()) {
                fBlitAntiH = this->buildProgram(Coverage::UniformA8);
            }
            for (int16_t run = *runs; run > 0; run = *runs) {
                this->updateUniforms(x+run, y);
                fBlitAntiH.eval(run, fUniforms.buf.data(), fDevice.addr(x,y), cov);

                x    += run;
                runs += run;
                cov  += run;
            }
        }

        void blitMask(const SkMask& mask, const SkIRect& clip) override {
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
                        program->eval(w, fUniforms.buf.data(), dptr, mptr + 1*plane
                                                                   , mptr + 2*plane
                                                                   , mptr + 0*plane);
                    } else {
                        program->eval(w, fUniforms.buf.data(), dptr, mptr);
                    }
                }
            }
        }
    };

}  // namespace

SkBlitter* SkCreateSkVMBlitter(const SkPixmap& device,
                               const SkPaint& paint,
                               const SkMatrix& ctm,
                               SkArenaAlloc* alloc,
                               sk_sp<SkShader> clip) {
    bool ok = true;
    auto blitter = alloc->make<Blitter>(device, paint, ctm, std::move(clip), &ok);
    return ok ? blitter : nullptr;
}
