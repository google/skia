/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkVM.h"

namespace {

    struct Color { skvm::I32 r,g,b,a; };

    class Blitter final : public SkBlitter {
    public:
        Blitter(const SkPixmap& device, const SkPaint& paint) : fDevice(device) {
            // Just for fun we'll pass the paint color as a uniform rather than bake it in.
            // This would help caching if we were to add it.
            fUniforms.paint_color = paint.getColor4f().premul().toBytes_RGBA();

            fOK = Build(device, paint,  true, &fBlitH)
               && Build(device, paint, false, &fBlitAntiH);
        }

        bool ok() const { return fOK; }

    private:
        static bool Build(const SkPixmap&,
                          const SkPaint&,
                          bool full_coverage,
                          skvm::Program*);

        bool fOK = false;
        SkPixmap fDevice;

        struct Uniforms {
            uint32_t paint_color;
            uint8_t  coverage;
        } fUniforms;

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

    bool Blitter::Build(const SkPixmap& device,
                        const SkPaint& paint,
                        bool full_coverage,
                        skvm::Program* program) {
        skvm::Builder _;

        skvm::Arg uniforms = _.uniform(),
                  dst_ptr  = _.arg(SkColorTypeBytesPerPixel(device.colorType()));

        skvm::I32 paint_color = _.uniform32(uniforms, offsetof(Uniforms, paint_color)),
                  coverage    = _.uniform8 (uniforms, offsetof(Uniforms, coverage   ));

        // When there's no shader and no color filter, the source color is the paint color.
        if (paint.getShader())      { return false; }
        if (paint.getColorFilter()) { return false; }
        Color src = {
            _.extract(paint_color,  0, _.splat(0xff)),
            _.extract(paint_color,  8, _.splat(0xff)),
            _.extract(paint_color, 16, _.splat(0xff)),
            _.extract(paint_color, 24, _.splat(0xff)),
        };

        // Next step would be to convert the source color to destination color space.
        if (SkColorSpaceXformSteps::Required(nullptr/*sRGB*/, device.colorSpace())) {
            return false;
        }

        if (device.alphaType() == kUnpremul_SkAlphaType) {
            return false;
        }

        Color dst;
        switch (device.colorType()) {
            default: return false;

            case kRGBA_8888_SkColorType: {
                skvm::I32 rgba = _.load32(dst_ptr);
                dst.r = _.extract(rgba,  0, _.splat(0xff));
                dst.g = _.extract(rgba,  8, _.splat(0xff));
                dst.b = _.extract(rgba, 16, _.splat(0xff));
                dst.a = _.extract(rgba, 24, _.splat(0xff));
            } break;

            case kBGRA_8888_SkColorType: {
                skvm::I32 bgra = _.load32(dst_ptr);
                dst.r = _.extract(bgra, 16, _.splat(0xff));
                dst.g = _.extract(bgra,  8, _.splat(0xff));
                dst.b = _.extract(bgra,  0, _.splat(0xff));
                dst.a = _.extract(bgra, 24, _.splat(0xff));
            } break;

            case kRGB_565_SkColorType: {
                // N.B. kRGB_565_SkColorType is named confusingly;
                //      blue is in the low bits and red the high.
                skvm::I32 bgr = _.load16(dst_ptr),
                          r   = _.extract(bgr, 11, _.splat(0b011'111)),
                          g   = _.extract(bgr,  5, _.splat(0b111'111)),
                          b   = _.extract(bgr,  0, _.splat(0b011'111));
                // Scale 565 up to 888.
                dst.r = _.bit_or(_.shl(r, 3), _.shr(r, 2));
                dst.g = _.bit_or(_.shl(g, 2), _.shr(g, 4));
                dst.b = _.bit_or(_.shl(b, 3), _.shr(b, 2));
                dst.a = _.splat(0xff);
            } break;
        }

        auto inv = [&](skvm::I32 x) {
            return _.sub(_.splat(255), x);
        };

        auto scale = [&](skvm::I32 x, skvm::I32 y) {
            // (xy + x)/256 is our standard approximation of (xy + 127)/255.
            return _.shr(_.add(_.mul(x, y), x), 8);
        };

        auto mix = [&](skvm::I32 x, skvm::I32 y, skvm::I32 t) {
            return _.add(scale(x, inv(t)),
                         scale(y,     t ));
        };

        switch (paint.getBlendMode()) {
            default: return false;

            case SkBlendMode::kSrc: break;

            case SkBlendMode::kSrcOver: {
                src.r = _.add(src.r, scale(dst.r, inv(src.a)));
                src.g = _.add(src.g, scale(dst.g, inv(src.a)));
                src.b = _.add(src.b, scale(dst.b, inv(src.a)));
                src.a = _.add(src.a, scale(dst.a, inv(src.a)));
            } break;
        }

        if (!full_coverage) {
            src.r = mix(dst.r, src.r, coverage);
            src.g = mix(dst.g, src.g, coverage);
            src.b = mix(dst.b, src.b, coverage);
            src.a = mix(dst.a, src.a, coverage);
        }

        switch (device.colorType()) {
            default:
                SkUNREACHABLE;
                return false;

            case kRGBA_8888_SkColorType: {
                _.store32(dst_ptr, _.pack(_.pack(src.r, src.g, 8),
                                          _.pack(src.b, src.a, 8), 16));
            } break;

            case kBGRA_8888_SkColorType: {
                _.store32(dst_ptr, _.pack(_.pack(src.b, src.g, 8),
                                          _.pack(src.r, src.a, 8), 16));
            } break;

            case kRGB_565_SkColorType: {
                // TODO: some sort of rounding?
                skvm::I32 r = _.shr(src.r, 3),
                          g = _.shr(src.g, 2),
                          b = _.shr(src.b, 3);
                _.store16(dst_ptr, _.pack(_.pack(b, g,5), r,11));
            } break;
        }

        *program = _.done();
        return true;
    }

}  // namespace


SkBlitter* SkCreateSkVMBlitter(const SkPixmap& device,
                               const SkPaint& paint,
                               const SkMatrix& ctm,
                               SkArenaAlloc* alloc) {
    auto blitter = alloc->make<Blitter>(device, paint);
    return blitter->ok() ? blitter
                         : nullptr;
}
