/* libs/graphics/animator/SkDrawExtraPathEffect.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "SkDrawExtraPathEffect.h"
#include "SkDrawPath.h"
#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkMemberInfo.h"
#include "SkPaintParts.h"
#include "SkPathEffect.h"
#include "SkCornerPathEffect.h"

#include "SkDashPathEffect.h"

class SkDrawShapePathEffect : public SkDrawPathEffect {
    DECLARE_PRIVATE_MEMBER_INFO(DrawShapePathEffect);
    SkDrawShapePathEffect();
    virtual ~SkDrawShapePathEffect();
    virtual bool add(SkAnimateMaker& , SkDisplayable* );
    virtual SkPathEffect* getPathEffect();
protected:
    SkDrawable* addPath;
    SkDrawable* addMatrix;
    SkDrawPath* path;
    SkPathEffect* fPathEffect;
    friend class SkShape1DPathEffect;
    friend class SkShape2DPathEffect;
};

class SkDrawShape1DPathEffect : public SkDrawShapePathEffect {
    DECLARE_EXTRAS_MEMBER_INFO(SkDrawShape1DPathEffect);
    SkDrawShape1DPathEffect(SkDisplayTypes );
    virtual ~SkDrawShape1DPathEffect();
    virtual void onEndElement(SkAnimateMaker& );
private:
    SkString phase;
    SkString spacing;
    friend class SkShape1DPathEffect;
    typedef SkDrawShapePathEffect INHERITED;
};

class SkDrawShape2DPathEffect : public SkDrawShapePathEffect {
    DECLARE_EXTRAS_MEMBER_INFO(SkDrawShape2DPathEffect);
    SkDrawShape2DPathEffect(SkDisplayTypes );
    virtual ~SkDrawShape2DPathEffect();
    virtual void onEndElement(SkAnimateMaker& );
private:
    SkDrawMatrix* matrix;
    friend class SkShape2DPathEffect;
    typedef SkDrawShapePathEffect INHERITED;
};

class SkDrawComposePathEffect : public SkDrawPathEffect {
    DECLARE_EXTRAS_MEMBER_INFO(SkDrawComposePathEffect);
    SkDrawComposePathEffect(SkDisplayTypes );
    virtual ~SkDrawComposePathEffect();
    virtual bool add(SkAnimateMaker& , SkDisplayable* );
    virtual SkPathEffect* getPathEffect();
    virtual bool isPaint() const;
private:
    SkDrawPathEffect* effect1;
    SkDrawPathEffect* effect2;
};

class SkDrawCornerPathEffect : public SkDrawPathEffect {
    DECLARE_EXTRAS_MEMBER_INFO(SkDrawCornerPathEffect);
    SkDrawCornerPathEffect(SkDisplayTypes );
    virtual ~SkDrawCornerPathEffect();
    virtual SkPathEffect* getPathEffect();
private:
    SkScalar radius;
};

//////////// SkShape1DPathEffect

#include "SkAnimateMaker.h"
#include "SkAnimatorScript.h"
#include "SkDisplayApply.h"
#include "SkDrawMatrix.h"
#include "SkPaint.h"

class SkShape1DPathEffect : public Sk1DPathEffect {
public:
    SkShape1DPathEffect(SkDrawShape1DPathEffect* draw, SkAnimateMaker* maker) :
        fDraw(draw), fMaker(maker) {
    }

protected:
    virtual SkScalar begin(SkScalar contourLength)
    {
        SkScriptValue value;
        SkAnimatorScript engine(*fMaker, NULL, SkType_Float);
        engine.propertyCallBack(GetContourLength, &contourLength);
        value.fOperand.fScalar = 0;
        engine.evaluate(fDraw->phase.c_str(), &value, SkType_Float);
        return value.fOperand.fScalar;
    }

    virtual SkScalar next(SkPath* dst, SkScalar distance, SkPathMeasure& )
    {
        fMaker->setExtraPropertyCallBack(fDraw->fType, GetDistance, &distance);
        SkDrawPath* drawPath = NULL;
        if (fDraw->addPath->isPath()) {
            drawPath = (SkDrawPath*) fDraw->addPath;
        } else {
            SkApply* apply = (SkApply*) fDraw->addPath;
            apply->refresh(*fMaker);
            apply->activate(*fMaker);
            apply->interpolate(*fMaker, SkScalarMulRound(distance, 1000));
            drawPath = (SkDrawPath*) apply->getScope();
        }
        SkMatrix m;
        m.reset();
        if (fDraw->addMatrix) {
            SkDrawMatrix* matrix;
            if (fDraw->addMatrix->getType() == SkType_Matrix)
                matrix = (SkDrawMatrix*) fDraw->addMatrix;
            else {
                SkApply* apply = (SkApply*) fDraw->addMatrix;
                apply->refresh(*fMaker);
                apply->activate(*fMaker);
                apply->interpolate(*fMaker, SkScalarMulRound(distance, 1000));
                matrix = (SkDrawMatrix*) apply->getScope();
            }
        }
        SkScalar result = 0;
        SkAnimatorScript::EvaluateFloat(*fMaker, NULL, fDraw->spacing.c_str(), &result);
        if (drawPath)
            dst->addPath(drawPath->getPath(), m);
        fMaker->clearExtraPropertyCallBack(fDraw->fType);
        return result;
    }

private:
    virtual void flatten(SkFlattenableWriteBuffer& ) {}
    virtual Factory getFactory() { return NULL; }

    static bool GetContourLength(const char* token, size_t len, void* clen, SkScriptValue* value) {
        if (SK_LITERAL_STR_EQUAL("contourLength", token, len)) {
            value->fOperand.fScalar = *(SkScalar*) clen;
            value->fType = SkType_Float;
            return true;
        }
        return false;
    }

    static bool GetDistance(const char* token, size_t len, void* dist, SkScriptValue* value) {
        if (SK_LITERAL_STR_EQUAL("distance", token, len)) {
            value->fOperand.fScalar = *(SkScalar*) dist;
            value->fType = SkType_Float;
            return true;
        }
        return false;
    }

    SkDrawShape1DPathEffect* fDraw;
    SkAnimateMaker* fMaker;
};

//////////// SkDrawShapePathEffect

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawShapePathEffect::fInfo[] = {
    SK_MEMBER(addMatrix, Drawable), // either matrix or apply
    SK_MEMBER(addPath, Drawable),   // either path or apply
    SK_MEMBER(path, Path),
};

#endif

DEFINE_GET_MEMBER(SkDrawShapePathEffect);

SkDrawShapePathEffect::SkDrawShapePathEffect() :
    addPath(NULL), addMatrix(NULL), path(NULL), fPathEffect(NULL) {
}

SkDrawShapePathEffect::~SkDrawShapePathEffect() {
    SkSafeUnref(fPathEffect);
}

bool SkDrawShapePathEffect::add(SkAnimateMaker& , SkDisplayable* child) {
    path = (SkDrawPath*) child;
    return true;
}

SkPathEffect* SkDrawShapePathEffect::getPathEffect() {
    fPathEffect->ref();
    return fPathEffect;
}

//////////// SkDrawShape1DPathEffect

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawShape1DPathEffect::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(phase, String),
    SK_MEMBER(spacing, String),
};

#endif

DEFINE_GET_MEMBER(SkDrawShape1DPathEffect);

SkDrawShape1DPathEffect::SkDrawShape1DPathEffect(SkDisplayTypes type) : fType(type) {
}

SkDrawShape1DPathEffect::~SkDrawShape1DPathEffect() {
}

void SkDrawShape1DPathEffect::onEndElement(SkAnimateMaker& maker) {
    if (addPath == NULL || (addPath->isPath() == false && addPath->isApply() == false))
        maker.setErrorCode(SkDisplayXMLParserError::kUnknownError); // !!! add error
    else
        fPathEffect = new SkShape1DPathEffect(this, &maker);
}

////////// SkShape2DPathEffect

class SkShape2DPathEffect : public Sk2DPathEffect {
public:
    SkShape2DPathEffect(SkDrawShape2DPathEffect* draw, SkAnimateMaker* maker,
        const SkMatrix& matrix) : Sk2DPathEffect(matrix), fDraw(draw), fMaker(maker) {
    }

protected:
    virtual void begin(const SkIRect& uvBounds, SkPath* )
    {
        fUVBounds.set(SkIntToScalar(uvBounds.fLeft), SkIntToScalar(uvBounds.fTop),
            SkIntToScalar(uvBounds.fRight), SkIntToScalar(uvBounds.fBottom));
    }

    virtual void next(const SkPoint& loc, int u, int v, SkPath* dst)
    {
        fLoc = loc;
        fU = u;
        fV = v;
        SkDrawPath* drawPath;
        fMaker->setExtraPropertyCallBack(fDraw->fType, Get2D, this);
        if (fDraw->addPath->isPath()) {
            drawPath = (SkDrawPath*) fDraw->addPath;
        } else {
            SkApply* apply = (SkApply*) fDraw->addPath;
            apply->refresh(*fMaker);
            apply->activate(*fMaker);
            apply->interpolate(*fMaker, v);
            drawPath = (SkDrawPath*) apply->getScope();
        }
        if (drawPath == NULL)
            goto clearCallBack;
        if (fDraw->matrix) {
            SkDrawMatrix* matrix;
            if (fDraw->matrix->getType() == SkType_Matrix)
                matrix = (SkDrawMatrix*) fDraw->matrix;
            else {
                SkApply* apply = (SkApply*) fDraw->matrix;
                apply->activate(*fMaker);
                apply->interpolate(*fMaker, v);
                matrix = (SkDrawMatrix*) apply->getScope();
            }
            if (matrix) {
                dst->addPath(drawPath->getPath(), matrix->getMatrix());
                goto clearCallBack;
            }
        }
        dst->addPath(drawPath->getPath());
clearCallBack:
        fMaker->clearExtraPropertyCallBack(fDraw->fType);
    }

private:

    static bool Get2D(const char* token, size_t len, void* s2D, SkScriptValue* value) {
        static const char match[] = "locX|locY|left|top|right|bottom|u|v" ;
        SkShape2DPathEffect* shape2D = (SkShape2DPathEffect*) s2D;
        int index;
        if (SkAnimatorScript::MapEnums(match, token, len, &index) == false)
            return false;
        SkASSERT((sizeof(SkPoint) +     sizeof(SkRect)) / sizeof(SkScalar) == 6);
        if (index < 6) {
            value->fType = SkType_Float;
            value->fOperand.fScalar = (&shape2D->fLoc.fX)[index];
        } else {
            value->fType = SkType_Int;
            value->fOperand.fS32 = (&shape2D->fU)[index - 6];
        }
        return true;
    }

    SkPoint fLoc;
    SkRect fUVBounds;
    int32_t fU;
    int32_t fV;
    SkDrawShape2DPathEffect* fDraw;
    SkAnimateMaker* fMaker;

    // illegal
    SkShape2DPathEffect(const SkShape2DPathEffect&);
    SkShape2DPathEffect& operator=(const SkShape2DPathEffect&);
};

////////// SkDrawShape2DPathEffect

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawShape2DPathEffect::fInfo[] = {
    SK_MEMBER_INHERITED,
    SK_MEMBER(matrix, Matrix)
};

#endif

DEFINE_GET_MEMBER(SkDrawShape2DPathEffect);

SkDrawShape2DPathEffect::SkDrawShape2DPathEffect(SkDisplayTypes type) : fType(type) {
}

SkDrawShape2DPathEffect::~SkDrawShape2DPathEffect() {
}

void SkDrawShape2DPathEffect::onEndElement(SkAnimateMaker& maker) {
    if (addPath == NULL || (addPath->isPath() == false && addPath->isApply() == false) ||
            matrix == NULL)
        maker.setErrorCode(SkDisplayXMLParserError::kUnknownError); // !!! add error
    else
        fPathEffect = new SkShape2DPathEffect(this, &maker, matrix->getMatrix());
}

////////// SkDrawComposePathEffect

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawComposePathEffect::fInfo[] = {
    SK_MEMBER(effect1, PathEffect),
    SK_MEMBER(effect2, PathEffect)
};

#endif

DEFINE_GET_MEMBER(SkDrawComposePathEffect);

SkDrawComposePathEffect::SkDrawComposePathEffect(SkDisplayTypes type) : fType(type),
    effect1(NULL), effect2(NULL) {
}

SkDrawComposePathEffect::~SkDrawComposePathEffect() {
    delete effect1;
    delete effect2;
}

bool SkDrawComposePathEffect::add(SkAnimateMaker& , SkDisplayable* child) {
    if (effect1 == NULL)
        effect1 = (SkDrawPathEffect*) child;
    else
        effect2 = (SkDrawPathEffect*) child;
    return true;
}

SkPathEffect* SkDrawComposePathEffect::getPathEffect() {
    SkPathEffect* e1 = effect1->getPathEffect();
    SkPathEffect* e2 = effect2->getPathEffect();
    SkPathEffect* composite = new SkComposePathEffect(e1, e2);
    e1->unref();
    e2->unref();
    return composite;
}

bool SkDrawComposePathEffect::isPaint() const {
    return true;
}

//////////// SkDrawCornerPathEffect

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawCornerPathEffect::fInfo[] = {
    SK_MEMBER(radius, Float)
};

#endif

DEFINE_GET_MEMBER(SkDrawCornerPathEffect);

SkDrawCornerPathEffect::SkDrawCornerPathEffect(SkDisplayTypes type):
    fType(type), radius(0) {
}

SkDrawCornerPathEffect::~SkDrawCornerPathEffect() {
}

SkPathEffect* SkDrawCornerPathEffect::getPathEffect() {
    return new SkCornerPathEffect(radius);
}

/////////

#include "SkExtras.h"

const char kDrawShape1DPathEffectName[] = "pathEffect:shape1D";
const char kDrawShape2DPathEffectName[] = "pathEffect:shape2D";
const char kDrawComposePathEffectName[] = "pathEffect:compose";
const char kDrawCornerPathEffectName[]  = "pathEffect:corner";

class SkExtraPathEffects : public SkExtras {
public:
    SkExtraPathEffects(SkAnimator* animator) :
            skDrawShape1DPathEffectType(SkType_Unknown),
            skDrawShape2DPathEffectType(SkType_Unknown),
            skDrawComposePathEffectType(SkType_Unknown),
            skDrawCornerPathEffectType(SkType_Unknown) {
    }

    virtual SkDisplayable* createInstance(SkDisplayTypes type) {
        SkDisplayable* result = NULL;
        if (skDrawShape1DPathEffectType == type)
            result = new SkDrawShape1DPathEffect(type);
        else if (skDrawShape2DPathEffectType == type)
            result = new SkDrawShape2DPathEffect(type);
        else if (skDrawComposePathEffectType == type)
            result = new SkDrawComposePathEffect(type);
        else if (skDrawCornerPathEffectType == type)
            result = new SkDrawCornerPathEffect(type);
        return result;
    }

    virtual bool definesType(SkDisplayTypes type) {
        return type == skDrawShape1DPathEffectType ||
            type == skDrawShape2DPathEffectType ||
            type == skDrawComposePathEffectType ||
            type == skDrawCornerPathEffectType;
    }

#if SK_USE_CONDENSED_INFO == 0
    virtual const SkMemberInfo* getMembers(SkDisplayTypes type, int* infoCountPtr) {
        const SkMemberInfo* info = NULL;
        int infoCount = 0;
        if (skDrawShape1DPathEffectType == type) {
            info = SkDrawShape1DPathEffect::fInfo;
            infoCount = SkDrawShape1DPathEffect::fInfoCount;
        } else if (skDrawShape2DPathEffectType == type) {
            info = SkDrawShape2DPathEffect::fInfo;
            infoCount = SkDrawShape2DPathEffect::fInfoCount;
        } else if (skDrawComposePathEffectType == type) {
            info = SkDrawComposePathEffect::fInfo;
            infoCount = SkDrawShape1DPathEffect::fInfoCount;
        } else if (skDrawCornerPathEffectType == type) {
            info = SkDrawCornerPathEffect::fInfo;
            infoCount = SkDrawCornerPathEffect::fInfoCount;
        }
        if (infoCountPtr)
            *infoCountPtr = infoCount;
        return info;
    }
#endif

#ifdef SK_DEBUG
    virtual const char* getName(SkDisplayTypes type) {
        if (skDrawShape1DPathEffectType == type)
            return kDrawShape1DPathEffectName;
        else if (skDrawShape2DPathEffectType == type)
            return kDrawShape2DPathEffectName;
        else if (skDrawComposePathEffectType == type)
            return kDrawComposePathEffectName;
        else if (skDrawCornerPathEffectType == type)
            return kDrawCornerPathEffectName;
        return NULL;
    }
#endif

    virtual SkDisplayTypes getType(const char name[], size_t len ) {
        SkDisplayTypes* type = NULL;
        if (SK_LITERAL_STR_EQUAL(kDrawShape1DPathEffectName, name, len))
            type = &skDrawShape1DPathEffectType;
        else if (SK_LITERAL_STR_EQUAL(kDrawShape2DPathEffectName, name, len))
            type = &skDrawShape2DPathEffectType;
        else if (SK_LITERAL_STR_EQUAL(kDrawComposePathEffectName, name, len))
            type = &skDrawComposePathEffectType;
        else if (SK_LITERAL_STR_EQUAL(kDrawCornerPathEffectName, name, len))
            type = &skDrawCornerPathEffectType;
        if (type) {
            if (*type == SkType_Unknown)
                *type = SkDisplayType::RegisterNewType();
            return *type;
        }
        return SkType_Unknown;
    }

private:
    SkDisplayTypes skDrawShape1DPathEffectType;
    SkDisplayTypes skDrawShape2DPathEffectType;
    SkDisplayTypes skDrawComposePathEffectType;
    SkDisplayTypes skDrawCornerPathEffectType;
};


void InitializeSkExtraPathEffects(SkAnimator* animator) {
    animator->addExtras(new SkExtraPathEffects(animator));
}

////////////////


SkExtras::SkExtras() : fExtraCallBack(NULL), fExtraStorage(NULL) {
}
