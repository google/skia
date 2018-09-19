/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrShape.h"

#include <utility>

GrShape& GrShape::operator=(const GrShape& that) {
    fStyle = that.fStyle;
    this->changeType(that.fType, Type::kPath == that.fType ? &that.path() : nullptr);
    switch (fType) {
        case Type::kEmpty:
            break;
        case Type::kInvertedEmpty:
            break;
        case Type::kRRect:
            fRRectData = that.fRRectData;
            break;
        case Type::kArc:
            fArcData = that.fArcData;
            break;
        case Type::kLine:
            fLineData = that.fLineData;
            break;
        case Type::kPath:
            fPathData.fGenID = that.fPathData.fGenID;
            break;
    }
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

static bool flip_inversion(bool originalIsInverted, GrShape::FillInversion inversion) {
    switch (inversion) {
        case GrShape::FillInversion::kPreserve:
            return false;
        case GrShape::FillInversion::kFlip:
            return true;
        case GrShape::FillInversion::kForceInverted:
            return !originalIsInverted;
        case GrShape::FillInversion::kForceNoninverted:
            return originalIsInverted;
    }
    return false;
}

static bool is_inverted(bool originalIsInverted, GrShape::FillInversion inversion) {
    switch (inversion) {
        case GrShape::FillInversion::kPreserve:
            return originalIsInverted;
        case GrShape::FillInversion::kFlip:
            return !originalIsInverted;
        case GrShape::FillInversion::kForceInverted:
            return true;
        case GrShape::FillInversion::kForceNoninverted:
            return false;
    }
    return false;
}

GrShape GrShape::MakeFilled(const GrShape& original, FillInversion inversion) {
    if (original.style().isSimpleFill() && !flip_inversion(original.inverseFilled(), inversion)) {
        // By returning the original rather than falling through we can preserve any inherited style
        // key. Otherwise, we wipe it out below since the style change invalidates it.
        return original;
    }
    GrShape result;
    if (original.fInheritedPathForListeners.isValid()) {
        result.fInheritedPathForListeners.set(*original.fInheritedPathForListeners.get());
    }
    switch (original.fType) {
        case Type::kRRect:
            result.fType = original.fType;
            result.fRRectData.fRRect = original.fRRectData.fRRect;
            result.fRRectData.fDir = kDefaultRRectDir;
            result.fRRectData.fStart = kDefaultRRectStart;
            result.fRRectData.fInverted = is_inverted(original.fRRectData.fInverted, inversion);
            break;
        case Type::kArc:
            result.fType = original.fType;
            result.fArcData.fOval = original.fArcData.fOval;
            result.fArcData.fStartAngleDegrees = original.fArcData.fStartAngleDegrees;
            result.fArcData.fSweepAngleDegrees = original.fArcData.fSweepAngleDegrees;
            result.fArcData.fUseCenter = original.fArcData.fUseCenter;
            result.fArcData.fInverted = is_inverted(original.fArcData.fInverted, inversion);
            break;
        case Type::kLine:
            // Lines don't fill.
            if (is_inverted(original.fLineData.fInverted, inversion)) {
                result.fType = Type::kInvertedEmpty;
            } else {
                result.fType = Type::kEmpty;
            }
            break;
        case Type::kEmpty:
            result.fType = is_inverted(false, inversion) ? Type::kInvertedEmpty :  Type::kEmpty;
            break;
        case Type::kInvertedEmpty:
            result.fType = is_inverted(true, inversion) ? Type::kInvertedEmpty :  Type::kEmpty;
            break;
        case Type::kPath:
            result.initType(Type::kPath, &original.fPathData.fPath);
            result.fPathData.fGenID = original.fPathData.fGenID;
            if (flip_inversion(original.fPathData.fPath.isInverseFillType(), inversion)) {
                result.fPathData.fPath.toggleInverseFillType();
            }
            if (!original.style().isSimpleFill()) {
                // Going from a non-filled style to fill may allow additional simplifications (e.g.
                // closing an open rect that wasn't closed in the original shape because it had
                // stroke style).
                result.attemptToSimplifyPath();
            }
            break;
    }
    // We don't copy the inherited key since it can contain path effect information that we just
    // stripped.
    return result;
}

SkRect GrShape::bounds() const {
    // Bounds where left == bottom or top == right can indicate a line or point shape. We return
    // inverted bounds for a truly empty shape.
    static constexpr SkRect kInverted = SkRect::MakeLTRB(1, 1, -1, -1);
    switch (fType) {
        case Type::kEmpty:
            return kInverted;
        case Type::kInvertedEmpty:
            return kInverted;
        case Type::kLine: {
            SkRect bounds;
            if (fLineData.fPts[0].fX < fLineData.fPts[1].fX) {
                bounds.fLeft = fLineData.fPts[0].fX;
                bounds.fRight = fLineData.fPts[1].fX;
            } else {
                bounds.fLeft = fLineData.fPts[1].fX;
                bounds.fRight = fLineData.fPts[0].fX;
            }
            if (fLineData.fPts[0].fY < fLineData.fPts[1].fY) {
                bounds.fTop = fLineData.fPts[0].fY;
                bounds.fBottom = fLineData.fPts[1].fY;
            } else {
                bounds.fTop = fLineData.fPts[1].fY;
                bounds.fBottom = fLineData.fPts[0].fY;
            }
            return bounds;
        }
        case Type::kRRect:
            return fRRectData.fRRect.getBounds();
        case Type::kArc:
            // Could make this less conservative by looking at angles.
            return fArcData.fOval;
        case Type::kPath:
            return this->path().getBounds();
    }
    SK_ABORT("Unknown shape type");
    return kInverted;
}

SkRect GrShape::styledBounds() const {
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
    if (verbCnt > GrShape::kMaxKeyFromDataVerbCnt) {
        return -1;
    }
    const int pointCnt = path.countPoints();
    const int conicWeightCnt = SkPathPriv::ConicWeightCnt(path);

    GR_STATIC_ASSERT(sizeof(SkPoint) == 2 * sizeof(uint32_t));
    GR_STATIC_ASSERT(sizeof(SkScalar) == sizeof(uint32_t));
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
    SkASSERT(verbCnt <= GrShape::kMaxKeyFromDataVerbCnt);
    SkASSERT(pointCnt && verbCnt);
    *key++ = path.getFillType();
    *key++ = verbCnt;
    memcpy(key, SkPathPriv::VerbData(path), verbCnt * sizeof(uint8_t));
    int verbKeySize = SkAlign4(verbCnt);
    // pad out to uint32_t alignment using value that will stand out when debugging.
    uint8_t* pad = reinterpret_cast<uint8_t*>(key)+ verbCnt;
    memset(pad, 0xDE, verbKeySize - verbCnt);
    key += verbKeySize >> 2;

    memcpy(key, SkPathPriv::PointData(path), sizeof(SkPoint) * pointCnt);
    GR_STATIC_ASSERT(sizeof(SkPoint) == 2 * sizeof(uint32_t));
    key += 2 * pointCnt;
    sk_careful_memcpy(key, SkPathPriv::ConicWeightData(path), sizeof(SkScalar) * conicWeightCnt);
    GR_STATIC_ASSERT(sizeof(SkScalar) == sizeof(uint32_t));
    SkDEBUGCODE(key += conicWeightCnt);
    SkASSERT(key - origKey == path_key_from_data_size(path));
}

int GrShape::unstyledKeySize() const {
    if (fInheritedKey.count()) {
        return fInheritedKey.count();
    }
    switch (fType) {
        case Type::kEmpty:
            return 1;
        case Type::kInvertedEmpty:
            return 1;
        case Type::kRRect:
            SkASSERT(!fInheritedKey.count());
            GR_STATIC_ASSERT(0 == SkRRect::kSizeInMemory % sizeof(uint32_t));
            // + 1 for the direction, start index, and inverseness.
            return SkRRect::kSizeInMemory / sizeof(uint32_t) + 1;
        case Type::kArc:
            SkASSERT(!fInheritedKey.count());
            GR_STATIC_ASSERT(0 == sizeof(fArcData) % sizeof(uint32_t));
            return sizeof(fArcData) / sizeof(uint32_t);
        case Type::kLine:
            GR_STATIC_ASSERT(2 * sizeof(uint32_t) == sizeof(SkPoint));
            // 4 for the end points and 1 for the inverseness
            return 5;
        case Type::kPath: {
            if (0 == fPathData.fGenID) {
                return -1;
            }
            int dataKeySize = path_key_from_data_size(fPathData.fPath);
            if (dataKeySize >= 0) {
                return dataKeySize;
            }
            // The key is the path ID and fill type.
            return 2;
        }
    }
    SK_ABORT("Should never get here.");
    return 0;
}

void GrShape::writeUnstyledKey(uint32_t* key) const {
    SkASSERT(this->unstyledKeySize());
    SkDEBUGCODE(uint32_t* origKey = key;)
    if (fInheritedKey.count()) {
        memcpy(key, fInheritedKey.get(), sizeof(uint32_t) * fInheritedKey.count());
        SkDEBUGCODE(key += fInheritedKey.count();)
    } else {
        switch (fType) {
            case Type::kEmpty:
                *key++ = 1;
                break;
            case Type::kInvertedEmpty:
                *key++ = 2;
                break;
            case Type::kRRect:
                fRRectData.fRRect.writeToMemory(key);
                key += SkRRect::kSizeInMemory / sizeof(uint32_t);
                *key = (fRRectData.fDir == SkPath::kCCW_Direction) ? (1 << 31) : 0;
                *key |= fRRectData.fInverted ? (1 << 30) : 0;
                *key++ |= fRRectData.fStart;
                SkASSERT(fRRectData.fStart < 8);
                break;
            case Type::kArc:
                memcpy(key, &fArcData, sizeof(fArcData));
                key += sizeof(fArcData) / sizeof(uint32_t);
                break;
            case Type::kLine:
                memcpy(key, fLineData.fPts, 2 * sizeof(SkPoint));
                key += 4;
                *key++ = fLineData.fInverted ? 1 : 0;
                break;
            case Type::kPath: {
                SkASSERT(fPathData.fGenID);
                int dataKeySize = path_key_from_data_size(fPathData.fPath);
                if (dataKeySize >= 0) {
                    write_path_key_from_data(fPathData.fPath, key);
                    return;
                }
                *key++ = fPathData.fGenID;
                // We could canonicalize the fill rule for paths that don't differentiate between
                // even/odd or winding fill (e.g. convex).
                *key++ = this->path().getFillType();
                break;
            }
        }
    }
    SkASSERT(key - origKey == this->unstyledKeySize());
}

void GrShape::setInheritedKey(const GrShape &parent, GrStyle::Apply apply, SkScalar scale) {
    SkASSERT(!fInheritedKey.count());
    // If the output shape turns out to be simple, then we will just use its geometric key
    if (Type::kPath == fType) {
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
                fPathData.fGenID = 0;
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
            fPathData.fGenID = 0;
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

const SkPath* GrShape::originalPathForListeners() const {
    if (fInheritedPathForListeners.isValid()) {
        return fInheritedPathForListeners.get();
    } else if (Type::kPath == fType && !fPathData.fPath.isVolatile()) {
        return &fPathData.fPath;
    }
    return nullptr;
}

void GrShape::addGenIDChangeListener(sk_sp<SkPathRef::GenIDChangeListener> listener) const {
    if (const auto* lp = this->originalPathForListeners()) {
        SkPathPriv::AddGenIDChangeListener(*lp, std::move(listener));
    }
}

GrShape GrShape::MakeArc(const SkRect& oval, SkScalar startAngleDegrees, SkScalar sweepAngleDegrees,
                         bool useCenter, const GrStyle& style) {
    GrShape result;
    result.changeType(Type::kArc);
    result.fArcData.fOval = oval;
    result.fArcData.fStartAngleDegrees = startAngleDegrees;
    result.fArcData.fSweepAngleDegrees = sweepAngleDegrees;
    result.fArcData.fUseCenter = useCenter;
    result.fArcData.fInverted = false;
    result.fStyle = style;
    result.attemptToSimplifyArc();
    return result;
}

GrShape::GrShape(const GrShape& that) : fStyle(that.fStyle) {
    const SkPath* thatPath = Type::kPath == that.fType ? &that.fPathData.fPath : nullptr;
    this->initType(that.fType, thatPath);
    switch (fType) {
        case Type::kEmpty:
            break;
        case Type::kInvertedEmpty:
            break;
        case Type::kRRect:
            fRRectData = that.fRRectData;
            break;
        case Type::kArc:
            fArcData = that.fArcData;
            break;
        case Type::kLine:
            fLineData = that.fLineData;
            break;
        case Type::kPath:
            fPathData.fGenID = that.fPathData.fGenID;
            break;
    }
    fInheritedKey.reset(that.fInheritedKey.count());
    sk_careful_memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
                      sizeof(uint32_t) * fInheritedKey.count());
    if (that.fInheritedPathForListeners.isValid()) {
        fInheritedPathForListeners.set(*that.fInheritedPathForListeners.get());
    }
}

GrShape::GrShape(const GrShape& parent, GrStyle::Apply apply, SkScalar scale) {
    // TODO: Add some quantization of scale for better cache performance here or leave that up
    // to caller?
    // TODO: For certain shapes and stroke params we could ignore the scale. (e.g. miter or bevel
    // stroke of a rect).
    if (!parent.style().applies() ||
        (GrStyle::Apply::kPathEffectOnly == apply && !parent.style().pathEffect())) {
        this->initType(Type::kEmpty);
        *this = parent;
        return;
    }

    SkPathEffect* pe = parent.fStyle.pathEffect();
    SkTLazy<SkPath> tmpPath;
    const GrShape* parentForKey = &parent;
    SkTLazy<GrShape> tmpParent;
    this->initType(Type::kPath);
    fPathData.fGenID = 0;
    if (pe) {
        const SkPath* srcForPathEffect;
        if (parent.fType == Type::kPath) {
            srcForPathEffect = &parent.path();
        } else {
            srcForPathEffect = tmpPath.init();
            parent.asPath(tmpPath.get());
        }
        // Should we consider bounds? Would have to include in key, but it'd be nice to know
        // if the bounds actually modified anything before including in key.
        SkStrokeRec strokeRec = parent.fStyle.strokeRec();
        if (!parent.fStyle.applyPathEffectToPath(&this->path(), &strokeRec, *srcForPathEffect,
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
            tmpParent.init(this->path(), GrStyle(strokeRec, nullptr));
            tmpParent.get()->setInheritedKey(parent, GrStyle::Apply::kPathEffectOnly, scale);
            if (!tmpPath.isValid()) {
                tmpPath.init();
            }
            tmpParent.get()->asPath(tmpPath.get());
            SkStrokeRec::InitStyle fillOrHairline;
            // The parent shape may have simplified away the strokeRec, check for that here.
            if (tmpParent.get()->style().applies()) {
                SkAssertResult(tmpParent.get()->style().applyToPath(&this->path(), &fillOrHairline,
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
        if (parent.fType == Type::kPath) {
            srcForParentStyle = &parent.path();
        } else {
            srcForParentStyle = tmpPath.init();
            parent.asPath(tmpPath.get());
        }
        SkStrokeRec::InitStyle fillOrHairline;
        SkASSERT(parent.fStyle.applies());
        SkASSERT(!parent.fStyle.pathEffect());
        SkAssertResult(parent.fStyle.applyToPath(&this->path(), &fillOrHairline, *srcForParentStyle,
                                                 scale));
        fStyle.resetToInitStyle(fillOrHairline);
    }
    if (parent.fInheritedPathForListeners.isValid()) {
        fInheritedPathForListeners.set(*parent.fInheritedPathForListeners.get());
    } else if (Type::kPath == parent.fType && !parent.fPathData.fPath.isVolatile()) {
        fInheritedPathForListeners.set(parent.fPathData.fPath);
    }
    this->attemptToSimplifyPath();
    this->setInheritedKey(*parentForKey, apply, scale);
}

void GrShape::attemptToSimplifyPath() {
    SkRect rect;
    SkRRect rrect;
    SkPath::Direction rrectDir;
    unsigned rrectStart;
    bool inverted = this->path().isInverseFillType();
    SkPoint pts[2];
    if (this->path().isEmpty()) {
        // Dashing ignores inverseness skbug.com/5421.
        this->changeType(inverted && !this->style().isDashed() ? Type::kInvertedEmpty
                                                               : Type::kEmpty);
    } else if (this->path().isLine(pts)) {
        this->changeType(Type::kLine);
        fLineData.fPts[0] = pts[0];
        fLineData.fPts[1] = pts[1];
        fLineData.fInverted = inverted;
    } else if (SkPathPriv::IsRRect(this->path(), &rrect, &rrectDir, &rrectStart)) {
        this->changeType(Type::kRRect);
        fRRectData.fRRect = rrect;
        fRRectData.fDir = rrectDir;
        fRRectData.fStart = rrectStart;
        fRRectData.fInverted = inverted;
        SkASSERT(!fRRectData.fRRect.isEmpty());
    } else if (SkPathPriv::IsOval(this->path(), &rect, &rrectDir, &rrectStart)) {
        this->changeType(Type::kRRect);
        fRRectData.fRRect.setOval(rect);
        fRRectData.fDir = rrectDir;
        fRRectData.fInverted = inverted;
        // convert from oval indexing to rrect indexiing.
        fRRectData.fStart = 2 * rrectStart;
    } else if (SkPathPriv::IsSimpleClosedRect(this->path(), &rect, &rrectDir, &rrectStart)) {
        this->changeType(Type::kRRect);
        // When there is a path effect we restrict rect detection to the narrower API that
        // gives us the starting position. Otherwise, we will retry with the more aggressive
        // isRect().
        fRRectData.fRRect.setRect(rect);
        fRRectData.fInverted = inverted;
        fRRectData.fDir = rrectDir;
        // convert from rect indexing to rrect indexiing.
        fRRectData.fStart = 2 * rrectStart;
    } else if (!this->style().hasPathEffect()) {
        bool closed;
        if (this->path().isRect(&rect, &closed, nullptr)) {
            if (closed || this->style().isSimpleFill()) {
                this->changeType(Type::kRRect);
                fRRectData.fRRect.setRect(rect);
                // Since there is no path effect the dir and start index is immaterial.
                fRRectData.fDir = kDefaultRRectDir;
                fRRectData.fStart = kDefaultRRectStart;
                // There isn't dashing so we will have to preserver inverseness.
                fRRectData.fInverted = inverted;
            }
        }
    }
    if (Type::kPath != fType) {
        fInheritedKey.reset(0);
        // Whenever we simplify to a non-path, break the chain so we no longer refer to the
        // original path. This prevents attaching genID listeners to temporary paths created when
        // drawing simple shapes.
        fInheritedPathForListeners.reset();
        if (Type::kRRect == fType) {
            this->attemptToSimplifyRRect();
        } else if (Type::kLine == fType) {
            this->attemptToSimplifyLine();
        }
    } else {
        if (fInheritedKey.count() || this->path().isVolatile()) {
            fPathData.fGenID = 0;
        } else {
            fPathData.fGenID = this->path().getGenerationID();
        }
        if (!this->style().hasNonDashPathEffect()) {
            if (this->style().strokeRec().getStyle() == SkStrokeRec::kStroke_Style ||
                this->style().strokeRec().getStyle() == SkStrokeRec::kHairline_Style) {
                // Stroke styles don't differentiate between winding and even/odd.
                // Moreover, dashing ignores inverseness (skbug.com/5421)
                bool inverse = !this->style().isDashed() && this->path().isInverseFillType();
                if (inverse) {
                    this->path().setFillType(kDefaultPathInverseFillType);
                } else {
                    this->path().setFillType(kDefaultPathFillType);
                }
            } else if (this->path().isConvex()) {
                // There is no distinction between even/odd and non-zero winding count for convex
                // paths.
                if (this->path().isInverseFillType()) {
                    this->path().setFillType(kDefaultPathInverseFillType);
                } else {
                    this->path().setFillType(kDefaultPathFillType);
                }
            }
        }
    }
}

void GrShape::attemptToSimplifyRRect() {
    SkASSERT(Type::kRRect == fType);
    SkASSERT(!fInheritedKey.count());
    if (fRRectData.fRRect.isEmpty()) {
        // An empty filled rrect is equivalent to a filled empty path with inversion preserved.
        if (fStyle.isSimpleFill()) {
            fType = fRRectData.fInverted ? Type::kInvertedEmpty : Type::kEmpty;
            fStyle = GrStyle::SimpleFill();
            return;
        }
        // Dashing a rrect with no width or height is equivalent to filling an emtpy path.
        // When skbug.com/7387 is fixed this should be modified or removed as a dashed zero length
        // line  will produce cap geometry if the effect begins in an "on" interval.
        if (fStyle.isDashed() && !fRRectData.fRRect.width() && !fRRectData.fRRect.height()) {
            // Dashing ignores the inverseness (currently). skbug.com/5421.
            fType = Type::kEmpty;
            fStyle = GrStyle::SimpleFill();
            return;
        }
    }
    if (!this->style().hasPathEffect()) {
        fRRectData.fDir = kDefaultRRectDir;
        fRRectData.fStart = kDefaultRRectStart;
    } else if (fStyle.isDashed()) {
        // Dashing ignores the inverseness (currently). skbug.com/5421
        fRRectData.fInverted = false;
        // Possible TODO here: Check whether the dash results in a single arc or line.
    }
    // Turn a stroke-and-filled miter rect into a filled rect. TODO: more rrect stroke shortcuts.
    if (!fStyle.hasPathEffect() &&
        fStyle.strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style &&
        fStyle.strokeRec().getJoin() == SkPaint::kMiter_Join &&
        fStyle.strokeRec().getMiter() >= SK_ScalarSqrt2 &&
        fRRectData.fRRect.isRect()) {
        SkScalar r = fStyle.strokeRec().getWidth() / 2;
        fRRectData.fRRect = SkRRect::MakeRect(fRRectData.fRRect.rect().makeOutset(r, r));
        fStyle = GrStyle::SimpleFill();
    }
}

void GrShape::attemptToSimplifyLine() {
    SkASSERT(Type::kLine == fType);
    SkASSERT(!fInheritedKey.count());
    if (fStyle.isDashed()) {
        bool allOffsZero = true;
        for (int i = 1; i < fStyle.dashIntervalCnt() && allOffsZero; i += 2) {
            allOffsZero = !fStyle.dashIntervals()[i];
        }
        if (allOffsZero && this->attemptToSimplifyStrokedLineToRRect()) {
            return;
        }
        // Dashing ignores inverseness.
        fLineData.fInverted = false;
        return;
    } else if (fStyle.hasPathEffect()) {
        return;
    }
    if (fStyle.strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style) {
        // Make stroke + fill be stroke since the fill is empty.
        SkStrokeRec rec = fStyle.strokeRec();
        rec.setStrokeStyle(fStyle.strokeRec().getWidth(), false);
        fStyle = GrStyle(rec, nullptr);
    }
    if (fStyle.isSimpleFill()) {
        this->changeType(fLineData.fInverted ? Type::kInvertedEmpty : Type::kEmpty);
        return;
    }
    if (fStyle.strokeRec().getStyle() == SkStrokeRec::kStroke_Style &&
        this->attemptToSimplifyStrokedLineToRRect()) {
        return;
    }
    // Only path effects could care about the order of the points. Otherwise canonicalize
    // the point order.
    SkPoint* pts = fLineData.fPts;
    if (pts[1].fY < pts[0].fY || (pts[1].fY == pts[0].fY && pts[1].fX < pts[0].fX)) {
        using std::swap;
        swap(pts[0], pts[1]);
    }
}

void GrShape::attemptToSimplifyArc() {
    SkASSERT(fType == Type::kArc);
    SkASSERT(!fArcData.fInverted);
    if (fArcData.fOval.isEmpty() || !fArcData.fSweepAngleDegrees) {
        this->changeType(Type::kEmpty);
        return;
    }

    // Assuming no path effect, a filled, stroked, hairline, or stroke-and-filled arc that traverses
    // the full circle and doesn't use the center point is an oval. Unless it has square or round
    // caps. They may protrude out of the oval. Round caps can't protrude out of a circle but we're
    // ignoring that for now.
    if (fStyle.isSimpleFill() || (!fStyle.pathEffect() && !fArcData.fUseCenter &&
                                  fStyle.strokeRec().getCap() == SkPaint::kButt_Cap)) {
        if (fArcData.fSweepAngleDegrees >= 360.f || fArcData.fSweepAngleDegrees <= -360.f) {
            auto oval = fArcData.fOval;
            this->changeType(Type::kRRect);
            this->fRRectData.fRRect.setOval(oval);
            this->fRRectData.fDir = kDefaultRRectDir;
            this->fRRectData.fStart = kDefaultRRectStart;
            this->fRRectData.fInverted = false;
            return;
        }
    }
    if (!fStyle.pathEffect()) {
        // Canonicalize the arc such that the start is always in [0, 360) and the sweep is always
        // positive.
        if (fArcData.fSweepAngleDegrees < 0) {
            fArcData.fStartAngleDegrees = fArcData.fStartAngleDegrees + fArcData.fSweepAngleDegrees;
            fArcData.fSweepAngleDegrees = -fArcData.fSweepAngleDegrees;
        }
    }
    if (this->fArcData.fStartAngleDegrees < 0 || this->fArcData.fStartAngleDegrees >= 360.f) {
        this->fArcData.fStartAngleDegrees = SkScalarMod(this->fArcData.fStartAngleDegrees, 360.f);
    }
    // Possible TODOs here: Look at whether dash pattern results in a single dash and convert to
    // non-dashed stroke. Stroke and fill can be fill if circular and no path effect. Just stroke
    // could as well if the stroke fills the center.
}

bool GrShape::attemptToSimplifyStrokedLineToRRect() {
    SkASSERT(Type::kLine == fType);
    SkASSERT(fStyle.strokeRec().getStyle() == SkStrokeRec::kStroke_Style);

    SkRect rect;
    SkVector outset;
    // If we allowed a rotation angle for rrects we could capture all cases here.
    if (fLineData.fPts[0].fY == fLineData.fPts[1].fY) {
        rect.fLeft = SkTMin(fLineData.fPts[0].fX, fLineData.fPts[1].fX);
        rect.fRight = SkTMax(fLineData.fPts[0].fX, fLineData.fPts[1].fX);
        rect.fTop = rect.fBottom = fLineData.fPts[0].fY;
        outset.fY = fStyle.strokeRec().getWidth() / 2.f;
        outset.fX = SkPaint::kButt_Cap == fStyle.strokeRec().getCap() ? 0.f : outset.fY;
    } else if (fLineData.fPts[0].fX == fLineData.fPts[1].fX) {
        rect.fTop = SkTMin(fLineData.fPts[0].fY, fLineData.fPts[1].fY);
        rect.fBottom = SkTMax(fLineData.fPts[0].fY, fLineData.fPts[1].fY);
        rect.fLeft = rect.fRight = fLineData.fPts[0].fX;
        outset.fX = fStyle.strokeRec().getWidth() / 2.f;
        outset.fY = SkPaint::kButt_Cap == fStyle.strokeRec().getCap() ? 0.f : outset.fX;
    } else {
        return false;
    }
    rect.outset(outset.fX, outset.fY);
    if (rect.isEmpty()) {
        this->changeType(Type::kEmpty);
        fStyle = GrStyle::SimpleFill();
        return true;
    }
    SkRRect rrect;
    if (fStyle.strokeRec().getCap() == SkPaint::kRound_Cap) {
        SkASSERT(outset.fX == outset.fY);
        rrect = SkRRect::MakeRectXY(rect, outset.fX, outset.fY);
    } else {
        rrect = SkRRect::MakeRect(rect);
    }
    bool inverted = fLineData.fInverted && !fStyle.hasPathEffect();
    this->changeType(Type::kRRect);
    fRRectData.fRRect = rrect;
    fRRectData.fInverted = inverted;
    fRRectData.fDir = kDefaultRRectDir;
    fRRectData.fStart = kDefaultRRectStart;
    fStyle = GrStyle::SimpleFill();
    return true;
}
