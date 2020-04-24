/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMathPriv.h"
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
    int count = fSelected.count();
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
                fTries = fSelected.count();
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
    for (int index = 0; index < fSelected.count(); ++index) {
        SkDebugf("%c", fSelected[index] ? 'x' : '-');
    }
#endif
    *sub = getSubsetPath();
    return true;
}

SubsetContours::SubsetContours(const SkPath& path)
        : SubsetPath(path) {
    SkPath::RawIter iter(fPath);
    uint8_t verb;
    SkPoint pts[4];
    bool foundCurve = false;
    int contourCount = 0;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kCubic_Verb:
                foundCurve = true;
                break;
            case SkPath::kClose_Verb:
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
    if (!fSelected.count()) {
        return result;
    }
    SkPath::RawIter iter(fPath);
    uint8_t verb;
    SkPoint pts[4];
    int contourCount = 0;
    bool enabled = fSelected[0];
    bool addMoveTo = true;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (enabled && addMoveTo) {
            result.moveTo(pts[0]);
            addMoveTo = false;
        }
        switch (verb) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
                if (enabled) {
                    result.lineTo(pts[1]);
                }
                break;
            case SkPath::kQuad_Verb:
                if (enabled) {
                    result.quadTo(pts[1], pts[2]);
                }
                break;
            case SkPath::kConic_Verb:
                if (enabled) {
                    result.conicTo(pts[1], pts[2], iter.conicWeight());
                }
                break;
            case SkPath::kCubic_Verb:
                 if (enabled) {
                    result.cubicTo(pts[1], pts[2], pts[3]);
                }
                break;
            case SkPath::kClose_Verb:
                if (enabled) {
                    result.close();
                }
                if (++contourCount >= fSelected.count()) {
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
    SkPath::RawIter iter(fPath);
    uint8_t verb;
    SkPoint pts[4];
    int verbCount = 0;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
            case SkPath::kCubic_Verb:
                ++verbCount;
                break;
            case SkPath::kClose_Verb:
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
    if (!fSelected.count()) {
        return result;
    }
    SkPath::RawIter iter(fPath);
    uint8_t verb;
    SkPoint pts[4];
    int verbIndex = 0;
    bool addMoveTo = true;
    bool addLineTo = false;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        bool enabled = SkPath::kLine_Verb <= verb && verb <= SkPath::kCubic_Verb
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
            case SkPath::kMove_Verb:
                break;
            case SkPath::kLine_Verb:
                if (enabled) {
                    result.lineTo(pts[1]);
                }
                break;
            case SkPath::kQuad_Verb:
                if (enabled) {
                    result.quadTo(pts[1], pts[2]);
                }
                break;
            case SkPath::kConic_Verb:
                if (enabled) {
                    result.conicTo(pts[1], pts[2], iter.conicWeight());
                }
                break;
            case SkPath::kCubic_Verb:
                 if (enabled) {
                    result.cubicTo(pts[1], pts[2], pts[3]);
                }
                break;
            case SkPath::kClose_Verb:
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
