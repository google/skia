#include "GrPath.h"

GrPath::GrPath() {}

GrPath::GrPath(const GrPath& src) : INHERITED() {
}

GrPath::GrPath(GrPathIter& iter) {
    this->resetFromIter(&iter);
}

GrPath::~GrPath() {
}

void GrPath::ensureMoveTo() {
    if (fVerbs.isEmpty() || this->wasLastVerb(kClose)) {
        *fVerbs.append() = kMove;
        fPts.append()->set(0, 0);
    }
}

void GrPath::moveTo(GrScalar x, GrScalar y) {
    if (this->wasLastVerb(kMove)) {
        // overwrite prev kMove value
        fPts[fPts.count() - 1].set(x, y);
    } else {
        *fVerbs.append() = kMove;
        fPts.append()->set(x, y);
    }
}

void GrPath::lineTo(GrScalar x, GrScalar y) {
    this->ensureMoveTo();
    *fVerbs.append() = kLine;
    fPts.append()->set(x, y);
}

void GrPath::quadTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1) {
    this->ensureMoveTo();
    *fVerbs.append() = kQuad;
    fPts.append()->set(x0, y0);
    fPts.append()->set(x1, y1);
}

void GrPath::cubicTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1,
                     GrScalar x2, GrScalar y2) {
    this->ensureMoveTo();
    *fVerbs.append() = kCubic;
    fPts.append()->set(x0, y0);
    fPts.append()->set(x1, y1);
    fPts.append()->set(x2, y2);
}

void GrPath::close() {
    if (!fVerbs.isEmpty() && !this->wasLastVerb(kClose)) {
        // should we allow kMove followed by kClose?
        *fVerbs.append() = kClose;
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrPath::resetFromIter(GrPathIter* iter) {
    fPts.reset();
    fVerbs.reset();

    GrPoint pts[4];
    GrPathIter::Command cmd;

    while ((cmd = iter->next(pts)) != GrPathIter::kEnd_Command) {
        switch (cmd) {
            case GrPathIter::kMove_Command:
                this->moveTo(pts[0].fX, pts[0].fY);
                break;
            case GrPathIter::kLine_Command:
                this->lineTo(pts[1].fX, pts[1].fY);
                break;
            case GrPathIter::kQuadratic_Command:
                this->quadTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case GrPathIter::kCubic_Command:
                this->cubicTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY,
                              pts[3].fX, pts[3].fY);
                break;
            case GrPathIter::kClose_Command:
                this->close();
                break;
            case GrPathIter::kEnd_Command:
                // never get here, but include it to avoid the warning
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

GrPath::Iter::Iter(const GrPath& path) : fPath(path) {
    this->rewind();
}

GrPathIter::Command GrPath::Iter::next(GrPoint points[]) {
    if (fVerbIndex == fPath.fVerbs.count()) {
        GrAssert(fPtIndex == fPath.fPts.count());
        return GrPathIter::kEnd_Command;
    } else {
        GrAssert(fVerbIndex < fPath.fVerbs.count());
    }

    uint8_t cmd = fPath.fVerbs[fVerbIndex++];
    const GrPoint* srcPts = fPath.fPts.begin() + fPtIndex;

    switch (cmd) {
        case kMove:
            if (points) {
                points[0] = srcPts[0];
            }
            fLastPt = srcPts[0];
            GrAssert(fPtIndex <= fPath.fPts.count() + 1);
            fPtIndex += 1;
            break;
        case kLine:
            if (points) {
                points[0] = fLastPt;
                points[1] = srcPts[0];
            }
            fLastPt = srcPts[0];
            GrAssert(fPtIndex <= fPath.fPts.count() + 1);
            fPtIndex += 1;
            break;
        case kQuad:
            if (points) {
                points[0] = fLastPt;
                points[1] = srcPts[0];
                points[2] = srcPts[1];
            }
            fLastPt = srcPts[2];
            GrAssert(fPtIndex <= fPath.fPts.count() + 2);
            fPtIndex += 2;
            break;
        case kCubic:
            if (points) {
                points[0] = fLastPt;
                points[1] = srcPts[0];
                points[2] = srcPts[1];
                points[3] = srcPts[2];
            }
            fLastPt = srcPts[2];
            GrAssert(fPtIndex <= fPath.fPts.count() + 3);
            fPtIndex += 3;
            break;
        case kClose:
            break;
        default:
            GrAssert(!"unknown grpath verb");
            break;
    }
    return (GrPathIter::Command)cmd;
}

GrPathIter::ConvexHint GrPath::Iter::hint() const {
    return fPath.getConvexHint();
}

GrPathIter::Command GrPath::Iter::next() {
    return this->next(NULL);
}

void GrPath::Iter::rewind() {
    fVerbIndex = fPtIndex = 0;
}



