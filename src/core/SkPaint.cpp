/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"

#include "include/core/SkData.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTo.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkOpts.h"
#include "src/core/SkPaintDefaults.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSafeRange.h"
#include "src/core/SkStringUtils.h"
#include "src/core/SkStroke.h"
#include "src/core/SkSurfacePriv.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

// define this to get a printf for out-of-range parameter in setters
// e.g. setTextSize(-1)
//#define SK_REPORT_API_RANGE_CHECK


SkPaint::SkPaint()
    : fColor4f{0, 0, 0, 1}  // opaque black
    , fWidth{0}
    , fMiterLimit{SkPaintDefaults_MiterLimit}
    , fBitfields{(unsigned)false,                   // fAntiAlias
                 (unsigned)false,                   // fDither
                 (unsigned)SkPaint::kDefault_Cap,   // fCapType
                 (unsigned)SkPaint::kDefault_Join,  // fJoinType
                 (unsigned)SkPaint::kFill_Style,    // fStyle
                 0}                                 // fPadding
{
    static_assert(sizeof(fBitfields) == sizeof(fBitfieldsUInt), "");
}

SkPaint::SkPaint(const SkColor4f& color, SkColorSpace* colorSpace) : SkPaint() {
    this->setColor(color, colorSpace);
}

SkPaint::SkPaint(const SkPaint& src) = default;

SkPaint::SkPaint(SkPaint&& src) = default;

SkPaint::~SkPaint() = default;

SkPaint& SkPaint::operator=(const SkPaint& src) = default;

SkPaint& SkPaint::operator=(SkPaint&& src) = default;

bool operator==(const SkPaint& a, const SkPaint& b) {
#define EQUAL(field) (a.field == b.field)
    return EQUAL(fPathEffect)
        && EQUAL(fShader)
        && EQUAL(fMaskFilter)
        && EQUAL(fColorFilter)
        && EQUAL(fBlender)
        && EQUAL(fImageFilter)
        && EQUAL(fColor4f)
        && EQUAL(fWidth)
        && EQUAL(fMiterLimit)
        && EQUAL(fBitfieldsUInt)
        ;
#undef EQUAL
}

#define DEFINE_FIELD_REF(type) \
    sk_sp<Sk##type> SkPaint::ref##type() const { return f##type; }
DEFINE_FIELD_REF(ColorFilter)
DEFINE_FIELD_REF(Blender)
DEFINE_FIELD_REF(ImageFilter)
DEFINE_FIELD_REF(MaskFilter)
DEFINE_FIELD_REF(PathEffect)
DEFINE_FIELD_REF(Shader)
#undef DEFINE_FIELD_REF

#define DEFINE_FIELD_SET(Field) \
    void SkPaint::set##Field(sk_sp<Sk##Field> f) { f##Field = std::move(f); }
DEFINE_FIELD_SET(ColorFilter)
DEFINE_FIELD_SET(ImageFilter)
DEFINE_FIELD_SET(MaskFilter)
DEFINE_FIELD_SET(PathEffect)
DEFINE_FIELD_SET(Shader)
#undef DEFINE_FIELD_SET

///////////////////////////////////////////////////////////////////////////////

void SkPaint::reset() { *this = SkPaint(); }

void SkPaint::setStyle(Style style) {
    if ((unsigned)style < kStyleCount) {
        fBitfields.fStyle = style;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStyle(%d) out of range\n", style);
#endif
    }
}

void SkPaint::setStroke(bool isStroke) {
    fBitfields.fStyle = isStroke ? kStroke_Style : kFill_Style;
}

void SkPaint::setColor(SkColor color) {
    fColor4f = SkColor4f::FromColor(color);
}

void SkPaint::setColor(const SkColor4f& color, SkColorSpace* colorSpace) {
    SkASSERT(fColor4f.fA >= 0 && fColor4f.fA <= 1.0f);

    SkColorSpaceXformSteps steps{colorSpace,          kUnpremul_SkAlphaType,
                                 sk_srgb_singleton(), kUnpremul_SkAlphaType};
    fColor4f = color;
    steps.apply(fColor4f.vec());
}

void SkPaint::setAlphaf(float a) {
    SkASSERT(a >= 0 && a <= 1.0f);
    fColor4f.fA = a;
}

void SkPaint::setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    this->setColor(SkColorSetARGB(a, r, g, b));
}

skstd::optional<SkBlendMode> SkPaint::asBlendMode() const {
    return fBlender ? as_BB(fBlender)->asBlendMode()
                    : SkBlendMode::kSrcOver;
}

SkBlendMode SkPaint::getBlendMode_or(SkBlendMode defaultMode) const {
    return this->asBlendMode().value_or(defaultMode);
}

bool SkPaint::isSrcOver() const {
    return !fBlender || as_BB(fBlender)->asBlendMode() == SkBlendMode::kSrcOver;
}

void SkPaint::setBlendMode(SkBlendMode mode) {
    this->setBlender(mode == SkBlendMode::kSrcOver ? nullptr : SkBlender::Mode(mode));
}

void SkPaint::setBlender(sk_sp<SkBlender> blender) {
    fBlender = std::move(blender);
}

void SkPaint::setStrokeWidth(SkScalar width) {
    if (width >= 0) {
        fWidth = width;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeWidth() called with negative value\n");
#endif
    }
}

void SkPaint::setStrokeMiter(SkScalar limit) {
    if (limit >= 0) {
        fMiterLimit = limit;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeMiter() called with negative value\n");
#endif
    }
}

void SkPaint::setStrokeCap(Cap ct) {
    if ((unsigned)ct < kCapCount) {
        fBitfields.fCapType = SkToU8(ct);
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeCap(%d) out of range\n", ct);
#endif
    }
}

void SkPaint::setStrokeJoin(Join jt) {
    if ((unsigned)jt < kJoinCount) {
        fBitfields.fJoinType = SkToU8(jt);
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeJoin(%d) out of range\n", jt);
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "include/core/SkStream.h"

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

///////////////////////////////////////////////////////////////////////////////

bool SkPaint::getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                          SkScalar resScale) const {
    return this->getFillPath(src, dst, cullRect, SkMatrix::Scale(resScale, resScale));
}

bool SkPaint::getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                          const SkMatrix& ctm) const {
    if (!src.isFinite()) {
        dst->reset();
        return false;
    }

    const SkScalar resScale = SkPaintPriv::ComputeResScaleForStroking(ctm);
    SkStrokeRec rec(*this, resScale);

#if defined(SK_BUILD_FOR_FUZZER)
    // Prevent lines with small widths from timing out.
    if (rec.getStyle() == SkStrokeRec::Style::kStroke_Style && rec.getWidth() < 0.001) {
        return false;
    }
#endif

    const SkPath* srcPtr = &src;
    SkPath tmpPath;

    if (fPathEffect && fPathEffect->filterPath(&tmpPath, src, &rec, cullRect, ctm)) {
        srcPtr = &tmpPath;
    }

    if (!rec.applyToPath(dst, *srcPtr)) {
        if (srcPtr == &tmpPath) {
            // If path's were copy-on-write, this trick would not be needed.
            // As it is, we want to save making a deep-copy from tmpPath -> dst
            // since we know we're just going to delete tmpPath when we return,
            // so the swap saves that copy.
            dst->swap(tmpPath);
        } else {
            *dst = *srcPtr;
        }
    }

    if (!dst->isFinite()) {
        dst->reset();
        return false;
    }
    return !rec.isHairlineStyle();
}

bool SkPaint::canComputeFastBounds() const {
    if (this->getImageFilter() && !this->getImageFilter()->canComputeFastBounds()) {
        return false;
    }
    // Pass nullptr for the bounds to determine if they can be computed
    if (this->getPathEffect() &&
        !as_PEB(this->getPathEffect())->computeFastBounds(nullptr)) {
        return false;
    }
    return true;
}

const SkRect& SkPaint::doComputeFastBounds(const SkRect& origSrc,
                                           SkRect* storage,
                                           Style style) const {
    SkASSERT(storage);

    const SkRect* src = &origSrc;

    SkRect tmpSrc;
    if (this->getPathEffect()) {
        tmpSrc = origSrc;
        SkAssertResult(as_PEB(this->getPathEffect())->computeFastBounds(&tmpSrc));
        src = &tmpSrc;
    }

    SkScalar radius = SkStrokeRec::GetInflationRadius(*this, style);
    *storage = src->makeOutset(radius, radius);

    if (this->getMaskFilter()) {
        as_MFB(this->getMaskFilter())->computeFastBounds(*storage, storage);
    }

    if (this->getImageFilter()) {
        *storage = this->getImageFilter()->computeFastBounds(*storage);
    }

    return *storage;
}

///////////////////////////////////////////////////////////////////////////////

// return true if the filter exists, and may affect alpha
static bool affects_alpha(const SkColorFilter* cf) {
    return cf && !as_CFB(cf)->isAlphaUnchanged();
}

// return true if the filter exists, and may affect alpha
static bool affects_alpha(const SkImageFilter* imf) {
    // TODO: check if we should allow imagefilters to broadcast that they don't affect alpha
    // ala colorfilters
    return imf != nullptr;
}

bool SkPaint::nothingToDraw() const {
    auto bm = this->asBlendMode();
    if (!bm) {
        return false;
    }
    switch (bm.value()) {
        case SkBlendMode::kSrcOver:
        case SkBlendMode::kSrcATop:
        case SkBlendMode::kDstOut:
        case SkBlendMode::kDstOver:
        case SkBlendMode::kPlus:
            if (0 == this->getAlpha()) {
                return !affects_alpha(fColorFilter.get()) && !affects_alpha(fImageFilter.get());
            }
            break;
        case SkBlendMode::kDst:
            return true;
        default:
            break;
    }
    return false;
}
