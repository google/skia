
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkMatrixParts.h"
#include "SkAnimateMaker.h"
#include "SkDrawMatrix.h"
#include "SkDrawRectangle.h"
#include "SkDrawPath.h"

SkMatrixPart::SkMatrixPart() : fMatrix(nullptr) {
}

void SkMatrixPart::dirty() {
    fMatrix->dirty();
}

SkDisplayable* SkMatrixPart::getParent() const {
    return fMatrix;
}

bool SkMatrixPart::setParent(SkDisplayable* parent) {
    SkASSERT(parent != nullptr);
    if (parent->isMatrix() == false)
        return true;
    fMatrix = (SkDrawMatrix*) parent;
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRotate::fInfo[] = {
    SK_MEMBER(center, Point),
    SK_MEMBER(degrees, Float)
};

#endif

DEFINE_GET_MEMBER(SkRotate);

SkRotate::SkRotate() : degrees(0) {
    center.fX = center.fY = 0;
}

bool SkRotate::add() {
    fMatrix->rotate(degrees, center);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkScale::fInfo[] = {
    SK_MEMBER(center, Point),
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkScale);

SkScale::SkScale() : x(SK_Scalar1), y(SK_Scalar1) {
    center.fX = center.fY = 0;
}

bool SkScale::add() {
    fMatrix->scale(x, y, center);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkSkew::fInfo[] = {
    SK_MEMBER(center, Point),
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkSkew);

SkSkew::SkSkew() : x(0), y(0) {
    center.fX = center.fY = 0;
}

bool SkSkew::add() {
    fMatrix->skew(x, y, center);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkTranslate::fInfo[] = {
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkTranslate);

SkTranslate::SkTranslate() : x(0), y(0) {
}

bool SkTranslate::add() {
    fMatrix->translate(x, y);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkFromPath::fInfo[] = {
    SK_MEMBER(mode, FromPathMode),
    SK_MEMBER(offset, Float),
    SK_MEMBER(path, Path)
};

#endif

DEFINE_GET_MEMBER(SkFromPath);

SkFromPath::SkFromPath() :
    mode(0), offset(0), path(nullptr) {
}

SkFromPath::~SkFromPath() {
}

bool SkFromPath::add() {
    if (path == nullptr)
        return true;
    static const uint8_t gFlags[] = {
        SkPathMeasure::kGetPosAndTan_MatrixFlag,    // normal
        SkPathMeasure::kGetTangent_MatrixFlag,      // angle
        SkPathMeasure::kGetPosition_MatrixFlag      // position
    };
    if ((unsigned)mode >= SK_ARRAY_COUNT(gFlags))
        return true;
    SkMatrix result;
    fPathMeasure.setPath(&path->getPath(), false);
    if (fPathMeasure.getMatrix(offset, &result, (SkPathMeasure::MatrixFlags)gFlags[mode]))
        fMatrix->set(result);
    return false;
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkRectToRect::fInfo[] = {
    SK_MEMBER(destination, Rect),
    SK_MEMBER(source, Rect)
};

#endif

DEFINE_GET_MEMBER(SkRectToRect);

SkRectToRect::SkRectToRect() :
    source(nullptr), destination(nullptr) {
}

SkRectToRect::~SkRectToRect() {
}

bool SkRectToRect::add() {
    if (source == nullptr || destination == nullptr)
        return true;
    SkMatrix temp;
    temp.setRectToRect(source->fRect, destination->fRect,
                       SkMatrix::kFill_ScaleToFit);
    fMatrix->set(temp);
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkRectToRect::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkDebugf("/>\n");
    SkDisplayList::fIndent += 4;
    if (source) {
        SkDebugf("%*s<source>\n", SkDisplayList::fIndent, "");
        SkDisplayList::fIndent += 4;
        source->dump(maker);
        SkDisplayList::fIndent -= 4;
        SkDebugf("%*s</source>\n", SkDisplayList::fIndent, "");
    }
    if (destination) {
        SkDebugf("%*s<destination>\n", SkDisplayList::fIndent, "");
        SkDisplayList::fIndent += 4;
        destination->dump(maker);
        SkDisplayList::fIndent -= 4;
        SkDebugf("%*s</destination>\n", SkDisplayList::fIndent, "");
    }
    SkDisplayList::fIndent -= 4;
    dumpEnd(maker);
}
#endif

const SkMemberInfo* SkRectToRect::preferredChild(SkDisplayTypes ) {
    if (source == nullptr)
        return getMember("source"); // !!! cwap! need to refer to member through enum like kScope instead
    else {
        SkASSERT(destination == nullptr);
        return getMember("destination");
    }
}


#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkPolyToPoly::fInfo[] = {
    SK_MEMBER(destination, Polygon),
    SK_MEMBER(source, Polygon)
};

#endif

DEFINE_GET_MEMBER(SkPolyToPoly);

SkPolyToPoly::SkPolyToPoly() : source(nullptr), destination(nullptr) {
}

SkPolyToPoly::~SkPolyToPoly() {
}

bool SkPolyToPoly::add() {
    SkASSERT(source);
    SkASSERT(destination);
    SkPoint src[4];
    SkPoint dst[4];
    SkPath& sourcePath = source->getPath();
    int srcPts = sourcePath.getPoints(src, 4);
    SkPath& destPath = destination->getPath();
    int dstPts = destPath.getPoints(dst, 4);
    if (srcPts != dstPts)
        return true;
    SkMatrix temp;
    temp.setPolyToPoly(src, dst, srcPts);
    fMatrix->set(temp);
    return false;
}

#ifdef SK_DUMP_ENABLED
void SkPolyToPoly::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkDebugf("/>\n");
    SkDisplayList::fIndent += 4;
    if (source) {
        SkDebugf("%*s<source>\n", SkDisplayList::fIndent, "");
        SkDisplayList::fIndent += 4;
        source->dump(maker);
        SkDisplayList::fIndent -= 4;
        SkDebugf("%*s</source>\n", SkDisplayList::fIndent, "");
    }
    if (destination) {
        SkDebugf("%*s<destination>\n", SkDisplayList::fIndent, "");
        SkDisplayList::fIndent += 4;
        destination->dump(maker);
        SkDisplayList::fIndent -= 4;
        SkDebugf("%*s</destination>\n", SkDisplayList::fIndent, "");
    }
    SkDisplayList::fIndent -= 4;
    dumpEnd(maker);
}
#endif

void SkPolyToPoly::onEndElement(SkAnimateMaker& ) {
    SkASSERT(source);
    SkASSERT(destination);
    if (source->childHasID() || destination->childHasID())
        fMatrix->setChildHasID();
}

const SkMemberInfo* SkPolyToPoly::preferredChild(SkDisplayTypes ) {
    if (source == nullptr)
        return getMember("source"); // !!! cwap! need to refer to member through enum like kScope instead
    else {
        SkASSERT(destination == nullptr);
        return getMember("destination");
    }
}
