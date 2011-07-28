
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

class SkGradient : public SkDrawShader {
    DECLARE_PRIVATE_MEMBER_INFO(Gradient);
    SkGradient();
    virtual ~SkGradient();
    virtual bool add(SkAnimateMaker& , SkDisplayable* child);
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

class SkLinearGradient : public SkGradient {
    DECLARE_MEMBER_INFO(LinearGradient);
    SkLinearGradient();
    virtual void onEndElement(SkAnimateMaker& );
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker*);
#endif
    virtual SkShader* getShader();
protected:
    SkTDScalarArray points;
private:
    typedef SkGradient INHERITED;
};

class SkRadialGradient : public SkGradient {
    DECLARE_MEMBER_INFO(RadialGradient);
    SkRadialGradient();
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker*);
#endif    
    virtual SkShader* getShader();
protected:
    SkPoint center;
    SkScalar radius;
private:
    typedef SkGradient INHERITED;
};

#endif // SkDrawGradient_DEFINED

