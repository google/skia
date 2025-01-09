/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkPaintDefaults.h"
#include "src/core/SkPathEffectBase.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <utility>

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
    SkColorSpaceXformSteps steps{colorSpace,          kUnpremul_SkAlphaType,
                                 sk_srgb_singleton(), kUnpremul_SkAlphaType};
    fColor4f = color.pinAlpha();
    steps.apply(fColor4f.vec());
}

void SkPaint::setAlphaf(float a) {
    fColor4f.fA = SkTPin(a, 0.0f, 1.0f);
}

void SkPaint::setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    this->setColor(SkColorSetARGB(a, r, g, b));
}

std::optional<SkBlendMode> SkPaint::asBlendMode() const {
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

const SkRect& SkPaint::computeFastBounds(const SkRect& orig, SkRect* storage) const {
    // Things like stroking, etc... will do math on the bounds rect, assuming that it's sorted.
    SkASSERT(orig.isSorted());
    SkPaint::Style style = this->getStyle();
    // ultra fast-case: filling with no effects that affect geometry
    if (kFill_Style == style) {
        uintptr_t effects = 0;
        effects |= reinterpret_cast<uintptr_t>(this->getMaskFilter());
        effects |= reinterpret_cast<uintptr_t>(this->getPathEffect());
        effects |= reinterpret_cast<uintptr_t>(this->getImageFilter());
        if (!effects) {
            return orig;
        }
    }

    return this->doComputeFastBounds(orig, storage, style);
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
