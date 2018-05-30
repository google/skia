#ifndef SkRect_DEFINED
#define SkRect_DEFINED

#include "SkPoint.h"

struct SK_API SkRect {
    SkScalar fLeft;
    SkScalar fTop;
    SkScalar fRight;
    SkScalar fBottom;

    SkScalar    left() const { return fLeft; }
    SkScalar    top() const { return fTop; }
    SkScalar    right() const { return fRight; }
    SkScalar    bottom() const { return fBottom; }

    void set(const SkPoint pts[], int count);

    void set(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
        fLeft   = left;
        fTop    = top;
        fRight  = right;
        fBottom = bottom;
    }

    bool isEmpty() const;
    void join(const SkRect& r);

    static bool Intersects(const SkRect& a, const SkRect& b) {
        return Intersects(a.fLeft, a.fTop, a.fRight, a.fBottom,
                          b.fLeft, b.fTop, b.fRight, b.fBottom);
    }

private:
    static bool Intersects(SkScalar al, SkScalar at, SkScalar ar, SkScalar ab,
                           SkScalar bl, SkScalar bt, SkScalar br, SkScalar bb) {
        SkScalar L = SkMaxScalar(al, bl);
        SkScalar R = SkMinScalar(ar, br);
        SkScalar T = SkMaxScalar(at, bt);
        SkScalar B = SkMinScalar(ab, bb);
        return L < R && T < B;
    }

};

#endif
