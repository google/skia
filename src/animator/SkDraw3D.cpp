
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDraw3D.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"
#include "SkTypedArray.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo Sk3D_Point::fInfo[] = {
    SK_MEMBER_ALIAS(x, fPoint.fX, Float),
    SK_MEMBER_ALIAS(y, fPoint.fY, Float),
    SK_MEMBER_ALIAS(z, fPoint.fZ, Float)
};

#endif

DEFINE_NO_VIRTUALS_GET_MEMBER(Sk3D_Point);

Sk3D_Point::Sk3D_Point() {
    fPoint.set(0, 0, 0);    
}

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo Sk3D_Camera::fInfo[] = {
    SK_MEMBER_ALIAS(axis, fCamera.fAxis, 3D_Point),
    SK_MEMBER(hackHeight, Float),
    SK_MEMBER(hackWidth, Float),
    SK_MEMBER_ALIAS(location, fCamera.fLocation, 3D_Point),
    SK_MEMBER_ALIAS(observer, fCamera.fObserver, 3D_Point),
    SK_MEMBER(patch, 3D_Patch),
    SK_MEMBER_ALIAS(zenith, fCamera.fZenith, 3D_Point),
};

#endif

DEFINE_GET_MEMBER(Sk3D_Camera);

Sk3D_Camera::Sk3D_Camera() : hackWidth(0), hackHeight(0), patch(NULL) {
}

Sk3D_Camera::~Sk3D_Camera() {
}

bool Sk3D_Camera::draw(SkAnimateMaker& maker) {
    fCamera.update();
    SkMatrix matrix;
    fCamera.patchToMatrix(patch->fPatch, &matrix);
    matrix.preTranslate(hackWidth / 2, -hackHeight / 2);
    matrix.postTranslate(hackWidth / 2, hackHeight / 2);
    maker.fCanvas->concat(matrix);
    return false;
}


enum Sk3D_Patch_Functions {
    SK_FUNCTION(rotateDegrees)
};

const SkFunctionParamType Sk3D_Patch::fFunctionParameters[] = {
    (SkFunctionParamType) SkType_Float,
    (SkFunctionParamType) SkType_Float,
    (SkFunctionParamType) SkType_Float,
    (SkFunctionParamType) 0 // terminator for parameter list (there may be multiple parameter lists)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo Sk3D_Patch::fInfo[] = {
    SK_MEMBER_ALIAS(origin, fPatch.fOrigin, 3D_Point),
    SK_MEMBER_FUNCTION(rotateDegrees, Float),
    SK_MEMBER_ALIAS(u, fPatch.fU, 3D_Point),
    SK_MEMBER_ALIAS(v, fPatch.fV, 3D_Point)
};

#endif

DEFINE_GET_MEMBER(Sk3D_Patch);

void Sk3D_Patch::executeFunction(SkDisplayable* target, int index, 
        SkTDArray<SkScriptValue>& parameters, SkDisplayTypes type,
        SkScriptValue* ) {
    SkASSERT(target == this);
    switch (index) {
        case SK_FUNCTION(rotateDegrees):
            SkASSERT(parameters.count() == 3);
            SkASSERT(type == SkType_Float);
            fPatch.rotateDegrees(parameters[0].fOperand.fScalar, 
                parameters[1].fOperand.fScalar, parameters[2].fOperand.fScalar);
            break;
        default:
            SkASSERT(0);
    }
}

const SkFunctionParamType* Sk3D_Patch::getFunctionsParameters() {
    return fFunctionParameters;
}



