
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDraw3D_DEFINED
#define SkDraw3D_DEFINED

#include "SkCamera.h"
#include "SkADrawable.h"
#include "SkMemberInfo.h"

class Sk3D_Patch;

struct Sk3D_Point {
    DECLARE_NO_VIRTUALS_MEMBER_INFO(3D_Point);
    Sk3D_Point();
private:
    SkPoint3D fPoint;
};

class Sk3D_Camera : public SkADrawable {
    DECLARE_MEMBER_INFO(3D_Camera);
    Sk3D_Camera();
    virtual ~Sk3D_Camera();
    bool draw(SkAnimateMaker& ) override;
private:
    SkScalar hackWidth;
    SkScalar hackHeight;
    SkCamera3D fCamera;
    Sk3D_Patch* patch;
};

class Sk3D_Patch : public SkDisplayable {
    DECLARE_MEMBER_INFO(3D_Patch);
private:
    void executeFunction(SkDisplayable* , int index,
        SkTDArray<SkScriptValue>& parameters, SkDisplayTypes type,
        SkScriptValue* ) override;
    const SkFunctionParamType* getFunctionsParameters() override;
    SkPatch3D  fPatch;
    static const SkFunctionParamType fFunctionParameters[];
    friend class Sk3D_Camera;
};

#endif // SkDraw3D_DEFINED
