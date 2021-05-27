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
    fShape      = that.fShape;
    fStyle      = that.fStyle;
    fGenID      = that.fGenID;
    fSimplified = that.fSimplified;

    fInheritedKey.reset(that.fInheritedKey.count());
    sk_careful_memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
                      sizeof(uint32_t) * fInheritedKey.count());
    if (that.fInheritedPathForListeners.isValid()) {
        fInheritedPathForListeners.set(*that.fInheritedPathForListeners);
    } else {
        fInheritedPathForListeners.reset();
    }
    return *this;
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
    bool newIsInverted = is_inverted(original.fShape.inverted(), inversion);
    if (original.style().isSimpleFill() && newIsInverted == original.fShape.inverted()) {
        // By returning the original rather than falling through we can preserve any inherited style
        // key. Otherwise, we wipe it out below since the style change invalidates it.
        return original;
    }
    GrStyledShape result;
    SkASSERT(result.fStyle.isSimpleFill());
    if (original.fInheritedPathForListeners.isValid()) {
        result.fInheritedPathForListeners.set(*original.fInheritedPathForListeners);
    }

    result.fShape = original.fShape;
    result.fGenID = original.fGenID;
    result.fShape.setInverted(newIsInverted);

    if (!original.style().isSimpleFill()) {
        // Going from a non-filled style to fill may allow additional simplifications (e.g.
        // closing an open rect that wasn't closed in the original shape because it had
        // stroke style).
        result.simplify();
        // The above simplify() call only sets simplified to true if its geometry was changed,
        // since it already sees its style as a simple fill. Since the original style was not a
        // simple fill, MakeFilled always simplifies.
        result.fSimplified = true;
    }

    // Verify that lines/points were converted to empty by the style change
    SkASSERT((!original.fShape.isLine() && !original.fShape.isPoint()) || result.fShape.isEmpty());

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
    // 1 is for the verb count. Each verb is a byte but we'll pad the verb data out to
    // a uint32_t length.
    return 1 + (SkAlign4(verbCnt) >> 2) + 2 * pointCnt + conicWeightCnt;
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

    int count = 1; // Every key has the state flags from the GrShape
    switch(fShape.type()) {
        case GrShape::Type::kPoint:
            static_assert(0 == sizeof(SkPoint) % sizeof(uint32_t));
            count += sizeof(SkPoint) / sizeof(uint32_t);
            break;
        case GrShape::Type::kRect:
            static_assert(0 == sizeof(SkRect) % sizeof(uint32_t));
            count += sizeof(SkRect) / sizeof(uint32_t);
            break;
        case GrShape::Type::kRRect:
            static_assert(0 == SkRRect::kSizeInMemory % sizeof(uint32_t));
            count += SkRRect::kSizeInMemory / sizeof(uint32_t);
            break;
        case GrShape::Type::kArc:
            static_assert(0 == sizeof(GrArc) % sizeof(uint32_t));
            count += sizeof(GrArc) / sizeof(uint32_t);
            break;
        case GrShape::Type::kLine:
            static_assert(0 == sizeof(GrLineSegment) % sizeof(uint32_t));
            count += sizeof(GrLineSegment) / sizeof(uint32_t);
            break;
        case GrShape::Type::kPath: {
            if (0 == fGenID) {
                return -1; // volatile, so won't be keyed
            }
            int dataKeySize = path_key_from_data_size(fShape.path());
            if (dataKeySize >= 0) {
                count += dataKeySize;
            } else {
                count++; // Just adds the gen ID.
            }
            break; }
        default:
            // else it's empty, which just needs the state flags for its key
            SkASSERT(fShape.isEmpty());
    }
    return count;
}

void GrStyledShape::writeUnstyledKey(uint32_t* key) const {
    SkASSERT(this->unstyledKeySize());
    SkDEBUGCODE(uint32_t* origKey = key;)
    if (fInheritedKey.count()) {
        memcpy(key, fInheritedKey.get(), sizeof(uint32_t) * fInheritedKey.count());
        SkDEBUGCODE(key += fInheritedKey.count();)
    } else {
        // Dir and start are only used for rect and rrect shapes, so are not included in other
        // shape type keys. Make sure that they are the defaults for other shapes so it doesn't
        // matter that we universally include them in the flag key value.
        SkASSERT((fShape.isRect() || fShape.isRRect()) ||
                 (fShape.dir() == GrShape::kDefaultDir &&
                  fShape.startIndex() == GrShape::kDefaultStart));

        // Every key starts with the state from the GrShape (this includes path fill type,
        // and any tracked winding, start, inversion, as well as the class of geometry).
        *key++ = fShape.stateKey();

        switch(fShape.type()) {
            case GrShape::Type::kPath: {
                SkASSERT(fGenID != 0);
                // Ensure that the path's inversion matches our state so that the path's key suffices.
                SkASSERT(fShape.inverted() == fShape.path().isInverseFillType());

                int dataKeySize = path_key_from_data_size(fShape.path());
                if (dataKeySize >= 0) {
                    write_path_key_from_data(fShape.path(), key);
                    return;
                } else {
                    *key++ = fGenID;
                }
                break; }
            case GrShape::Type::kPoint:
                memcpy(key, &fShape.point(), sizeof(SkPoint));
                key += sizeof(SkPoint) / sizeof(uint32_t);
                break;
            case GrShape::Type::kRect:
                memcpy(key, &fShape.rect(), sizeof(SkRect));
                key += sizeof(SkRect) / sizeof(uint32_t);
                break;
            case GrShape::Type::kRRect:
                fShape.rrect().writeToMemory(key);
                key += SkRRect::kSizeInMemory / sizeof(uint32_t);
                break;
            case GrShape::Type::kArc:
                // Write dense floats first
                memcpy(key, &fShape.arc(), sizeof(SkRect) + 2 * sizeof(float));
                key += (sizeof(GrArc) / sizeof(uint32_t) - 1);
                // Then write the final bool as an int, to make sure upper bits are set
                *key++ = fShape.arc().fUseCenter ? 1 : 0;
                break;
            case GrShape::Type::kLine:
                memcpy(key, &fShape.line(), sizeof(GrLineSegment));
                key += sizeof(GrLineSegment) / sizeof(uint32_t);
                break;
            default:
                // Nothing other than the flag state is needed in the key for an empty shape
                SkASSERT(fShape.isEmpty());
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
                                     const GrStyle& style, DoSimplify doSimplify) {
    GrStyledShape result;
    result.fShape.setArc({oval.makeSorted(), startAngleDegrees, sweepAngleDegrees, useCenter});
    result.fStyle = style;
    if (doSimplify == DoSimplify::kYes) {
        result.simplify();
    }
    return result;
}

GrStyledShape::GrStyledShape(const GrStyledShape& that)
        : fShape(that.fShape)
        , fStyle(that.fStyle)
        , fGenID(that.fGenID)
        , fSimplified(that.fSimplified) {
    fInheritedKey.reset(that.fInheritedKey.count());
    sk_careful_memcpy(fInheritedKey.get(), that.fInheritedKey.get(),
                      sizeof(uint32_t) * fInheritedKey.count());
    if (that.fInheritedPathForListeners.isValid()) {
        fInheritedPathForListeners.set(*that.fInheritedPathForListeners);
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
            *this = tmpParent->applyStyle(apply, scale);
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
            tmpParent->setInheritedKey(parent, GrStyle::Apply::kPathEffectOnly, scale);
            if (!tmpPath.isValid()) {
                tmpPath.init();
            }
            tmpParent->asPath(tmpPath.get());
            SkStrokeRec::InitStyle fillOrHairline;
            // The parent shape may have simplified away the strokeRec, check for that here.
            if (tmpParent->style().applies()) {
                SkAssertResult(tmpParent.get()->style().applyToPath(&fShape.path(), &fillOrHairline,
                                                                    *tmpPath.get(), scale));
            } else if (tmpParent->style().isSimpleFill()) {
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

    if (parent.fInheritedPathForListeners.isValid()) {
        fInheritedPathForListeners.set(*parent.fInheritedPathForListeners);
    } else if (parent.fShape.isPath() && !parent.fShape.path().isVolatile()) {
        fInheritedPathForListeners.set(parent.fShape.path());
    }
    this->simplify();
    this->setInheritedKey(*parentForKey, apply, scale);
}

bool GrStyledShape::asRRect(SkRRect* rrect, SkPathDirection* dir, unsigned* start,
                            bool* inverted) const {
    if (!fShape.isRRect() && !fShape.isRect()) {
        return false;
    }

    // Validity check here, if we don't have a path effect on the style, we should have passed
    // appropriate flags to GrShape::simplify() to have reset these parameters.
    SkASSERT(fStyle.hasPathEffect() || (fShape.dir() == GrShape::kDefaultDir &&
                                        fShape.startIndex() == GrShape::kDefaultStart));

    // If the shape is a regular rect, map to round rect winding parameters, including accounting
    // for the automatic sorting of edges that SkRRect::MakeRect() performs.
    if (fShape.isRect()) {
        if (rrect) {
            *rrect = SkRRect::MakeRect(fShape.rect());
        }
        // Don't bother mapping these if we don't have a path effect, however.
        if (!fStyle.hasPathEffect()) {
            if (dir) {
                *dir = GrShape::kDefaultDir;
            }
            if (start) {
                *start = GrShape::kDefaultStart;
            }
        } else {
            // In SkPath a rect starts at index 0 by default. This is the top left corner. However,
            // we store rects as rrects. RRects don't preserve the invertedness, but rather sort the
            // rect edges. Thus, we may need to modify the rrect's start index and direction.
            SkPathDirection rectDir = fShape.dir();
            unsigned rectStart = fShape.startIndex();

            if (fShape.rect().fLeft > fShape.rect().fRight) {
                // Toggle direction, and modify index by mapping through the array
                static const unsigned kMapping[] = {1, 0, 3, 2};
                rectDir = rectDir == SkPathDirection::kCCW ? SkPathDirection::kCW
                                                           : SkPathDirection::kCCW;
                rectStart = kMapping[rectStart];
            }
            if (fShape.rect().fTop > fShape.rect().fBottom) {
                // Toggle direction and map index by 3 - start
                // NOTE: if we earlier flipped for X as well, this results in no net direction
                // change and effectively flipping the start index to the diagonal corners of the
                // rect (matching what we'd expect for a rect with both X and Y flipped).
                rectDir = rectDir == SkPathDirection::kCCW ? SkPathDirection::kCW
                                                           : SkPathDirection::kCCW;
                rectStart = 3 - rectStart;
            }

            if (dir) {
                *dir = rectDir;
            }
            if (start) {
                // Convert to round rect indexing
                *start = 2 * rectStart;
            }
        }
    } else {
        // Straight forward export
        if (rrect) {
            *rrect = fShape.rrect();
        }
        if (dir) {
            *dir = fShape.dir();
        }
        if (start) {
            *start = fShape.startIndex();
            // Canonicalize the index if the rrect is an oval, which GrShape doesn't treat special
            // but we do for dashing placement
            if (fShape.rrect().isOval()) {
                *start &= 0b110;
            }
        }
    }

    if (inverted) {
        *inverted = fShape.inverted();
    }

    return true;
}

bool GrStyledShape::asLine(SkPoint pts[2], bool* inverted) const {
    if (!fShape.isLine()) {
        return false;
    }

    if (pts) {
        pts[0] = fShape.line().fP1;
        pts[1] = fShape.line().fP2;
    }
    if (inverted) {
        *inverted = fShape.inverted();
    }
    return true;
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

class AutoRestoreInverseness {
public:
    AutoRestoreInverseness(GrShape* shape, const GrStyle& style)
            // Dashing ignores inverseness skbug.com/5421.
            : fShape(shape), fInverted(!style.isDashed() && fShape->inverted()) {}

    ~AutoRestoreInverseness() {
        // Restore invertedness after any modifications were made to the shape type
        fShape->setInverted(fInverted);
        SkASSERT(!fShape->isPath() || fInverted == fShape->path().isInverseFillType());
    }

private:
    GrShape* fShape;
    bool fInverted;
};

void GrStyledShape::simplify() {
    AutoRestoreInverseness ari(&fShape, fStyle);

    unsigned simplifyFlags = 0;
    if (fStyle.isSimpleFill()) {
        simplifyFlags = GrShape::kAll_Flags;
    } else if (!fStyle.hasPathEffect()) {
        // Everything but arcs with caps that might extend beyond the oval edge can ignore winding
        if (!fShape.isArc() || fStyle.strokeRec().getCap() == SkPaint::kButt_Cap) {
            simplifyFlags |= GrShape::kIgnoreWinding_Flag;
        }
        simplifyFlags |= GrShape::kMakeCanonical_Flag;
    } // else if there's a path effect, every destructive simplification is disabledd

    // Remember if the original shape was closed; in the event we simplify to a point or line
    // because of degenerate geometry, we need to update joins and caps.
    GrShape::Type oldType = fShape.type();
    fClosed = fShape.simplify(simplifyFlags);
    fSimplified = oldType != fShape.type();

    if (fShape.isPath()) {
        // The shape remains a path, so configure the gen ID and canonicalize fill type if possible
        if (fInheritedKey.count() || fShape.path().isVolatile()) {
            fGenID = 0;
        } else {
            fGenID = fShape.path().getGenerationID();
        }
        if (!fStyle.hasNonDashPathEffect() &&
            (fStyle.strokeRec().getStyle() == SkStrokeRec::kStroke_Style ||
             fStyle.strokeRec().getStyle() == SkStrokeRec::kHairline_Style ||
             fShape.path().isConvex())) {
            // Stroke styles don't differentiate between winding and even/odd. There is no
            // distinction between even/odd and non-zero winding count for convex paths.
            // Moreover, dashing ignores inverseness (skbug.com/5421)
            fShape.path().setFillType(GrShape::kDefaultFillType);
        }
    } else {
        fInheritedKey.reset(0);
        // Whenever we simplify to a non-path, break the chain so we no longer refer to the
        // original path. This prevents attaching genID listeners to temporary paths created when
        // drawing simple shapes.
        fInheritedPathForListeners.reset();
        // Further simplifications to the shape based on the style
        this->simplifyStroke();
    }
}

void GrStyledShape::simplifyStroke() {
    AutoRestoreInverseness ari(&fShape, fStyle);

    // For stroke+filled rects, a mitered shape becomes a larger rect and a rounded shape
    // becomes a round rect.
    if (!fStyle.hasPathEffect() && fShape.isRect() &&
        fStyle.strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style) {
        if (fStyle.strokeRec().getJoin() == SkPaint::kBevel_Join ||
            (fStyle.strokeRec().getJoin() == SkPaint::kMiter_Join &&
             fStyle.strokeRec().getMiter() < SK_ScalarSqrt2)) {
            // Bevel-stroked rect needs path rendering
            return;
        }

        SkScalar r = fStyle.strokeRec().getWidth() / 2;
        fShape.rect().outset(r, r);
        if (fStyle.strokeRec().getJoin() == SkPaint::kRound_Join) {
            // There's no dashing to worry about if we got here, so it's okay that this resets
            // winding parameters
            fShape.setRRect(SkRRect::MakeRectXY(fShape.rect(), r, r));
        }
        fStyle = GrStyle::SimpleFill();
        fSimplified = true;
        return;
    }

    // Otherwise, if we're a point or a line, we might be able to explicitly apply some of the
    // stroking (and even some of the dashing). Any other shape+style is too complicated to reduce.
    if ((!fShape.isPoint() && !fShape.isLine()) || fStyle.hasNonDashPathEffect() ||
        fStyle.strokeRec().isHairlineStyle()) {
        return;
    }

    // Tracks style simplifications, even if the geometry can't be further simplified.
    bool styleSimplified = false;
    if (fStyle.isDashed()) {
        // For dashing a point, if the first interval is on, we can drop the dash and just draw
        // the caps. For dashing a line, if every off interval is 0 length, its a stroke.
        bool dropDash = false;
        if (fShape.isPoint()) {
            dropDash = fStyle.dashIntervalCnt() > 0 &&
                       SkToBool(fStyle.dashIntervals()[0]);
        } else {
            dropDash = true;
            for (int i = 1; i < fStyle.dashIntervalCnt(); i += 2) {
                if (SkToBool(fStyle.dashIntervals()[i])) {
                    // An off interval has non-zero length so this won't convert to a simple line
                    dropDash = false;
                    break;
                }
            }
        }

        if (!dropDash) {
            return;
        }
        // Fall through to modifying the shape to respect the new stroke geometry
        fStyle = GrStyle(fStyle.strokeRec(), nullptr);
        // Since the reduced the line or point after dashing is dependent on the caps of the dashes,
        // we reset to be unclosed so we don't override the style based on joins later.
        fClosed = false;
        styleSimplified = true;
    }

    // At this point, we're a line or point with no path effects. Any fill portion of the style
    // is empty, so a fill-only style can be empty, and a stroke+fill becomes a stroke.
    if (fStyle.isSimpleFill()) {
        fShape.reset();
        fSimplified = true;
        return;
    } else if (fStyle.strokeRec().getStyle() == SkStrokeRec::kStrokeAndFill_Style) {
        // Stroke only
        SkStrokeRec rec = fStyle.strokeRec();
        rec.setStrokeStyle(fStyle.strokeRec().getWidth(), false);
        fStyle = GrStyle(rec, nullptr);
        styleSimplified = true;
    }

    // A point or line that was formed by a degenerate closed shape needs its style updated to
    // reflect the fact that it doesn't actually produce caps.
    if (fClosed) {
        SkPaint::Cap cap;
        if (fShape.isLine() && fStyle.strokeRec().getJoin() == SkPaint::kRound_Join) {
            // As a closed shape, the line moves from a to b and back to a, producing a 180 degree
            // turn. With round joins, this would make a semi-circle at each end, which is visually
            // identical to a round cap on the reduced line geometry.
            cap = SkPaint::kRound_Cap;
        } else {
            // If this were a closed line, the 180 degree turn either is a miter join that exceeds
            // the miter limit and becomes a bevel, or a bevel join. In either case, the bevel shape
            // of a 180 degreen corner is equivalent to a butt cap.
            //  - to match the SVG spec, the 0-length sides of an empty rectangle are skipped, so
            //    it fits this closed line description (it is not two 90 degree turns that could
            //    produce miter geometry).
            cap = SkPaint::kButt_Cap;
        }

        if (cap != fStyle.strokeRec().getCap() ||
            SkPaint::kDefault_Join != fStyle.strokeRec().getJoin()) {
            SkStrokeRec rec = fStyle.strokeRec();
            rec.setStrokeParams(cap, SkPaint::kDefault_Join, fStyle.strokeRec().getMiter());
            fStyle = GrStyle(rec, nullptr);
            styleSimplified = true;
        }
    }

    if (fShape.isPoint()) {
        // The drawn geometry is entirely based on the cap style and stroke width. A butt cap point
        // doesn't draw anything, a round cap is an oval and a square cap is a square.
        if (fStyle.strokeRec().getCap() == SkPaint::kButt_Cap) {
            fShape.reset();
        } else {
            SkScalar w = fStyle.strokeRec().getWidth() / 2.f;
            SkRect r = {fShape.point().fX, fShape.point().fY, fShape.point().fX, fShape.point().fY};
            r.outset(w, w);

            if (fStyle.strokeRec().getCap() == SkPaint::kRound_Cap) {
                fShape.setRRect(SkRRect::MakeOval(r));
            } else {
                fShape.setRect(r);
            }
        }
    } else {
        // Stroked lines reduce to rectangles or round rects when they are axis-aligned. If we
        // allowed rotation angle, this would work for any lines.
        SkRect rect;
        SkVector outset;
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
            // Geometrically can't apply the style and turn into a fill, but might still be simpler
            // than before based solely on changes to fStyle.
            fSimplified |= styleSimplified;
            return;
        }
        rect.outset(outset.fX, outset.fY);
        if (rect.isEmpty()) {
            fShape.reset();
        } else if (fStyle.strokeRec().getCap() == SkPaint::kRound_Cap) {
            SkASSERT(outset.fX == outset.fY);
            fShape.setRRect(SkRRect::MakeRectXY(rect, outset.fX, outset.fY));
        } else {
            fShape.setRect(rect);
        }
    }
    // If we made it here, the stroke was fully applied to the new shape so we can become a fill.
    fStyle = GrStyle::SimpleFill();
    fSimplified = true;
    return;
}
