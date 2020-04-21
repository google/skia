/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/geometry/GrStyledShape.h"

#include "include/private/SkIDChangeListener.h"

#include <utility>

GrStyledShape& GrStyledShape::operator=(const GrStyledShape& that) {
    fStyle = that.fStyle;
    fShape = that.fShape;
    fInverted = that.fInverted;
    fDir = that.fDir;
    fStart = that.fStart;
    fGenID = that.fGenID;

    fInheritedKey.reset(that.fInheritedKey.count());
    sk_careful_memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
                      sizeof(uint32_t) * fInheritedKey.count());
    if (that.fInheritedPathForListeners.isValid()) {
        fInheritedPathForListeners.set(*that.fInheritedPathForListeners.get());
    } else {
        fInheritedPathForListeners.reset();
    }
    return *this;
}

static bool flip_inversion(bool originalIsInverted, GrStyledShape::FillInversion inversion) {
    switch (inversion) {
        case GrStyledShape::FillInversion::kPreserve:
            return false;
        case GrStyledShape::FillInversion::kFlip:
            return true;
        case GrStyledShape::FillInversion::kForceInverted:
            return !originalIsInverted;
        case GrStyledShape::FillInversion::kForceNoninverted:
            return originalIsInverted;
    }
    return false;
}

static bool is_inverted(bool originalIsInverted, GrStyledShape::FillInversion inversion) {
    switch (inversion) {
        case GrStyledShape::FillInversion::kPreserve:
            return originalIsInverted;
        case GrStyledShape::FillInversion::kFlip:
            return !originalIsInverted;
        case GrStyledShape::FillInversion::kForceInverted:
            return true;
        case GrStyledShape::FillInversion::kForceNoninverted:
            return false;
    }
    return false;
}

GrStyledShape GrStyledShape::MakeFilled(const GrStyledShape& original, FillInversion inversion) {
    if (original.style().isSimpleFill() && !flip_inversion(original.inverseFilled(), inversion)) {
        // By returning the original rather than falling through we can preserve any inherited style
        // key. Otherwise, we wipe it out below since the style change invalidates it.
        return original;
    }
    GrStyledShape result;
    SkASSERT(result.fStyle.isSimpleFill());
    if (original.fInheritedPathForListeners.isValid()) {
        result.fInheritedPathForListeners.set(*original.fInheritedPathForListeners.get());
    }

    result.fInverted = is_inverted(original.fInverted, inversion);
    // For the most part, the shapes copy over to result as-is, with a few modifications
    if (!original.fShape.isLine()) {
        result.fShape = original.fShape;
        result.fGenID = original.fGenID;
        // We explicitly don't preserve the direction and start of the round rect
        SkASSERT(result.fDir == kDefaultRRectDir);
        SkASSERT(result.fStart == kDefaultRRectStart);

        if (result.fShape.isPath()) {
            if (flip_inversion(original.fShape.path().isInverseFillType(), inversion)) {
                result.fShape.path().toggleInverseFillType();
            }
            if (!original.style().isSimpleFill()) {
                // Going from a non-filled style to fill may allow additional simplifications (e.g.
                // closing an open rect that wasn't closed in the original shape because it had
                // stroke style).
                result.simplify();
            }
        }
    } else {
        // Lines don't fill
        SkASSERT(result.fShape.isEmpty());
    }

    // We don't copy the inherited key since it can contain path effect information that we just
    // stripped.
    return result;
}

SkRect GrStyledShape::styledBounds() const {
    if (this->isEmpty() && !fStyle.hasNonDashPathEffect()) {
        return SkRect::MakeEmpty();
    }

    SkRect bounds;
    fStyle.adjustBounds(&bounds, this->bounds());
    return bounds;
}

// If the path is small enough to be keyed from its data this returns key length, otherwise -1.
static int path_key_from_data_size(const SkPath& path) {
    const int verbCnt = path.countVerbs();
    if (verbCnt > GrStyledShape::kMaxKeyFromDataVerbCnt) {
        return -1;
    }
    const int pointCnt = path.countPoints();
    const int conicWeightCnt = SkPathPriv::ConicWeightCnt(path);

    static_assert(sizeof(SkPoint) == 2 * sizeof(uint32_t));
    static_assert(sizeof(SkScalar) == sizeof(uint32_t));
    // 2 is for the verb cnt and a fill type. Each verb is a byte but we'll pad the verb data out to
    // a uint32_t length.
    return 2 + (SkAlign4(verbCnt) >> 2) + 2 * pointCnt + conicWeightCnt;
}

// Writes the path data key into the passed pointer.
static void write_path_key_from_data(const SkPath& path, uint32_t* origKey) {
    uint32_t* key = origKey;
    // The check below should take care of negative values casted positive.
    const int verbCnt = path.countVerbs();
    const int pointCnt = path.countPoints();
    const int conicWeightCnt = SkPathPriv::ConicWeightCnt(path);
    SkASSERT(verbCnt <= GrStyledShape::kMaxKeyFromDataVerbCnt);
    SkASSERT(pointCnt && verbCnt);
    *key++ = (uint32_t)path.getFillType();
    *key++ = verbCnt;
    memcpy(key, SkPathPriv::VerbData(path), verbCnt * sizeof(uint8_t));
    int verbKeySize = SkAlign4(verbCnt);
    // pad out to uint32_t alignment using value that will stand out when debugging.
    uint8_t* pad = reinterpret_cast<uint8_t*>(key)+ verbCnt;
    memset(pad, 0xDE, verbKeySize - verbCnt);
    key += verbKeySize >> 2;

    memcpy(key, SkPathPriv::PointData(path), sizeof(SkPoint) * pointCnt);
    static_assert(sizeof(SkPoint) == 2 * sizeof(uint32_t));
    key += 2 * pointCnt;
    sk_careful_memcpy(key, SkPathPriv::ConicWeightData(path), sizeof(SkScalar) * conicWeightCnt);
    static_assert(sizeof(SkScalar) == sizeof(uint32_t));
    SkDEBUGCODE(key += conicWeightCnt);
    SkASSERT(key - origKey == path_key_from_data_size(path));
}

int GrStyledShape::unstyledKeySize() const {
    if (fInheritedKey.count()) {
        return fInheritedKey.count();
    }
    if (fShape.isEmpty()) {
        return 1;
    } else if (fShape.isRect()) {
        SkASSERT(!fInheritedKey.count());
        static_assert(0 == sizeof(SkRect) % sizeof(uint32_t));
        // + 1 for the direction, start index, and inverseness
        return sizeof(SkRect) / sizeof(uint32_t) + 1;
    } else if (fShape.isRRect()) {
        SkASSERT(!fInheritedKey.count());
        static_assert(0 == SkRRect::kSizeInMemory % sizeof(uint32_t));
        // + 1 for the direction, start index, and inverseness.
        return SkRRect::kSizeInMemory / sizeof(uint32_t) + 1;
    } else if (fShape.isArc()) {
        SkASSERT(!fInheritedKey.count());
        static_assert(0 == sizeof(GrArc) % sizeof(uint32_t));
        // + 1 for the inverseness
        return sizeof(GrArc) / sizeof(uint32_t) + 1;
    } else if (fShape.isLine()) {
        static_assert(0 == sizeof(GrLineSegment) % sizeof(uint32_t));
        // + 1 for the inverseness
        return sizeof(GrLineSegment) / sizeof(uint32_t) + 1;
    } else {
        SkASSERT(fShape.isPath());
        if (0 == fGenID) {
            return -1;
        }
        int dataKeySize = path_key_from_data_size(fShape.path());
        if (dataKeySize >= 0) {
            return dataKeySize;
        }
        // The key is the path ID and fill type.
        return 2;
    }
}

void GrStyledShape::writeUnstyledKey(uint32_t* key) const {
    // Additional bits set for rect and line shapes to disambiguate them from
    // each other. Otherwise they are both 4 floats + flags, which can conflit
    // (e.g. draw a diagonal line between two corners of the rect).
    static constexpr uint32_t kRectFlag = 1 << 29;
    static constexpr uint32_t kLineFlag = 1 << 28;

    SkASSERT(this->unstyledKeySize());
    SkDEBUGCODE(uint32_t* origKey = key;)
    if (fInheritedKey.count()) {
        memcpy(key, fInheritedKey.get(), sizeof(uint32_t) * fInheritedKey.count());
        SkDEBUGCODE(key += fInheritedKey.count();)
    } else {
        // Dir and start are only used for rect and rrect shapes, so are not included in other
        // shape type keys.
        SkASSERT(fStart < 8);

        if (fShape.isEmpty()) {
            // Value distinguishes between empty(1) and inverted-empty(2)
            *key++ = (fInverted ? 2 : 1);

        } else if (fShape.isRect()) {
            memcpy(key, &fShape.rect(), sizeof(SkRect));
            key += sizeof(SkRect) / sizeof(uint32_t);
            *key = kRectFlag;
            *key |= (fDir == SkPathDirection::kCCW) ? (1 << 31) : 0;
            *key |= fInverted ? (1 << 30) : 0;
            *key++ |= fStart;
        } else if (fShape.isRRect()) {
            fShape.rrect().writeToMemory(key);
            key += SkRRect::kSizeInMemory / sizeof(uint32_t);
            *key = (fDir == SkPathDirection::kCCW) ? (1 << 31) : 0;
            *key |= fInverted ? (1 << 30) : 0;
            *key++ |= fStart;
        } else if (fShape.isArc()) {
            // Write dense floats first
            memcpy(key, &fShape.arc(), sizeof(SkRect) + 2 * sizeof(float));
            key += (sizeof(GrArc) / sizeof(uint32_t) - 1);
            // Then write the final bool as an int
            *key++ = fShape.arc().fUseCenter ? 1 : 0;
            *key++ = fInverted ? 1 : 0;
        } else if (fShape.isLine()) {
            memcpy(key, &fShape.line(), sizeof(GrLineSegment));
            key += sizeof(GrLineSegment) / sizeof(uint32_t);
            *key++ = kLineFlag | (fInverted ? 1 : 0);
        } else {
            SkASSERT(fShape.isPath() && fGenID != 0);
            // Ensure that the path's inversion matches our state so that the path's key suffices.
            SkASSERT(fInverted == fShape.path().isInverseFillType());

            int dataKeySize = path_key_from_data_size(fShape.path());
            if (dataKeySize >= 0) {
                write_path_key_from_data(fShape.path(), key);
                return;
            }
            *key++ = fGenID;
            // We could canonicalize the fill rule for paths that don't differentiate between
            // even/odd or winding fill (e.g. convex).
            *key++ = (uint32_t) fShape.path().getFillType();
        }
    }
    SkASSERT(key - origKey == this->unstyledKeySize());
}

void GrStyledShape::setInheritedKey(const GrStyledShape &parent, GrStyle::Apply apply,
                                    SkScalar scale) {
    SkASSERT(!fInheritedKey.count());
    // If the output shape turns out to be simple, then we will just use its geometric key
    if (fShape.isPath()) {
        // We want ApplyFullStyle(ApplyPathEffect(shape)) to have the same key as
        // ApplyFullStyle(shape).
        // The full key is structured as (geo,path_effect,stroke).
        // If we do ApplyPathEffect we get geo,path_effect as the inherited key. If we then
        // do ApplyFullStyle we'll memcpy geo,path_effect into the new inherited key
        // and then append the style key (which should now be stroke only) at the end.
        int parentCnt = parent.fInheritedKey.count();
        bool useParentGeoKey = !parentCnt;
        if (useParentGeoKey) {
            parentCnt = parent.unstyledKeySize();
            if (parentCnt < 0) {
                // The parent's geometry has no key so we will have no key.
                fGenID = 0;
                return;
            }
        }
        uint32_t styleKeyFlags = 0;
        if (parent.knownToBeClosed()) {
            styleKeyFlags |= GrStyle::kClosed_KeyFlag;
        }
        if (parent.asLine(nullptr, nullptr)) {
            styleKeyFlags |= GrStyle::kNoJoins_KeyFlag;
        }
        int styleCnt = GrStyle::KeySize(parent.fStyle, apply, styleKeyFlags);
        if (styleCnt < 0) {
            // The style doesn't allow a key, set the path gen ID to 0 so that we fail when
            // we try to get a key for the shape.
            fGenID = 0;
            return;
        }
        fInheritedKey.reset(parentCnt + styleCnt);
        if (useParentGeoKey) {
            // This will be the geo key.
            parent.writeUnstyledKey(fInheritedKey.get());
        } else {
            // This should be (geo,path_effect).
            memcpy(fInheritedKey.get(), parent.fInheritedKey.get(),
                   parentCnt * sizeof(uint32_t));
        }
        // Now turn (geo,path_effect) or (geo) into (geo,path_effect,stroke)
        GrStyle::WriteKey(fInheritedKey.get() + parentCnt, parent.fStyle, apply, scale,
                          styleKeyFlags);
    }
}

const SkPath* GrStyledShape::originalPathForListeners() const {
    if (fInheritedPathForListeners.isValid()) {
        return fInheritedPathForListeners.get();
    } else if (fShape.isPath() && !fShape.path().isVolatile()) {
        return &fShape.path();
    }
    return nullptr;
}

void GrStyledShape::addGenIDChangeListener(sk_sp<SkIDChangeListener> listener) const {
    if (const auto* lp = this->originalPathForListeners()) {
        SkPathPriv::AddGenIDChangeListener(*lp, std::move(listener));
    }
}

GrStyledShape GrStyledShape::MakeArc(const SkRect& oval, SkScalar startAngleDegrees,
                                     SkScalar sweepAngleDegrees, bool useCenter,
                                     const GrStyle& style) {
    GrStyledShape result;
    result.fShape.setArc({oval, startAngleDegrees, sweepAngleDegrees, useCenter});
    result.fStyle = style;
    result.simplify();
    return result;
}

GrStyledShape::GrStyledShape(const GrStyledShape& that)
        : fShape(that.fShape)
        , fInverted(that.fInverted)
        , fDir(that.fDir)
        , fStart(that.fStart)
        , fGenID(that.fGenID)
        , fStyle(that.fStyle) {
    fInheritedKey.reset(that.fInheritedKey.count());
    sk_careful_memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
                      sizeof(uint32_t) * fInheritedKey.count());
    if (that.fInheritedPathForListeners.isValid()) {
        fInheritedPathForListeners.set(*that.fInheritedPathForListeners.get());
    }
}

GrStyledShape::GrStyledShape(const GrStyledShape& parent, GrStyle::Apply apply, SkScalar scale) {
    // TODO: Add some quantization of scale for better cache performance here or leave that up
    // to caller?
    // TODO: For certain shapes and stroke params we could ignore the scale. (e.g. miter or bevel
    // stroke of a rect).
    if (!parent.style().applies() ||
        (GrStyle::Apply::kPathEffectOnly == apply && !parent.style().pathEffect())) {
        *this = parent;
        return;
    }

    SkPathEffect* pe = parent.fStyle.pathEffect();
    SkTLazy<SkPath> tmpPath;
    const GrStyledShape* parentForKey = &parent;
    SkTLazy<GrStyledShape> tmpParent;

    // Start out as an empty path that is filled in by the applied style
    fShape.setPath(SkPath());

    if (pe) {
        const SkPath* srcForPathEffect;
        if (parent.fShape.isPath()) {
            srcForPathEffect = &parent.fShape.path();
        } else {
            srcForPathEffect = tmpPath.init();
            parent.asPath(tmpPath.get());
        }
        // Should we consider bounds? Would have to include in key, but it'd be nice to know
        // if the bounds actually modified anything before including in key.
        SkStrokeRec strokeRec = parent.fStyle.strokeRec();
        if (!parent.fStyle.applyPathEffectToPath(&fShape.path(), &strokeRec, *srcForPathEffect,
                                                 scale)) {
            tmpParent.init(*srcForPathEffect, GrStyle(strokeRec, nullptr));
            *this = tmpParent.get()->applyStyle(apply, scale);
            return;
        }
        // A path effect has access to change the res scale but we aren't expecting it to and it
        // would mess up our key computation.
        SkASSERT(scale == strokeRec.getResScale());
        if (GrStyle::Apply::kPathEffectAndStrokeRec == apply && strokeRec.needToApply()) {
            // The intermediate shape may not be a general path. If we we're just applying
            // the path effect then attemptToReduceFromPath would catch it. This means that
            // when we subsequently applied the remaining strokeRec we would have a non-path
            // parent shape that would be used to determine the the stroked path's key.
            // We detect that case here and change parentForKey to a temporary that represents
            // the simpler shape so that applying both path effect and the strokerec all at
            // once produces the same key.
            tmpParent.init(fShape.path(), GrStyle(strokeRec, nullptr));
            tmpParent.get()->setInheritedKey(parent, GrStyle::Apply::kPathEffectOnly, scale);
            if (!tmpPath.isValid()) {
                tmpPath.init();
            }
            tmpParent.get()->asPath(tmpPath.get());
            SkStrokeRec::InitStyle fillOrHairline;
            // The parent shape may have simplified away the strokeRec, check for that here.
            if (tmpParent.get()->style().applies()) {
                SkAssertResult(tmpParent.get()->style().applyToPath(&fShape.path(), &fillOrHairline,
                                                                    *tmpPath.get(), scale));
            } else if (tmpParent.get()->style().isSimpleFill()) {
                fillOrHairline = SkStrokeRec::kFill_InitStyle;
            } else {
                SkASSERT(tmpParent.get()->style().isSimpleHairline());
                fillOrHairline = SkStrokeRec::kHairline_InitStyle;
            }
            fStyle.resetToInitStyle(fillOrHairline);
            parentForKey = tmpParent.get();
        } else {
            fStyle = GrStyle(strokeRec, nullptr);
        }
    } else {
        const SkPath* srcForParentStyle;
        if (parent.fShape.isPath()) {
            srcForParentStyle = &parent.fShape.path();
        } else {
            srcForParentStyle = tmpPath.init();
            parent.asPath(tmpPath.get());
        }
        SkStrokeRec::InitStyle fillOrHairline;
        SkASSERT(parent.fStyle.applies());
        SkASSERT(!parent.fStyle.pathEffect());
        SkAssertResult(parent.fStyle.applyToPath(&fShape.path(), &fillOrHairline,
                                                 *srcForParentStyle, scale));
        fStyle.resetToInitStyle(fillOrHairline);
    }

    // Sync invertedness from applied path
    fInverted = fShape.path().isInverseFillType();

    if (parent.fInheritedPathForListeners.isValid()) {
        fInheritedPathForListeners.set(*parent.fInheritedPathForListeners.get());
    } else if (parent.fShape.isPath() && !parent.fShape.path().isVolatile()) {
        fInheritedPathForListeners.set(parent.fShape.path());
    }
    this->simplify();
    this->setInheritedKey(*parentForKey, apply, scale);
}

bool GrStyledShape::asNestedRects(SkRect rects[2]) const {
    if (!fShape.isPath()) {
        return false;
    }

    // TODO: it would be better two store DRRects natively in the shape rather than converting
    // them to a path and then reextracting the nested rects
    if (fShape.path().isInverseFillType()) {
        return false;
    }

    SkPathDirection dirs[2];
    if (!SkPathPriv::IsNestedFillRects(fShape.path(), rects, dirs)) {
        return false;
    }

    if (SkPathFillType::kWinding == fShape.path().getFillType() && dirs[0] == dirs[1]) {
        // The two rects need to be wound opposite to each other
        return false;
    }

    // Right now, nested rects where the margin is not the same width
    // all around do not render correctly
    const SkScalar* outer = rects[0].asScalars();
    const SkScalar* inner = rects[1].asScalars();

    bool allEq = true;

    SkScalar margin = SkScalarAbs(outer[0] - inner[0]);
    bool allGoE1 = margin >= SK_Scalar1;

    for (int i = 1; i < 4; ++i) {
        SkScalar temp = SkScalarAbs(outer[i] - inner[i]);
        if (temp < SK_Scalar1) {
            allGoE1 = false;
        }
        if (!SkScalarNearlyEqual(margin, temp)) {
            allEq = false;
        }
    }

    return allEq || allGoE1;
}

void GrStyledShape::simplify() {
    if (this->style().isDashed()) {
        // Dashing ignores inverseness skbug.com/5421.
        fInverted = false;
    }

    // With no path effect or fill, a stroked, hairline, or stroke-and-filled arc that traverses
    // the full circle and doesn't use the center point is an oval, unless it has square or
    // round caps. Those caps may protrude out of the oval. Round caps can't protrude out of a
    // circle but we're ignoring that for now.
    bool arcCanBeOval = fShape.isArc() && !fShape.arc().fUseCenter;
    bool styleCanBeOval = this->style().isSimpleFill() || (this->style().strokeRec().getCap() == SkPaint::kButt_Cap);

    // dir and start out with the default and will be filled in with the extracted dir and start
    // by GrShape::simplify
    if (this->style().hasPathEffect() || (arcCanBeOval && !styleCanBeOval)) {
        fShape.simplifyForPathEffect(&fDir, &fStart);
    } else {
        bool simpleFill = this->style().isSimpleFill() || (arcCanBeOval && styleCanBeOval);
        fShape.simplify(simpleFill);
    }

    if (fShape.isPath()) {
        // The shape remains a path, so configure the gen ID and canonicalize fill type if possible
        if (fInheritedKey.count() || fShape.path().isVolatile()) {
            fGenID = 0;
        } else {
            fGenID = fShape.path().getGenerationID();
        }
        if (!this->style().hasNonDashPathEffect()) {
            if (this->style().strokeRec().getStyle() == SkStrokeRec::kStroke_Style ||
                this->style().strokeRec().getStyle() == SkStrokeRec::kHairline_Style ||
                fShape.path().isConvex()) {
                // Stroke styles don't differentiate between winding and even/odd. There is no
                // distinction between even/odd and non-zero winding count for convex paths.
                // Moreover, dashing ignores inverseness (skbug.com/5421)
                if (fInverted) {
                    fShape.path().setFillType(kDefaultPathInverseFillType);
                } else {
                    fShape.path().setFillType(kDefaultPathFillType);
                }
            }
        }
        if (!fInverted && fShape.path().isInverseFillType()) {
            // The path was inverted but the style is dashed, but we expect path fill type to match
            fShape.path().toggleInverseFillType();
        }
        SkASSERT(fInverted == fShape.path().isInverseFillType());
    } else {
        fInheritedKey.reset(0);
        // Whenever we simplify to a non-path, break the chain so we no longer refer to the
        // original path. This prevents attaching genID listeners to temporary paths created when
        // drawing simple shapes.
        fInheritedPathForListeners.reset();

        // Further simplifications to the shape based on the style
        bool lineRRect = (fShape.isRRect() && !fShape.rrect().width() && !fShape.rrect().height()) ||
                         (fShape.isRect() && !fShape.rect().width() && !fShape.rect().height());
        if (this->style().isDashed() && lineRRect) {
            // We had a path effect so simplifyForPathEffect didn't change to empty, but we know
            // it's just a dash and dashing a rrect with no width or height is equivalent to filling
            // an empty path. When skbug.com/7387 is fixed this should be modified or removed as a
            // dashed zero length line  will produce cap geometry if the effect begins in an "on"
            // interval.
            fShape.reset();
            fStyle = GrStyle::SimpleFill();
        } else if (!this->style().hasPathEffect() &&
                   fShape.isRect() &&
                   this->style().strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style &&
                   this->style().strokeRec().getJoin() == SkPaint::kMiter_Join &&
                   this->style().strokeRec().getMiter() >= SK_ScalarSqrt2) {
            // A mitered stroke + fill on a rect is equivalent to a larger fill-only rect.
            SkScalar r = fStyle.strokeRec().getWidth() / 2;
            fShape.rect().outset(r, r);
            fStyle = GrStyle::SimpleFill();
        } else if (fShape.isArc()) {
            if (!fStyle.pathEffect()) {
                // Canonicalize the arc such that the start is always in [0, 360) and the sweep is
                // always positive.
                if (fShape.arc().fSweepAngle < 0) {
                    fShape.arc().fStartAngle = fShape.arc().fStartAngle + fShape.arc().fSweepAngle;
                    fShape.arc().fSweepAngle = -fShape.arc().fSweepAngle;
                }
            }
            if (fShape.arc().fStartAngle < 0 || fShape.arc().fStartAngle >= 360.f) {
                fShape.arc().fStartAngle = SkScalarMod(fShape.arc().fStartAngle, 360.f);
            }
            // Possible TODOs here: Look at whether dash pattern results in a single dash and convert to
            // non-dashed stroke. Stroke and fill can be fill if circular and no path effect. Just stroke
            // could as well if the stroke fills the center.
        } else if (fShape.isLine()) {
            if (this->style().isDashed()) {
                // If there's a single dash in the line, attempt to apply the stroke explicitly
                bool allOffsZero = true;
                for (int i = 1; i < fStyle.dashIntervalCnt() && allOffsZero; i += 2) {
                    allOffsZero = !fStyle.dashIntervals()[i];
                }
                if (allOffsZero) {
                    this->applyStrokeToLine();
                }
            } else if (!this->style().hasPathEffect()) {
                // Make stroke + fill be a stroke since the fill is empty
                if (this->style().strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style) {
                    SkStrokeRec rec = fStyle.strokeRec();
                    rec.setStrokeStyle(fStyle.strokeRec().getWidth(), false);
                    fStyle = GrStyle(rec, nullptr);
                } else if (this->style().isSimpleFill()) {
                    // And a fill is empty
                    fShape.reset();
                }

                if (!fShape.isEmpty() && !this->applyStrokeToLine()) {
                    // Only path effects could care about the order of the points. Otherwise
                    // canonicalize the point order.
                    GrLineSegment& line = fShape.line();
                    if (line.fP2.fY < line.fP1.fY ||
                        (line.fP2.fY == line.fP1.fY && line.fP2.fX < line.fP1.fX)) {
                        using std::swap;
                        swap(line.fP1, line.fP2);
                    }
                }
            }
        }
    }
}

bool GrStyledShape::applyStrokeToLine() {
    SkASSERT(fShape.isLine());

    if (fStyle.strokeRec().getStyle() != SkStrokeRec::kStroke_Style) {
        return false;
    }

    SkRect rect;
    SkVector outset;
    // If we allowed a rotation angle for rrects we could capture all cases here.
    if (fShape.line().fP1.fY == fShape.line().fP2.fY) {
        rect.fLeft = std::min(fShape.line().fP1.fX, fShape.line().fP2.fX);
        rect.fRight = std::max(fShape.line().fP1.fX, fShape.line().fP2.fX);
        rect.fTop = rect.fBottom = fShape.line().fP1.fY;
        outset.fY = fStyle.strokeRec().getWidth() / 2.f;
        outset.fX = SkPaint::kButt_Cap == fStyle.strokeRec().getCap() ? 0.f : outset.fY;
    } else if (fShape.line().fP1.fX == fShape.line().fP2.fX) {
        rect.fTop = std::min(fShape.line().fP1.fY, fShape.line().fP2.fY);
        rect.fBottom = std::max(fShape.line().fP1.fY, fShape.line().fP2.fY);
        rect.fLeft = rect.fRight = fShape.line().fP1.fX;
        outset.fX = fStyle.strokeRec().getWidth() / 2.f;
        outset.fY = SkPaint::kButt_Cap == fStyle.strokeRec().getCap() ? 0.f : outset.fX;
    } else {
        return false;
    }
    rect.outset(outset.fX, outset.fY);
    if (rect.isEmpty()) {
        fShape.reset();
        fStyle = GrStyle::SimpleFill();
        return true;
    }

    if (fStyle.strokeRec().getCap() == SkPaint::kRound_Cap) {
        SkASSERT(outset.fX == outset.fY);
        fShape.setRRect(SkRRect::MakeRectXY(rect, outset.fX, outset.fY));
    } else {
        fShape.setRect(rect);
    }
    fStyle = GrStyle::SimpleFill();
    return true;
}
