#include "GrPath.h"

GrPath::GrPath() {
    fConvexHint = kNone_ConvexHint;
}

GrPath::GrPath(const GrPath& src) : INHERITED() {
    GrPath::Iter iter(src);
    this->resetFromIter(&iter);
}

GrPath::GrPath(GrPathIter& iter) {
    this->resetFromIter(&iter);
}

GrPath::~GrPath() {
}

bool GrPath::operator ==(const GrPath& path) const {
    if (fCmds.count() != path.fCmds.count() ||
        fPts.count() != path.fPts.count()) {
        return false;
    }

    for (int v = 0; v < fCmds.count(); ++v) {
        if (fCmds[v] != path.fCmds[v]) {
            return false;
        }
    }

    for (int p = 0; p < fPts.count(); ++p) {
        if (fPts[p] != path.fPts[p]) {
            return false;
        }
    }
    return true;
}

void GrPath::ensureMoveTo() {
    if (fCmds.isEmpty() || this->wasLastVerb(kClose_PathCmd)) {
        *fCmds.append() = kMove_PathCmd;
        fPts.append()->set(0, 0);
    }
}

void GrPath::moveTo(GrScalar x, GrScalar y) {
    if (this->wasLastVerb(kMove_PathCmd)) {
        // overwrite prev kMove value
        fPts[fPts.count() - 1].set(x, y);
    } else {
        *fCmds.append() = kMove_PathCmd;
        fPts.append()->set(x, y);
    }
}

void GrPath::lineTo(GrScalar x, GrScalar y) {
    this->ensureMoveTo();
    *fCmds.append() = kLine_PathCmd;
    fPts.append()->set(x, y);
}

void GrPath::quadTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1) {
    this->ensureMoveTo();
    *fCmds.append() = kQuadratic_PathCmd;
    fPts.append()->set(x0, y0);
    fPts.append()->set(x1, y1);
}

void GrPath::cubicTo(GrScalar x0, GrScalar y0, GrScalar x1, GrScalar y1,
                     GrScalar x2, GrScalar y2) {
    this->ensureMoveTo();
    *fCmds.append() = kCubic_PathCmd;
    fPts.append()->set(x0, y0);
    fPts.append()->set(x1, y1);
    fPts.append()->set(x2, y2);
}

void GrPath::close() {
    if (!fCmds.isEmpty() && !this->wasLastVerb(kClose_PathCmd)) {
        // should we allow kMove followed by kClose?
        *fCmds.append() = kClose_PathCmd;
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrPath::offset(GrScalar tx, GrScalar ty) {
    if (!tx && !ty) {
        return; // nothing to do
    }

    GrPoint* iter = fPts.begin();
    GrPoint* stop = fPts.end();
    while (iter < stop) {
        iter->offset(tx, ty);
        ++iter;
    }
}

///////////////////////////////////////////////////////////////////////////////

static bool check_two_vecs(const GrVec& prevVec,
                           const GrVec& currVec,
                           GrScalar turnDir,
                           int* xDir,
                           int* yDir,
                           int* flipX,
                           int* flipY) {
    if (currVec.fX * *xDir < 0) {
        ++*flipX;
        if (*flipX > 2) {
            return false;
        }
        *xDir = -*xDir;
    }
    if (currVec.fY * *yDir < 0) {
        ++*flipY;
        if (*flipY > 2) {
            return false;
        }
        *yDir = -*yDir;
    }
    GrScalar d = prevVec.cross(currVec);
    return (d * turnDir) >= 0;
}

static void init_from_two_vecs(const GrVec& firstVec,
                               const GrVec& secondVec,
                               GrScalar* turnDir,
                               int* xDir, int* yDir) {
    *turnDir = firstVec.cross(secondVec);
    if (firstVec.fX > 0) {
        *xDir = 1;
    } else if (firstVec.fX < 0) {
        *xDir = -1;
    } else {
        *xDir = 0;
    }
    if (firstVec.fY > 0) {
        *yDir = 1;
    } else if (firstVec.fY < 0) {
        *yDir = -1;
    } else {
        *yDir = 0;
    }
}

void GrPath::resetFromIter(GrPathIter* iter) {
    fPts.reset();
    fCmds.reset();

    fConvexHint = iter->convexHint();

    // first point of the subpath
    GrPoint firstPt(0,0);
    // first edge of the subpath
    GrVec firstVec(0,0);
    // vec of most recently processed edge, that wasn't degenerate
    GrVec previousVec(0,0);
    // most recently processed point
    GrPoint previousPt(0,0);

    // sign indicates whether we're bending left or right
    GrScalar turnDir = 0;
    // number of times the direction has flipped in x or y

    // we track which direction we are moving in x/y and the
    // number of times it changes.
    int xDir = 0;
    int yDir = 0;
    int flipX = 0;
    int flipY = 0;

    // counts number of sub path pts that didn't add a degenerate edge.
    int subPathPts = 0;
    bool subPathClosed = false;

    int numSubPaths = 0;
    iter->rewind();
    GrPathCmd cmd;
    GrPoint pts[4];
    do {
        cmd = iter->next(pts);
        // If the convexity test is ever updated to handle multiple subpaths
        // the loop has to be adjusted to handle moving to a new subpath without
        // closing the previous one. Currently the implicit closing vectors for a
        // filled path would never be examined.
        switch (cmd) {
            case kMove_PathCmd:
                this->moveTo(pts[0].fX, pts[0].fY);
                subPathPts = 0;
                subPathClosed = false;
                break;
            case kLine_PathCmd:
                this->lineTo(pts[1].fX, pts[1].fY);
                break;
            case kQuadratic_PathCmd:
                this->quadTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case kCubic_PathCmd:
                this->cubicTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY,
                              pts[3].fX, pts[3].fY);
                break;
            case kClose_PathCmd:
                this->close();
                subPathClosed = true;
                break;
            case kEnd_PathCmd:
                break;
        }
        int n = NumPathCmdPoints(cmd);
        if (0 == subPathPts && n > 0) {
            previousPt = pts[0];
            firstPt = previousPt;
            flipX = 0;
            flipY = 0;
            turnDir = 0;
            subPathPts = 1;
            ++numSubPaths;
        }
        // either we skip the first pt because it is redundant with
        // last point of the previous subpath cmd or we just ate it
        // in the above if.
        int consumed = 1;
        if (numSubPaths < 2 && kNone_ConvexHint == fConvexHint) {
            while (consumed < n) {
                GrAssert(pts[consumed-1] == previousPt);
                GrVec vec;
                vec.setBetween(previousPt, pts[consumed]);
                if (vec.fX || vec.fY) {
                    if (subPathPts >= 2) {
                        if (0 == turnDir) {
                            firstVec = previousVec;
                            init_from_two_vecs(firstVec, vec,
                                               &turnDir, &xDir, &yDir);
                            // here we aren't checking whether the x/y dirs
                            // change between the first and second edge. It
                            // gets covered when the path is closed.
                        } else {
                            if (!check_two_vecs(previousVec, vec, turnDir,
                                                &xDir, &yDir,
                                                &flipX, &flipY)) {
                                fConvexHint = kConcave_ConvexHint;
                                break;
                            }
                        }
                    }
                    previousVec = vec;
                    previousPt = pts[consumed];
                    ++subPathPts;
                }
                ++consumed;
            }
            if (subPathPts > 2 && (kClose_PathCmd == cmd ||
                        (!subPathClosed && kEnd_PathCmd == cmd ))) {
                // if an additional vector is needed to close the loop check
                // that it validates against the previous vector.
                GrVec vec;
                vec.setBetween(previousPt, firstPt);
                if (vec.fX || vec.fY) {
                    if (!check_two_vecs(previousVec, vec, turnDir,
                                        &xDir, &yDir, &flipX, &flipY)) {
                        fConvexHint = kConcave_ConvexHint;
                        break;
                    }
                    previousVec = vec;
                }
                // check that closing vector validates against the first vector.
                if (!check_two_vecs(previousVec, firstVec, turnDir,
                                    &xDir, &yDir, &flipX, &flipY)) {
                    fConvexHint = kConcave_ConvexHint;
                    break;
                }
            }
        }
    } while (cmd != kEnd_PathCmd);
    if (kNone_ConvexHint == fConvexHint && numSubPaths < 2) {
        fConvexHint = kConvex_ConvexHint;
    } else {
        bool recurse = false;
        if (recurse) {
            this->resetFromIter(iter);
        }
    }
}

void GrPath::ConvexUnitTest() {
    GrPath testPath;
    GrPath::Iter testIter;

    GrPath pt;
    pt.moveTo(0, 0);
    pt.close();

    testIter.reset(pt);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());

    GrPath line;
    line.moveTo(GrIntToScalar(12), GrIntToScalar(20));
    line.lineTo(GrIntToScalar(-12), GrIntToScalar(-20));
    line.close();

    testIter.reset(line);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());

    GrPath triLeft;
    triLeft.moveTo(0, 0);
    triLeft.lineTo(1, 0);
    triLeft.lineTo(1, 1);
    triLeft.close();

    testIter.reset(triLeft);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());

    GrPath triRight;
    triRight.moveTo(0, 0);
    triRight.lineTo(-1, 0);
    triRight.lineTo(1, 1);
    triRight.close();

    testIter.reset(triRight);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());

    GrPath square;
    square.moveTo(0, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 1);
    square.lineTo(0, 1);
    square.close();

    testIter.reset(square);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());

    GrPath redundantSquare;
    square.moveTo(0, 0);
    square.lineTo(0, 0);
    square.lineTo(0, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 0);
    square.lineTo(1, 1);
    square.lineTo(1, 1);
    square.lineTo(1, 1);
    square.lineTo(0, 1);
    square.lineTo(0, 1);
    square.lineTo(0, 1);
    square.close();

    testIter.reset(redundantSquare);
    testPath.resetFromIter(&testIter);
    GrAssert(kConvex_ConvexHint == testPath.getConvexHint());

    GrPath bowTie;
    bowTie.moveTo(0, 0);
    bowTie.lineTo(0, 0);
    bowTie.lineTo(0, 0);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 1);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(1, 0);
    bowTie.lineTo(0, 1);
    bowTie.lineTo(0, 1);
    bowTie.lineTo(0, 1);
    bowTie.close();

    testIter.reset(bowTie);
    testPath.resetFromIter(&testIter);
    GrAssert(kConcave_ConvexHint == testPath.getConvexHint());

    GrPath spiral;
    spiral.moveTo(0, 0);
    spiral.lineTo(1, 0);
    spiral.lineTo(1, 1);
    spiral.lineTo(0, 1);
    spiral.lineTo(0,.5);
    spiral.lineTo(.5,.5);
    spiral.lineTo(.5,.75);
    spiral.close();

    testIter.reset(spiral);
    testPath.resetFromIter(&testIter);
    GrAssert(kConcave_ConvexHint == testPath.getConvexHint());

    GrPath dent;
    dent.moveTo(0, 0);
    dent.lineTo(1, 1);
    dent.lineTo(0, 1);
    dent.lineTo(-.5,2);
    dent.lineTo(-2, 1);
    dent.close();

    testIter.reset(dent);
    testPath.resetFromIter(&testIter);
    GrAssert(kConcave_ConvexHint == testPath.getConvexHint());
}
///////////////////////////////////////////////////////////////////////////////

GrPath::Iter::Iter() : fPath(NULL) {
}

GrPath::Iter::Iter(const GrPath& path) : fPath(&path) {
    this->rewind();
}

GrPathCmd GrPath::Iter::next(GrPoint points[]) {
    if (fCmdIndex == fPath->fCmds.count()) {
        GrAssert(fPtIndex == fPath->fPts.count());
        return kEnd_PathCmd;
    } else {
        GrAssert(fCmdIndex < fPath->fCmds.count());
    }

    GrPathCmd cmd = fPath->fCmds[fCmdIndex++];
    const GrPoint* srcPts = fPath->fPts.begin() + fPtIndex;

    switch (cmd) {
        case kMove_PathCmd:
            if (points) {
                points[0] = srcPts[0];
            }
            fLastPt = srcPts[0];
            GrAssert(fPtIndex <= fPath->fPts.count() + 1);
            fPtIndex += 1;
            break;
        case kLine_PathCmd:
            if (points) {
                points[0] = fLastPt;
                points[1] = srcPts[0];
            }
            fLastPt = srcPts[0];
            GrAssert(fPtIndex <= fPath->fPts.count() + 1);
            fPtIndex += 1;
            break;
        case kQuadratic_PathCmd:
            if (points) {
                points[0] = fLastPt;
                points[1] = srcPts[0];
                points[2] = srcPts[1];
            }
            fLastPt = srcPts[1];
            GrAssert(fPtIndex <= fPath->fPts.count() + 2);
            fPtIndex += 2;
            break;
        case kCubic_PathCmd:
            if (points) {
                points[0] = fLastPt;
                points[1] = srcPts[0];
                points[2] = srcPts[1];
                points[3] = srcPts[2];
            }
            fLastPt = srcPts[2];
            GrAssert(fPtIndex <= fPath->fPts.count() + 3);
            fPtIndex += 3;
            break;
        case kClose_PathCmd:
            break;
        default:
            GrAssert(!"unknown grpath cmd");
            break;
    }
    return cmd;
}

GrConvexHint GrPath::Iter::convexHint() const {
    return fPath->getConvexHint();
}

GrPathCmd GrPath::Iter::next() {
    return this->next(NULL);
}

void GrPath::Iter::rewind() {
    this->reset(*fPath);
}

void GrPath::Iter::reset(const GrPath& path) {
    fPath = &path;
    fCmdIndex = fPtIndex = 0;
}


