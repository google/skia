/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkPathPriv.h"
#include "tests/SubsetPath.h"

SubsetPath::SubsetPath(const SkPath& path)
        : fPath(path)
        , fSubset(1) {
}

int SubsetPath::range(int* end) const {
    int leadingZero = SkCLZ(fSubset);
    int parts = 1 << (31 - leadingZero);
    int partIndex = fSubset - parts;
    SkASSERT(partIndex >= 0);
    int count = fSelected.size();
    int start = count * partIndex / parts;
    *end = count * (partIndex + 1) / parts;
    return start;
}

bool SubsetPath::subset(bool testFailed, SkPath* sub) {
    int start, end;
    if (!testFailed) {
        start = range(&end);
        for (; start < end; ++start) {
            fSelected[start] = true;
        }
    }
    do {
        do {
            ++fSubset;
            start = range(&end);
 //           SkDebugf("%d s=%d e=%d t=%d\n", fSubset, start, end, fTries);
            if (end - start > 1) {
                fTries = fSelected.size();
            } else if (end - start == 1) {
                if (--fTries <= 0) {
                    return false;
                }
            }
        } while (start == end);
    } while (!fSelected[start]);
    for (; start < end; ++start) {
        fSelected[start] = false;
    }
#if 1
    SkDebugf("selected: ");
    for (int index = 0; index < fSelected.size(); ++index) {
        SkDebugf("%c", fSelected[index] ? 'x' : '-');
    }
#endif
    *sub = getSubsetPath();
    return true;
}

SubsetContours::SubsetContours(const SkPath& path)
        : SubsetPath(path) {
    bool foundCurve = false;
    int contourCount = 0;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        switch (verb) {
            case SkPathVerb::kMove:
                break;
            case SkPathVerb::kLine:
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
                foundCurve = true;
                break;
            case SkPathVerb::kClose:
                ++contourCount;
                foundCurve = false;
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
    contourCount += foundCurve;
    for (int index = 0; index < contourCount; ++index) {
        *fSelected.append() = true;
    }
    fTries = contourCount;
}

SkPath SubsetContours::getSubsetPath() const {
    SkPath result;
    result.setFillType(fPath.getFillType());
    if (!fSelected.size()) {
        return result;
    }
    int contourCount = 0;
    bool enabled = fSelected[0];
    bool addMoveTo = true;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        if (enabled && addMoveTo) {
            result.moveTo(pts[0]);
            addMoveTo = false;
        }
        switch (verb) {
            case SkPathVerb::kMove:
                break;
            case SkPathVerb::kLine:
                if (enabled) {
                    result.lineTo(pts[1]);
                }
                break;
            case SkPathVerb::kQuad:
                if (enabled) {
                    result.quadTo(pts[1], pts[2]);
                }
                break;
            case SkPathVerb::kConic:
                if (enabled) {
                    result.conicTo(pts[1], pts[2], *w);
                }
                break;
            case SkPathVerb::kCubic:
                 if (enabled) {
                    result.cubicTo(pts[1], pts[2], pts[3]);
                }
                break;
            case SkPathVerb::kClose:
                if (enabled) {
                    result.close();
                }
                if (++contourCount >= fSelected.size()) {
                    break;
                }
                enabled = fSelected[contourCount];
                addMoveTo = true;
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return result;
        }
    }
    return result;
}

SubsetVerbs::SubsetVerbs(const SkPath& path)
        : SubsetPath(path) {
    int verbCount = 0;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        switch (verb) {
            case SkPathVerb::kMove:
                break;
            case SkPathVerb::kLine:
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
                ++verbCount;
                break;
            case SkPathVerb::kClose:
                break;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
    }
    for (int index = 0; index < verbCount; ++index) {
        *fSelected.append() = true;
    }
    fTries = verbCount;
}

SkPath SubsetVerbs::getSubsetPath() const {
    SkPath result;
    result.setFillType(fPath.getFillType());
    if (!fSelected.size()) {
        return result;
    }
    int verbIndex = 0;
    bool addMoveTo = true;
    bool addLineTo = false;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(fPath)) {
        bool enabled = SkPathVerb::kLine <= verb && verb <= SkPathVerb::kCubic
            ? fSelected[verbIndex++] : false;
        if (enabled) {
            if (addMoveTo) {
                result.moveTo(pts[0]);
                addMoveTo = false;
            } else if (addLineTo) {
                result.lineTo(pts[0]);
                addLineTo = false;
            }
        }
        switch (verb) {
            case SkPathVerb::kMove:
                break;
            case SkPathVerb::kLine:
                if (enabled) {
                    result.lineTo(pts[1]);
                }
                break;
            case SkPathVerb::kQuad:
                if (enabled) {
                    result.quadTo(pts[1], pts[2]);
                }
                break;
            case SkPathVerb::kConic:
                if (enabled) {
                    result.conicTo(pts[1], pts[2], *w);
                }
                break;
            case SkPathVerb::kCubic:
                 if (enabled) {
                    result.cubicTo(pts[1], pts[2], pts[3]);
                }
                break;
            case SkPathVerb::kClose:
                result.close();
                addMoveTo = true;
                addLineTo = false;
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return result;
        }
        addLineTo = !enabled;
    }
    return result;
}
