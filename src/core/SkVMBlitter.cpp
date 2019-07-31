/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkVM.h"

namespace {

    struct Uniforms {
        uint32_t paint_color;
        uint8_t  coverage;
    };

    struct Color { skvm::I32 r,g,b,a; };

    enum class Coverage {
        Full,
        UniformA8,
        MaskA8,
        MaskLCD16,
        Mask3D,
    };

    struct Builder : public skvm::Builder {
        bool ok = false;

        Builder(const SkPixmap& device, const SkPaint& paint, Coverage coverage) {
            skvm::Arg uniforms = uniform(),
                      dst_ptr  = arg(SkColorTypeBytesPerPixel(device.colorType()));

            // When there's no shader and no color filter, the source color is the paint color.
            if (paint.getShader())      { return; }
            if (paint.getColorFilter()) { return; }
            skvm::I32 paint_color = uniform32(uniforms, offsetof(Uniforms, paint_color));
            Color src = {
                extract(paint_color,  0, splat(0xff)),
                extract(paint_color,  8, splat(0xff)),
                extract(paint_color, 16, splat(0xff)),
                extract(paint_color, 24, splat(0xff)),
            };

            // Load up the destination color.
            Color dst;
            switch (device.colorType()) {
                default: return;

                case kRGBA_8888_SkColorType: {
                    skvm::I32 rgba = load32(dst_ptr);
                    dst.r = extract(rgba,  0, splat(0xff));
                    dst.g = extract(rgba,  8, splat(0xff));
                    dst.b = extract(rgba, 16, splat(0xff));
                    dst.a = extract(rgba, 24, splat(0xff));
                } break;

                case kBGRA_8888_SkColorType: {
                    skvm::I32 bgra = load32(dst_ptr);
                    dst.r = extract(bgra, 16, splat(0xff));
                    dst.g = extract(bgra,  8, splat(0xff));
                    dst.b = extract(bgra,  0, splat(0xff));
                    dst.a = extract(bgra, 24, splat(0xff));
                } break;

                case kRGB_565_SkColorType: {
                    // N.B. kRGB_565_SkColorType is named confusingly;
                    //      blue is in the low bits and red the high.
                    skvm::I32 bgr = load16(dst_ptr),
                              r   = extract(bgr, 11, splat(0b011'111)),
                              g   = extract(bgr,  5, splat(0b111'111)),
                              b   = extract(bgr,  0, splat(0b011'111));
                    // Scale 565 up to 888.
                    dst.r = bit_or(shl(r, 3), shr(r, 2));
                    dst.g = bit_or(shl(g, 2), shr(g, 4));
                    dst.b = bit_or(shl(b, 3), shr(b, 2));
                    dst.a = splat(0xff);
                } break;
            }

            // We'd need to premul dst after loading and unpremul before storing.
            if (device.alphaType() == kUnpremul_SkAlphaType) { return; }


            auto inv = [&](skvm::I32 x) {
                return sub(splat(255), x);
            };

            auto scale = [&](skvm::I32 x, skvm::I32 y) {
                // (xy + x)/256 is our standard approximation of (xy + 127)/255.
                return shr(add(mul(x, y), x), 8);
            };

            auto mix = [&](skvm::I32 x, skvm::I32 y, skvm::I32 t) {
                return add(scale(x, inv(t)),
                           scale(y,     t ));
            };

            // Blend src and dst.
            switch (paint.getBlendMode()) {
                default: return;

                case SkBlendMode::kSrc: break;

                case SkBlendMode::kSrcOver: {
                    src.r = add(src.r, scale(dst.r, inv(src.a)));
                    src.g = add(src.g, scale(dst.g, inv(src.a)));
                    src.b = add(src.b, scale(dst.b, inv(src.a)));
                    src.a = add(src.a, scale(dst.a, inv(src.a)));
                } break;
            }

            // Lerp with coverage if needed.
            switch (coverage) {
                case Coverage::Full: break;

                case Coverage::UniformA8: {
                    skvm::I32 c = uniform8(uniforms, offsetof(Uniforms, coverage));
                    src.r = mix(dst.r, src.r, c);
                    src.g = mix(dst.g, src.g, c);
                    src.b = mix(dst.b, src.b, c);
                    src.a = mix(dst.a, src.a, c);
                } break;

                case Coverage::MaskA8:
                case Coverage::MaskLCD16:
                case Coverage::Mask3D:    return;   // TODO
            }

            // Store back to the destination.
            switch (device.colorType()) {
                default:
                    SkUNREACHABLE;
                    return;

                case kRGBA_8888_SkColorType: {
                    store32(dst_ptr, pack(pack(src.r, src.g, 8),
                                          pack(src.b, src.a, 8), 16));
                } break;

                case kBGRA_8888_SkColorType: {
                    store32(dst_ptr, pack(pack(src.b, src.g, 8),
                                          pack(src.r, src.a, 8), 16));
                } break;

                case kRGB_565_SkColorType: {
                    // TODO: some sort of rounding?
                    skvm::I32 r = shr(src.r, 3),
                              g = shr(src.g, 2),
                              b = shr(src.b, 3);
                    store16(dst_ptr, pack(pack(b, g,5), r,11));
                } break;
            }

            // Hooray!
            ok = true;
        }
    };

    class Blitter final : public SkBlitter {
    public:
        bool ok = false;

        Blitter(const SkPixmap& device, const SkPaint& paint) : fDevice(device) {
            SkColor4f color = paint.getColor4f();
            SkColorSpaceXformSteps{sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                   device.colorSpace(), kUnpremul_SkAlphaType}.apply(color.vec());
            if (!color.fitsInBytes()) {
                // TODO: Wide colors, and really, any further color space support.
                return;
            }

            // We'll pass the paint color as a uniform rather than bake it in.
            // This would help caching if we were to add it.
            fUniforms.paint_color = color.premul().toBytes_RGBA();

            auto build = [&](Coverage coverage, skvm::Program* program) {
                Builder builder{device, paint, coverage};
                if (builder.ok) {
                    *program = builder.done();
                    return true;
                }
                return false;
            };

            // TODO: check conditions once, then build these lazily?
            ok = build(Coverage::Full,      &fBlitH)
              && build(Coverage::UniformA8, &fBlitAntiH);
            // TODO: masks
        }

    private:
        SkPixmap      fDevice;
        Uniforms      fUniforms;
        skvm::Program fBlitH,
                      fBlitAntiH;

        void blitH(int x, int y, int w) override {
            fBlitH.eval(w, &fUniforms, fDevice.addr(x,y));
        }

        void blitAntiH(int x, int y, const SkAlpha cov[], const int16_t runs[]) override {
            for (int16_t run = *runs; run > 0; run = *runs) {
                fUniforms.coverage = *cov;
                fBlitAntiH.eval(run, &fUniforms, fDevice.addr(x,y));

                x    += run;
                runs += run;
                cov  += run;
            }
        }
        // TODO: blitMask() is also a mandatory override, even though it's not virtual=0.
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
