
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
    bool addChild(SkAnimateMaker& , SkDisplayable* child) override;
    bool childHasID() { return SkToBool(fChildHasID); }
    bool childrenNeedDisposing() const override;
    void dirty() override;
    bool draw(SkAnimateMaker& ) override;
    SkDisplayable* getParent() const override;
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
    SkPath& getPath();
    bool getProperty(int index, SkScriptValue* value) const override;
    bool setProperty(int index, SkScriptValue& value) override;
    void onEndElement(SkAnimateMaker& ) override;
    void setChildHasID() override;
    bool setParent(SkDisplayable* parent) override;
    bool isPath() const override { return true; }
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
    bool addChild(SkAnimateMaker& , SkDisplayable*) override;
    void onEndElement(SkAnimateMaker& ) override;
protected:
    SkTDScalarArray points;
private:
    typedef SkDrawPath INHERITED;
};

class SkPolygon : public SkPolyline {
    DECLARE_MEMBER_INFO(Polygon);
    void onEndElement(SkAnimateMaker& ) override;
private:
    typedef SkPolyline INHERITED;
};

#endif // SkDrawPath_DEFINED
