/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMacros.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkVM.h"

namespace {

    enum class Coverage { Full, UniformA8, MaskA8, MaskLCD16, Mask3D };

    SK_BEGIN_REQUIRE_DENSE;
    struct Key {
        SkColorType    colorType;
        SkAlphaType    alphaType;
        Coverage       coverage;
        SkBlendMode    blendMode;
        SkShader*      shader;
        SkColorFilter* colorFilter;

        Key withCoverage(Coverage c) const {
            Key k = *this;
            k.coverage = c;
            return k;
        }
    };
    SK_END_REQUIRE_DENSE;

    static bool operator==(const Key& x, const Key& y) {
        return x.colorType   == y.colorType
            && x.alphaType   == y.alphaType
            && x.coverage    == y.coverage
            && x.blendMode   == y.blendMode
            && x.shader      == y.shader
            && x.colorFilter == y.colorFilter;
    }

    static SkLRUCache<Key, skvm::Program>* try_acquire_program_cache() {
        thread_local static auto* cache = new SkLRUCache<Key, skvm::Program>{8};
        return cache;
    }

    static void release_program_cache() { }


    struct Uniforms {
        uint32_t paint_color;
        uint8_t  coverage;   // Used when Coverage::UniformA8.
    };

    struct Builder : public skvm::Builder {
        //using namespace skvm;

        struct Color { skvm::I32 r,g,b,a; };


        skvm::I32 inv(skvm::I32 x) {
            return sub(splat(255), x);
        }

        // TODO: provide this in skvm::Builder, with a custom NEON impl.
        skvm::I32 div255(skvm::I32 v) {
            // This should be a bit-perfect version of (v+127)/255,
            // implemented as (v + ((v+128)>>8) + 128)>>8.
            skvm::I32 v128 = add(v, splat(128));
            return shr(add(v128, shr(v128, 8)), 8);
        }

        skvm::I32 mix(skvm::I32 x, skvm::I32 y, skvm::I32 t) {
            return div255(add(mul(x, inv(t)),
                              mul(y,     t )));
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
            skvm::I32 r = div255(mul(c.r, splat(31))),
                      g = div255(mul(c.g, splat(63))),
                      b = div255(mul(c.b, splat(31)));
            return pack(pack(b, g,5), r,11);
        }

        // TODO: add native min/max ops to skvm::Builder
        skvm::I32 min(skvm::I32 x, skvm::I32 y) { return select(lt(x,y), x,y); }
        skvm::I32 max(skvm::I32 x, skvm::I32 y) { return select(gt(x,y), x,y); }

        static bool CanBuild(const Key& key) {
            // These checks parallel the TODOs in Builder::Builder().
            if (key.shader)      { return false; }
            if (key.colorFilter) { return false; }

            switch (key.colorType) {
                default: return false;
                case kRGB_565_SkColorType:   break;
                case kRGBA_8888_SkColorType: break;
                case kBGRA_8888_SkColorType: break;
            }

            if (key.alphaType == kUnpremul_SkAlphaType) { return false; }

            switch (key.blendMode) {
                default: return false;
                case SkBlendMode::kSrc:     break;
                case SkBlendMode::kSrcOver: break;
            }

            return true;
        }

        explicit Builder(const Key& key) {
        #define TODO SkUNREACHABLE
            SkASSERT(CanBuild(key));
            skvm::Arg uniforms = uniform(),
                      dst_ptr  = arg(SkColorTypeBytesPerPixel(key.colorType));
            // When coverage is MaskA8 or MaskLCD16 there will be one more mask varying,
            // and when coverage is Mask3D there will be three more mask varyings.


            // When there's no shader and no color filter, the source color is the paint color.
            if (key.shader)      { TODO; }
            if (key.colorFilter) { TODO; }
            Color src = unpack_8888(uniform32(uniforms, offsetof(Uniforms, paint_color)));

            // Load up the destination color.
            Color dst;
            switch (key.colorType) {
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
            const bool force_opaque = true && key.alphaType == kOpaque_SkAlphaType;
            if (force_opaque) { dst.a = splat(0xff); }

            // We'd need to premul dst after loading and unpremul before storing.
            if (key.alphaType == kUnpremul_SkAlphaType) { TODO; }

            // Blend src and dst.
            switch (key.blendMode) {
                default: TODO;

                case SkBlendMode::kSrc: break;

                case SkBlendMode::kSrcOver: {
                    src.r = add(src.r, div255(mul(dst.r, inv(src.a))));
                    src.g = add(src.g, div255(mul(dst.g, inv(src.a))));
                    src.b = add(src.b, div255(mul(dst.b, inv(src.a))));
                    src.a = add(src.a, div255(mul(dst.a, inv(src.a))));
                } break;
            }

            // Lerp with coverage if needed.
            bool apply_coverage = true;
            Color cov;
            switch (key.coverage) {
                case Coverage::Full: apply_coverage = false;
                                     break;

                case Coverage::UniformA8: cov.r = cov.g = cov.b = cov.a =
                                          uniform8(uniforms, offsetof(Uniforms, coverage));
                                          break;

                case Coverage::MaskA8: cov.r = cov.g = cov.b = cov.a =
                                       load8(varying<uint8_t>());
                                       break;

                case Coverage::MaskLCD16:
                    cov = unpack_565(load16(varying<uint16_t>()));
                    cov.a = select(lt(src.a, dst.a), min(cov.r, min(cov.g,cov.b))
                                                   , max(cov.r, max(cov.g,cov.b)));
                    break;

                case Coverage::Mask3D: TODO;
            }
            if (apply_coverage) {
                src.r = mix(dst.r, src.r, cov.r);
                src.g = mix(dst.g, src.g, cov.g);
                src.b = mix(dst.b, src.b, cov.b);
                src.a = mix(dst.a, src.a, cov.a);
            }

            if (force_opaque) { src.a = splat(0xff); }

            // Store back to the destination.
            switch (key.colorType) {
                default: SkUNREACHABLE;

                case kRGB_565_SkColorType:   store16(dst_ptr, pack_565(src)); break;

                case kBGRA_8888_SkColorType: std::swap(src.r, src.b);  // fallthrough
                case kRGBA_8888_SkColorType: store32(dst_ptr, pack_8888(src)); break;
            }
        #undef TODO
        }
    };

    class Blitter final : public SkBlitter {
    public:
        bool ok = false;

        Blitter(const SkPixmap& device, const SkPaint& paint)
            : fDevice(device)
            , fKey {
                device.colorType(),
                device.alphaType(),
                Coverage::Full,
                paint.getBlendMode(),
                paint.getShader(),
                paint.getColorFilter(),
            }
        {
            SkColor4f color = paint.getColor4f();
            SkColorSpaceXformSteps{sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                   device.colorSpace(), kUnpremul_SkAlphaType}.apply(color.vec());

            if (color.fitsInBytes() && Builder::CanBuild(fKey)) {
                fUniforms.paint_color = color.premul().toBytes_RGBA();
                ok = true;
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
                cache_program(std::move(fBlitMaskLCD16), Coverage::MaskLCD16);

                release_program_cache();
            }
        }

    private:
        SkPixmap      fDevice;  // TODO: can this be const&?
        const Key     fKey;
        Uniforms      fUniforms;
        skvm::Program fBlitH,
                      fBlitAntiH,
                      fBlitMaskA8,
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
            return Builder{key}.done();
        }

        void blitH(int x, int y, int w) override {
            if (fBlitH.empty()) {
                fBlitH = this->buildProgram(Coverage::Full);
            }
            fBlitH.eval(w, &fUniforms, fDevice.addr(x,y));
        }

        void blitAntiH(int x, int y, const SkAlpha cov[], const int16_t runs[]) override {
            if (fBlitAntiH.empty()) {
                fBlitAntiH = this->buildProgram(Coverage::UniformA8);
            }
            for (int16_t run = *runs; run > 0; run = *runs) {
                fUniforms.coverage = *cov;
                fBlitAntiH.eval(run, &fUniforms, fDevice.addr(x,y));

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

                case SkMask::k3D_Format:    // TODO: the mul and add 3D mask planes too
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
                    program->eval(clip.width(),
                                  &fUniforms,
                                  fDevice.addr(clip.left(), y),
                                  mask.getAddr(clip.left(), y));
                }
            }
        }
    };

}  // namespace


SkBlitter* SkCreateSkVMBlitter(const SkPixmap& device,
                               const SkPaint& paint,
                               const SkMatrix& ctm,
                               SkArenaAlloc* alloc) {
    auto blitter = alloc->make<Blitter>(device, paint);
    return blitter->ok ? blitter
                       : nullptr;
}
