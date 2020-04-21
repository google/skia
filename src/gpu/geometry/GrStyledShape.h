/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStyledShape_DEFINED
#define GrStyledShape_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/geometry/GrShape.h"
#include <new>

class SkIDChangeListener;

/**
 * Represents a geometric shape (rrect or path) and the GrStyle that it should be rendered with.
 * It is possible to apply the style to the GrStyledShape to produce a new GrStyledShape where the
 * geometry reflects the styling information (e.g. is stroked). It is also possible to apply just
 * the path effect from the style. In this case the resulting shape will include any remaining
 * stroking information that is to be applied after the path effect.
 *
 * Shapes can produce keys that represent only the geometry information, not the style. Note that
 * when styling information is applied to produce a new shape then the style has been converted
 * to geometric information and is included in the new shape's key. When the same style is applied
 * to two shapes that reflect the same underlying geometry the computed keys of the stylized shapes
 * will be the same.
 *
 * Currently this can only be constructed from a path, rect, or rrect though it can become a path
 * applying style to the geometry. The idea is to expand this to cover most or all of the geometries
 * that have fast paths in the GPU backend.
 */
class GrStyledShape {
public:
    // Keys for paths may be extracted from the path data for small paths. Clients aren't supposed
    // to have to worry about this. This value is exposed for unit tests.
    static constexpr int kMaxKeyFromDataVerbCnt = 10;

    GrStyledShape() {}

    explicit GrStyledShape(const SkPath& path) : GrStyledShape(path, GrStyle::SimpleFill()) {}

    explicit GrStyledShape(const SkRRect& rrect) : GrStyledShape(rrect, GrStyle::SimpleFill()) {}

    explicit GrStyledShape(const SkRect& rect) : GrStyledShape(rect, GrStyle::SimpleFill()) {}

    GrStyledShape(const SkPath& path, const SkPaint& paint) : GrStyledShape(path, GrStyle(paint)) {}

    GrStyledShape(const SkRRect& rrect, const SkPaint& paint)
            : GrStyledShape(rrect, GrStyle(paint)) {}

    GrStyledShape(const SkRect& rect, const SkPaint& paint) : GrStyledShape(rect, GrStyle(paint)) {}

    GrStyledShape(const SkPath& path, const GrStyle& style)
            : fShape(path)
            , fInverted(path.isInverseFillType())
            , fStyle(style) {
        this->simplify();
    }

    GrStyledShape(const SkRRect& rrect, const GrStyle& style)
            : fShape(rrect)
            , fStyle(style) {
        fStart = DefaultRRectDirAndStartIndex(rrect, style.hasPathEffect(), &fDir);
        this->simplify();
    }

    GrStyledShape(const SkRRect& rrect, SkPathDirection dir, unsigned start, bool inverted,
                  const GrStyle& style)
            : fShape(rrect)
            , fInverted(inverted)
            , fStyle(style) {
        if (style.hasPathEffect()) {
            fDir = dir;
            if (rrect.isRect()) {
                fStart = (start + 1) & 0b110;
            } else if (rrect.isOval()) {
                fStart = start & 0b110;
            } else {
                fStart = start;
            }
        } else {
            // With no path effect, dir and start index don't matter
            fStart = DefaultRRectDirAndStartIndex(rrect, false, &fDir);
        }
        this->simplify();
    }

    GrStyledShape(const SkRect& rect, const GrStyle& style)
            : fShape(rect)
            , fStyle(style) {
        // Correctly handles rect being unsorted and fixes direction/start as needed
        fStart = DefaultRectDirAndStartIndex(rect, style.hasPathEffect(), &fDir);
        // But update the geometry since GrShape would otherwise consider the shape empty
        if (!rect.isSorted()) {
            fShape.rect().sort();
        }
        this->simplify();
    }

    static GrStyledShape MakeArc(const SkRect& oval, SkScalar startAngleDegrees,
                                 SkScalar sweepAngleDegrees, bool useCenter, const GrStyle& style);

    GrStyledShape(const GrStyledShape&);
    GrStyledShape& operator=(const GrStyledShape& that);

    /**
     * Informs MakeFilled on how to modify that shape's fill rule when making a simple filled
     * version of the shape.
     */
    enum class FillInversion {
        kPreserve,
        kFlip,
        kForceNoninverted,
        kForceInverted
    };
    /**
     * Makes a filled shape from the pre-styled original shape and optionally modifies whether
     * the fill is inverted or not. It's important to note that the original shape's geometry
     * may already have been modified if doing so was neutral with respect to its style
     * (e.g. filled paths are always closed when stored in a shape and dashed paths are always
     * made non-inverted since dashing ignores inverseness).
     */
    static GrStyledShape MakeFilled(const GrStyledShape& original,
                                    FillInversion = FillInversion::kPreserve);

    const GrStyle& style() const { return fStyle; }

    /**
     * Returns a shape that has either applied the path effect or path effect and stroking
     * information from this shape's style to its geometry. Scale is used when approximating the
     * output geometry and typically is computed from the view matrix
     */
    GrStyledShape applyStyle(GrStyle::Apply apply, SkScalar scale) const {
        return GrStyledShape(*this, apply, scale);
    }

    bool isRect() const {
        // Should have simplified a rrect to a rect if possible already.
        SkASSERT(!fShape.isRRect() || !fShape.rrect().isRect());
        return fShape.isRect();
    }

    /** Returns the unstyled geometry as a rrect if possible. */
    bool asRRect(SkRRect* rrect, SkPathDirection* dir, unsigned* start, bool* inverted) const {
        if (!fShape.isRRect() && !fShape.isRect()) {
            return false;
        }
        if (rrect) {
            *rrect = fShape.isRRect() ? fShape.rrect() : SkRRect::MakeRect(fShape.rect());
        }
        // Even if the shape is a rect, dir and start have been canonicalized to match rrect.
        if (dir) {
            *dir = fDir;
        }
        if (start) {
            *start = fStart;
        }
        if (inverted) {
            *inverted = fInverted;
        }
        return true;
    }

    /**
     * If the unstyled shape is a straight line segment, returns true and sets pts to the endpoints.
     * An inverse filled line path is still considered a line.
     */
    bool asLine(SkPoint pts[2], bool* inverted) const {
        if (!fShape.isLine()) {
            return false;
        }

        if (pts) {
            pts[0] = fShape.line().fP1;
            pts[1] = fShape.line().fP2;
        }
        if (inverted) {
            *inverted = fInverted;
        }
        return true;
    }

    /** Returns the unstyled geometry as a path. */
    void asPath(SkPath* out) const {
        if (fShape.isArc() && this->style().isSimpleFill()) {
            // FIXME maybe shape's asPath() takes a simpleFill arg too?
            SkPathPriv::CreateDrawArcPath(out, fShape.arc().fOval, fShape.arc().fStartAngle, fShape.arc().fSweepAngle, fShape.arc().fUseCenter, true);
        } else {
            fShape.asPath(out);
        }
        // Now factor in the invertedness, assuming the base shape wasn't already a path
        // (since that also remembers invertedness).
        SkASSERT(!fShape.isPath() || fInverted == out->isInverseFillType());
        if (!fShape.isPath()) {
            out->setFillType(fInverted ? kDefaultPathInverseFillType
                                       : kDefaultPathFillType);
        }
    }

    // Can this shape be drawn as a pair of filled nested rectangles?
    bool asNestedRects(SkRect rects[2]) const;

    /**
     * Returns whether the geometry is empty. Note that applying the style could produce a
     * non-empty shape. It also may have an inverse fill.
     */
    bool isEmpty() const { return fShape.isEmpty(); }

    /**
     * Gets the bounds of the geometry without reflecting the shape's styling. This ignores
     * the inverse fill nature of the geometry.
     */
    SkRect bounds() const { return fShape.bounds(); }

    /**
     * Gets the bounds of the geometry reflecting the shape's styling (ignoring inverse fill
     * status).
     */
    SkRect styledBounds() const;

    /**
     * Is this shape known to be convex, before styling is applied. An unclosed but otherwise
     * convex path is considered to be closed if they styling reflects a fill and not otherwise.
     * This is because filling closes all contours in the path.
     */
    bool knownToBeConvex() const {
        return fShape.convex(this->style().isSimpleFill());
    }

    /**
     * Does the shape have a known winding direction. Some degenerate convex shapes may not have
     * a computable direction, but this is not always a requirement for path renderers so it is
     * kept separate from knownToBeConvex().
     */
    bool knownDirection() const {
        // Assuming this is called after knownToBeConvex(), this should just be relying on
        // cached convexity and direction and will be cheap.
        return !fShape.isPath() || !SkPathPriv::CheapIsFirstDirection(
                                           fShape.path(), SkPathPriv::kUnknown_FirstDirection);
    }

    /** Is the pre-styled geometry inverse filled? */
    bool inverseFilled() const {
        // Since the path tracks invertes-fillness itself, it should match what we recorded.
        SkASSERT(!fShape.isPath() || fInverted == fShape.path().isInverseFillType());
        // Dashing ignores inverseness. We should have caught this earlier. skbug.com/5421
        SkASSERT(!(fInverted && this->style().isDashed()));
        return fInverted;
    }

    /**
     * Might applying the styling to the geometry produce an inverse fill. The "may" part comes in
     * because an arbitrary path effect could produce an inverse filled path. In other cases this
     * can be thought of as "inverseFilledAfterStyling()".
     */
    bool mayBeInverseFilledAfterStyling() const {
         // An arbitrary path effect can produce an arbitrary output path, which may be inverse
         // filled.
        if (this->style().hasNonDashPathEffect()) {
            return true;
        }
        return this->inverseFilled();
    }

    /**
     * Is it known that the unstyled geometry has no unclosed contours. This means that it will
     * not have any caps if stroked (modulo the effect of any path effect).
     */
    bool knownToBeClosed() const {
        // This refers to the base shape and does not depend on invertedness.
        return fShape.closed();
    }

    uint32_t segmentMask() const {
        // This refers to the base shape and does not depend on invertedness.
        return fShape.segmentMask();
    }

    /**
     * Gets the size of the key for the shape represented by this GrStyledShape (ignoring its
     * styling). A negative value is returned if the shape has no key (shouldn't be cached).
     */
    int unstyledKeySize() const;

    bool hasUnstyledKey() const { return this->unstyledKeySize() >= 0; }

    /**
     * Writes unstyledKeySize() bytes into the provided pointer. Assumes that there is enough
     * space allocated for the key and that unstyledKeySize() does not return a negative value
     * for this shape.
     */
    void writeUnstyledKey(uint32_t* key) const;

    /**
     * Adds a listener to the *original* path. Typically used to invalidate cached resources when
     * a path is no longer in-use. If the shape started out as something other than a path, this
     * does nothing.
     */
    void addGenIDChangeListener(sk_sp<SkIDChangeListener>) const;

    /**
     * Helpers that are only exposed for unit tests, to determine if the shape is a path, and get
     * the generation ID of the *original* path. This is the path that will receive
     * GenIDChangeListeners added to this shape.
     */
    uint32_t testingOnly_getOriginalGenerationID() const;
    bool testingOnly_isPath() const;
    bool testingOnly_isNonVolatilePath() const;

private:
    /** Constructor used by the applyStyle() function */
    GrStyledShape(const GrStyledShape& parentShape, GrStyle::Apply, SkScalar scale);

    /**
     * Determines the key we should inherit from the input shape's geometry and style when
     * we are applying the style to create a new shape.
     */
    void setInheritedKey(const GrStyledShape& parentShape, GrStyle::Apply, SkScalar scale);

    // Similar to GrShape::simplify but also takes into account style and stroking, possibly
    // applying the style explicitly to produce a new analytic shape with a simpler style.
    void simplify();
    bool applyStrokeToLine();

    /** Gets the path that gen id listeners should be added to. */
    const SkPath* originalPathForListeners() const;

    // Defaults to use when there is no distinction between even/odd and winding fills.
    static constexpr SkPathFillType kDefaultPathFillType = SkPathFillType::kEvenOdd;
    static constexpr SkPathFillType kDefaultPathInverseFillType = SkPathFillType::kInverseEvenOdd;

    static constexpr SkPathDirection kDefaultRRectDir = SkPathDirection::kCW;
    static constexpr unsigned kDefaultRRectStart = 0;

    static unsigned DefaultRectDirAndStartIndex(const SkRect& rect, bool hasPathEffect,
                                                SkPathDirection* dir) {
        *dir = kDefaultRRectDir;
        // This comes from SkPath's interface. The default for adding a SkRect is counter clockwise
        // beginning at index 0 (which happens to correspond to rrect index 0 or 7).
        if (!hasPathEffect) {
            // It doesn't matter what start we use, just be consistent to avoid redundant keys.
            return kDefaultRRectStart;
        }
        // In SkPath a rect starts at index 0 by default. This is the top left corner. However,
        // we store rects as rrects. RRects don't preserve the invertedness, but rather sort the
        // rect edges. Thus, we may need to modify the rrect's start index to account for the sort.
        bool swapX = rect.fLeft > rect.fRight;
        bool swapY = rect.fTop > rect.fBottom;
        if (swapX && swapY) {
            // 0 becomes start index 2 and times 2 to convert from rect the rrect indices.
            return 2 * 2;
        } else if (swapX) {
            *dir = SkPathDirection::kCCW;
            // 0 becomes start index 1 and times 2 to convert from rect the rrect indices.
            return 2 * 1;
        } else if (swapY) {
            *dir = SkPathDirection::kCCW;
            // 0 becomes start index 3 and times 2 to convert from rect the rrect indices.
            return 2 * 3;
        }
        return 0;
    }

    static unsigned DefaultRRectDirAndStartIndex(const SkRRect& rrect, bool hasPathEffect,
                                                 SkPathDirection* dir) {
        // This comes from SkPath's interface. The default for adding a SkRRect to a path is
        // clockwise beginning at starting index 6.
        static constexpr unsigned kPathRRectStartIdx = 6;
        *dir = kDefaultRRectDir;
        if (!hasPathEffect) {
            // It doesn't matter what start we use, just be consistent to avoid redundant keys.
            return kDefaultRRectStart;
        }
        return kPathRRectStartIdx;
    }

    GrShape         fShape;
    bool            fInverted = false;
    // Even if a shape is simplified from a path, the start and winding affects dashing
    SkPathDirection fDir      = kDefaultRRectDir;
    unsigned        fStart    = kDefaultRRectStart;
    // Gen ID of the original path (path may be modified or simplified away).
    int32_t         fGenID    = 0;

    GrStyle fStyle;
    SkTLazy<SkPath> fInheritedPathForListeners;
    SkAutoSTArray<8, uint32_t>  fInheritedKey;
};
#endif
