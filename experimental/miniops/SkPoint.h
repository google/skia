#ifndef SkPoint_DEFINED
#define SkPoint_DEFINED

#include "SkTypes.h"
#include "SkScalar.h"

struct SkPoint;
typedef SkPoint SkVector;

struct SK_API SkPoint {
    SkScalar fX;
    SkScalar fY;

    friend bool operator==(const SkPoint& a, const SkPoint& b) {
        return a.fX == b.fX && a.fY == b.fY;
    }

    friend bool operator!=(const SkPoint& a, const SkPoint& b) {
        return a.fX != b.fX || a.fY != b.fY;
    }

    friend SkVector operator-(const SkPoint& a, const SkPoint& b) {
        return {a.fX - b.fX, a.fY - b.fY};
    }

    SkScalar dot(const SkVector& vec) const;

};


#endif
