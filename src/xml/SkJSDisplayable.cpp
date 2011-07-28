
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <jsapi.h>
#include "SkJS.h"
#include "SkDisplayType.h"
//#include "SkAnimateColor.h"
#include "SkAnimateMaker.h"
#include "SkAnimateSet.h"
//#include "SkAnimateTransform.h"
#include "SkCanvas.h"
//#include "SkDimensions.h"
#include "SkDisplayAdd.h"
#include "SkDisplayApply.h"
//#include "SkDisplayBefore.h"
#include "SkDisplayEvent.h"
//#include "SkDisplayFocus.h"
#include "SkDisplayInclude.h"
#include "SkDisplayPost.h"
#include "SkDisplayRandom.h"
#include "SkDraw3D.h"
#include "SkDrawBitmap.h"
#include "SkDrawClip.h"
#include "SkDrawDash.h"
#include "SkDrawDiscrete.h"
#include "SkDrawEmboss.h"
//#include "SkDrawFont.h"
#include "SkDrawFull.h"
#include "SkDrawGradient.h"
#include "SkDrawLine.h"
//#include "SkDrawMaskFilter.h"
#include "SkDrawMatrix.h"
#include "SkDrawOval.h"
#include "SkDrawPaint.h"
#include "SkDrawPath.h"
#include "SkDrawPoint.h"
// #include "SkDrawStroke.h"
#include "SkDrawText.h"
#include "SkDrawTo.h"
//#include "SkDrawTransferMode.h"
#include "SkDrawTransparentShader.h"
//#include "SkDrawUse.h"
#include "SkMatrixParts.h"
#include "SkPathParts.h"
#include "SkPostParts.h"
#include "SkScript.h"
#include "SkSnapshot.h"
#include "SkTextOnPath.h"
#include "SkTextToPath.h"


class SkJSDisplayable {
public:
    SkJSDisplayable() : fDisplayable(NULL) {}
    ~SkJSDisplayable() { delete fDisplayable; }
    static void Destructor(JSContext *cx, JSObject *obj);
    static JSBool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
    static JSBool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
    static SkCanvas* gCanvas;
    static SkPaint* gPaint;
    static JSBool Draw(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
    SkDisplayable* fDisplayable;
};

SkCanvas* SkJSDisplayable::gCanvas;
SkPaint* SkJSDisplayable::gPaint;

JSBool SkJSDisplayable::Draw(JSContext *cx, JSObject *obj, uintN argc,
                                    jsval *argv, jsval *rval)
{
    SkJSDisplayable *p = (SkJSDisplayable*) JS_GetPrivate(cx, obj);
    SkASSERT(p->fDisplayable->isDrawable());
    SkDrawable* drawable = (SkDrawable*) p->fDisplayable;
    SkAnimateMaker maker(NULL, gCanvas, gPaint);
    drawable->draw(maker);
    return JS_TRUE;
}


JSFunctionSpec SkJSDisplayable_methods[] = 
{
    { "draw", SkJSDisplayable::Draw, 1, 0, 0 },
    { 0 }
};

static JSPropertySpec* gDisplayableProperties[kNumberOfTypes];
static JSClass gDisplayableClasses[kNumberOfTypes];

#define JS_INIT(_prefix, _class) \
static JSBool _class##Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) { \
    SkJSDisplayable* jsDisplayable = new SkJSDisplayable(); \
    jsDisplayable->fDisplayable = new _prefix##_class(); \
    JS_SetPrivate(cx, obj, (void*) jsDisplayable); \
    return JS_TRUE; \
} \
    \
static JSObject* _class##Init(JSContext *cx, JSObject *obj, JSObject *proto) { \
    JSObject *newProtoObj = JS_InitClass(cx, obj, proto, &gDisplayableClasses[SkType_##_class], \
        _class##Constructor, 0, \
        NULL, SkJSDisplayable_methods , \
        NULL, NULL); \
    JS_DefineProperties(cx, newProtoObj, gDisplayableProperties[SkType_##_class]); \
    return newProtoObj; \
}

JS_INIT(Sk, Add)
JS_INIT(Sk, AddCircle)
JS_INIT(Sk, AddOval)
JS_INIT(Sk, AddPath)
JS_INIT(Sk, AddRectangle)
JS_INIT(Sk, AddRoundRect)
//JS_INIT(Sk, After)
JS_INIT(Sk, Apply)
// JS_INIT(Sk, Animate)
//JS_INIT(Sk, AnimateColor)
JS_INIT(Sk, AnimateField)
//JS_INIT(Sk, AnimateRotate)
//JS_INIT(Sk, AnimateScale)
//JS_INIT(Sk, AnimateTranslate)
JS_INIT(SkDraw, Bitmap)
JS_INIT(Sk, BaseBitmap)
//JS_INIT(Sk, Before)
JS_INIT(SkDraw, BitmapShader)
JS_INIT(SkDraw, Blur)
JS_INIT(SkDraw, Clip)
JS_INIT(SkDraw, Color)
JS_INIT(Sk, CubicTo)
JS_INIT(Sk, Dash)
JS_INIT(Sk, Data)
//JS_INIT(Sk, Dimensions)
JS_INIT(Sk, Discrete)
JS_INIT(Sk, DrawTo)
JS_INIT(SkDraw, Emboss)
JS_INIT(SkDisplay, Event)
// JS_INIT(SkDraw, Font)
// JS_INIT(Sk, Focus)
JS_INIT(Sk, Image)
JS_INIT(Sk, Include)
// JS_INIT(Sk, Input)
JS_INIT(Sk, Line)
JS_INIT(Sk, LinearGradient)
JS_INIT(Sk, LineTo)
JS_INIT(SkDraw, Matrix)
JS_INIT(Sk, Move)
JS_INIT(Sk, MoveTo)
JS_INIT(Sk, Oval)
JS_INIT(SkDraw, Path)
JS_INIT(SkDraw, Paint)
JS_INIT(Sk, DrawPoint)
JS_INIT(Sk, PolyToPoly)
JS_INIT(Sk, Polygon)
JS_INIT(Sk, Polyline)
JS_INIT(Sk, Post)
JS_INIT(Sk, QuadTo)
JS_INIT(Sk, RadialGradient)
JS_INIT(SkDisplay, Random)
JS_INIT(Sk, RectToRect)
JS_INIT(Sk, Rectangle)
JS_INIT(Sk, Remove)
JS_INIT(Sk, Replace)
JS_INIT(Sk, Rotate)
JS_INIT(Sk, RoundRect)
JS_INIT(Sk, Scale)
JS_INIT(Sk, Set)
JS_INIT(Sk, Skew)
// JS_INIT(Sk, 3D_Camera)
// JS_INIT(Sk, 3D_Patch)
JS_INIT(Sk, Snapshot)
// JS_INIT(SkDraw, Stroke)
JS_INIT(Sk, Text)
JS_INIT(Sk, TextOnPath)
JS_INIT(Sk, TextToPath)
JS_INIT(Sk, Translate)
//JS_INIT(Sk, Use)

#if SK_USE_CONDENSED_INFO == 0
static void GenerateTables() {
    for (int index = 0; index < kTypeNamesSize; index++) {
        int infoCount;
        SkDisplayTypes type = gTypeNames[index].fType;
        const SkMemberInfo* info = SkDisplayType::GetMembers(NULL /* fMaker */, type, &infoCount);
        if (info == NULL)
            continue;
        gDisplayableProperties[type] = new JSPropertySpec[infoCount + 1];
        JSPropertySpec* propertySpec = gDisplayableProperties[type];
        memset(propertySpec, 0, sizeof (JSPropertySpec) * (infoCount + 1));
        for (int inner = 0; inner < infoCount; inner++) {
            if (info[inner].fType == SkType_BaseClassInfo)
                continue;
            propertySpec[inner].name = info[inner].fName;
            propertySpec[inner].tinyid = inner;
            propertySpec[inner].flags = JSPROP_ENUMERATE;
        }
        gDisplayableClasses[type].name = gTypeNames[index].fName;
        gDisplayableClasses[type].flags = JSCLASS_HAS_PRIVATE;
        gDisplayableClasses[type].addProperty = JS_PropertyStub;
        gDisplayableClasses[type].delProperty = JS_PropertyStub;
        gDisplayableClasses[type].getProperty = SkJSDisplayable::GetProperty;
        gDisplayableClasses[type].setProperty = SkJSDisplayable::SetProperty;
        gDisplayableClasses[type].enumerate = JS_EnumerateStub;
        gDisplayableClasses[type].resolve = JS_ResolveStub;
        gDisplayableClasses[type].convert = JS_ConvertStub;
        gDisplayableClasses[type].finalize = SkJSDisplayable::Destructor;
    }
}
#endif

void SkJSDisplayable::Destructor(JSContext *cx, JSObject *obj) {
    delete (SkJSDisplayable*) JS_GetPrivate(cx, obj);
}

JSBool SkJSDisplayable::GetProperty(JSContext *cx, JSObject *obj, jsval id,
                                 jsval *vp)
{
    if (JSVAL_IS_INT(id) == 0)
        return JS_TRUE; 
    SkJSDisplayable *p = (SkJSDisplayable *) JS_GetPrivate(cx, obj);
    SkDisplayable* displayable = p->fDisplayable;
    SkDisplayTypes displayableType = displayable->getType();
    int members;
    const SkMemberInfo* info = SkDisplayType::GetMembers(NULL /* fMaker */, displayableType, &members);
    int idIndex = JSVAL_TO_INT(id);
    SkASSERT(idIndex >= 0 && idIndex < members);
    info = &info[idIndex];
    SkDisplayTypes infoType = (SkDisplayTypes) info->fType;
    SkScalar scalar = 0;
    S32 s32 = 0;
    SkString* string= NULL;
    JSString *str;
    if (infoType == SkType_MemberProperty) {
        infoType = info->propertyType();
        switch (infoType) {
            case SkType_Scalar: {
                SkScriptValue scriptValue;
                bool success = displayable->getProperty(info->propertyIndex(), &scriptValue);
                SkASSERT(scriptValue.fType == SkType_Scalar);
                scalar = scriptValue.fOperand.fScalar;
                } break;
            default:
                SkASSERT(0); // !!! unimplemented
        }
    } else {
        SkASSERT(info->fCount == 1);
        switch (infoType) {
            case SkType_Boolean:
            case SkType_Color:
            case SkType_S32:
                s32 = *(S32*) info->memberData(displayable);
                break;
            case SkType_String:
                info->getString(displayable, &string);
                break;
            case SkType_Scalar:
                SkOperand operand;
                info->getValue(displayable, &operand, 1);
                scalar = operand.fScalar;
                break;
            default:
                SkASSERT(0); // !!! unimplemented
        }
    }
    switch (infoType) {
        case SkType_Boolean:
            *vp = BOOLEAN_TO_JSVAL(s32);
            break;
        case SkType_Color:
        case SkType_S32:
            *vp = INT_TO_JSVAL(s32);
            break;
        case SkType_Scalar:
            if (SkScalarFraction(scalar) == 0)
                *vp = INT_TO_JSVAL(SkScalarFloor(scalar));
            else
#ifdef SK_SCALAR_IS_FLOAT
            *vp = DOUBLE_TO_JSVAL(scalar);
#else
            *vp = DOUBLE_TO_JSVAL(scalar / 65536.0f );
#endif
            break;
        case SkType_String:
            str = JS_NewStringCopyN(cx, string->c_str(), string->size());
            *vp = STRING_TO_JSVAL(str);
            break;
        default:
            SkASSERT(0); // !!! unimplemented
    }
    return JS_TRUE;
}

JSBool SkJSDisplayable::SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    if (JSVAL_IS_INT(id) == 0)
        return JS_TRUE; 
    SkJSDisplayable *p = (SkJSDisplayable *) JS_GetPrivate(cx, obj);
    SkDisplayable* displayable = p->fDisplayable;
    SkDisplayTypes displayableType = displayable->getType();
    int members;
    const SkMemberInfo* info = SkDisplayType::GetMembers(NULL /* fMaker */, displayableType, &members);
    int idIndex = JSVAL_TO_INT(id);
    SkASSERT(idIndex >= 0 && idIndex < members);
    info = &info[idIndex];
    SkDisplayTypes infoType = info->getType();
    SkScalar scalar = 0;
    S32 s32 = 0;
    SkString string;
    JSString* str;
    jsval value = *vp;
    switch (infoType) {
        case SkType_Boolean:
            s32 = JSVAL_TO_BOOLEAN(value);
            break;
        case SkType_Color:
        case SkType_S32:
            s32 = JSVAL_TO_INT(value);
            break;
        case SkType_Scalar:
            if (JSVAL_IS_INT(value))
                scalar = SkIntToScalar(JSVAL_TO_INT(value));
            else {
                SkASSERT(JSVAL_IS_DOUBLE(value));
#ifdef SK_SCALAR_IS_FLOAT
                scalar = (float) *(double*) JSVAL_TO_DOUBLE(value);
#else
                scalar = (SkFixed)  (*(double*)JSVAL_TO_DOUBLE(value) * 65536.0);
#endif
            }
            break;
        case SkType_String:
            str = JS_ValueToString(cx, value);
            string.set(JS_GetStringBytes(str));
            break;
        default:
            SkASSERT(0); // !!! unimplemented
    }
    if (info->fType == SkType_MemberProperty) {
        switch (infoType) {
            case SkType_Scalar: {
                SkScriptValue scriptValue;
                scriptValue.fType = SkType_Scalar;
                scriptValue.fOperand.fScalar = scalar;
                displayable->setProperty(-1 - (int) info->fOffset, scriptValue);
                } break;
            default:
                SkASSERT(0); // !!! unimplemented
        }
    } else {
        SkASSERT(info->fCount == 1);
        switch (infoType) {
            case SkType_Boolean:
            case SkType_Color:
            case SkType_S32:
                s32 = *(S32*) ((const char*) displayable + info->fOffset);
                break;
            case SkType_String:
                info->setString(displayable, &string);
                break;
            case SkType_Scalar:
                SkOperand operand;
                operand.fScalar = scalar;
                info->setValue(displayable, &operand, 1);
                break;
            default:
                SkASSERT(0); // !!! unimplemented
        }
    }
    return JS_TRUE;
}

void SkJS::InitializeDisplayables(const SkBitmap& bitmap, JSContext *cx, JSObject *obj, JSObject *proto) {
    SkJSDisplayable::gCanvas = new SkCanvas(bitmap);
    SkJSDisplayable::gPaint = new SkPaint();
#if SK_USE_CONDENSED_INFO == 0
    GenerateTables();
#else
    SkASSERT(0); // !!! compressed version hasn't been implemented
#endif
    AddInit(cx, obj, proto);
    AddCircleInit(cx, obj, proto);
    AddOvalInit(cx, obj, proto);
    AddPathInit(cx, obj, proto);
    AddRectangleInit(cx, obj, proto);
    AddRoundRectInit(cx, obj, proto);
//  AfterInit(cx, obj, proto);
    ApplyInit(cx, obj, proto);
    // AnimateInit(cx, obj, proto);
//  AnimateColorInit(cx, obj, proto);
    AnimateFieldInit(cx, obj, proto);
//  AnimateRotateInit(cx, obj, proto);
//  AnimateScaleInit(cx, obj, proto);
//  AnimateTranslateInit(cx, obj, proto);
    BitmapInit(cx, obj, proto);
//  BaseBitmapInit(cx, obj, proto);
//  BeforeInit(cx, obj, proto);
    BitmapShaderInit(cx, obj, proto);
    BlurInit(cx, obj, proto);
    ClipInit(cx, obj, proto);
    ColorInit(cx, obj, proto);
    CubicToInit(cx, obj, proto);
    DashInit(cx, obj, proto);
    DataInit(cx, obj, proto);
//  DimensionsInit(cx, obj, proto);
    DiscreteInit(cx, obj, proto);
    DrawToInit(cx, obj, proto);
    EmbossInit(cx, obj, proto);
    EventInit(cx, obj, proto);
//  FontInit(cx, obj, proto);
//  FocusInit(cx, obj, proto);
    ImageInit(cx, obj, proto);
    IncludeInit(cx, obj, proto);
//  InputInit(cx, obj, proto);
    LineInit(cx, obj, proto);
    LinearGradientInit(cx, obj, proto);
    LineToInit(cx, obj, proto);
    MatrixInit(cx, obj, proto);
    MoveInit(cx, obj, proto);
    MoveToInit(cx, obj, proto);
    OvalInit(cx, obj, proto);
    PathInit(cx, obj, proto);
    PaintInit(cx, obj, proto);
    DrawPointInit(cx, obj, proto);
    PolyToPolyInit(cx, obj, proto);
    PolygonInit(cx, obj, proto);
    PolylineInit(cx, obj, proto);
    PostInit(cx, obj, proto);
    QuadToInit(cx, obj, proto);
    RadialGradientInit(cx, obj, proto);
    RandomInit(cx, obj, proto);
    RectToRectInit(cx, obj, proto);
    RectangleInit(cx, obj, proto);
    RemoveInit(cx, obj, proto);
    ReplaceInit(cx, obj, proto);
    RotateInit(cx, obj, proto);
    RoundRectInit(cx, obj, proto);
    ScaleInit(cx, obj, proto);
    SetInit(cx, obj, proto);
    SkewInit(cx, obj, proto);
    // 3D_CameraInit(cx, obj, proto);
    // 3D_PatchInit(cx, obj, proto);
    SnapshotInit(cx, obj, proto);
//  StrokeInit(cx, obj, proto);
    TextInit(cx, obj, proto);
    TextOnPathInit(cx, obj, proto);
    TextToPathInit(cx, obj, proto);
    TranslateInit(cx, obj, proto);
//  UseInit(cx, obj, proto);
}

void SkJS::DisposeDisplayables() {
    delete SkJSDisplayable::gPaint;
    delete SkJSDisplayable::gCanvas;
    for (int index = 0; index < kTypeNamesSize; index++) {
        SkDisplayTypes type = gTypeNames[index].fType;
        delete[] gDisplayableProperties[type];
    }
}
