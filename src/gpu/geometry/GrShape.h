/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShape_DEFINED
#define GrShape_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"

// Represents an arc along an oval boundary, or a closed wedge of the oval.
struct GrArc {
    SkRect   fOval;       // The sorted, bounding box defining the oval the arc is traced along
    SkScalar fStartAngle; // In degrees
    SkScalar fSweepAngle; // In degrees
    bool     fUseCenter;  // True if the arc includes the center point of the oval
};

// Represents a line segment between two points.
struct GrLineSegment {
    SkPoint fP1;
    SkPoint fP2;
};

/**
 * GrShape is a convenience class to represent the many different specialized geometries that
 * Ganesh can handle, including rects, round rects, lines, as well as paths. It is intended as
 * a data-only class where any additional complex behavior is handled by an owning type (e.g.
 * GrStyledShape). However, it does include some basic utilities that unify common functionality
 * (such as contains()) from the underlying shape types.
 *
 * In order to have lossless simplification of the geometry, it also tracks winding direction, start
 * index, and fill inversion. The direction and index are match the SkPath indexing scheme for
 * the shape's type (e.g. rect, rrect, or oval).
 *
 * Regarding GrShape's empty shape:
 * - GrShape uses empty to refer to the absence of any geometric data
 * - SkRect::isEmpty() returns true if the rect is not sorted, even if it has area. GrShape will not
 *   simplify these shapes to an empty GrShape. Rects with actual 0 width and height will simplify
 *   to a point or line, not empty. This is to preserve geometric data for path effects and strokes.
 * - SkRRect::isEmpty() is true when the bounds have 0 width or height, so GrShape will simplify it
 *   to a point or line, just like a rect. SkRRect does not have the concept of unsorted edges.
 */
class GrShape {
public:
    // The current set of types GrShape can represent directly
    enum class Type : uint8_t {
        kEmpty, kPoint, kRect, kRRect, kPath, kArc, kLine
    };
    inline static constexpr int kTypeCount = static_cast<int>(Type::kLine) + 1;

    // The direction and start index used when a shape does not have a representable winding,
    // or when that information was discarded during simplification (kIgnoreWinding_Flag).
    inline static constexpr SkPathDirection kDefaultDir   = SkPathDirection::kCW;
    inline static constexpr unsigned        kDefaultStart = 0;
    // The fill rule that is used by asPath() for shapes that aren't already a path.
    inline static constexpr SkPathFillType  kDefaultFillType = SkPathFillType::kEvenOdd;

    GrShape() {}
    explicit GrShape(const SkPoint& point) { this->setPoint(point); }
    explicit GrShape(const SkRect& rect) { this->setRect(rect); }
    explicit GrShape(const SkRRect& rrect) { this->setRRect(rrect); }
    explicit GrShape(const SkPath& path) { this->setPath(path); }
    explicit GrShape(const GrArc& arc) { this->setArc(arc); }
    explicit GrShape(const GrLineSegment& line){ this->setLine(line); }

    GrShape(const GrShape& shape) { *this = shape; }

    ~GrShape() { this->reset(); }

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for GrShape.
    GrShape& operator=(const GrShape& shape);

    // These type queries reflect the shape type provided when assigned, it does not incorporate
    // any potential simplification (e.g. if isRRect() is true and rrect().isRect() is true,
    // isRect() will still be false, until simplify() is called).
    bool isEmpty() const { return this->type() == Type::kEmpty; }
    bool isPoint() const { return this->type() == Type::kPoint; }
    bool isRect() const { return this->type() == Type::kRect; }
    bool isRRect() const { return this->type() == Type::kRRect; }
    bool isPath() const { return this->type() == Type::kPath; }
    bool isArc() const { return this->type() == Type::kArc; }
    bool isLine() const { return this->type() == Type::kLine; }

    Type type() const { return fType; }

    // Report the shape type, winding direction, start index, and invertedness as a value suitable
    // for use in a resource key. This does not include any geometry coordinates into the key value.
    uint32_t stateKey() const;

    // Whether or not the shape is meant to be the inverse of its geometry (i.e. its exterior).
    bool inverted() const {
        return this->isPath() ? fPath.isInverseFillType() : SkToBool(fInverted);
    }

    // Returns the path direction extracted from the path during simplification, if the shape's
    // type represents a rrect, rect, or oval.
    SkPathDirection dir() const { return fCW ? SkPathDirection::kCW : SkPathDirection::kCCW; }
    // Returns the start index extracted from the path during simplification, if the shape's
    // type represents a rrect, rect, or oval.
    unsigned startIndex() const { return fStart; }

    // Override the direction and start parameters for the simplified contour. These are only
    // meaningful for rects, rrects, and ovals.
    void setPathWindingParams(SkPathDirection dir, unsigned start) {
        SkASSERT((this->isRect() && start < 4) || (this->isRRect() && start < 8) ||
                 (dir == kDefaultDir && start == kDefaultStart));
        fCW = dir == SkPathDirection::kCW;
        fStart = static_cast<uint8_t>(start);
    }

    void setInverted(bool inverted) {
        if (this->isPath()) {
            if (inverted != fPath.isInverseFillType()) {
                fPath.toggleInverseFillType();
            }
        } else {
            fInverted = inverted;
        }
    }

    // Access the actual geometric description of the shape. May only access the appropriate type
    // based on what was last set. The type may change after simplify() is called.
    SkPoint& point() { SkASSERT(this->isPoint()); return fPoint; }
    const SkPoint& point() const { SkASSERT(this->isPoint()); return fPoint; }

    SkRect& rect() { SkASSERT(this->isRect()); return fRect; }
    const SkRect& rect() const { SkASSERT(this->isRect()); return fRect; }

    SkRRect& rrect() { SkASSERT(this->isRRect()); return fRRect; }
    const SkRRect& rrect() const { SkASSERT(this->isRRect()); return fRRect; }

    SkPath& path() { SkASSERT(this->isPath()); return fPath; }
    const SkPath& path() const { SkASSERT(this->isPath()); return fPath; }

    GrArc& arc() { SkASSERT(this->isArc()); return fArc; }
    const GrArc& arc() const { SkASSERT(this->isArc()); return fArc; }

    GrLineSegment& line() { SkASSERT(this->isLine()); return fLine; }
    const GrLineSegment& line() const { SkASSERT(this->isLine()); return fLine; }

    // Update the geometry stored in the GrShape and update its associated type to match. This
    // performs no simplification, so calling setRRect() with a round rect that has isRect() return
    // true will still be considered an rrect by this shape until simplify() is called.
    //
    // These also reset any extracted direction, start, and inverted state from a prior simplified
    // path, since these functions ared used to describe a new geometry.
    void setPoint(const SkPoint& point) {
        this->reset(Type::kPoint);
        fPoint = point;
    }
    void setRect(const SkRect& rect) {
        this->reset(Type::kRect);
        fRect = rect;
    }
    void setRRect(const SkRRect& rrect) {
        this->reset(Type::kRRect);
        fRRect = rrect;
    }
    void setArc(const GrArc& arc) {
        this->reset(Type::kArc);
        fArc = arc;
    }
    void setLine(const GrLineSegment& line) {
        this->reset(Type::kLine);
        fLine = line;
    }
    void setPath(const SkPath& path) {
        if (this->isPath()) {
            // Assign directly
            fPath = path;
        } else {
            // In-place initialize
            this->setType(Type::kPath);
            new (&fPath) SkPath(path);
        }
        // Must also set these since we didn't call reset() like other setX functions.
        this->setPathWindingParams(kDefaultDir, kDefaultStart);
        fInverted = path.isInverseFillType();
    }
    void reset() {
        this->reset(Type::kEmpty);
    }

    // Flags that enable more aggressive, "destructive" simplifications to the geometry
    enum SimplifyFlags : unsigned {
        // If set, it is assumed the original shape would have been implicitly filled when drawn or
        // clipped, so simpler shape types that are closed can still be considered. Shapes with
        // 0 area (i.e. points and lines) can be turned into empty.
        kSimpleFill_Flag    = 0b001,
        // If set, simplifications that would impact a directional stroke or path effect can still
        // be taken (e.g. dir and start are not required, arcs can be converted to ovals).
        kIgnoreWinding_Flag = 0b010,
        // If set, the geometry will be updated to have sorted coordinates (rects, lines), modulated
        // sweep angles (arcs).
        kMakeCanonical_Flag = 0b100,

        kAll_Flags          = 0b111
    };
    // Returns true if the shape was originally closed based on type (or detected type within a
    // path), even if the final simplification results in a point, line, or empty.
    bool simplify(unsigned flags = kAll_Flags);

    // True if the given bounding box is completely inside the shape, if it's conservatively treated
    // as a filled, closed shape.
    bool conservativeContains(const SkRect& rect) const;
    bool conservativeContains(const SkPoint& point) const;

    // True if the underlying geometry represents a closed shape, without the need for an
    // implicit close (note that if simplified earlier with 'simpleFill' = true, a shape that was
    // not closed may become closed).
    bool closed() const;

    // True if the underlying shape is known to be convex, assuming no other styles. If 'simpleFill'
    // is true, it is assumed the contours will be implicitly closed when drawn or used.
    bool convex(bool simpleFill = true) const;

    // The bounding box of the shape.
    SkRect bounds() const;

    // The segment masks that describe the shape, were it to be converted to an SkPath
    uint32_t segmentMask() const;

    // Convert the shape into a path that describes the same geometry.
    void asPath(SkPath* out, bool simpleFill = true) const;

private:

    void setType(Type type) {
        if (this->isPath() && type != Type::kPath) {
            fInverted = fPath.isInverseFillType();
            fPath.~SkPath();
        }
        fType = type;
    }

    void reset(Type type) {
        this->setType(type);
        this->setPathWindingParams(kDefaultDir, kDefaultStart);
        this->setInverted(false);
    }

    // Paths and arcs are root shapes, another type will never simplify to them, so they do
    // not take the geometry to simplify as an argument. Since they are root shapes, they also
    // return whether or not they were originally closed before being simplified.
    bool simplifyPath(unsigned flags);
    bool simplifyArc(unsigned flags);

    // The simpler type classes do take the geometry because it may represent an in-progress
    // simplification that hasn't been set on the GrShape yet. The simpler types do not report
    // whether or not they were closed because it's implicit in their type.
    void simplifyLine(const SkPoint& p1, const SkPoint& p2, unsigned flags);
    void simplifyPoint(const SkPoint& point, unsigned flags);

    // RRects and rects care about winding for path effects and will set the path winding state
    // of the shape as well.
    void simplifyRRect(const SkRRect& rrect, SkPathDirection dir, unsigned start, unsigned flags);
    void simplifyRect(const SkRect& rect, SkPathDirection dir, unsigned start, unsigned flags);

    union {
        SkPoint       fPoint;
        SkRect        fRect;
        SkRRect       fRRect;
        SkPath        fPath;
        GrArc         fArc;
        GrLineSegment fLine;
    };

    Type            fType = Type::kEmpty;
    uint8_t         fStart; // Restricted to rrects and simpler, so this will be < 8
    bool            fCW;
    bool            fInverted;
};

#endif
