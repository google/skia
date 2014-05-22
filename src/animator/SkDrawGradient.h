
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawGradient_DEFINED
#define SkDrawGradient_DEFINED

#include "SkDrawColor.h"
#include "SkDrawShader.h"
#include "SkIntArray.h"

class SkUnitMapper;

class SkDrawGradient : public SkDrawShader {
    DECLARE_PRIVATE_MEMBER_INFO(DrawGradient);
    SkDrawGradient();
    virtual ~SkDrawGradient();
    virtual bool addChild(SkAnimateMaker& , SkDisplayable* child) SK_OVERRIDE;
#ifdef SK_DUMP_ENABLED
    virtual void dumpRest(SkAnimateMaker*);
#endif
    virtual void onEndElement(SkAnimateMaker& );
protected:
    SkTDScalarArray offsets;
    SkString unitMapper;
    SkTDColorArray fColors;
    SkTDDrawColorArray fDrawColors;
    SkUnitMapper* fUnitMapper;
    int addPrelude();
private:
    typedef SkDrawShader INHERITED;
};

class SkDrawLinearGradient : public SkDrawGradient {
    DECLARE_MEMBER_INFO(DrawLinearGradient);
    SkDrawLinearGradient();
    virtual void onEndElement(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker*);
#endif
    virtual SkShader* getShader();
protected:
    SkTDScalarArray points;
private:
    typedef SkDrawGradient INHERITED;
};

class SkDrawRadialGradient : public SkDrawGradient {
    DECLARE_MEMBER_INFO(DrawRadialGradient);
    SkDrawRadialGradient();
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker*);
#endif
    virtual SkShader* getShader();
protected:
    SkPoint center;
    SkScalar radius;
private:
    typedef SkDrawGradient INHERITED;
};

#endif // SkDrawGradient_DEFINED
