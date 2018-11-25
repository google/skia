/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRRect_DEFINED
#define SkRRect_DEFINED

#include "SkRect.h"
#include "SkPoint.h"

class SkPath;
class SkMatrix;
class SkRBuffer;
class SkWBuffer;

// Path forward:
//   core work
//      add contains(SkRect&)  - for clip stack
//      add contains(SkRRect&) - for clip stack
//      add heart rect computation (max rect inside RR)
//      add 9patch rect computation
//      add growToInclude(SkPath&)
//   analysis
//      use growToInclude to fit skp round rects & generate stats (RRs vs. real paths)
//      check on # of rectorus's the RRs could handle
//   rendering work
//      update SkPath.addRRect() to only use quads
//      add GM and bench
//   further out
//      detect and triangulate RRectorii rather than falling back to SW in Ganesh
//

/** \class SkRRect

    The SkRRect class represents a rounded rect with a potentially different
    radii for each corner. It does not have a constructor so must be
    initialized with one of the initialization functions (e.g., setEmpty,
    setRectRadii, etc.)

    This class is intended to roughly match CSS' border-*-*-radius capabilities.
    This means:
        If either of a corner's radii are 0 the corner will be square.
        Negative radii are not allowed (they are clamped to zero).
        If the corner curves overlap they will be proportionally reduced to fit.
*/
class SK_API SkRRect {
public:
    /** Default initialized to a rrect at the origin with zero width and height. */
    SkRRect() = default;

    SkRRect(const SkRRect&) = default;
    SkRRect& operator=(const SkRRect&) = default;

    /**
     * Enum to capture the various possible subtypes of RR. Accessed
     * by type(). The subtypes become progressively less restrictive.
     */
    enum Type {
        // !< The RR has zero width and/or zero height. All radii are zero.
        kEmpty_Type,

        //!< The RR is actually a (non-empty) rect (i.e., at least one radius
        //!< at each corner is zero)
        kRect_Type,

        //!< The RR is actually a (non-empty) oval (i.e., all x radii are equal
        //!< and >= width/2 and all the y radii are equal and >= height/2
        kOval_Type,

        //!< The RR is non-empty and all the x radii are equal & all y radii
        //!< are equal but it is not an oval (i.e., there are lines between
        //!< the curves) nor a rect (i.e., both radii are non-zero)
        kSimple_Type,

        //!< The RR is non-empty and the two left x radii are equal, the two top
        //!< y radii are equal, and the same for the right and bottom but it is
        //!< neither an rect, oval, nor a simple RR. It is called "nine patch"
        //!< because the centers of the corner ellipses form an axis aligned
        //!< rect with edges that divide the RR into an 9 rectangular patches:
        //!< an interior patch, four edge patches, and four corner patches.
        kNinePatch_Type,

        //!< A fully general (non-empty) RR. Some of the x and/or y radii are
        //!< different from the others and there must be one corner where
        //!< both radii are non-zero.
        kComplex_Type,

        kLastType = kComplex_Type,
    };

    /**
     * Returns the RR's sub type.
     */
    Type getType() const {
        SkASSERT(this->isValid());
        return static_cast<Type>(fType);
    }

    Type type() const { return this->getType(); }

    inline bool isEmpty() const { return kEmpty_Type == this->getType(); }
    inline bool isRect() const { return kRect_Type == this->getType(); }
    inline bool isOval() const { return kOval_Type == this->getType(); }
    inline bool isSimple() const { return kSimple_Type == this->getType(); }
    // TODO: should isSimpleCircular & isCircle take a tolerance? This could help
    // instances where the mapping to device space is noisy.
    inline bool isSimpleCircular() const {
        return this->isSimple() && SkScalarNearlyEqual(fRadii[0].fX, fRadii[0].fY);
    }
    inline bool isCircle() const {
        return this->isOval() && SkScalarNearlyEqual(fRadii[0].fX, fRadii[0].fY);
    }
    inline bool isNinePatch() const { return kNinePatch_Type == this->getType(); }
    inline bool isComplex() const { return kComplex_Type == this->getType(); }

    bool allCornersCircular(SkScalar tolerance = SK_ScalarNearlyZero) const;

    SkScalar width() const { return fRect.width(); }
    SkScalar height() const { return fRect.height(); }

    /**
     * Same as default initialized - zero width and height at the origin.
     */
    void setEmpty() { *this = SkRRect(); }

    /**
     * Set this RR to match the supplied rect. All radii will be 0.
     */
    void setRect(const SkRect& rect) {
        if (!this->initializeRect(rect)) {
            return;
        }

        memset(fRadii, 0, sizeof(fRadii));
        fType = kRect_Type;

        SkASSERT(this->isValid());
    }

    /** Makes an empty rrect at the origin with zero width and height. */
    static SkRRect MakeEmpty() { return SkRRect(); }

    static SkRRect MakeRect(const SkRect& r) {
        SkRRect rr;
        rr.setRect(r);
        return rr;
    }

    static SkRRect MakeOval(const SkRect& oval) {
        SkRRect rr;
        rr.setOval(oval);
        return rr;
    }

    static SkRRect MakeRectXY(const SkRect& rect, SkScalar xRad, SkScalar yRad) {
        SkRRect rr;
        rr.setRectXY(rect, xRad, yRad);
        return rr;
    }

    /**
     * Set this RR to match the supplied oval. All x radii will equal half the
     * width and all y radii will equal half the height.
     */
    void setOval(const SkRect& oval) {
        if (!this->initializeRect(oval)) {
            return;
        }

        SkScalar xRad = SkScalarHalf(fRect.width());
        SkScalar yRad = SkScalarHalf(fRect.height());

        for (int i = 0; i < 4; ++i) {
            fRadii[i].set(xRad, yRad);
        }
        fType = kOval_Type;

        SkASSERT(this->isValid());
    }

    /**
     * Initialize the RR with the same radii for all four corners.
     */
    void setRectXY(const SkRect& rect, SkScalar xRad, SkScalar yRad);

    /**
     * Initialize the rr with one radius per-side.
     */
    void setNinePatch(const SkRect& rect, SkScalar leftRad, SkScalar topRad,
                      SkScalar rightRad, SkScalar bottomRad);

    /**
     * Initialize the RR with potentially different radii for all four corners.
     */
    void setRectRadii(const SkRect& rect, const SkVector radii[4]);

    // The radii are stored in UL, UR, LR, LL order.
    enum Corner {
        kUpperLeft_Corner,
        kUpperRight_Corner,
        kLowerRight_Corner,
        kLowerLeft_Corner
    };

    const SkRect& rect() const { return fRect; }
    const SkVector& radii(Corner corner) const { return fRadii[corner]; }
    const SkRect& getBounds() const { return fRect; }

    /**
     *  When a rrect is simple, all of its radii are equal. This returns one
     *  of those radii. This call requires the rrect to be non-complex.
     */
    const SkVector& getSimpleRadii() const {
        SkASSERT(!this->isComplex());
        return fRadii[0];
    }

    friend bool operator==(const SkRRect& a, const SkRRect& b) {
        return a.fRect == b.fRect && SkScalarsEqual(&a.fRadii[0].fX, &b.fRadii[0].fX, 8);
    }

    friend bool operator!=(const SkRRect& a, const SkRRect& b) {
        return a.fRect != b.fRect || !SkScalarsEqual(&a.fRadii[0].fX, &b.fRadii[0].fX, 8);
    }

    /**
     *  Call inset on the bounds, and adjust the radii to reflect what happens
     *  in stroking: If the corner is sharp (no curvature), leave it alone,
     *  otherwise we grow/shrink the radii by the amount of the inset. If a
     *  given radius becomes negative, it is pinned to 0.
     *
     *  If the inset amount is larger than the width/height then the rrect collapses to
     *  a degenerate line or point.
     *
     *  If the inset is sufficiently negative to cause the bounds to become infinite then
     *  the result is a default initialized rrect.
     *
     *  It is valid for dst == this.
     */
    void inset(SkScalar dx, SkScalar dy, SkRRect* dst) const;

    void inset(SkScalar dx, SkScalar dy) {
        this->inset(dx, dy, this);
    }

    /**
     *  Call outset on the bounds, and adjust the radii to reflect what happens
     *  in stroking: If the corner is sharp (no curvature), leave it alone,
     *  otherwise we grow/shrink the radii by the amount of the inset. If a
     *  given radius becomes negative, it is pinned to 0.
     *
     *  It is valid for dst == this.
     */
    void outset(SkScalar dx, SkScalar dy, SkRRect* dst) const {
        this->inset(-dx, -dy, dst);
    }
    void outset(SkScalar dx, SkScalar dy) {
        this->inset(-dx, -dy, this);
    }

    /**
     * Translate the rrect by (dx, dy).
     */
    void offset(SkScalar dx, SkScalar dy) {
        fRect.offset(dx, dy);
    }

    SkRRect SK_WARN_UNUSED_RESULT makeOffset(SkScalar dx, SkScalar dy) const {
        return SkRRect(fRect.makeOffset(dx, dy), fRadii, fType);
    }

    /**
     *  Returns true if 'rect' is wholy inside the RR, and both
     *  are not empty.
     */
    bool contains(const SkRect& rect) const;

    bool isValid() const;
    static bool AreRectAndRadiiValid(const SkRect&, const SkVector[4]);

    enum {
        kSizeInMemory = 12 * sizeof(SkScalar)
    };

    /**
     *  Write the rrect into the specified buffer. This is guaranteed to always
     *  write kSizeInMemory bytes, and that value is guaranteed to always be
     *  a multiple of 4. Return kSizeInMemory.
     */
    size_t writeToMemory(void* buffer) const;
    void writeToBuffer(SkWBuffer*) const;

    /**
     * Reads the rrect from the specified buffer
     *
     * If the specified buffer is large enough, this will read kSizeInMemory bytes,
     * and that value is guaranteed to always be a multiple of 4.
     *
     * @param buffer Memory to read from
     * @param length Amount of memory available in the buffer
     * @return number of bytes read (must be a multiple of 4) or
     *         0 if there was not enough memory available
     */
    size_t readFromMemory(const void* buffer, size_t length);
    bool readFromBuffer(SkRBuffer*);

    /**
     *  Transform by the specified matrix, and put the result in dst.
     *
     *  @param matrix SkMatrix specifying the transform. Must only contain
     *      scale and/or translate, or this call will fail.
     *  @param dst SkRRect to store the result. It is an error to use this,
     *      which would make this function no longer const.
     *  @return true on success, false on failure.
     */
    bool transform(const SkMatrix& matrix, SkRRect* dst) const;

    void dump(bool asHex) const;
    void dump() const { this->dump(false); }
    void dumpHex() const { this->dump(true); }

private:
    SkRRect(const SkRect& rect, const SkVector radii[4], int32_t type)
        : fRect(rect)
        , fRadii{radii[0], radii[1], radii[2], radii[3]}
        , fType(type) {}

    /**
     * Initializes fRect. If the passed in rect is not finite or empty the rrect will be fully
     * initialized and false is returned. Otherwise, just fRect is initialized and true is returned.
     */
    bool initializeRect(const SkRect&);

    void computeType();
    bool checkCornerContainment(SkScalar x, SkScalar y) const;
    void scaleRadii();

    SkRect fRect = SkRect::MakeEmpty();
    // Radii order is UL, UR, LR, LL. Use Corner enum to index into fRadii[]
    SkVector fRadii[4] = {{0, 0}, {0, 0}, {0,0}, {0,0}};
    // use an explicitly sized type so we're sure the class is dense (no uninitialized bytes)
    int32_t fType = kEmpty_Type;
    // TODO: add padding so we can use memcpy for flattening and not copy uninitialized data

    // to access fRadii directly
    friend class SkPath;
};

#endif
