/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMacros.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkVM.h"

namespace {

    // Uniforms set by the Blitter itself,
    // rather than by the Shader, which follow this struct in the buffer.
    struct Uniforms {
        int right;  // First device x + blit run length n, used to get device x coordiate.
        int y;      // Device y coordiate.
    };
    static_assert(SkIsAlign4(sizeof(Uniforms)), "");

    enum class Coverage { Full, UniformA8, MaskA8, MaskLCD16, Mask3D };

    struct Params {
        sk_sp<SkColorSpace> colorSpace;
        sk_sp<SkShader>     shader;
        SkColorType         colorType;
        SkAlphaType         alphaType;
        SkBlendMode         blendMode;
        Coverage            coverage;

        Params withCoverage(Coverage c) const {
            Params p = *this;
            p.coverage = c;
            return p;
        }
    };

    SK_BEGIN_REQUIRE_DENSE;
    struct Key {
        uint64_t colorSpace;
        uint32_t shader;
        uint8_t  colorType,
                 alphaType,
                 blendMode,
                 coverage;

        bool operator==(const Key& that) const {
            return this->colorSpace == that.colorSpace
                && this->shader     == that.shader
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
        return SkStringPrintf("CT%d-AT%d-Cov%d-Blend%d-CS%llx-Shader%x",
                              key.colorType,
                              key.alphaType,
                              key.coverage,
                              key.blendMode,
                              key.colorSpace,
                              key.shader);
    }

    static bool debug_dump(const Key& key) {
    #if 0
        SkDebugf("%s\n", debug_name(key).c_str());
        return true;
    #else
        return false;
    #endif
    }

    static SkLRUCache<Key, skvm::Program>* try_acquire_program_cache() {
    #if 0 || defined(SK_BUILD_FOR_IOS)
        // iOS doesn't support thread_local on versions less than 9.0. pthread
        // based fallbacks must be used there. We could also use an SkSpinlock
        // and tryAcquire()/release(), or...
        return nullptr;  // ... we could just not cache programs on those platforms.
    #else
        thread_local static auto* cache = new SkLRUCache<Key, skvm::Program>{8};
        return cache;
    #endif
    }

    static void release_program_cache() { }


    struct Builder : public skvm::Builder {
        //using namespace skvm;

        struct Color { skvm::I32 r,g,b,a; };


        // TODO: provide this in skvm::Builder, with a custom NEON impl.
        skvm::I32 div255(skvm::I32 v) {
            // This should be a bit-perfect version of (v+127)/255,
            // implemented as (v + ((v+128)>>8) + 128)>>8.
            skvm::I32 v128 = add(v, splat(128));
            return shr(add(v128, shr(v128, 8)), 8);
        }

        skvm::I32 scale_unorm8(skvm::I32 x, skvm::I32 y) {
            return div255(mul(x,y));
        }

        skvm::I32 lerp_unorm8(skvm::I32 x, skvm::I32 y, skvm::I32 t) {
            return div255(add(mul(x, sub(splat(255), t)),
                              mul(y,                 t )));
        }

        Color unpack_8888(skvm::I32 rgba) {
            return {
                extract(rgba,  0, splat(0xff)),
                extract(rgba,  8, splat(0xff)),
                extract(rgba, 16, splat(0xff)),
                extract(rgba, 24, splat(0xff)),
            };
        }

        skvm::I32 pack_8888(Color c) {
            return pack(pack(c.r, c.g, 8),
                        pack(c.b, c.a, 8), 16);
        }

        Color unpack_565(skvm::I32 bgr) {
            // N.B. kRGB_565_SkColorType is named confusingly;
            //      blue is in the low bits and red the high.
            skvm::I32 r = extract(bgr, 11, splat(0b011'111)),
                      g = extract(bgr,  5, splat(0b111'111)),
                      b = extract(bgr,  0, splat(0b011'111));
            return {
                // Scale 565 up to 888.
                bit_or(shl(r, 3), shr(r, 2)),
                bit_or(shl(g, 2), shr(g, 4)),
                bit_or(shl(b, 3), shr(b, 2)),
                splat(0xff),
            };
        }

        skvm::I32 pack_565(Color c) {
            skvm::I32 r = scale_unorm8(c.r, splat(31)),
                      g = scale_unorm8(c.g, splat(63)),
                      b = scale_unorm8(c.b, splat(31));
            return pack(pack(b, g,5), r,11);
        }

        // TODO: add native min/max ops to skvm::Builder
        skvm::I32 min(skvm::I32 x, skvm::I32 y) { return select(lt(x,y), x,y); }
        skvm::I32 max(skvm::I32 x, skvm::I32 y) { return select(gt(x,y), x,y); }

        // If Builder can't build this program, CacheKey() sets *ok to false.
        static Key CacheKey(const Params& params, bool* ok) {
            SkASSERT(params.shader);
            uint32_t shaderHash = 0;
            {
                const SkShaderBase* shader = as_SB(params.shader);
                skvm::Builder p;
                skvm::Arg uniforms = skvm::Arg{0};
                skvm::F32 x = p.to_f32(p.sub(p.uniform32(uniforms, offsetof(Uniforms, right)),
                                             p.index())),
                          y = p.to_f32(p.uniform32(uniforms, offsetof(Uniforms, y)));
                skvm::I32 r,g,b,a;
                if (shader->program(&p,
                                    params.colorSpace.get(),
                                    uniforms, sizeof(Uniforms),
                                    x,y, &r,&g,&b,&a)) {
                    shaderHash = p.hash();
                } else {
                    *ok = false;
                }
            }

            switch (params.colorType) {
                default: *ok = false;        break;
                case kRGB_565_SkColorType:   break;
                case kRGBA_8888_SkColorType: break;
                case kBGRA_8888_SkColorType: break;
            }

            if (params.alphaType == kUnpremul_SkAlphaType) { *ok = false; }

            switch (params.blendMode) {
                default: *ok = false;       break;
                case SkBlendMode::kSrc:     break;
                case SkBlendMode::kSrcOver: break;
            }

            return {
                params.colorSpace ? params.colorSpace->hash() : 0,
                shaderHash,
                SkToU8(params.colorType),
                SkToU8(params.alphaType),
                SkToU8(params.blendMode),
                SkToU8(params.coverage),
            };
        }

        explicit Builder(const Params& params) {
        #define TODO SkUNREACHABLE
            SkDEBUGCODE(bool ok = true; (void)CacheKey(params, &ok); SkASSERT(ok);)
            skvm::Arg uniforms = uniform(),
                      dst_ptr  = arg(SkColorTypeBytesPerPixel(params.colorType));
            // If coverage is Mask3D there'll next come two varyings for mul and add planes,
            // and then finally if coverage is any Mask?? format, a varying for the mask.

            Color src;
            SkASSERT(params.shader);
            skvm::F32 x = to_f32(sub(uniform32(uniforms, offsetof(Uniforms, right)),
                                     index())),
                      y = to_f32(uniform32(uniforms, offsetof(Uniforms, y)));
            SkAssertResult(as_SB(params.shader)->program(this,
                                                         params.colorSpace.get(),
                                                         uniforms, sizeof(Uniforms),
                                                         x,y, &src.r, &src.g, &src.b, &src.a));

            if (params.coverage == Coverage::Mask3D) {
                skvm::I32 M = load8(varying<uint8_t>()),
                          A = load8(varying<uint8_t>());

                src.r = min(add(scale_unorm8(src.r, M), A), src.a);
                src.g = min(add(scale_unorm8(src.g, M), A), src.a);
                src.b = min(add(scale_unorm8(src.b, M), A), src.a);
            }

            // There are several orderings here of when we load dst and coverage
            // and how coverage is applied, and to complicate things, LCD coverage
            // needs to know dst.a.  We're careful to assert it's loaded in time.
            Color dst;
            SkDEBUGCODE(bool dst_loaded = false;)

            // load_coverage() returns false when there's no need to apply coverage.
            auto load_coverage = [&](Color* cov) {
                switch (params.coverage) {
                    case Coverage::Full: return false;

                    case Coverage::UniformA8: cov->r = cov->g = cov->b = cov->a =
                                              uniform8(uniform());
                                              return true;

                    case Coverage::Mask3D:
                    case Coverage::MaskA8: cov->r = cov->g = cov->b = cov->a =
                                           load8(varying<uint8_t>());
                                           return true;

                    case Coverage::MaskLCD16:
                        SkASSERT(dst_loaded);
                        *cov = unpack_565(load16(varying<uint16_t>()));
                        cov->a = select(lt(src.a, dst.a), min(cov->r, min(cov->g,cov->b))
                                                        , max(cov->r, max(cov->g,cov->b)));
                        return true;
                }
                // GCC insists...
                return false;
            };

            // The math for some blend modes lets us fold coverage into src before the blend,
            // obviating the need for the lerp afterwards. This early-coverage strategy tends
            // to be both faster and require fewer registers.
            bool lerp_coverage_post_blend = true;
            if (SkBlendMode_ShouldPreScaleCoverage(params.blendMode,
                                                   params.coverage == Coverage::MaskLCD16)) {
                Color cov;
                if (load_coverage(&cov)) {
                    src.r = scale_unorm8(src.r, cov.r);
                    src.g = scale_unorm8(src.g, cov.g);
                    src.b = scale_unorm8(src.b, cov.b);
                    src.a = scale_unorm8(src.a, cov.a);
                }
                lerp_coverage_post_blend = false;
            }

            // Load up the destination color.
            SkDEBUGCODE(dst_loaded = true;)
            switch (params.colorType) {
                default: TODO;
                case kRGB_565_SkColorType:   dst = unpack_565 (load16(dst_ptr)); break;
                case kRGBA_8888_SkColorType: dst = unpack_8888(load32(dst_ptr)); break;
                case kBGRA_8888_SkColorType: dst = unpack_8888(load32(dst_ptr));
                                             std::swap(dst.r, dst.b);
                                             break;
            }

            // When a destination is tagged opaque, we may assume it both starts and stays fully
            // opaque, ignoring any math that disagrees.  So anything involving force_opaque is
            // optional, and sometimes helps cut a small amount of work in these programs.
            const bool force_opaque = true && params.alphaType == kOpaque_SkAlphaType;
            if (force_opaque) { dst.a = splat(0xff); }

            // We'd need to premul dst after loading and unpremul before storing.
            if (params.alphaType == kUnpremul_SkAlphaType) { TODO; }

            // Blend src and dst.
            switch (params.blendMode) {
                default: TODO;

                case SkBlendMode::kSrc: break;

                case SkBlendMode::kSrcOver: {
                    auto invA = sub(splat(255), src.a);
                    src.r = add(src.r, scale_unorm8(dst.r, invA));
                    src.g = add(src.g, scale_unorm8(dst.g, invA));
                    src.b = add(src.b, scale_unorm8(dst.b, invA));
                    src.a = add(src.a, scale_unorm8(dst.a, invA));
                } break;
            }

            // Lerp with coverage post-blend if needed.
            Color cov;
            if (lerp_coverage_post_blend && load_coverage(&cov)) {
                src.r = lerp_unorm8(dst.r, src.r, cov.r);
                src.g = lerp_unorm8(dst.g, src.g, cov.g);
                src.b = lerp_unorm8(dst.b, src.b, cov.b);
                src.a = lerp_unorm8(dst.a, src.a, cov.a);
            }

            if (force_opaque) { src.a = splat(0xff); }

            // Store back to the destination.
            switch (params.colorType) {
                default: SkUNREACHABLE;

                case kRGB_565_SkColorType:   store16(dst_ptr, pack_565(src)); break;

                case kBGRA_8888_SkColorType: std::swap(src.r, src.b);  // fallthrough
                case kRGBA_8888_SkColorType: store32(dst_ptr, pack_8888(src)); break;
            }
        #undef TODO
        }
    };

    // Scale the output of another shader by an 8-bit alpha.
    struct AlphaShader : public SkShaderBase {
        AlphaShader(sk_sp<SkShader> shader, uint8_t alpha)
            : fShader(std::move(shader))
            , fAlpha(alpha) {}

        sk_sp<SkShader> fShader;
        uint32_t        fAlpha;  // [0,255], 4 bytes to keep nice alignment in uniform buffer.

        bool onProgram(skvm::Builder* p,
                       SkColorSpace* dstCS,
                       skvm::Arg uniforms, size_t offset,
                       skvm::F32 x, skvm::F32 y,
                       skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) const override {
            if (as_SB(fShader)->program(p, dstCS,
                                        uniforms, offset + sizeof(fAlpha),
                                        x,y, r,g,b,a)) {
                // TODO: move the helpers onto skvm::Builder so I don't have to duplicate?
                auto div255 = [&](skvm::I32 v) {
                    skvm::I32 v128 = p->add(v, p->splat(128));
                    return p->shr(p->add(v128, p->shr(v128, 8)), 8);
                };
                auto scale_unorm8 = [&](skvm::I32 x, skvm::I32 y) {
                    return div255(p->mul(x,y));
                };

                skvm::I32 A = p->uniform32(uniforms, offset);
                *r = scale_unorm8(*r, A);
                *g = scale_unorm8(*g, A);
                *b = scale_unorm8(*b, A);
                *a = scale_unorm8(*a, A);
                return true;
            }
            return false;
        }

        void uniforms(SkColorSpace* dstCS, std::vector<uint32_t>* buf) const override {
            buf->push_back(fAlpha);
            as_SB(fShader)->uniforms(dstCS, buf);
        }

        // Only created here, should never be flattened / unflattened.
        Factory getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return "AlphaShader"; }
    };

    static Params effective_params(const SkPixmap& device, const SkPaint& paint) {
        // Color filters have been handled for us by SkBlitter::Choose().
        SkASSERT(!paint.getColorFilter());

        // If there's no explicit shader, the paint color is the shader,
        // but if there is a shader, it's modulated by the paint alpha.
        sk_sp<SkShader> shader = paint.refShader();
        if (!shader) {
            shader = SkShaders::Color(paint.getColor4f(), nullptr);
        } else if (paint.getAlpha() < 0xff) {
            shader = sk_make_sp<AlphaShader>(std::move(shader), paint.getAlpha());
        }

        // The most common blend mode is SrcOver, and it can be strength-reduced
        // _greatly_ to Src mode when the shader is opaque.
        SkBlendMode blendMode = paint.getBlendMode();
        if (blendMode == SkBlendMode::kSrcOver && shader->isOpaque()) {
            blendMode =  SkBlendMode::kSrc;
        }

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

        return {
            device.refColorSpace(),
            std::move(shader),
            device.colorType(),
            device.alphaType(),
            blendMode,
            Coverage::Full,  // Placeholder... withCoverage() will change as needed.
        };
    }

    class Blitter final : public SkBlitter {
    public:
        Blitter(const SkPixmap& device, const SkPaint& paint, bool* ok)
            : fDevice(device)
            , fParams(effective_params(device, paint))
            , fKey(Builder::CacheKey(fParams, ok))
            , fUniforms(sizeof(Uniforms) / sizeof(fUniforms[0]))
        {
            if (*ok) {
                as_SB(fParams.shader)->uniforms(fParams.colorSpace.get(), &fUniforms);
            }
        }

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
        SkPixmap              fDevice;  // TODO: can this be const&?
        const Params          fParams;
        const Key             fKey;
        std::vector<uint32_t> fUniforms;
        skvm::Program fBlitH,
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
        #if 0
            static std::atomic<int> done{0};
            if (0 == done++) {
                atexit([]{ SkDebugf("%d calls to done\n", done.load()); });
            }
        #endif
            Builder builder{fParams.withCoverage(coverage)};
            skvm::Program program = builder.done(debug_name(key).c_str());
            if (!program.hasJIT() && debug_dump(key)) {
                SkDebugf("\nfalling back to interpreter for blitter with this key.\n");
                builder.dump();
                program.dump();
            }
            return program;
        }

        void updateUniforms(int right, int y) {
            Uniforms uniforms{right, y};
            memcpy(fUniforms.data(), &uniforms, sizeof(Uniforms));
        }

        void blitH(int x, int y, int w) override {
            if (fBlitH.empty()) {
                fBlitH = this->buildProgram(Coverage::Full);
            }
            this->updateUniforms(x+w, y);
            fBlitH.eval(w, fUniforms.data(), fDevice.addr(x,y));
        }

        void blitAntiH(int x, int y, const SkAlpha cov[], const int16_t runs[]) override {
            if (fBlitAntiH.empty()) {
                fBlitAntiH = this->buildProgram(Coverage::UniformA8);
            }
            for (int16_t run = *runs; run > 0; run = *runs) {
                this->updateUniforms(x+run, y);
                fBlitAntiH.eval(run, fUniforms.data(), fDevice.addr(x,y), cov);

                x    += run;
                runs += run;
                cov  += run;
            }
        }

        void blitMask(const SkMask& mask, const SkIRect& clip) override {
            if (mask.fFormat == SkMask::kBW_Format) {
                // TODO: native BW masks?
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
                        program->eval(w, fUniforms.data(), dptr, mptr + 1*plane
                                                               , mptr + 2*plane
                                                               , mptr + 0*plane);
                    } else {
                        program->eval(w, fUniforms.data(), dptr, mptr);
                    }
                }
            }
        }
    };

}  // namespace


SkBlitter* SkCreateSkVMBlitter(const SkPixmap& device,
                               const SkPaint& paint,
                               const SkMatrix& ctm,
                               SkArenaAlloc* alloc) {
    bool ok = true;
    auto blitter = alloc->make<Blitter>(device, paint, &ok);
    return ok ? blitter : nullptr;
}
