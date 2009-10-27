#ifndef SkBoundaryPatch_DEFINED
#define SkBoundaryPatch_DEFINED

#include "SkPoint.h"
#include "SkRefCnt.h"

class SkBoundaryCurve : public SkRefCnt {
public:
    virtual SkPoint evaluate(SkScalar unitInterval) = 0;
};

class SkBoundaryPatch {
public:
    SkBoundaryPatch();
    ~SkBoundaryPatch();

    enum Edge {
        kLeft,
        kTop,
        kRight,
        kBottom
    };
    
    SkBoundaryCurve* getCurve(Edge e) const { return fCurve[e]; }
    SkBoundaryCurve* setCurve(Edge e, SkBoundaryCurve*);

    SkPoint evaluate(SkScalar unitU, SkScalar unitV);

private:
    SkBoundaryCurve* fCurve[4];
};

////////////////////////////////////////////////////////////////////////

class SkLineBoundaryCurve : public SkBoundaryCurve {
public:
    SkPoint fPts[2];
    
    // override
    virtual SkPoint evaluate(SkScalar);
};

class SkCubicBoundaryCurve : public SkBoundaryCurve {
public:
    SkPoint fPts[4];
    
    // override
    virtual SkPoint evaluate(SkScalar);
};

#endif

