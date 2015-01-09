
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMatrixParts_DEFINED
#define SkMatrixParts_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"
#include "SkPathMeasure.h"

class SkDrawPath;
class SkDrawRect;
class SkPolygon;

class SkDrawMatrix;
// class SkMatrix;

class SkMatrixPart : public SkDisplayable {
public:
    SkMatrixPart();
    virtual bool add() = 0;
    virtual void dirty();
    virtual SkDisplayable* getParent() const;
    virtual bool setParent(SkDisplayable* parent);
#ifdef SK_DEBUG
    virtual bool isMatrixPart() const { return true; }
#endif
protected:
    SkDrawMatrix* fMatrix;
};

class SkRotate : public SkMatrixPart {
    DECLARE_MEMBER_INFO(Rotate);
    SkRotate();
protected:
    bool add() SK_OVERRIDE;
    SkScalar degrees;
    SkPoint center;
};

class SkScale : public SkMatrixPart {
    DECLARE_MEMBER_INFO(Scale);
    SkScale();
protected:
    bool add() SK_OVERRIDE;
    SkScalar x;
    SkScalar y;
    SkPoint center;
};

class SkSkew : public SkMatrixPart {
    DECLARE_MEMBER_INFO(Skew);
    SkSkew();
protected:
    bool add() SK_OVERRIDE;
    SkScalar x;
    SkScalar y;
    SkPoint center;
};

class SkTranslate : public SkMatrixPart {
    DECLARE_MEMBER_INFO(Translate);
    SkTranslate();
protected:
    bool add() SK_OVERRIDE;
    SkScalar x;
    SkScalar y;
};

class SkFromPath : public SkMatrixPart {
    DECLARE_MEMBER_INFO(FromPath);
    SkFromPath();
    virtual ~SkFromPath();
protected:
    bool add() SK_OVERRIDE;
    int32_t mode;
    SkScalar offset;
    SkDrawPath* path;
    SkPathMeasure fPathMeasure;
};

class SkRectToRect : public SkMatrixPart {
    DECLARE_MEMBER_INFO(RectToRect);
    SkRectToRect();
    virtual ~SkRectToRect();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    const SkMemberInfo* preferredChild(SkDisplayTypes type) SK_OVERRIDE;
protected:
    bool add() SK_OVERRIDE;
    SkDrawRect* source;
    SkDrawRect* destination;
};

class SkPolyToPoly : public SkMatrixPart {
    DECLARE_MEMBER_INFO(PolyToPoly);
    SkPolyToPoly();
    virtual ~SkPolyToPoly();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
    const SkMemberInfo* preferredChild(SkDisplayTypes type) SK_OVERRIDE;
protected:
    bool add() SK_OVERRIDE;
    SkPolygon* source;
    SkPolygon* destination;
};

// !!! add concat matrix ?

#endif // SkMatrixParts_DEFINED
