/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShape_DEFINED
#define GrShape_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrStyle.h"
#include <new>

/**
 * Represents a geometric shape (rrect or path) and the GrStyle that it should be rendered with.
 * It is possible to apply the style to the GrShape to produce a new GrShape where the geometry
 * reflects the styling information (e.g. is stroked). It is also possible to apply just the
 * path effect from the style. In this case the resulting shape will include any remaining
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
class GrShape {
public:
    // Keys for paths may be extracted from the path data for small paths. Clients aren't supposed
    // to have to worry about this. This value is exposed for unit tests.
    static constexpr int kMaxKeyFromDataVerbCnt = 10;

    GrShape() { this->initType(Type::kEmpty); }

    explicit GrShape(const SkPath& path) : GrShape(path, GrStyle::SimpleFill()) {}

    explicit GrShape(const SkRRect& rrect) : GrShape(rrect, GrStyle::SimpleFill()) {}

    explicit GrShape(const SkRect& rect) : GrShape(rect, GrStyle::SimpleFill()) {}

    GrShape(const SkPath& path, const GrStyle& style) : fStyle(style) {
        this->initType(Type::kPath, &path);
        this->attemptToSimplifyPath();
    }

    GrShape(const SkRRect& rrect, const GrStyle& style) : fStyle(style) {
        this->initType(Type::kRRect);
        fRRectData.fRRect = rrect;
        fRRectData.fInverted = false;
        fRRectData.fStart = DefaultRRectDirAndStartIndex(rrect, style.hasPathEffect(),
                                                         &fRRectData.fDir);
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkRRect& rrect, SkPathDirection dir, unsigned start, bool inverted,
            const GrStyle& style)
        : fStyle(style) {
        this->initType(Type::kRRect);
        fRRectData.fRRect = rrect;
        fRRectData.fInverted = inverted;
        if (style.pathEffect()) {
            fRRectData.fDir = dir;
            fRRectData.fStart = start;
            if (fRRectData.fRRect.getType() == SkRRect::kRect_Type) {
                fRRectData.fStart = (fRRectData.fStart + 1) & 0b110;
            } else if (fRRectData.fRRect.getType() == SkRRect::kOval_Type) {
                fRRectData.fStart &= 0b110;
            }
        } else {
            fRRectData.fStart = DefaultRRectDirAndStartIndex(rrect, false, &fRRectData.fDir);
        }
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkRect& rect, const GrStyle& style) : fStyle(style) {
        this->initType(Type::kRRect);
        fRRectData.fRRect = SkRRect::MakeRect(rect);
        fRRectData.fInverted = false;
        fRRectData.fStart = DefaultRectDirAndStartIndex(rect, style.hasPathEffect(),
                                                        &fRRectData.fDir);
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkPath& path, const SkPaint& paint) : fStyle(paint) {
        this->initType(Type::kPath, &path);
        this->attemptToSimplifyPath();
    }

    GrShape(const SkRRect& rrect, const SkPaint& paint) : fStyle(paint) {
        this->initType(Type::kRRect);
        fRRectData.fRRect = rrect;
        fRRectData.fInverted = false;
        fRRectData.fStart = DefaultRRectDirAndStartIndex(rrect, fStyle.hasPathEffect(),
                                                         &fRRectData.fDir);
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkRect& rect, const SkPaint& paint) : fStyle(paint) {
        this->initType(Type::kRRect);
        fRRectData.fRRect = SkRRect::MakeRect(rect);
        fRRectData.fInverted = false;
        fRRectData.fStart = DefaultRectDirAndStartIndex(rect, fStyle.hasPathEffect(),
                                                        &fRRectData.fDir);
        this->attemptToSimplifyRRect();
    }

    static GrShape MakeArc(const SkRect& oval, SkScalar startAngleDegrees,
                           SkScalar sweepAngleDegrees, bool useCenter, const GrStyle& style);

    GrShape(const GrShape&);
    GrShape& operator=(const GrShape& that);

    ~GrShape() { this->changeType(Type::kEmpty); }

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
    static GrShape MakeFilled(const GrShape& original, FillInversion = FillInversion::kPreserve);

    const GrStyle& style() const { return fStyle; }

    /**
     * Returns a shape that has either applied the path effect or path effect and stroking
     * information from this shape's style to its geometry. Scale is used when approximating the
     * output geometry and typically is computed from the view matrix
     */
    GrShape applyStyle(GrStyle::Apply apply, SkScalar scale) const {
        return GrShape(*this, apply, scale);
    }

    bool isRect() const {
        if (Type::kRRect != fType) {
            return false;
        }

        return fRRectData.fRRect.isRect();
    }

    /** Returns the unstyled geometry as a rrect if possible. */
    bool asRRect(SkRRect* rrect, SkPathDirection* dir, unsigned* start, bool* inverted) const {
        if (Type::kRRect != fType) {
            return false;
        }
        if (rrect) {
            *rrect = fRRectData.fRRect;
        }
        if (dir) {
            *dir = fRRectData.fDir;
        }
        if (start) {
            *start = fRRectData.fStart;
        }
        if (inverted) {
            *inverted = fRRectData.fInverted;
        }
        return true;
    }

    /**
     * If the unstyled shape is a straight line segment, returns true and sets pts to the endpoints.
     * An inverse filled line path is still considered a line.
     */
    bool asLine(SkPoint pts[2], bool* inverted) const {
        if (fType != Type::kLine) {
            return false;
        }
        if (pts) {
            pts[0] = fLineData.fPts[0];
            pts[1] = fLineData.fPts[1];
        }
        if (inverted) {
            *inverted = fLineData.fInverted;
        }
        return true;
    }

    /** Returns the unstyled geometry as a path. */
    void asPath(SkPath* out) const {
        switch (fType) {
            case Type::kEmpty:
                out->reset();
                break;
            case Type::kInvertedEmpty:
                out->reset();
                out->setFillType(kDefaultPathInverseFillType);
                break;
            case Type::kRRect:
                out->reset();
                out->addRRect(fRRectData.fRRect, fRRectData.fDir, fRRectData.fStart);
                // Below matches the fill type that attemptToSimplifyPath uses.
                if (fRRectData.fInverted) {
                    out->setFillType(kDefaultPathInverseFillType);
                } else {
                    out->setFillType(kDefaultPathFillType);
                }
                break;
            case Type::kArc:
                SkPathPriv::CreateDrawArcPath(out, fArcData.fOval, fArcData.fStartAngleDegrees,
                                              fArcData.fSweepAngleDegrees, fArcData.fUseCenter,
                                              fStyle.isSimpleFill());
                if (fArcData.fInverted) {
                    out->setFillType(kDefaultPathInverseFillType);
                } else {
                    out->setFillType(kDefaultPathFillType);
                }
                break;
            case Type::kLine:
                out->reset();
                out->moveTo(fLineData.fPts[0]);
                out->lineTo(fLineData.fPts[1]);
                if (fLineData.fInverted) {
                    out->setFillType(kDefaultPathInverseFillType);
                } else {
                    out->setFillType(kDefaultPathFillType);
                }
                break;
            case Type::kPath:
                *out = this->path();
                break;
        }
    }

    // Can this shape be drawn as a pair of filled nested rectangles?
    bool asNestedRects(SkRect rects[2]) const {
        if (Type::kPath != fType) {
            return false;
        }

        // TODO: it would be better two store DRRects natively in the shape rather than converting
        // them to a path and then reextracting the nested rects
        if (this->path().isInverseFillType()) {
            return false;
        }

        SkPathDirection dirs[2];
        if (!SkPathPriv::IsNestedFillRects(this->path(), rects, dirs)) {
            return false;
        }

        if (SkPathFillType::kWinding == (SkPathFillType)this->path().getFillType() && dirs[0] == dirs[1]) {
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

    /**
     * Returns whether the geometry is empty. Note that applying the style could produce a
     * non-empty shape. It also may have an inverse fill.
     */
    bool isEmpty() const { return Type::kEmpty == fType || Type::kInvertedEmpty == fType; }

    /**
     * Gets the bounds of the geometry without reflecting the shape's styling. This ignores
     * the inverse fill nature of the geometry.
     */
    SkRect bounds() const;

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
        switch (fType) {
            case Type::kEmpty:
                return true;
            case Type::kInvertedEmpty:
                return true;
            case Type::kRRect:
                return true;
            case Type::kArc:
                return SkPathPriv::DrawArcIsConvex(fArcData.fSweepAngleDegrees,
                                                   SkToBool(fArcData.fUseCenter),
                                                   fStyle.isSimpleFill());
            case Type::kLine:
                return true;
            case Type::kPath:
                // SkPath.isConvex() really means "is this path convex were it to be closed" and
                // thus doesn't give the correct answer for stroked paths, hence we also check
                // whether the path is either filled or closed. Convex paths may only have one
                // contour hence isLastContourClosed() is a sufficient for a convex path.
                return (this->style().isSimpleFill() || this->path().isLastContourClosed()) &&
                        this->path().isConvex();
        }
        return false;
    }

    /**
     * Does the shape have a known winding direction. Some degenerate convex shapes may not have
     * a computable direction, but this is not always a requirement for path renderers so it is
     * kept separate from knownToBeConvex().
     */
    bool knownDirection() const {
        switch (fType) {
            case Type::kEmpty:
                return true;
            case Type::kInvertedEmpty:
                return true;
            case Type::kRRect:
                return true;
            case Type::kArc:
                return true;
            case Type::kLine:
                return true;
            case Type::kPath:
                // Assuming this is called after knownToBeConvex(), this should just be relying on
                // cached convexity and direction and will be cheap.
                return !SkPathPriv::CheapIsFirstDirection(this->path(),
                                                          SkPathPriv::kUnknown_FirstDirection);
        }
        return false;
    }

    /** Is the pre-styled geometry inverse filled? */
    bool inverseFilled() const {
        bool ret = false;
        switch (fType) {
            case Type::kEmpty:
                ret = false;
                break;
            case Type::kInvertedEmpty:
                ret = true;
                break;
            case Type::kRRect:
                ret = fRRectData.fInverted;
                break;
            case Type::kArc:
                ret = fArcData.fInverted;
                break;
            case Type::kLine:
                ret = fLineData.fInverted;
                break;
            case Type::kPath:
                ret = this->path().isInverseFillType();
                break;
        }
        // Dashing ignores inverseness. We should have caught this earlier. skbug.com/5421
        SkASSERT(!(ret && this->style().isDashed()));
        return ret;
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
        switch (fType) {
            case Type::kEmpty:
                return true;
            case Type::kInvertedEmpty:
                return true;
            case Type::kRRect:
                return true;
            case Type::kArc:
                return fArcData.fUseCenter;
            case Type::kLine:
                return false;
            case Type::kPath:
                // SkPath doesn't keep track of the closed status of each contour.
                return SkPathPriv::IsClosedSingleContour(this->path());
        }
        return false;
    }

    uint32_t segmentMask() const {
        switch (fType) {
            case Type::kEmpty:
                return 0;
            case Type::kInvertedEmpty:
                return 0;
            case Type::kRRect:
                if (fRRectData.fRRect.getType() == SkRRect::kOval_Type) {
                    return SkPath::kConic_SegmentMask;
                } else if (fRRectData.fRRect.getType() == SkRRect::kRect_Type ||
                           fRRectData.fRRect.getType() == SkRRect::kEmpty_Type) {
                    return SkPath::kLine_SegmentMask;
                }
                return SkPath::kLine_SegmentMask | SkPath::kConic_SegmentMask;
            case Type::kArc:
                if (fArcData.fUseCenter) {
                    return SkPath::kConic_SegmentMask | SkPath::kLine_SegmentMask;
                }
                return SkPath::kConic_SegmentMask;
            case Type::kLine:
                return SkPath::kLine_SegmentMask;
            case Type::kPath:
                return this->path().getSegmentMasks();
        }
        return 0;
    }

    /**
     * Gets the size of the key for the shape represented by this GrShape (ignoring its styling).
     * A negative value is returned if the shape has no key (shouldn't be cached).
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
    void addGenIDChangeListener(sk_sp<SkPathRef::GenIDChangeListener>) const;

    /**
     * Helpers that are only exposed for unit tests, to determine if the shape is a path, and get
     * the generation ID of the *original* path. This is the path that will receive
     * GenIDChangeListeners added to this shape.
     */
    uint32_t testingOnly_getOriginalGenerationID() const;
    bool testingOnly_isPath() const;
    bool testingOnly_isNonVolatilePath() const;

private:
    enum class Type {
        kEmpty,
        kInvertedEmpty,
        kRRect,
        kArc,
        kLine,
        kPath,
    };

    void initType(Type type, const SkPath* path = nullptr) {
        fType = Type::kEmpty;
        this->changeType(type, path);
    }

    void changeType(Type type, const SkPath* path = nullptr) {
        bool wasPath = Type::kPath == fType;
        fType = type;
        bool isPath = Type::kPath == type;
        SkASSERT(!path || isPath);
        if (wasPath && !isPath) {
            fPathData.fPath.~SkPath();
        } else if (!wasPath && isPath) {
            if (path) {
                new (&fPathData.fPath) SkPath(*path);
            } else {
                new (&fPathData.fPath) SkPath();
            }
        } else if (isPath && path) {
            fPathData.fPath = *path;
        }
        // Whether or not we use the path's gen ID is decided in attemptToSimplifyPath.
        fPathData.fGenID = 0;
    }

    SkPath& path() {
        SkASSERT(Type::kPath == fType);
        return fPathData.fPath;
    }

    const SkPath& path() const {
        SkASSERT(Type::kPath == fType);
        return fPathData.fPath;
    }

    /** Constructor used by the applyStyle() function */
    GrShape(const GrShape& parentShape, GrStyle::Apply, SkScalar scale);

    /**
     * Determines the key we should inherit from the input shape's geometry and style when
     * we are applying the style to create a new shape.
     */
    void setInheritedKey(const GrShape& parentShape, GrStyle::Apply, SkScalar scale);

    void attemptToSimplifyPath();
    void attemptToSimplifyRRect();
    void attemptToSimplifyLine();
    void attemptToSimplifyArc();

    bool attemptToSimplifyStrokedLineToRRect();

    /** Gets the path that gen id listeners should be added to. */
    const SkPath* originalPathForListeners() const;

    // Defaults to use when there is no distinction between even/odd and winding fills.
    static constexpr SkPathFillType kDefaultPathFillType = SkPathFillType::kEvenOdd;
    static constexpr SkPathFillType kDefaultPathInverseFillType =
            SkPathFillType::kInverseEvenOdd;

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

    union {
        struct {
            SkRRect fRRect;
            SkPathDirection fDir;
            unsigned fStart;
            bool fInverted;
        } fRRectData;
        struct {
            SkRect fOval;
            SkScalar fStartAngleDegrees;
            SkScalar fSweepAngleDegrees;
            int16_t fUseCenter;
            int16_t fInverted;
        } fArcData;
        struct {
            SkPath fPath;
            // Gen ID of the original path (fPath may be modified)
            int32_t fGenID;
        } fPathData;
        struct {
            SkPoint fPts[2];
            bool fInverted;
        } fLineData;
    };
    GrStyle fStyle;
    SkTLazy<SkPath> fInheritedPathForListeners;
    SkAutoSTArray<8, uint32_t>  fInheritedKey;
    Type fType;
};
#endif
