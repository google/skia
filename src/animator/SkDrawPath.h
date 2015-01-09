
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawPath_DEFINED
#define SkDrawPath_DEFINED

#include "SkBoundable.h"
#include "SkIntArray.h"
#include "SkMemberInfo.h"
#include "SkPath.h"

class SkDrawPath : public SkBoundable {
    DECLARE_DRAW_MEMBER_INFO(Path);
    SkDrawPath();
    virtual ~SkDrawPath();
    bool addChild(SkAnimateMaker& , SkDisplayable* child) SK_OVERRIDE;
    bool childHasID() { return SkToBool(fChildHasID); }
    bool childrenNeedDisposing() const SK_OVERRIDE;
    void dirty() SK_OVERRIDE;
    bool draw(SkAnimateMaker& ) SK_OVERRIDE;
    SkDisplayable* getParent() const SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) SK_OVERRIDE;
#endif
    SkPath& getPath();
    bool getProperty(int index, SkScriptValue* value) const SK_OVERRIDE;
    bool setProperty(int index, SkScriptValue& value) SK_OVERRIDE;
    void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
    void setChildHasID() SK_OVERRIDE;
    bool setParent(SkDisplayable* parent) SK_OVERRIDE;
    bool isPath() const SK_OVERRIDE { return true; }
public:
    SkPath fPath;
protected:
    void parseSVG();
    SkString d;
    SkTDPathPartArray fParts;
    mutable SkScalar fLength;
    SkDisplayable* fParent; // SkPolyToPoly or SkFromPath, for instance
    SkBool8 fChildHasID;
    SkBool8 fDirty;
private:
    typedef SkBoundable INHERITED;
};

class SkPolyline : public SkDrawPath {
    DECLARE_MEMBER_INFO(Polyline);
    bool addChild(SkAnimateMaker& , SkDisplayable*) SK_OVERRIDE;
    void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
protected:
    SkTDScalarArray points;
private:
    typedef SkDrawPath INHERITED;
};

class SkPolygon : public SkPolyline {
    DECLARE_MEMBER_INFO(Polygon);
    void onEndElement(SkAnimateMaker& ) SK_OVERRIDE;
private:
    typedef SkPolyline INHERITED;
};

#endif // SkDrawPath_DEFINED
