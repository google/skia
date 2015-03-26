
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPathParts_DEFINED
#define SkPathParts_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"
#include "SkPath.h"

class SkDrawPath;
class SkDrawMatrix;

class SkPathPart : public SkDisplayable {
public:
    SkPathPart();
    virtual bool add() = 0;
    virtual void dirty();
    virtual SkDisplayable* getParent() const;
    virtual bool setParent(SkDisplayable* parent);
#ifdef SK_DEBUG
    virtual bool isPathPart() const { return true; }
#endif
protected:
    SkDrawPath* fPath;
};

class SkMoveTo : public SkPathPart {
    DECLARE_MEMBER_INFO(MoveTo);
    SkMoveTo();
    bool add() override;
protected:
    SkScalar x;
    SkScalar y;
};

class SkRMoveTo : public SkMoveTo {
    DECLARE_MEMBER_INFO(RMoveTo);
    bool add() override;
private:
    typedef SkMoveTo INHERITED;
};

class SkLineTo : public SkPathPart {
    DECLARE_MEMBER_INFO(LineTo);
    SkLineTo();
    bool add() override;
protected:
    SkScalar x;
    SkScalar y;
};

class SkRLineTo : public SkLineTo {
    DECLARE_MEMBER_INFO(RLineTo);
    bool add() override;
private:
    typedef SkLineTo INHERITED;
};

class SkQuadTo : public SkPathPart {
    DECLARE_MEMBER_INFO(QuadTo);
    SkQuadTo();
    bool add() override;
protected:
    SkScalar x1;
    SkScalar y1;
    SkScalar x2;
    SkScalar y2;
};

class SkRQuadTo : public SkQuadTo {
    DECLARE_MEMBER_INFO(RQuadTo);
    bool add() override;
private:
    typedef SkQuadTo INHERITED;
};

class SkCubicTo : public SkPathPart {
    DECLARE_MEMBER_INFO(CubicTo);
    SkCubicTo();
    bool add() override;
protected:
    SkScalar x1;
    SkScalar y1;
    SkScalar x2;
    SkScalar y2;
    SkScalar x3;
    SkScalar y3;
};

class SkRCubicTo : public SkCubicTo {
    DECLARE_MEMBER_INFO(RCubicTo);
    bool add() override;
private:
    typedef SkCubicTo INHERITED;
};

class SkClose : public SkPathPart {
    DECLARE_EMPTY_MEMBER_INFO(Close);
    bool add() override;
};

class SkAddGeom : public SkPathPart {
    DECLARE_PRIVATE_MEMBER_INFO(AddGeom);
    SkAddGeom();
protected:
    int /*SkPath::Direction*/ direction;
};

class SkAddRect : public SkAddGeom {
    DECLARE_MEMBER_INFO(AddRect);
    SkAddRect();
    bool add() override;
protected:
    SkRect fRect;
private:
    typedef SkAddGeom INHERITED;
};

class SkAddOval : public SkAddRect {
    DECLARE_MEMBER_INFO(AddOval);
    bool add() override;
private:
    typedef SkAddRect INHERITED;
};

class SkAddCircle : public SkAddGeom {
    DECLARE_MEMBER_INFO(AddCircle);
    SkAddCircle();
    bool add() override;
private:
    SkScalar radius;
    SkScalar x;
    SkScalar y;
    typedef SkAddGeom INHERITED;
};

class SkAddRoundRect : public SkAddRect {
    DECLARE_MEMBER_INFO(AddRoundRect);
    SkAddRoundRect();
    bool add() override;
private:
    SkScalar rx;
    SkScalar ry;
    typedef SkAddRect INHERITED;
};

class SkAddPath : public SkPathPart {
    DECLARE_MEMBER_INFO(AddPath);
    SkAddPath();
    bool add() override;
private:
    typedef SkPathPart INHERITED;
    SkDrawMatrix* matrix;
    SkDrawPath* path;
};

#endif // SkPathParts_DEFINED
