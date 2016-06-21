/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShape_DEFINED
#define GrShape_DEFINED

#include "GrStyle.h"
#include "SkPath.h"
#include "SkPathPriv.h"
#include "SkRRect.h"
#include "SkTemplates.h"
#include "SkTLazy.h"

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
 * that have SkCanvas::draw APIs.
 */
class GrShape {
public:
    GrShape() : fType(Type::kEmpty) {}

    explicit GrShape(const SkPath& path)
        : fType(Type::kPath)
        , fPath(&path) {
        this->attemptToSimplifyPath();
    }

    explicit GrShape(const SkRRect& rrect)
        : fType(Type::kRRect)
        , fRRect(rrect)
        , fRRectIsInverted(false) {
        fRRectStart = DefaultRRectDirAndStartIndex(rrect, false, &fRRectDir);
        this->attemptToSimplifyRRect();
    }

    explicit GrShape(const SkRect& rect)
        : fType(Type::kRRect)
        , fRRect(SkRRect::MakeRect(rect))
        , fRRectIsInverted(false) {
        fRRectStart = DefaultRectDirAndStartIndex(rect, false, &fRRectDir);
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkPath& path, const GrStyle& style)
        : fType(Type::kPath)
        , fPath(&path)
        , fStyle(style) {
        this->attemptToSimplifyPath();
    }

    GrShape(const SkRRect& rrect, const GrStyle& style)
        : fType(Type::kRRect)
        , fRRect(rrect)
        , fRRectIsInverted(false)
        , fStyle(style) {
        fRRectStart = DefaultRRectDirAndStartIndex(rrect, style.hasPathEffect(), &fRRectDir);
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkRRect& rrect, SkPath::Direction dir, unsigned start, bool inverted,
            const GrStyle& style)
        : fType(Type::kRRect)
        , fRRect(rrect)
        , fRRectIsInverted(inverted)
        , fStyle(style) {
        if (style.pathEffect()) {
            fRRectDir = dir;
            fRRectStart = start;
            if (fRRect.getType() == SkRRect::kRect_Type) {
                fRRectStart = (fRRectStart + 1) & 0b110;
            } else if (fRRect.getType() == SkRRect::kOval_Type) {
                fRRectStart &= 0b110;
            }
        } else {
            fRRectStart = DefaultRRectDirAndStartIndex(rrect, false, &fRRectDir);
        }
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkRect& rect, const GrStyle& style)
        : fType(Type::kRRect)
        , fRRect(SkRRect::MakeRect(rect))
        , fRRectIsInverted(false)
        , fStyle(style) {
        fRRectStart = DefaultRectDirAndStartIndex(rect, style.hasPathEffect(), &fRRectDir);
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkPath& path, const SkPaint& paint)
        : fType(Type::kPath)
        , fPath(&path)
        , fStyle(paint) {
        this->attemptToSimplifyPath();
    }

    GrShape(const SkRRect& rrect, const SkPaint& paint)
        : fType(Type::kRRect)
        , fRRect(rrect)
        , fRRectIsInverted(false)
        , fStyle(paint) {
        fRRectStart = DefaultRRectDirAndStartIndex(rrect, fStyle.hasPathEffect(), &fRRectDir);
        this->attemptToSimplifyRRect();
    }

    GrShape(const SkRect& rect, const SkPaint& paint)
        : fType(Type::kRRect)
        , fRRect(SkRRect::MakeRect(rect))
        , fRRectIsInverted(false)
        , fStyle(paint) {
        fRRectStart = DefaultRectDirAndStartIndex(rect, fStyle.hasPathEffect(), &fRRectDir);
        this->attemptToSimplifyRRect();
    }

    GrShape(const GrShape&);
    GrShape& operator=(const GrShape& that);

    ~GrShape() {
        if (Type::kPath == fType) {
            fPath.reset();
        }
    }

    const GrStyle& style() const { return fStyle; }

    /**
     * Returns a shape that has either applied the path effect or path effect and stroking
     * information from this shape's style to its geometry. Scale is used when approximating the
     * output geometry and typically is computed from the view matrix
     */
    GrShape applyStyle(GrStyle::Apply apply, SkScalar scale) {
        return GrShape(*this, apply, scale);
    }

    /** Returns the unstyled geometry as a rrect if possible. */
    bool asRRect(SkRRect* rrect, SkPath::Direction* dir, unsigned* start, bool* inverted) const {
        if (Type::kRRect != fType) {
            return false;
        }
        if (rrect) {
            *rrect = fRRect;
        }
        if (dir) {
            *dir = fRRectDir;
        }
        if (start) {
            *start = fRRectStart;
        }
        if (inverted) {
            *inverted = fRRectIsInverted;
        }
        return true;
    }

    /**
     * If the unstyled shape is a straight line segment, returns true and sets pts to the endpoints.
     * An inverse filled line path is still considered a line.
     */
     bool asLine(SkPoint pts[2]) const {
         if (fType != Type::kPath) {
             return false;
         }
         return fPath.get()->isLine(pts);
     }

    /** Returns the unstyled geometry as a path. */
    void asPath(SkPath* out) const {
        switch (fType) {
            case Type::kEmpty:
                out->reset();
                break;
            case Type::kRRect:
                out->reset();
                out->addRRect(fRRect, fRRectDir, fRRectStart);
                // Below matches the fill type that attemptToSimplifyPath uses.
                if (fRRectIsInverted) {
                    out->setFillType(SkPath::kInverseEvenOdd_FillType);
                } else {
                    out->setFillType(SkPath::kEvenOdd_FillType);
                }
                break;
            case Type::kPath:
                *out = *fPath.get();
                break;
        }
    }

    /**
     * Returns whether the geometry is empty. Note that applying the style could produce a
     * non-empty shape.
     */
    bool isEmpty() const { return Type::kEmpty == fType; }

    /**
     * Gets the bounds of the geometry without reflecting the shape's styling. This ignores
     * the inverse fill nature of the geometry.
     */
    const SkRect& bounds() const;

    /**
     * Gets the bounds of the geometry reflecting the shape's styling (ignoring inverse fill
     * status).
     */
    void styledBounds(SkRect* bounds) const;

    /**
     * Is it known that the unstyled geometry has no unclosed contours. This means that it will
     * not have any caps if stroked (modulo the effect of any path effect).
     */
    bool knownToBeClosed() const {
        switch (fType) {
            case Type::kEmpty:
                return true;
            case Type::kRRect:
                return true;
            case Type::kPath:
                return false;
        }
        return false;
    }

    uint32_t segmentMask() const {
        switch (fType) {
            case Type::kEmpty:
                return 0;
            case Type::kRRect:
                if (fRRect.getType() == SkRRect::kOval_Type) {
                    return SkPath::kConic_SegmentMask;
                } else if (fRRect.getType() == SkRRect::kRect_Type) {
                    return SkPath::kLine_SegmentMask;
                }
                return SkPath::kLine_SegmentMask | SkPath::kConic_SegmentMask;
            case Type::kPath:
                return fPath.get()->getSegmentMasks();
        }
        return 0;
    }

    /**
     * Gets the size of the key for the shape represented by this GrShape (ignoring its styling).
     * A negative value is returned if the shape has no key (shouldn't be cached).
     */
    int unstyledKeySize() const;

    /**
     * Writes unstyledKeySize() bytes into the provided pointer. Assumes that there is enough
     * space allocated for the key and that unstyledKeySize() does not return a negative value
     * for this shape.
     */
    void writeUnstyledKey(uint32_t* key) const;

private:
    enum class Type {
        kEmpty,
        kRRect,
        kPath,
    };

    /** Constructor used by the applyStyle() function */
    GrShape(const GrShape& parentShape, GrStyle::Apply, SkScalar scale);

    /**
     * Determines the key we should inherit from the input shape's geometry and style when
     * we are applying the style to create a new shape.
     */
    void setInheritedKey(const GrShape& parentShape, GrStyle::Apply, SkScalar scale);

    void attemptToSimplifyPath();
    void attemptToSimplifyRRect();

    static constexpr SkPath::Direction kDefaultRRectDir = SkPath::kCW_Direction;
    static constexpr unsigned kDefaultRRectStart = 0;

    static unsigned DefaultRectDirAndStartIndex(const SkRect& rect, bool hasPathEffect,
                                                SkPath::Direction* dir) {
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
            *dir = SkPath::kCCW_Direction;
            // 0 becomes start index 1 and times 2 to convert from rect the rrect indices.
            return 2 * 1;
        } else if (swapY) {
            *dir = SkPath::kCCW_Direction;
            // 0 becomes start index 3 and times 2 to convert from rect the rrect indices.
            return 2 * 3;
        }
        return 0;
    }

    static unsigned DefaultRRectDirAndStartIndex(const SkRRect& rrect, bool hasPathEffect,
                                                 SkPath::Direction* dir) {
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

    Type                        fType;
    SkRRect                     fRRect;
    SkPath::Direction           fRRectDir;
    unsigned                    fRRectStart;
    bool                        fRRectIsInverted;
    SkTLazy<SkPath>             fPath;
    // Gen ID of the original path (fPath may be modified)
    int32_t                     fPathGenID = 0;
    GrStyle                     fStyle;
    SkAutoSTArray<8, uint32_t>  fInheritedKey;
};
#endif
