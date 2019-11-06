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
#include "src/core/SkVMBlitter.h"

namespace {

    // Uniforms set by the Blitter itself,
    // rather than by the Shader, which follow this struct in the skvm::Uniforms buffer.
    struct BlitterUniforms {
        int right;  // First device x + blit run length n, used to get device x coordiate.
        int y;      // Device y coordiate.
    };
    static_assert(SkIsAlign4(sizeof(BlitterUniforms)), "");
    static constexpr int kBlitterUniformsCount = sizeof(BlitterUniforms) / 4;

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
        skvm::Color unpack_8888(skvm::I32 rgba) {
            return {
                extract(rgba,  0, splat(0xff)),
                extract(rgba,  8, splat(0xff)),
                extract(rgba, 16, splat(0xff)),
                extract(rgba, 24, splat(0xff)),
            };
        }

        skvm::I32 pack_8888(skvm::Color c) {
            return pack(pack(c.r, c.g, 8),
                        pack(c.b, c.a, 8), 16);
        }

        skvm::Color unpack_565(skvm::I32 bgr) {
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

        skvm::I32 pack_565(skvm::Color c) {
            skvm::I32 r = scale_unorm8(c.r, splat(31)),
                      g = scale_unorm8(c.g, splat(63)),
                      b = scale_unorm8(c.b, splat(31));
            return pack(pack(b, g,5), r,11);
        }

        // If Builder can't build this program, CacheKey() sets *ok to false.
        static Key CacheKey(const Params& params, skvm::Uniforms* uniforms, bool* ok) {
            SkASSERT(params.shader);
            uint32_t shaderHash = 0;
            {
                const SkShaderBase* shader = as_SB(params.shader);
                skvm::Builder p;
                skvm::F32 x = p.to_f32(p.sub(p.uniform32(uniforms->ptr,
                                                         offsetof(BlitterUniforms, right)),
                                             p.index())),
                          y = p.to_f32(p.uniform32(uniforms->ptr,
                                                   offsetof(BlitterUniforms, y)));
                skvm::I32 r,g,b,a;
                if (shader->program(&p,
                                    params.colorSpace.get(),
                                    uniforms,
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

            if (!skvm::BlendModeSupported(params.blendMode)) {
                *ok = false;
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

        Builder(const Params& params, skvm::Uniforms* uniforms) {
        #define TODO SkUNREACHABLE
            // First two arguments are always uniforms and the destination buffer.
            uniforms->ptr     = uniform();
            skvm::Arg dst_ptr = arg(SkColorTypeBytesPerPixel(params.colorType));
            // Other arguments depend on params.coverage:
            //    - Full:      (no more arguments)
            //    - Mask3D:    mul varying, add varying, 8-bit coverage varying
            //    - MaskA8:    8-bit coverage varying
            //    - MaskLCD16: 565 coverage varying
            //    - UniformA8: 8-bit coverage uniform

            skvm::Color src;
            SkASSERT(params.shader);
            skvm::F32 x = to_f32(sub(uniform32(uniforms->ptr,
                                               offsetof(BlitterUniforms, right)),
                                     index())),
                      y = to_f32(uniform32(uniforms->ptr,
                                           offsetof(BlitterUniforms, y)));
            SkAssertResult(as_SB(params.shader)->program(this,
                                                         params.colorSpace.get(),
                                                         uniforms,
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
            skvm::Color dst;
            SkDEBUGCODE(bool dst_loaded = false;)

            // load_coverage() returns false when there's no need to apply coverage.
            auto load_coverage = [&](skvm::Color* cov) {
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
                skvm::Color cov;
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

            src = skvm::BlendModeProgram(this, params.blendMode, src, dst);

            // Lerp with coverage post-blend if needed.
            skvm::Color cov;
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
        uint8_t         fAlpha;

        bool onProgram(skvm::Builder* p,
                       SkColorSpace* dstCS,
                       skvm::Uniforms* uniforms,
                       skvm::F32 x, skvm::F32 y,
                       skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) const override {
            if (as_SB(fShader)->program(p, dstCS,
                                        uniforms,
                                        x,y, r,g,b,a)) {
                skvm::I32 A = p->uniform32(uniforms->push(fAlpha));
                *r = p->scale_unorm8(*r, A);
                *g = p->scale_unorm8(*g, A);
                *b = p->scale_unorm8(*b, A);
                *a = p->scale_unorm8(*a, A);
                return true;
            }
            return false;
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
            , fUniforms(kBlitterUniformsCount)
            , fParams(effective_params(device, paint))
            , fKey(Builder::CacheKey(fParams, &fUniforms, ok))
        {}

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
        SkPixmap       fDevice;  // TODO: can this be const&?
        skvm::Uniforms fUniforms;
        const Params   fParams;
        const Key      fKey;
        skvm::Program  fBlitH,
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
            Builder builder{fParams.withCoverage(coverage), &fUniforms};
            SkASSERT(fUniforms.buf.size() == prev);

            skvm::Program program = builder.done(debug_name(key).c_str());
            if (debug_dump(key)) {
                static std::atomic<int> done{0};
                if (0 == done++) {
                    atexit([]{ SkDebugf("%d calls to done\n", done.load()); });
                }

                if (!program.hasJIT()) {
                    SkDebugf("\nfalling back to interpreter for blitter with this key.\n");
                    builder.dump();
                    program.dump();
                }
            }
            return program;
        }

        void updateUniforms(int right, int y) {
            BlitterUniforms uniforms{right, y};
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

bool skvm::BlendModeSupported(SkBlendMode mode) {
    switch (mode) {
        default: break;
        case SkBlendMode::kSrc:
        case SkBlendMode::kSrcOver: return true;
    }
    return false;
}

skvm::Color skvm::BlendModeProgram(skvm::Builder* p, SkBlendMode mode, const skvm::Color& src,
                                   const skvm::Color& dst) {
    skvm::Color res;
    switch (mode) {
        default: SkASSERT(false);

        case SkBlendMode::kSrc: res = src; break;

        case SkBlendMode::kSrcOver: {
            auto invA = p->sub(p->splat(255), src.a);
            res.r = p->add(src.r, p->scale_unorm8(dst.r, invA));
            res.g = p->add(src.g, p->scale_unorm8(dst.g, invA));
            res.b = p->add(src.b, p->scale_unorm8(dst.b, invA));
            res.a = p->add(src.a, p->scale_unorm8(dst.a, invA));
        } break;
    }
    return res;
}

SkBlitter* SkCreateSkVMBlitter(const SkPixmap& device,
                               const SkPaint& paint,
                               const SkMatrix& ctm,
                               SkArenaAlloc* alloc) {
    bool ok = true;
    auto blitter = alloc->make<Blitter>(device, paint, &ok);
    return ok ? blitter : nullptr;
}
