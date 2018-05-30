#ifndef SkPointPriv_DEFINED
#define SkPointPriv_DEFINED

class SkPointPriv {
public:

    static SkScalar DistanceToSqd(const SkPoint& pt, const SkPoint& a) {
        SkScalar dx = pt.fX - a.fX;
        SkScalar dy = pt.fY - a.fY;
        return dx * dx + dy * dy;
    }

};

#endif
