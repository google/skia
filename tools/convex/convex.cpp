/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cctype>
#include <cstring>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using std::string;
using std::map;
using std::pair;
using std::vector;

#include "SkCommandLineFlags.h"
#include "SkPath.h"
#include "SkPathPriv.h"

DEFINE_string2(paths, p, "", "File containing paths to test for convexity");

bool file_exists(string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void check_params(float params[6], int count) {
    for (int index = 0; index < 6; ++index) {
        assert((index >= count) == SkScalarIsNaN(params[index]));
    }
}

const string evenOdd = "setFillType(SkPath::kEvenOdd_FillType);";
const string winding = "setFillType(SkPath::kWinding_FillType);";
const vector<std::pair<string, int>> verbs = {
    {"moveTo(", 2},
    {"lineTo(", 2},
    {"quadTo(", 4},
    {"conicTo(", 5},
    {"cubicTo(", 6},
    {"close(", 0},
};

struct Path {
    string fString;
    SkPath fPath;
    int fMoves { 0 };
    int fLines { 0 };
    int fQuads { 0 };
    int fConics { 0 };
    int fCubics { 0 };
    int fCloses { 0 };
    int fCount { 0 };
    bool fIsConcaveBySign { false };
    bool fIsConcaveByVerb { false };
    bool fIsConcaveByCross { false };
    int fAxisCrossed { 0 };
    int fVerbsCounted { 0 };
    int fNoOpPt { 0 };
    int fAlignedCross { 0 };
    int fUnalignedCross { 0 };
    int fUnalignedDot { 0 };
};

map<size_t, Path> pathMap;
map<string, int> convexTypes;

struct MinMaxAvg {
    void add(int val, bool isConvex) {
        if (!val) {
            return;
        }
        if (isConvex) {
            fConvexMin = std::min(fConvexMin, val);
            fConvexAvg += val;
            fConvexMax = std::max(fConvexMax, val);
            fConvexCount++;
        } else {
            fConcaveMin = std::min(fConcaveMin, val);
            fConcaveAvg += val;
            fConcaveMax = std::max(fConcaveMax, val);
            fConcaveCount++;
        }
    }

    float concaveAvg() const {
        return (float) fConcaveAvg / fConcaveCount;
    }

    float convexAvg() const {
        return (float) fConvexAvg / fConvexCount;
    }

    void statsOut(const char* note = "") {
        SkDebugf("    %s concave min=%d avg=%3.2g max=%d\n", note, fConcaveMin, concaveAvg(),
                fConcaveMax);
        SkDebugf("    %s convex min=%d avg=%3.2g max=%d\n", note, fConvexMin, convexAvg(),
                fConvexMax);
    }

    int fConcaveMin { INT_MAX };
    int fConcaveAvg { 0 };
    int fConcaveMax { 0 };
    int fConcaveCount { 0 };
    int fConvexMin { INT_MAX };
    int fConvexAvg { 0 };
    int fConvexMax { 0 };
    int fConvexCount { 0 };
};

static void path_stats(int total) {
    int convex = 0;
    int concave = 0;
    int rects = 0;
    int ovals = 0;
    int roundRects = 0;
    int max = 0;
    MinMaxAvg verbCheck;
    MinMaxAvg axisCheck;
    MinMaxAvg crossAligned;
    MinMaxAvg crossUnaligned;
    MinMaxAvg noOpPt;
    MinMaxAvg dotUnaligned;
    int concaveBySign = 0;
    int concaveByVerb = 0;
    int concaveByCross = 0;
    for (auto& onePath : pathMap) {
        const SkPath& path = onePath.second.fPath;
        bool isConvex = path.isConvex();
        if (path.isRect(nullptr)) {
            assert(isConvex);
            rects += 1;
        }
        if (path.isOval(nullptr)) {
            assert(isConvex);
            ovals += 1;
        }
        if (path.isRRect(nullptr)) {
            assert(isConvex);
            roundRects += 1;
        }
        if (isConvex) {
            convex += 1;
        } else {
            concave += 1;
        }
        max = std::max(max, onePath.second.fCount);
        verbCheck.add(onePath.second.fVerbsCounted, isConvex);
        axisCheck.add(onePath.second.fAxisCrossed, isConvex);
        crossAligned.add(onePath.second.fAlignedCross, isConvex);
        crossUnaligned.add(onePath.second.fUnalignedCross, isConvex);
        noOpPt.add(onePath.second.fNoOpPt, isConvex);
        dotUnaligned.add(onePath.second.fUnalignedDot, isConvex);
        concaveBySign += onePath.second.fIsConcaveBySign;
        concaveByVerb += onePath.second.fIsConcaveByVerb;
        concaveByCross += onePath.second.fIsConcaveByCross;
    }
    SkDebugf("\ntotal paths = %d\n", total);
    SkDebugf("unique paths = %d\n", pathMap.size());
    SkDebugf("unique convex = %d\n", convex);
    SkDebugf("  rects = %d\n", rects);
    SkDebugf("  ovals = %d\n", ovals);
    SkDebugf("  round rects = %d\n", roundRects);
    SkDebugf("unique concave = %d\n", concave);
    SkDebugf("  by verbs alone = %d\n", concaveByVerb);
    verbCheck.statsOut();
    SkDebugf("  by sign alone = %d\n", concaveBySign);
    axisCheck.statsOut();
    SkDebugf("  by cross product alone = %d\n", concaveByCross);
    noOpPt.statsOut(" duplicate pts");
    crossAligned.statsOut(" axis-aligned cross products");
    crossUnaligned.statsOut(" axis-unaligned cross products");
    dotUnaligned.statsOut(" axis-unaligned dot products");
    SkDebugf(" max duplicate paths = %d\n\n", max);
    vector<pair<string, int> > sorted;
    for (auto convexType : convexTypes) {
        sorted.push_back(convexType);
    }
    std::sort(sorted.begin(), sorted.end(), [=](pair<string, int>& a, pair<string, int>& b) {
        return a.second > b.second;
    } );
    SkDebugf("makeup of convex paths passed to isConvex()\n");
    SkDebugf("[count] verbs: count ... \n");
    SkDebugf("(moves is not shown if count is 1; closes is not shown if count is 1)\n");
    for (auto entry : sorted) {
        string key = entry.first;
        int index = 0;
        SkDebugf("[%d] ", entry.second);
        for (char c : "mlqkcz") {
            assert(c == key[index]);
            string count;
            while (isdigit(key[++index])) {
                count += key[index];
            }
            switch (c) {
                case 'm':
                    if ("1" != count) SkDebugf("moves: %s  ", count.c_str());
                    break;
                case 'l':
                    if ("0" != count) SkDebugf("lines: %s  ", count.c_str());
                    break;
                case 'q':
                    if ("0" != count) SkDebugf("quads: %s  ", count.c_str());
                    break;
                case 'k':
                    if ("0" != count) SkDebugf("conics: %s  ", count.c_str());
                    break;
                case 'c':
                    if ("0" != count) SkDebugf("cubics: %s  ", count.c_str());
                    break;
                case 'z':
                    if ("1" != count) SkDebugf("closes: %s  ", count.c_str());
                    break;
                case '\0':
                    break;
                default:
                    assert(0);
            }
        }
        SkDebugf("\n");
    }
}

// count verbs to determine is concave
static SkPath::Convexity by_verbs(const SkPath& path, int* consumed) {
    SkPoint pts[4];
    SkPath::Verb verb;
    SkPath::Iter iter(path, true);
    int contourCount = 0;
    *consumed = 0;
    bool sawCurve = false;
    while ((verb = iter.next(pts, false, false)) != SkPath::kDone_Verb) {
        *consumed += 1;
        switch (verb) {
            case SkPath::kMove_Verb:
                if (sawCurve) {
                    return SkPath::kConcave_Convexity;
                }
                break;
            case SkPath::kLine_Verb:
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kCubic_Verb:
                sawCurve = true;
                break;
            case SkPath::kClose_Verb:
                break;
            default:
                SkDEBUGFAIL("bad verb");
        }
    }
    return SkPath::kConvex_Convexity;
}

// count axis reversals to determine is concave
static int sign(SkScalar x) { return x < 0; }
#define kValueNeverReturnedBySign   2

static SkPath::Convexity by_sign(const SkPath& path, int* consumed) {
    SkPoint         pts[4];
    SkPath::Iter    iter(path, true);
    int initialMoves = 0;
    while (SkPath::kMove_Verb == iter.next(pts, false, false)) {
        ++initialMoves;
    }
    if (initialMoves == 0) {
        return SkPath::kConvex_Convexity;
    }
    std::vector<SkPoint> pointArray;
    pointArray.resize(path.getPoints(nullptr, 0));
    (void) path.getPoints(&pointArray.front(), pointArray.size());
    int skip = initialMoves - 1;
    SkPoint* points = &pointArray.front() + skip;
    size_t count = pointArray.size() - skip;

    const SkPoint* start = points;
    const SkPoint* last = points + count;
    SkPoint currPt = *points++;
    SkPoint firstPt = currPt;
    int dxes = 0;
    int dyes = 0;
    int lastSx = kValueNeverReturnedBySign;
    int lastSy = kValueNeverReturnedBySign;
    for (int outerLoop = 0; outerLoop < 2; ++outerLoop ) {
        while (points != last) {
            *consumed += 1;
            SkVector vec = *points - currPt;
            if (!vec.isZero()) {
                // give up if vector construction failed
                if (!vec.isFinite()) {
                    return SkPath::kUnknown_Convexity;
                }
                int sx = sign(vec.fX);
                int sy = sign(vec.fY);
                dxes += (sx != lastSx);
                dyes += (sy != lastSy);
                if (dxes > 3 || dyes > 3) {
                    return SkPath::kConcave_Convexity;
                }
                lastSx = sx;
                lastSy = sy;
            }
            currPt = *points++;
            if (outerLoop) {
                break;
            }
        }
        points = &firstPt;
    }
    return SkPath::kConvex_Convexity;  // that is, it may be convex, don't know yet
}

// count cross products to determine is concave
    // count axis aligned checks
enum DirChange {
    kUnknown_DirChange,
    kLeft_DirChange,
    kRight_DirChange,
    kStraight_DirChange,
    kConcave_DirChange,   // if cross on diagonal is too small, assume concave
    kBackwards_DirChange, // if double back, allow simple lines to be convex
    kInvalid_DirChange
};

static bool almost_equal(SkScalar compA, SkScalar compB) {
    // The error epsilon was empirically derived; worse case round rects
    // with a mid point outset by 2x float epsilon in tests had an error
    // of 12.
    const int epsilon = 16;
    if (!SkScalarIsFinite(compA) || !SkScalarIsFinite(compB)) {
        return false;
    }
    // no need to check for small numbers because SkPath::Iter has removed degenerate values
    int aBits = SkFloatAs2sCompliment(compA);
    int bBits = SkFloatAs2sCompliment(compB);
    return aBits < bBits + epsilon && bBits < aBits + epsilon;
}

static int sign(SkScalar x1, SkScalar x2) {
    SkASSERT(x1 != x2);
    return x2 < x1;
}

static DirChange same_sign(SkScalar curr, SkScalar last, SkScalar prior) {
    return sign(curr, last) == sign(last, prior) ? kStraight_DirChange : kBackwards_DirChange;
}

struct XConvexicator {

    DirChange directionChange() {
        // if both vectors are axis-aligned, don't do cross product
        fCurrAligned = fCurrPt.fX == fLastPt.fX || fCurrPt.fY == fLastPt.fY;
        if (fLastAligned && fCurrAligned) {
            ++fAlignedCross;
            bool noYChange = fCurrPt.fY == fLastPt.fY && fLastPt.fY == fPriorPt.fY;
            if (fCurrPt.fX == fLastPt.fX && fLastPt.fX == fPriorPt.fX) {
                if (noYChange) {
                    return kStraight_DirChange;
                }
                return same_sign(fCurrPt.fY, fLastPt.fY, fPriorPt.fY);
            }
            if (!noYChange) { // must be turn to left or right
                bool flip = fCurrPt.fX != fLastPt.fX;
                SkASSERT(flip ? fCurrPt.fY == fLastPt.fY &&
                        fLastPt.fY != fPriorPt.fY && fLastPt.fX == fPriorPt.fX :
                        fCurrPt.fY != fLastPt.fY &&
                        fLastPt.fY == fPriorPt.fY && fLastPt.fX != fPriorPt.fX);
                bool product = flip ? (fCurrPt.fX > fLastPt.fX) != (fLastPt.fY > fPriorPt.fY) :
                        (fCurrPt.fY > fLastPt.fY) == (fLastPt.fX > fPriorPt.fX);
                SkDEBUGCODE(SkVector lastV = fLastPt - fPriorPt);
                SkDEBUGCODE(SkVector curV = fCurrPt - fLastPt);
                SkDEBUGCODE(SkScalar crossV = SkPoint::CrossProduct(lastV, curV));
                SkDEBUGCODE(int signV = SkScalarSignAsInt(crossV));
                SkASSERT(signV == (product ? 1 : -1));
                return product ? kRight_DirChange : kLeft_DirChange;
            }
            return same_sign(fCurrPt.fX, fLastPt.fX, fPriorPt.fX);
        }
        // there are no subtractions above this line; axis aligned paths
        // are robust and can handle arbitrary values
        SkVector lastVec = fLastPt - fPriorPt;
        SkVector curVec = fCurrPt - fLastPt;
        SkScalar cross = SkPoint::CrossProduct(lastVec, curVec);
        ++fUnalignedCross;
        if (!SkScalarIsFinite(cross)) {
                return kUnknown_DirChange;
        }
        SkScalar smallest = SkTMin(fCurrPt.fX, SkTMin(fCurrPt.fY, SkTMin(fLastPt.fX, fLastPt.fY)));
        SkScalar largest = SkTMax(fCurrPt.fX, SkTMax(fCurrPt.fY, SkTMax(fLastPt.fX, fLastPt.fY)));
        largest = SkTMax(largest, -smallest);

        if (almost_equal(largest, largest + cross)) {
    #if SK_TREAT_COLINEAR_DIAGONAL_POINTS_AS_CONCAVE
    // colinear diagonals are not allowed; they aren't numerically stable
    #define COLINEAR_POINT_DIR_CHANGE kConcave_DirChange
    #else
    // colinear diagonals are allowed; we can survive dealing with 'close enough'
    #define COLINEAR_POINT_DIR_CHANGE kStraight_DirChange
    #endif

            ++fUnalignedDot;
            SkScalar dot = lastVec.dot(curVec);
            return dot < 0 ? kBackwards_DirChange : COLINEAR_POINT_DIR_CHANGE;
        }
        return 1 == SkScalarSignAsInt(cross) ? kRight_DirChange : kLeft_DirChange;
    }

    bool addVec() {
        DirChange dir = this->directionChange();
        switch (dir) {
            case kLeft_DirChange:       // fall through
            case kRight_DirChange:
                if (kInvalid_DirChange == fExpectedDir) {
                    fExpectedDir = dir;
                    fFirstDirection = (kRight_DirChange == dir) ? SkPathPriv::kCW_FirstDirection
                                                                : SkPathPriv::kCCW_FirstDirection;
                } else if (dir != fExpectedDir) {
                    fFirstDirection = SkPathPriv::kUnknown_FirstDirection;
                    return false;
                }
                break;
            case kStraight_DirChange:
                break;
            case kConcave_DirChange:
                fFirstDirection = SkPathPriv::kUnknown_FirstDirection;
                return false;
            case kBackwards_DirChange:
                //  allow path to reverse direction twice
                //    Given path.moveTo(0, 0); path.lineTo(1, 1);
                //    - 1st reversal: direction change formed by line (0,0 1,1), line (1,1 0,0)
                //    - 2nd reversal: direction change formed by line (1,1 0,0), line (0,0 1,1)
                return ++fReversals < 3;
            case kUnknown_DirChange:
                return (fIsFinite = false);
            case kInvalid_DirChange:
                SK_ABORT("Use of invalid direction change flag");
                break;
        }
        return true;
    }

    bool addPt(const SkPoint& pt) {
        if (fCurrPt == pt) {
            ++fNoOpPt;
            return true;
        }
        fCurrPt = pt;
        if (fPriorPt == fLastPt) {  // should only be true for first non-zero vector
            ++fNoOpPt;
            fFirstPt = pt;
            fCurrAligned = pt.fX == fLastPt.fX || pt.fY == fLastPt.fY;
        } else if (!this->addVec()) {
            return false;
        }
        fPriorPt = fLastPt;
        fLastPt = fCurrPt;
        fLastAligned = fCurrAligned;
        return true;
    }

    SkPath::Convexity byCross(const SkPath& path) {
        SkPoint         pts[4];
        SkPath::Verb    verb;
        SkPath::Iter    iter(path, true);
        int count;

        while ((verb = iter.next(pts, false, false)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                    fPriorPt = fLastPt = fCurrPt = pts[0];
                    count = 0;
                    break;
                case SkPath::kLine_Verb:
                    count = 1;
                    break;
                case SkPath::kQuad_Verb:
                    // fall through
                case SkPath::kConic_Verb:
                    count = 2;
                    break;
                case SkPath::kCubic_Verb:
                    count = 3;
                    break;
                case SkPath::kClose_Verb:
                    if (!this->addPt(fFirstPt)) {
                        return SkPath::kConcave_Convexity;
                    }
                    count = 0;
                    break;
                default:
                    SkDEBUGFAIL("bad verb");
            }
            for (int i = 1; i <= count; i++) {
                if (!this->addPt(pts[i])) {
                    return SkPath::kConcave_Convexity;
                }
            }
        }
        return SkPath::kConvex_Convexity;
    }

    SkPoint             fFirstPt {0, 0};
    SkPoint             fPriorPt {0, 0};
    SkPoint             fLastPt {0, 0};
    SkPoint             fCurrPt {0, 0};
    DirChange           fExpectedDir { kInvalid_DirChange };
    SkPathPriv::FirstDirection   fFirstDirection { SkPathPriv::kUnknown_FirstDirection };
    int                 fReversals { 0 };
    bool                fIsFinite { true };
    bool                fLastAligned { true };
    bool                fCurrAligned { true };

    int fNoOpPt { 0 };
    int fAlignedCross { 0 };
    int fUnalignedCross { 0 };
    int fUnalignedDot { 0 };
};

int main(int argc, char** const argv) {
    SkCommandLineFlags::Parse(argc, argv);
    string pathFileName = FLAGS_paths.count() ? FLAGS_paths[0] :
        "C://Users/Cary Clark/Downloads/skpout.txt";
    assert(file_exists(pathFileName));
    string line;
    string pathString;
    SkPath path;
    std::ifstream infile(pathFileName);
    int moves = 0;
    int lines = 0;
    int quads = 0;
    int conics = 0;
    int cubics = 0;
    int closes = 0;
    int total = 0;
    auto add_path = [&]() {
        if (!pathString.empty()) {
            if (path.isConvex()) {
                string key = "m" + std::to_string(moves)
                        + "l" + std::to_string(lines)
                        + "q" + std::to_string(quads)
                        + "k" + std::to_string(conics)
                        + "c" + std::to_string(cubics)
                        + "z" + std::to_string(closes);
                auto convexIter = convexTypes.find(key);
                if (convexTypes.end() == convexIter) {
                    convexTypes[key] = 1;
                } else {
                    convexIter->second++;
                }
            }
            int axisCrossed = 0;
            SkPath::Convexity bySignConvexity = by_sign(path, &axisCrossed);
            int verbsCounted = 0;
            SkPath::Convexity byVerbConvexity = by_verbs(path, &verbsCounted);
            XConvexicator convexicator;
            SkPath::Convexity byCrossConvexity = convexicator.byCross(path);

            size_t pathHash = std::hash<string>{}(pathString);
            auto iter = pathMap.find(pathHash);
            if (pathMap.end() == iter) {
                Path entry = { pathString, path, moves, lines, quads, conics, cubics, closes, 1,
                        bySignConvexity == SkPath::kConcave_Convexity,
                        byVerbConvexity == SkPath::kConcave_Convexity,
                        byCrossConvexity == SkPath::kConcave_Convexity,
                        axisCrossed, verbsCounted, convexicator.fNoOpPt, convexicator.fAlignedCross,
                        convexicator.fUnalignedCross, convexicator.fUnalignedDot };
                pathMap[pathHash] = entry;
            } else {
                iter->second.fCount += 1;
            }
            path.reset();
        }
    };
    while (std::getline(infile, line)) {
        assert("path." == line.substr(0, 5));
        string part = line.substr(5);
        if (evenOdd == part || winding == part) {
            add_path();
            path.setFillType(evenOdd == part ?
                    SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType);
            pathString = line;
            moves = lines = quads = conics = cubics = closes = 0;
            ++total;
            continue;
        }
        pathString += line;
        SkDEBUGCODE(bool sawClose = false);
        for (auto verb : verbs) {
            if (verb.first == part.substr(0, verb.first.length())) {
                part = part.substr(verb.first.length());
                float params[6] = { SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN,
                                    SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN };
                for (int index = 0; index < verb.second; ++index) {
                    const string bits = "SkBits2Float(";
                    assert(bits == part.substr(0, bits.length()));
                    part = part.substr(bits.length());
                    size_t eaten = 0;
                    unsigned int hex = std::stoul(part, &eaten, 16);
                    assert(10 == eaten);
                    params[index] = SkBits2Float(hex);
                    part = part.substr(eaten);
                    assert(')' == part[0]);
                    if (index + 1 < verb.second) {
                        assert(", " == part.substr(1, 2));
                        part = part.substr(3);
                    }
                }
                if ("moveTo(" == verb.first) {
                    check_params(params, 2);
                    path.moveTo(params[0], params[1]);
                    ++moves;
                } else if ("lineTo(" == verb.first) {
                    check_params(params, 2);
                    path.lineTo(params[0], params[1]);
                    ++lines;
                } else if ("quadTo(" == verb.first) {
                    check_params(params, 4);
                    path.quadTo(params[0], params[1], params[2], params[3]);
                    ++quads;
                } else if ("conicTo(" == verb.first) {
                    check_params(params, 5);
                    path.conicTo(params[0], params[1], params[2], params[3], params[4]);
                    ++conics;
                } else if ("cubicTo(" == verb.first) {
                    check_params(params, 6);
                    path.cubicTo(params[0], params[1], params[2], params[3], params[4], params[5]);
                    ++cubics;
                } else if ("close(" == verb.first) {
                    check_params(params, 0);
                    path.close();
                    ++closes;
                    SkDEBUGCODE(sawClose = true);
                } else {
                    assert(0);
                }
                break;
            }
        }
        assert(sawClose || "));  //" == part.substr(0, 7));
    }
    add_path();
    path_stats(total);
}
