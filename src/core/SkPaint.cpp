/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"

#include "include/core/SkColorFilter.h"
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
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkOpts.h"
#include "src/core/SkPaintDefaults.h"
#include "src/core/SkPaintPriv.h"
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
                 (unsigned)kNone_SkFilterQuality,   // fFilterQuality
                 (unsigned)SkBlendMode::kSrcOver,   // fBlendMode
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
        && EQUAL(fDrawLooper)
        && EQUAL(fImageFilter)
        && EQUAL(fColor4f)
        && EQUAL(fWidth)
        && EQUAL(fMiterLimit)
        && EQUAL(fBitfieldsUInt)
        ;
#undef EQUAL
}

#define DEFINE_REF_FOO(type)    sk_sp<Sk##type> SkPaint::ref##type() const { return f##type; }
DEFINE_REF_FOO(ColorFilter)
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
DEFINE_REF_FOO(DrawLooper)
#endif
DEFINE_REF_FOO(ImageFilter)
DEFINE_REF_FOO(MaskFilter)
DEFINE_REF_FOO(PathEffect)
DEFINE_REF_FOO(Shader)
#undef DEFINE_REF_FOO

void SkPaint::reset() { *this = SkPaint(); }

void SkPaint::setFilterQuality(SkFilterQuality quality) {
    fBitfields.fFilterQuality = quality;
}

void SkPaint::setStyle(Style style) {
    if ((unsigned)style < kStyleCount) {
        fBitfields.fStyle = style;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStyle(%d) out of range\n", style);
#endif
    }
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

#define MOVE_FIELD(Field) void SkPaint::set##Field(sk_sp<Sk##Field> f) { f##Field = std::move(f); }
MOVE_FIELD(ImageFilter)
MOVE_FIELD(Shader)
MOVE_FIELD(ColorFilter)
MOVE_FIELD(PathEffect)
MOVE_FIELD(MaskFilter)
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
MOVE_FIELD(DrawLooper)
#endif
#undef MOVE_FIELD

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
void SkPaint::setLooper(sk_sp<SkDrawLooper> looper) { fDrawLooper = std::move(looper); }
#endif

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

enum BitsPerField {
    kFlags_BPF  = 16,
    kHint_BPF   = 2,
    kFilter_BPF = 2,
    kFlatFlags_BPF  = 3,
};

static inline int BPF_Mask(int bits) {
    return (1 << bits) - 1;
}

// SkPaint originally defined flags, some of which now apply to SkFont. These are renames
// of those flags, split into categories depending on which objects they (now) apply to.

enum PaintFlagsForPaint {
    kAA_PaintFlagForPaint     = 0x01,
    kDither_PaintFlagForPaint = 0x04,
};

enum PaintFlagsForFont {
    kFakeBold_PaintFlagForFont       = 0x20,
    kLinear_PaintFlagForFont         = 0x40,
    kSubpixel_PaintFlagForFont       = 0x80,
    kLCD_PaintFlagForFont            = 0x200,
    kEmbeddedBitmap_PaintFlagForFont = 0x400,
    kAutoHinting_PaintFlagForFont    = 0x800,
};

static FlatFlags unpack_paint_flags(SkPaint* paint, uint32_t packed, SkFont* font) {
    uint32_t f = packed >> 16;
    paint->setAntiAlias((f & kAA_PaintFlagForPaint) != 0);
    paint->setDither((f & kDither_PaintFlagForPaint) != 0);
    if (font) {
        font->setEmbolden((f & kFakeBold_PaintFlagForFont) != 0);
        font->setLinearMetrics((f & kLinear_PaintFlagForFont) != 0);
        font->setSubpixel((f & kSubpixel_PaintFlagForFont) != 0);
        font->setEmbeddedBitmaps((f & kEmbeddedBitmap_PaintFlagForFont) != 0);
        font->setForceAutoHinting((f & kAutoHinting_PaintFlagForFont) != 0);

        font->setHinting((SkFontHinting)((packed >> 14) & BPF_Mask(kHint_BPF)));

        if (f & kAA_PaintFlagForPaint) {
            if (f & kLCD_PaintFlagForFont) {
                font->setEdging(SkFont::Edging::kSubpixelAntiAlias);
            } else {
                font->setEdging(SkFont::Edging::kAntiAlias);
            }
        } else {
            font->setEdging(SkFont::Edging::kAlias);
        }
    }

    paint->setFilterQuality((SkFilterQuality)((packed >> 10) & BPF_Mask(kFilter_BPF)));
    return (FlatFlags)(packed & kFlatFlagMask);
}

template <typename T> uint32_t shift_bits(T value, unsigned shift, unsigned bits) {
    SkASSERT(shift + bits <= 32);
    uint32_t v = static_cast<uint32_t>(value);
    ASSERT_FITS_IN(v, bits);
    return v << shift;
}

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
    packed |= shift_bits(((unsigned)paint.isDither() << 1) |
                          (unsigned)paint.isAntiAlias(), 0, 8);
    packed |= shift_bits(paint.getBlendMode(),      8, 8);
    packed |= shift_bits(paint.getStrokeCap(),     16, 2);
    packed |= shift_bits(paint.getStrokeJoin(),    18, 2);
    packed |= shift_bits(paint.getStyle(),         20, 2);
    packed |= shift_bits(paint.getFilterQuality(), 22, 2);
    packed |= shift_bits(flatFlags,                24, 8);
    return packed;
}

static uint32_t unpack_v68(SkPaint* paint, uint32_t packed, SkSafeRange& safe) {
    paint->setAntiAlias((packed & 1) != 0);
    paint->setDither((packed & 2) != 0);
    packed >>= 8;
    paint->setBlendMode(safe.checkLE(packed & 0xFF, SkBlendMode::kLastMode));
    packed >>= 8;
    paint->setStrokeCap(safe.checkLE(packed & 0x3, SkPaint::kLast_Cap));
    packed >>= 2;
    paint->setStrokeJoin(safe.checkLE(packed & 0x3, SkPaint::kLast_Join));
    packed >>= 2;
    paint->setStyle(safe.checkLE(packed & 0x3, SkPaint::kStrokeAndFill_Style));
    packed >>= 2;
    paint->setFilterQuality(safe.checkLE(packed & 0x3, kLast_SkFilterQuality));
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
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
        paint.getLooper() ||
#endif
        paint.getImageFilter()) {
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
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
        buffer.writeFlattenable(paint.getLooper());
#else
        buffer.write32(0);
#endif
        buffer.writeFlattenable(paint.getImageFilter());
    }
}

SkReadPaintResult SkPaintPriv::Unflatten_PreV68(SkPaint* paint, SkReadBuffer& buffer, SkFont* font) {
    SkSafeRange safe;

    {
        SkScalar sz = buffer.readScalar();
        SkScalar sx = buffer.readScalar();
        SkScalar kx = buffer.readScalar();
        if (font) {
            font->setSize(sz);
            font->setScaleX(sx);
            font->setSkewX(kx);
        }
    }

    paint->setStrokeWidth(buffer.readScalar());
    paint->setStrokeMiter(buffer.readScalar());
    if (buffer.isVersionLT(SkReadBuffer::kFloat4PaintColor_Version)) {
        paint->setColor(buffer.readColor());
    } else {
        SkColor4f color;
        buffer.readColor4f(&color);
        paint->setColor(color, sk_srgb_singleton());
    }

    unsigned flatFlags = unpack_paint_flags(paint, buffer.readUInt(), font);

    uint32_t tmp = buffer.readUInt();
    paint->setStrokeCap(safe.checkLE((tmp >> 24) & 0xFF, SkPaint::kLast_Cap));
    paint->setStrokeJoin(safe.checkLE((tmp >> 16) & 0xFF, SkPaint::kLast_Join));
    paint->setStyle(safe.checkLE((tmp >> 12) & 0xF, SkPaint::kStrokeAndFill_Style));
    paint->setBlendMode(safe.checkLE(tmp & 0xFF, SkBlendMode::kLastMode));

    sk_sp<SkTypeface> tf;
    if (flatFlags & kHasTypeface_FlatFlag) {
        tf = buffer.readTypeface();
    }
    if (font) {
        font->setTypeface(tf);
    }

    if (flatFlags & kHasEffects_FlatFlag) {
        paint->setPathEffect(buffer.readPathEffect());
        paint->setShader(buffer.readShader());
        paint->setMaskFilter(buffer.readMaskFilter());
        paint->setColorFilter(buffer.readColorFilter());
        (void)buffer.read32();  // use to be SkRasterizer
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
        paint->setLooper(buffer.readDrawLooper());
#else
        (void)buffer.read32();  // used to be drawlooper
#endif
        paint->setImageFilter(buffer.readImageFilter());
    } else {
        paint->setPathEffect(nullptr);
        paint->setShader(nullptr);
        paint->setMaskFilter(nullptr);
        paint->setColorFilter(nullptr);
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
        paint->setLooper(nullptr);
#endif
        paint->setImageFilter(nullptr);
    }

    if (!buffer.validate(safe)) {
        paint->reset();
        return kFailed_ReadPaint;
    }
    return kSuccess_PaintAndFont;
}

SkReadPaintResult SkPaintPriv::Unflatten(SkPaint* paint, SkReadBuffer& buffer, SkFont* font) {
    if (buffer.isVersionLT(SkReadBuffer::kPaintDoesntSerializeFonts_Version)) {
        return Unflatten_PreV68(paint, buffer, font);
    }

    SkSafeRange safe;

    paint->setStrokeWidth(buffer.readScalar());
    paint->setStrokeMiter(buffer.readScalar());
    {
        SkColor4f color;
        buffer.readColor4f(&color);
        paint->setColor(color, sk_srgb_singleton());
    }

    unsigned flatFlags = unpack_v68(paint, buffer.readUInt(), safe);

    if (flatFlags & kHasEffects_FlatFlag) {
        paint->setPathEffect(buffer.readPathEffect());
        paint->setShader(buffer.readShader());
        paint->setMaskFilter(buffer.readMaskFilter());
        paint->setColorFilter(buffer.readColorFilter());
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
        paint->setLooper(buffer.readDrawLooper());
#else
        (void)buffer.readDrawLooper();
#endif
        paint->setImageFilter(buffer.readImageFilter());
    } else {
        paint->setPathEffect(nullptr);
        paint->setShader(nullptr);
        paint->setMaskFilter(nullptr);
        paint->setColorFilter(nullptr);
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
        paint->setLooper(nullptr);
#endif
        paint->setImageFilter(nullptr);
    }

    if (!buffer.validate(safe)) {
        paint->reset();
        return kFailed_ReadPaint;
    }
    return kSuccess_JustPaint;
}

///////////////////////////////////////////////////////////////////////////////

bool SkPaint::getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                          SkScalar resScale) const {
    if (!src.isFinite()) {
        dst->reset();
        return false;
    }

    SkStrokeRec rec(*this, resScale);

    const SkPath* srcPtr = &src;
    SkPath tmpPath;

    if (fPathEffect && fPathEffect->filterPath(&tmpPath, src, &rec, cullRect)) {
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
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
    if (this->getLooper()) {
        return this->getLooper()->canComputeFastBounds(*this);
    }
#endif
    if (this->getImageFilter() && !this->getImageFilter()->canComputeFastBounds()) {
        return false;
    }
    return true;
}

const SkRect& SkPaint::doComputeFastBounds(const SkRect& origSrc,
                                           SkRect* storage,
                                           Style style) const {
    SkASSERT(storage);

    const SkRect* src = &origSrc;

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
if (this->getLooper()) {
        SkASSERT(this->getLooper()->canComputeFastBounds(*this));
        this->getLooper()->computeFastBounds(*this, *src, storage);
        return *storage;
    }
#endif

    SkRect tmpSrc;
    if (this->getPathEffect()) {
        this->getPathEffect()->computeFastBounds(&tmpSrc, origSrc);
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
    return cf && !(cf->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
}

// return true if the filter exists, and may affect alpha
static bool affects_alpha(const SkImageFilter* imf) {
    // TODO: check if we should allow imagefilters to broadcast that they don't affect alpha
    // ala colorfilters
    return imf != nullptr;
}

bool SkPaint::nothingToDraw() const {
    if (fDrawLooper) {
        return false;
    }
    switch (this->getBlendMode()) {
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

uint32_t SkPaint::getHash() const {
    // We're going to hash 6 pointers and 6 floats, finishing up with fBitfields,
    // so fBitfields should be 6 pointers and 6 floats from the start.
    static_assert(offsetof(SkPaint, fBitfieldsUInt) == 6 * sizeof(void*) + 6 * sizeof(float),
                  "SkPaint_notPackedTightly");
    return SkOpts::hash(reinterpret_cast<const uint32_t*>(this),
                        offsetof(SkPaint, fBitfieldsUInt) + sizeof(fBitfieldsUInt));
}
