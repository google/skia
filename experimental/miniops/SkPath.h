#ifndef SkPath_DEFINED
#define SkPath_DEFINED

#include "SkMatrix.h"

class SK_API SkPath {
public:

    enum FillType {
        kWinding_FillType,
        kEvenOdd_FillType,
        kInverseWinding_FillType,
        kInverseEvenOdd_FillType,
    };

    FillType getFillType() const { return (FillType)fFillType; }

    void setFillType(FillType ft) {
        fFillType = SkToU8(ft);
    }
    enum Verb {
        kMove_Verb,  //!< Starts new contour at next SkPoint.
        kLine_Verb,
        kQuad_Verb,
        kConic_Verb,
        kCubic_Verb,
        kClose_Verb, //!< Closes contour, connecting last point to kMove_Verb SkPoint.
        kDone_Verb,  //!< Terminates SkPath. Not in verb array, but returned by SkPath iterator.
    };

    void moveTo(const SkPoint& p);
    void lineTo(const SkPoint& p);
    void quadTo(const SkPoint& p1, const SkPoint& p2);
    void conicTo(const SkPoint& p1, const SkPoint& p2, SkScalar w);
    void cubicTo(const SkPoint& p1, const SkPoint& p2, const SkPoint& p3);

    static bool IsInverseFillType(FillType fill) {
        static_assert(0 == kWinding_FillType, "fill_type_mismatch");
        static_assert(1 == kEvenOdd_FillType, "fill_type_mismatch");
        static_assert(2 == kInverseWinding_FillType, "fill_type_mismatch");
        static_assert(3 == kInverseEvenOdd_FillType, "fill_type_mismatch");
        return (fill & 2) != 0;
    }

    enum AddPathMode {
        kAppend_AddPathMode,
        kExtend_AddPathMode,
    };

    void addPath(const SkPath& src, AddPathMode mode = kAppend_AddPathMode);
    int countVerbs() const;
    const SkRect& getBounds() const;
    bool getLastPt(SkPoint* lastPt) const;
    int getVerbs(uint8_t verbs[], int max) const;
    void close();
    void reset();
    bool isConvex() const;
    bool isEmpty() const;
    bool isFinite() const;
    bool isInverseFillType() const { return IsInverseFillType((FillType)fFillType); }

    void reversePathTo(const SkPath&);
    void transform(const SkMatrix& matrix);

    class SK_API RawIter {
    public:
        RawIter(const SkPath& path);
        Verb next(SkPoint pts[4]);
        SkScalar conicWeight() const;
    };

private:
    uint8_t                                              fFillType;

};

#endif
