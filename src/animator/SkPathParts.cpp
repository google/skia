
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPathParts.h"
#include "SkAnimateMaker.h"
#include "SkDrawMatrix.h"
#include "SkDrawRectangle.h"
#include "SkDrawPath.h"

SkPathPart::SkPathPart() : fPath(NULL) {
}

void SkPathPart::dirty() {
    fPath->dirty();
}

SkDisplayable* SkPathPart::getParent() const {
    return fPath;
}

bool SkPathPart::setParent(SkDisplayable* parent) {
    SkASSERT(parent != NULL);
    if (parent->isPath() == false)
        return true;
    fPath = (SkDrawPath*) parent;
    return false;
}

// MoveTo
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkMoveTo::fInfo[] = {
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkMoveTo);

SkMoveTo::SkMoveTo() : x(0), y(0) {
}

bool SkMoveTo::add() {
    fPath->fPath.moveTo(x, y);
    return false;
}


// RMoveTo
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRMoveTo::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkRMoveTo);

bool SkRMoveTo::add() {
    fPath->fPath.rMoveTo(x, y);
    return false;
}


// LineTo
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkLineTo::fInfo[] = {
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkLineTo);

SkLineTo::SkLineTo() : x(0), y(0) {
}

bool SkLineTo::add() {
    fPath->fPath.lineTo(x, y);
    return false;
}


// RLineTo
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRLineTo::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkRLineTo);

bool SkRLineTo::add() {
    fPath->fPath.rLineTo(x, y);
    return false;
}


// QuadTo
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkQuadTo::fInfo[] = {
    SK_MEMBER(x1, Float),
    SK_MEMBER(x2, Float),
    SK_MEMBER(y1, Float),
    SK_MEMBER(y2, Float)
};

#endif

DEFINE_GET_MEMBER(SkQuadTo);

SkQuadTo::SkQuadTo() : x1(0), y1(0), x2(0), y2(0) {
}

bool SkQuadTo::add() {
    fPath->fPath.quadTo(x1, y1, x2, y2);
    return false;
}


// RQuadTo
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRQuadTo::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkRQuadTo);

bool SkRQuadTo::add() {
    fPath->fPath.rQuadTo(x1, y1, x2, y2);
    return false;
}


// CubicTo
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkCubicTo::fInfo[] = {
    SK_MEMBER(x1, Float),
    SK_MEMBER(x2, Float),
    SK_MEMBER(x3, Float),
    SK_MEMBER(y1, Float),
    SK_MEMBER(y2, Float),
    SK_MEMBER(y3, Float)
};

#endif

DEFINE_GET_MEMBER(SkCubicTo);

SkCubicTo::SkCubicTo() : x1(0), y1(0), x2(0), y2(0), x3(0), y3(0) {
}

bool SkCubicTo::add() {
    fPath->fPath.cubicTo(x1, y1, x2, y2, x3, y3);
    return false;
}


// RCubicTo
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRCubicTo::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkRCubicTo);

bool SkRCubicTo::add() {
    fPath->fPath.rCubicTo(x1, y1, x2, y2, x3, y3);
    return false;
}


// SkClose
bool SkClose::add() {
    fPath->fPath.close();
    return false;
}


// SkAddGeom
#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAddGeom::fInfo[] = {
    SK_MEMBER(direction, PathDirection)
};

#endif

DEFINE_GET_MEMBER(SkAddGeom);

SkAddGeom::SkAddGeom() : direction(SkPath::kCCW_Direction) {
}

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAddRect::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER_ALIAS(bottom, fRect.fBottom, Float),
    SK_MEMBER_ALIAS(left, fRect.fLeft, Float),
    SK_MEMBER_ALIAS(right, fRect.fRight, Float),
    SK_MEMBER_ALIAS(top, fRect.fTop, Float)
};

#endif

DEFINE_GET_MEMBER(SkAddRect);

SkAddRect::SkAddRect() {
    fRect.setEmpty();
}

bool SkAddRect::add() {
    fPath->fPath.addRect(fRect, (SkPath::Direction) direction);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAddOval::fInfo[] = {
    SK_MEMBER_INHERITED
};

#endif

DEFINE_GET_MEMBER(SkAddOval);

bool SkAddOval::add() {
    fPath->fPath.addOval(fRect,  (SkPath::Direction) direction);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAddCircle::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(radius, Float),
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkAddCircle);

SkAddCircle::SkAddCircle() : radius(0), x(0), y(0) {
}

bool SkAddCircle::add() {
    fPath->fPath.addCircle(x, y, radius,  (SkPath::Direction) direction);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAddRoundRect::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(rx, Float),
    SK_MEMBER(ry, Float)
};

#endif

DEFINE_GET_MEMBER(SkAddRoundRect);

SkAddRoundRect::SkAddRoundRect() : rx(0), ry(0) {
}

bool SkAddRoundRect::add() {
    fPath->fPath.addRoundRect(fRect, rx, ry,  (SkPath::Direction) direction);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkAddPath::fInfo[] = {
    SK_MEMBER(matrix, Matrix),
    SK_MEMBER(path, Path)
};

#endif

DEFINE_GET_MEMBER(SkAddPath);

SkAddPath::SkAddPath() : matrix(NULL), path(NULL) {
}

bool SkAddPath::add() {
    SkASSERT (path != NULL);
    if (matrix)
        fPath->fPath.addPath(path->fPath, matrix->getMatrix());
    else
        fPath->fPath.addPath(path->fPath);
    return false;
}
