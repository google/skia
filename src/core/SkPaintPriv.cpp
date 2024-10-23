/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPaintPriv.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSafeRange.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/shaders/SkColorFilterShader.h"
#include "src/shaders/SkShaderBase.h"

#include <cstdint>
#include <optional>

class SkColorSpace;

static bool changes_alpha(const SkPaint& paint) {
    SkColorFilter* cf = paint.getColorFilter();
    return cf && !as_CFB(cf)->isAlphaUnchanged();
}

enum SrcColorOpacity {
    // The src color is known to be opaque (alpha == 255)
    kOpaque_SrcColorOpacity = 0,
    // The src color is known to be fully transparent (color == 0)
    kTransparentBlack_SrcColorOpacity = 1,
    // The src alpha is known to be fully transparent (alpha == 0)
    kTransparentAlpha_SrcColorOpacity = 2,
    // The src color opacity is unknown
    kUnknown_SrcColorOpacity = 3
};

static bool blend_mode_is_opaque(SkBlendMode mode, SrcColorOpacity opacityType) {
    SkBlendModeCoeff src, dst;
    if (!SkBlendMode_AsCoeff(mode, &src, &dst)) {
        return false;
    }

    switch (src) {
        case SkBlendModeCoeff::kDA:
        case SkBlendModeCoeff::kDC:
        case SkBlendModeCoeff::kIDA:
        case SkBlendModeCoeff::kIDC:
            return false;
        default:
            break;
    }

    switch (dst) {
        case SkBlendModeCoeff::kZero:
            return true;
        case SkBlendModeCoeff::kISA:
            return kOpaque_SrcColorOpacity == opacityType;
        case SkBlendModeCoeff::kSA:
            return kTransparentBlack_SrcColorOpacity == opacityType ||
                   kTransparentAlpha_SrcColorOpacity == opacityType;
        case SkBlendModeCoeff::kSC:
            return kTransparentBlack_SrcColorOpacity == opacityType;
        default:
            return false;
    }
}

bool SkPaintPriv::Overwrites(const SkPaint* paint, ShaderOverrideOpacity overrideOpacity) {
    if (!paint) {
        // No paint means we default to SRC_OVER, so we overwrite iff our shader-override
        // is opaque, or we don't have one.
        return overrideOpacity != kNotOpaque_ShaderOverrideOpacity;
    }

    SrcColorOpacity opacityType = kUnknown_SrcColorOpacity;

    if (!changes_alpha(*paint)) {
        const unsigned paintAlpha = paint->getAlpha();
        if (0xff == paintAlpha && overrideOpacity != kNotOpaque_ShaderOverrideOpacity &&
            (!paint->getShader() || paint->getShader()->isOpaque())) {
            opacityType = kOpaque_SrcColorOpacity;
        } else if (0 == paintAlpha) {
            if (overrideOpacity == kNone_ShaderOverrideOpacity && !paint->getShader()) {
                opacityType = kTransparentBlack_SrcColorOpacity;
            } else {
                opacityType = kTransparentAlpha_SrcColorOpacity;
            }
        }
    }

    const auto bm = paint->asBlendMode();
    if (!bm) {
        return false;   // don't know for sure, so we play it safe and return false.
    }
    return blend_mode_is_opaque(bm.value(), opacityType);
}

bool SkPaintPriv::ShouldDither(const SkPaint& p, SkColorType dstCT) {
    // The paint dither flag can veto.
    if (!p.isDither()) {
        return false;
    }

    if (dstCT == kUnknown_SkColorType) {
        return false;
    }

    // We always dither 565 or 4444 when requested.
    if (dstCT == kRGB_565_SkColorType || dstCT == kARGB_4444_SkColorType) {
        return true;
    }

    // Otherwise, dither is only needed for non-const paints.
    return p.getImageFilter() || p.getMaskFilter() ||
           (p.getShader() && !as_SB(p.getShader())->isConstant());
}

// return true if the paint is just a single color (i.e. not a shader). If its
// a shader, then we can't compute a const luminance for it :(
static bool just_a_color(const SkPaint& paint, SkColor4f* color) {
    SkColor4f c = paint.getColor4f();

    const auto* shader = as_SB(paint.getShader());
    if (shader && !shader->asLuminanceColor(&c)) {
        return false;
    }
    if (paint.getColorFilter()) {
        // TODO: This colorspace is meaningless, replace it with something else
        SkColorSpace* cs = nullptr;
        c = paint.getColorFilter()->filterColor4f(c, cs, cs);
    }
    if (color) {
        *color = c;
    }
    return true;
}

SkColor SkPaintPriv::ComputeLuminanceColor(const SkPaint& paint) {
    SkColor4f c;
    if (!just_a_color(paint, &c)) {
        c = { 0.5f, 0.5f, 0.5f, 1.0f};
    }
    return c.toSkColor();
}

void SkPaintPriv::RemoveColorFilter(SkPaint* p, SkColorSpace* dstCS) {
    if (SkColorFilter* filter = p->getColorFilter()) {
        if (SkShader* shader = p->getShader()) {
            // SkColorFilterShader will modulate the shader color by paint alpha
            // before applying the filter, so we'll reset it to opaque.
            p->setShader(SkColorFilterShader::Make(sk_ref_sp(shader),
                                                   p->getAlphaf(),
                                                   sk_ref_sp(filter)));
            p->setAlphaf(1.0f);
        } else {
            p->setColor(filter->filterColor4f(p->getColor4f(), sk_srgb_singleton(), dstCS), dstCS);
        }
        p->setColorFilter(nullptr);
    }
}

#ifdef SK_DEBUG
    static void ASSERT_FITS_IN(uint32_t value, int bitCount) {
        SkASSERT(bitCount > 0 && bitCount <= 32);
        uint32_t mask = ~0U;
        mask >>= (32 - bitCount);
        SkASSERT(0 == (value & ~mask));
    }
#else
    #define ASSERT_FITS_IN(value, bitcount)
#endif

enum FlatFlags {
    kHasTypeface_FlatFlag = 0x1,
    kHasEffects_FlatFlag  = 0x2,

    kFlatFlagMask         = 0x3,
};

// SkPaint originally defined flags, some of which now apply to SkFont. These are renames
// of those flags, split into categories depending on which objects they (now) apply to.

template <typename T> uint32_t shift_bits(T value, unsigned shift, unsigned bits) {
    SkASSERT(shift + bits <= 32);
    uint32_t v = static_cast<uint32_t>(value);
    ASSERT_FITS_IN(v, bits);
    return v << shift;
}

constexpr uint8_t CUSTOM_BLEND_MODE_SENTINEL = 0xFF;

/*  Packing the paint
 flags :  8  // 2...
 blend :  8  // 30+
 cap   :  2  // 3
 join  :  2  // 3
 style :  2  // 3
 filter:  2  // 4
 flat  :  8  // 1...
 total : 32
 */
static uint32_t pack_v68(const SkPaint& paint, unsigned flatFlags) {
    uint32_t packed = 0;
    const auto bm = paint.asBlendMode();
    const unsigned mode = bm ? static_cast<unsigned>(bm.value())
                             : CUSTOM_BLEND_MODE_SENTINEL;

    packed |= shift_bits(((unsigned)paint.isDither() << 1) |
                          (unsigned)paint.isAntiAlias(), 0, 8);
    packed |= shift_bits(mode,                      8, 8);
    packed |= shift_bits(paint.getStrokeCap(),     16, 2);
    packed |= shift_bits(paint.getStrokeJoin(),    18, 2);
    packed |= shift_bits(paint.getStyle(),         20, 2);
    packed |= shift_bits(0,                        22, 2); // was filterquality
    packed |= shift_bits(flatFlags,                24, 8);
    return packed;
}

static uint32_t unpack_v68(SkPaint* paint, uint32_t packed, SkSafeRange& safe) {
    paint->setAntiAlias((packed & 1) != 0);
    paint->setDither((packed & 2) != 0);
    packed >>= 8;
    {
        unsigned mode = packed & 0xFF;
        if (mode != CUSTOM_BLEND_MODE_SENTINEL) { // sentinel for custom blender
            paint->setBlendMode(safe.checkLE(mode, SkBlendMode::kLastMode));
        }
        // else we will unflatten the custom blender
    }
    packed >>= 8;
    paint->setStrokeCap(safe.checkLE(packed & 0x3, SkPaint::kLast_Cap));
    packed >>= 2;
    paint->setStrokeJoin(safe.checkLE(packed & 0x3, SkPaint::kLast_Join));
    packed >>= 2;
    paint->setStyle(safe.checkLE(packed & 0x3, SkPaint::kStrokeAndFill_Style));
    packed >>= 2;
    // skip the (now ignored) filterquality bits
    packed >>= 2;

    return packed;
}

/*  To save space/time, we analyze the paint, and write a truncated version of
    it if there are not tricky elements like shaders, etc.
 */
void SkPaintPriv::Flatten(const SkPaint& paint, SkWriteBuffer& buffer) {
    uint8_t flatFlags = 0;

    if (paint.getPathEffect() ||
        paint.getShader() ||
        paint.getMaskFilter() ||
        paint.getColorFilter() ||
        paint.getImageFilter() ||
        !paint.asBlendMode()) {
        flatFlags |= kHasEffects_FlatFlag;
    }

    buffer.writeScalar(paint.getStrokeWidth());
    buffer.writeScalar(paint.getStrokeMiter());
    buffer.writeColor4f(paint.getColor4f());

    buffer.write32(pack_v68(paint, flatFlags));

    if (flatFlags & kHasEffects_FlatFlag) {
        buffer.writeFlattenable(paint.getPathEffect());
        buffer.writeFlattenable(paint.getShader());
        buffer.writeFlattenable(paint.getMaskFilter());
        buffer.writeFlattenable(paint.getColorFilter());
        buffer.writeFlattenable(paint.getImageFilter());
        buffer.writeFlattenable(paint.getBlender());
    }
}

SkPaint SkPaintPriv::Unflatten(SkReadBuffer& buffer) {
    SkPaint paint;

    paint.setStrokeWidth(buffer.readScalar());
    paint.setStrokeMiter(buffer.readScalar());
    {
        SkColor4f color;
        buffer.readColor4f(&color);
        paint.setColor(color, sk_srgb_singleton());
    }

    SkSafeRange safe;
    unsigned flatFlags = unpack_v68(&paint, buffer.readUInt(), safe);

    if (!(flatFlags & kHasEffects_FlatFlag)) {
        // This is a simple SkPaint without any effects, so clear all the effect-related fields.
        paint.setPathEffect(nullptr);
        paint.setShader(nullptr);
        paint.setMaskFilter(nullptr);
        paint.setColorFilter(nullptr);
        paint.setImageFilter(nullptr);
    } else if (buffer.isVersionLT(SkPicturePriv::kSkBlenderInSkPaint)) {
        // This paint predates the introduction of user blend functions (via SkBlender).
        paint.setPathEffect(buffer.readPathEffect());
        paint.setShader(buffer.readShader());
        paint.setMaskFilter(buffer.readMaskFilter());
        paint.setColorFilter(buffer.readColorFilter());
        (void)buffer.read32();  // was drawLooper (now deprecated)
        paint.setImageFilter(buffer.readImageFilter());
    } else {
        paint.setPathEffect(buffer.readPathEffect());
        paint.setShader(buffer.readShader());
        paint.setMaskFilter(buffer.readMaskFilter());
        paint.setColorFilter(buffer.readColorFilter());
        paint.setImageFilter(buffer.readImageFilter());
        paint.setBlender(buffer.readBlender());
    }

    if (!buffer.validate(safe.ok())) {
        paint.reset();
    }
    return paint;
}
