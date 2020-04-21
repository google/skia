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
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"

// Represents an arc along an oval boundary, or a closed wedge of the oval.
struct GrArc {
    SkRect   fOval;       // The bounding box defining the oval the arc is traced along
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
 * GrStyledShape). However, it does include some basic utilities that unifies common functionality
 * (such as contains()) included in the underlying shape types.
 */
class GrShape {
public:
    GrShape() {}
    explicit GrShape(const SkRect& rect) { this->setRect(rect); }
    explicit GrShape(const SkRRect& rrect) { this->setRRect(rrect); }
    explicit GrShape(const SkPath& path) { this->setPath(path); }
    explicit GrShape(const GrArc& arc) { this->setArc(arc); }
    explicit GrShape(const GrLineSegment& line) { this->setLine(line); }

    explicit GrShape(const GrShape& shape) { *this = shape; }

    ~GrShape() { this->reset(); }

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for GrShape.
    GrShape& operator=(const GrShape& shape);

    // These type queries reflect the shape type provided when assigned, it does not incorporate
    // any potential simplification (e.g. if isRRect() is true and rrect().isRect() is true,
    // isRect() will still be false, until simplify() is called).
    bool isEmpty() const { return fType == Type::kEmpty; }
    bool isRect() const { return fType == Type::kRect; }
    bool isRRect() const { return fType == Type::kRRect; }
    bool isPath() const { return fType == Type::kPath; }
    bool isArc() const { return fType == Type::kArc; }
    bool isLine() const { return fType == Type::kLine; }

    // Access the actual geometric description of the shape. May only access the appropriate type
    // based on what was last set. The type may change after simplify() is called.
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
            new (&fPath) SkPath(path);
            fType = Type::kPath;
        }
    }
    void reset() {
        this->reset(Type::kEmpty);
    }

    // Attempt to simplify the type of this shape to a simpler class that is equivalent. Returns
    // true if the shape was simplified. The following simplifications are possible:
    //  - path to arc, line, rrect, rect, or empty
    //  - arc to rrect or empty
    //  - rrect to rect or empty
    //  - rect and line to empty
    //
    // Note that currently, because rectangles and round rects with one dimension of 0 and the
    // other non-zero are still reported as empty, this function maintains that and will map
    // those degenerate shapes to empty. Similarly, a line between the same two points is considered
    // empty, not a point.
    bool simplify(bool simpleFill = true) {
        return this->simplify(simpleFill, nullptr, nullptr);
    }

    // Attempt to simplify the shape, while preserving all necessary information for path
    // effects to be rendered properly. This will make less aggressive decisions than
    // simplify(simpleFill = false). 'dir' and 'start' will only be modified if the simplification
    // extracts winding information (i.e. path to rrect, oval, or rect), but it will be normalized
    // to the round rect index scheme.
    bool simplifyForPathEffect(SkPathDirection* dir, unsigned* start) {
        SkASSERT(dir && start);
        return this->simplify(false, dir, start);
    }

    // True if the given bounding box is completly inside the shape.
    bool contains(const SkRect& rect) const;

    // True if the underlying geometry represents a closed shape, without the need for an
    // implicit close (note that if simplified earlier with 'simpleFill' = true, a shape that was
    // not closed may become closed).
    bool closed() const;

    // True if the underlying shape is known to be convex, assuming no other styles. If 'simpleFill'
    // is true, it is assumed the contours will be implicitly closed when filled.
    bool convex(bool simpleFill = true) const;

    // The bounding box of the shape.
    SkRect bounds() const;

    // The segment masks that describe the shape, were it to be converted to an SkPath
    uint32_t segmentMask() const;

    // Convert the shape into a path that describes the same geometry.
    void asPath(SkPath* out) const;

private:
    enum class Type {
        kEmpty, kRect, kRRect, kPath, kArc, kLine
    };

    bool simplify(bool simpleFill, SkPathDirection* dir, unsigned* start);

    void reset(Type type) {
        if (this->isPath()) {
            fPath.~SkPath();
        }
        fType = type;
    }

    union {
        SkRect        fRect;
        SkRRect       fRRect;
        SkPath        fPath;
        GrArc         fArc;
        GrLineSegment fLine;
    };

    Type fType = Type::kEmpty;
};

#endif
