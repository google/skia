/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStyledShape_DEFINED
#define GrStyledShape_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/geometry/GrShape.h"

#include <cstdint>

struct SkArc;
class SkIDChangeListener;
class SkPaint;
struct SkPoint;

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
    inline static constexpr int kMaxKeyFromDataVerbCnt = 10;

    GrStyledShape() {}

    enum class DoSimplify : bool { kNo = false, kYes };

    explicit GrStyledShape(const SkPath& path, DoSimplify doSimplify = DoSimplify::kYes)
            : GrStyledShape(path, GrStyle::SimpleFill(), doSimplify) {}

    explicit GrStyledShape(const SkRRect& rrect, DoSimplify doSimplify = DoSimplify::kYes)
            : GrStyledShape(rrect, GrStyle::SimpleFill(), doSimplify) {}

    explicit GrStyledShape(const SkRect& rect, DoSimplify doSimplify = DoSimplify::kYes)
            : GrStyledShape(rect, GrStyle::SimpleFill(), doSimplify) {}

    GrStyledShape(const SkPath& path, const SkPaint& paint,
                  DoSimplify doSimplify = DoSimplify::kYes)
            : GrStyledShape(path, GrStyle(paint), doSimplify) {}

    GrStyledShape(const SkRRect& rrect, const SkPaint& paint,
                  DoSimplify doSimplify = DoSimplify::kYes)
            : GrStyledShape(rrect, GrStyle(paint), doSimplify) {}

    GrStyledShape(const SkRect& rect, const SkPaint& paint,
                  DoSimplify doSimplify = DoSimplify::kYes)
            : GrStyledShape(rect, GrStyle(paint), doSimplify) {}

    GrStyledShape(const SkPath& path, const GrStyle& style,
                  DoSimplify doSimplify = DoSimplify::kYes)
            : fShape(path), fStyle(style) {
        if (doSimplify == DoSimplify::kYes) {
            this->simplify();
        }
    }

    GrStyledShape(const SkRRect& rrect, const GrStyle& style,
                  DoSimplify doSimplify = DoSimplify::kYes)
            // Preserve legacy indices (6 for CW), see SkPathBuilder::addRRect().
            : GrStyledShape(rrect, SkPathDirection::kCW, 6, false, style, doSimplify) {}

    GrStyledShape(const SkRRect& rrect, SkPathDirection dir, unsigned start, bool inverted,
                  const GrStyle& style, DoSimplify doSimplify = DoSimplify::kYes)
            : fShape(rrect)
            , fStyle(style) {
        fShape.setPathWindingParams(dir, start);
        fShape.setInverted(inverted);
        if (doSimplify == DoSimplify::kYes) {
            this->simplify();
        }
    }

    GrStyledShape(const SkRect& rect, const GrStyle& style,
                  DoSimplify doSimplify = DoSimplify::kYes)
            : fShape(rect), fStyle(style) {
        if (doSimplify == DoSimplify::kYes) {
            this->simplify();
        }
    }

    GrStyledShape(const GrStyledShape&);

    static GrStyledShape MakeArc(const SkArc& arc,
                                 const GrStyle& style,
                                 DoSimplify = DoSimplify::kYes);

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

    // True if the shape and/or style were modified into a simpler, equivalent pairing
    bool simplified() const { return fSimplified; }

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
    bool asRRect(SkRRect* rrect, SkPathDirection* dir, unsigned* start, bool* inverted) const;

    /**
     * If the unstyled shape is a straight line segment, returns true and sets pts to the endpoints.
     * An inverse filled line path is still considered a line.
     */
    bool asLine(SkPoint pts[2], bool* inverted) const;

    // Can this shape be drawn as a pair of filled nested rectangles?
    bool asNestedRects(SkRect rects[2]) const;

    /** Returns the unstyled geometry as a path. */
    void asPath(SkPath* out) const {
        fShape.asPath(out, fStyle.isSimpleFill());
    }

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
        return fShape.convex(fStyle.isSimpleFill());
    }

    /**
     * Does the shape have a known winding direction. Some degenerate convex shapes may not have
     * a computable direction, but this is not always a requirement for path renderers so it is
     * kept separate from knownToBeConvex().
     */
    bool knownDirection() const {
        // Assuming this is called after knownToBeConvex(), this should just be relying on
        // cached convexity and direction and will be cheap.
        return !fShape.isPath() ||
               SkPathPriv::ComputeFirstDirection(fShape.path()) != SkPathFirstDirection::kUnknown;
    }

    /** Is the pre-styled geometry inverse filled? */
    bool inverseFilled() const {
        // Since the path tracks inverted-fillness itself, it should match what was recorded.
        SkASSERT(!fShape.isPath() || fShape.inverted() == fShape.path().isInverseFillType());
        // Dashing ignores inverseness. We should have caught this earlier. skbug.com/5421
        SkASSERT(!(fShape.inverted() && this->style().isDashed()));
        return fShape.inverted();
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

    /**
     * Similar to GrShape::simplify but also takes into account style and stroking, possibly
     * applying the style explicitly to produce a new analytic shape with a simpler style.
     * Unless "doSimplify" is kNo, this method gets called automatically during construction.
     */
    void simplify();

private:
    /** Constructor used by the applyStyle() function */
    GrStyledShape(const GrStyledShape& parentShape, GrStyle::Apply, SkScalar scale);

    /**
     * Determines the key we should inherit from the input shape's geometry and style when
     * we are applying the style to create a new shape.
     */
    void setInheritedKey(const GrStyledShape& parentShape, GrStyle::Apply, SkScalar scale);

    /**
     * As part of the simplification process, some shapes can have stroking trivially evaluated
     * and form a new geometry with just a fill.
     */
    void simplifyStroke();

    /** Gets the path that gen id listeners should be added to. */
    const SkPath* originalPathForListeners() const;

    GrShape fShape;
    GrStyle fStyle;
    // Gen ID of the original path (path may be modified or simplified away).
    int32_t fGenID      = 0;
    bool    fClosed     = false;
    bool    fSimplified = false;

    SkTLazy<SkPath>            fInheritedPathForListeners;
    skia_private::AutoSTArray<8, uint32_t> fInheritedKey;
};
#endif
