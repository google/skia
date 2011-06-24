/* libs/graphics/animator/SkDisplayType.cpp
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

#include "SkDisplayType.h"
#include "SkAnimateMaker.h"
#include "SkAnimateSet.h"
#include "SkDisplayAdd.h"
#include "SkDisplayApply.h"
#include "SkDisplayBounds.h"
#include "SkDisplayEvent.h"
#include "SkDisplayInclude.h"
#ifdef SK_DEBUG
#include "SkDisplayList.h"
#endif
#include "SkDisplayMath.h"
#include "SkDisplayMovie.h"
#include "SkDisplayNumber.h"
#include "SkDisplayPost.h"
#include "SkDisplayRandom.h"
#include "SkDisplayTypes.h"
#include "SkDraw3D.h"
#include "SkDrawBitmap.h"
#include "SkDrawClip.h"
#include "SkDrawDash.h"
#include "SkDrawDiscrete.h"
#include "SkDrawEmboss.h"
#include "SkDrawFull.h"
#include "SkDrawGradient.h"
#include "SkDrawLine.h"
#include "SkDrawMatrix.h"
#include "SkDrawOval.h"
#include "SkDrawPaint.h"
#include "SkDrawPath.h"
#include "SkDrawPoint.h"
#include "SkDrawSaveLayer.h"
#include "SkDrawText.h"
#include "SkDrawTextBox.h"
#include "SkDrawTo.h"
#include "SkDrawTransparentShader.h"
#include "SkDump.h"
#include "SkExtras.h"
#include "SkHitClear.h"
#include "SkHitTest.h"
#include "SkMatrixParts.h"
#include "SkPathParts.h"
#include "SkPostParts.h"
#include "SkSnapshot.h"
#include "SkTextOnPath.h"
#include "SkTextToPath.h"
#include "SkTSearch.h"

#define CASE_NEW(_class) \
    case SkType_##_class: result = new Sk##_class(); break
#define CASE_DRAW_NEW(_class) \
    case SkType_##_class: result = new SkDraw##_class(); break
#define CASE_DISPLAY_NEW(_class) \
    case SkType_##_class: result = new SkDisplay##_class(); break
#ifdef SK_DEBUG
    #define CASE_DEBUG_RETURN_NIL(_class) \
        case SkType_##_class: return NULL
#else
    #define CASE_DEBUG_RETURN_NIL(_class)
#endif
    

SkDisplayTypes SkDisplayType::gNewTypes = kNumberOfTypes;

SkDisplayable* SkDisplayType::CreateInstance(SkAnimateMaker* maker, SkDisplayTypes type) {
    SkDisplayable* result = NULL;
    switch (type) {
        // unknown
        CASE_DISPLAY_NEW(Math);
        CASE_DISPLAY_NEW(Number);
        CASE_NEW(Add);
        CASE_NEW(AddCircle);
        // addgeom
        CASE_DEBUG_RETURN_NIL(AddMode);
        CASE_NEW(AddOval);
        CASE_NEW(AddPath);
        CASE_NEW(AddRect);
        CASE_NEW(AddRoundRect);
        CASE_DEBUG_RETURN_NIL(Align);
        CASE_NEW(Animate);
        // animatebase
        CASE_NEW(Apply);
        CASE_DEBUG_RETURN_NIL(ApplyMode);
        CASE_DEBUG_RETURN_NIL(ApplyTransition);
        CASE_DISPLAY_NEW(Array);
        // argb
        // base64
        // basebitmap
        // baseclassinfo
        CASE_DRAW_NEW(Bitmap);
        // bitmapencoding
        // bitmapformat
        CASE_DRAW_NEW(BitmapShader);
        CASE_DRAW_NEW(Blur);
        CASE_DISPLAY_NEW(Boolean);
        // boundable
        CASE_DISPLAY_NEW(Bounds);
        CASE_DEBUG_RETURN_NIL(Cap);
        CASE_NEW(Clear);
        CASE_DRAW_NEW(Clip);
        CASE_NEW(Close);
        CASE_DRAW_NEW(Color);
        CASE_NEW(CubicTo);
        CASE_NEW(Dash);
        CASE_NEW(DataInput);
        CASE_NEW(Discrete);
        // displayable
        // drawable
        CASE_NEW(DrawTo);
        CASE_NEW(Dump);
        // dynamicstring
        CASE_DRAW_NEW(Emboss);
        CASE_DISPLAY_NEW(Event);
        CASE_DEBUG_RETURN_NIL(EventCode);
        CASE_DEBUG_RETURN_NIL(EventKind);
        CASE_DEBUG_RETURN_NIL(EventMode);
        // filltype
        // filtertype
        CASE_DISPLAY_NEW(Float);
        CASE_NEW(FromPath);
        CASE_DEBUG_RETURN_NIL(FromPathMode);
        CASE_NEW(Full);
        // gradient
        CASE_NEW(Group);
        CASE_NEW(HitClear);
        CASE_NEW(HitTest);
        CASE_NEW(Image);
        CASE_NEW(Include);
        CASE_NEW(Input);
        CASE_DISPLAY_NEW(Int);
        CASE_DEBUG_RETURN_NIL(Join);
        CASE_NEW(Line);
        CASE_NEW(LineTo);
        CASE_NEW(LinearGradient);
        CASE_DRAW_NEW(MaskFilter);
        CASE_DEBUG_RETURN_NIL(MaskFilterBlurStyle);
        // maskfilterlight
        CASE_DRAW_NEW(Matrix);
        // memberfunction
        // memberproperty
        CASE_NEW(Move);
        CASE_NEW(MoveTo);
        CASE_DISPLAY_NEW(Movie);
        // msec
        CASE_NEW(Oval);
        CASE_DRAW_NEW(Paint);
        CASE_DRAW_NEW(Path);
        // pathdirection
        CASE_DRAW_NEW(PathEffect);
        // point
        CASE_NEW(DrawPoint);
        CASE_NEW(PolyToPoly);
        CASE_NEW(Polygon);
        CASE_NEW(Polyline);
        CASE_NEW(Post);
        CASE_NEW(QuadTo);
        CASE_NEW(RCubicTo);
        CASE_NEW(RLineTo);
        CASE_NEW(RMoveTo);
        CASE_NEW(RQuadTo);
        CASE_NEW(RadialGradient);
        CASE_DISPLAY_NEW(Random);
        CASE_DRAW_NEW(Rect);
        CASE_NEW(RectToRect);
        CASE_NEW(Remove);
        CASE_NEW(Replace);
        CASE_NEW(Rotate);
        CASE_NEW(RoundRect);
        CASE_NEW(Save);
        CASE_NEW(SaveLayer);
        CASE_NEW(Scale);
        // screenplay
        CASE_NEW(Set);
        CASE_DRAW_NEW(Shader);
        CASE_NEW(Skew);
        CASE_NEW(3D_Camera);
        CASE_NEW(3D_Patch);
        // 3dpoint
        CASE_NEW(Snapshot);
        CASE_DISPLAY_NEW(String);
        // style
        CASE_NEW(Text);
        CASE_DRAW_NEW(TextBox);
        // textboxalign
        // textboxmode
        CASE_NEW(TextOnPath);
        CASE_NEW(TextToPath);
        CASE_DEBUG_RETURN_NIL(TileMode);
        CASE_NEW(Translate);
        CASE_DRAW_NEW(TransparentShader);
        CASE_DRAW_NEW(Typeface);
        CASE_DEBUG_RETURN_NIL(Xfermode);
        default:
            SkExtras** end = maker->fExtras.end();
            for (SkExtras** extraPtr = maker->fExtras.begin(); extraPtr < end; extraPtr++) {
                if ((result = (*extraPtr)->createInstance(type)) != NULL)
                    return result;
            }
            SkASSERT(0);
    }
    return result;
}

#undef CASE_NEW
#undef CASE_DRAW_NEW
#undef CASE_DISPLAY_NEW

#if SK_USE_CONDENSED_INFO == 0

#define CASE_GET_INFO(_class) case SkType_##_class: \
    info = Sk##_class::fInfo; infoCount = Sk##_class::fInfoCount; break
#define CASE_GET_DRAW_INFO(_class) case SkType_##_class: \
    info = SkDraw##_class::fInfo; infoCount = SkDraw##_class::fInfoCount; break
#define CASE_GET_DISPLAY_INFO(_class) case SkType_##_class: \
    info = SkDisplay##_class::fInfo; infoCount = SkDisplay##_class::fInfoCount; \
    break

const SkMemberInfo* SkDisplayType::GetMembers(SkAnimateMaker* maker, 
        SkDisplayTypes type, int* infoCountPtr) {
    const SkMemberInfo* info = NULL;
    int infoCount = 0;
    switch (type) {
        // unknown
        CASE_GET_DISPLAY_INFO(Math);
        CASE_GET_DISPLAY_INFO(Number);
        CASE_GET_INFO(Add);
        CASE_GET_INFO(AddCircle);
        CASE_GET_INFO(AddGeom);
        // addmode
        CASE_GET_INFO(AddOval);
        CASE_GET_INFO(AddPath);
        CASE_GET_INFO(AddRect);
        CASE_GET_INFO(AddRoundRect);
        // align
        CASE_GET_INFO(Animate);
        CASE_GET_INFO(AnimateBase);
        CASE_GET_INFO(Apply);
        // applymode
        // applytransition
        CASE_GET_DISPLAY_INFO(Array);
        // argb
        // base64
        CASE_GET_INFO(BaseBitmap);
        // baseclassinfo
        CASE_GET_DRAW_INFO(Bitmap);
        // bitmapencoding
        // bitmapformat
        CASE_GET_DRAW_INFO(BitmapShader);
        CASE_GET_DRAW_INFO(Blur);
        CASE_GET_DISPLAY_INFO(Boolean);
        // boundable
        CASE_GET_DISPLAY_INFO(Bounds);
        // cap
        // clear
        CASE_GET_DRAW_INFO(Clip);
        // close
        CASE_GET_DRAW_INFO(Color);
        CASE_GET_INFO(CubicTo);
        CASE_GET_INFO(Dash);
        CASE_GET_INFO(DataInput);
        CASE_GET_INFO(Discrete);
        // displayable
        // drawable
        CASE_GET_INFO(DrawTo);
        CASE_GET_INFO(Dump);
        // dynamicstring
        CASE_GET_DRAW_INFO(Emboss);
        CASE_GET_DISPLAY_INFO(Event);
        // eventcode
        // eventkind
        // eventmode
        // filltype
        // filtertype
        CASE_GET_DISPLAY_INFO(Float);
        CASE_GET_INFO(FromPath);
        // frompathmode
        // full
        CASE_GET_INFO(Gradient);
        CASE_GET_INFO(Group);
        CASE_GET_INFO(HitClear);
        CASE_GET_INFO(HitTest);
        CASE_GET_INFO(Image);
        CASE_GET_INFO(Include);
        CASE_GET_INFO(Input);
        CASE_GET_DISPLAY_INFO(Int);
        // join
        CASE_GET_INFO(Line);
        CASE_GET_INFO(LineTo);
        CASE_GET_INFO(LinearGradient);
        // maskfilter
        // maskfilterblurstyle
        // maskfilterlight
        CASE_GET_DRAW_INFO(Matrix);
        // memberfunction
        // memberproperty
        CASE_GET_INFO(Move);
        CASE_GET_INFO(MoveTo);
        CASE_GET_DISPLAY_INFO(Movie);
        // msec
        CASE_GET_INFO(Oval);
        CASE_GET_DRAW_INFO(Path);
        CASE_GET_DRAW_INFO(Paint);
        // pathdirection
        // patheffect
        case SkType_Point: info = Sk_Point::fInfo; infoCount = Sk_Point::fInfoCount; break; // no virtual flavor
        CASE_GET_INFO(DrawPoint); // virtual flavor
        CASE_GET_INFO(PolyToPoly);
        CASE_GET_INFO(Polygon);
        CASE_GET_INFO(Polyline);
        CASE_GET_INFO(Post);
        CASE_GET_INFO(QuadTo);
        CASE_GET_INFO(RCubicTo);
        CASE_GET_INFO(RLineTo);
        CASE_GET_INFO(RMoveTo);
        CASE_GET_INFO(RQuadTo);
        CASE_GET_INFO(RadialGradient);
        CASE_GET_DISPLAY_INFO(Random);
        CASE_GET_DRAW_INFO(Rect);
        CASE_GET_INFO(RectToRect);
        CASE_GET_INFO(Remove);
        CASE_GET_INFO(Replace);
        CASE_GET_INFO(Rotate);
        CASE_GET_INFO(RoundRect);
        CASE_GET_INFO(Save);
        CASE_GET_INFO(SaveLayer);
        CASE_GET_INFO(Scale);
        // screenplay
        CASE_GET_INFO(Set);
        CASE_GET_DRAW_INFO(Shader);
        CASE_GET_INFO(Skew);
        CASE_GET_INFO(3D_Camera);
        CASE_GET_INFO(3D_Patch);
        CASE_GET_INFO(3D_Point);
        CASE_GET_INFO(Snapshot);
        CASE_GET_DISPLAY_INFO(String);
        // style
        CASE_GET_INFO(Text);
        CASE_GET_DRAW_INFO(TextBox);
        // textboxalign
        // textboxmode
        CASE_GET_INFO(TextOnPath);
        CASE_GET_INFO(TextToPath);
        // tilemode
        CASE_GET_INFO(Translate);
        // transparentshader
        CASE_GET_DRAW_INFO(Typeface);
        // xfermode
        // knumberoftypes
        default: 
            if (maker) {
                SkExtras** end = maker->fExtras.end();
                for (SkExtras** extraPtr = maker->fExtras.begin(); extraPtr < end; extraPtr++) {
                    if ((info = (*extraPtr)->getMembers(type, infoCountPtr)) != NULL)
                        return info;
                }
            }
            return NULL;
    }
    if (infoCountPtr)
        *infoCountPtr = infoCount;
    return info;
}

const SkMemberInfo* SkDisplayType::GetMember(SkAnimateMaker* maker, 
        SkDisplayTypes type, const char** matchPtr ) {
    int infoCount;
    const SkMemberInfo* info = GetMembers(maker, type, &infoCount);
    info = SkMemberInfo::Find(info, infoCount, matchPtr);
//  SkASSERT(info);
    return info;
}

#undef CASE_GET_INFO
#undef CASE_GET_DRAW_INFO
#undef CASE_GET_DISPLAY_INFO

#endif // SK_USE_CONDENSED_INFO == 0

#if defined SK_DEBUG || defined SK_BUILD_CONDENSED
    #define DRAW_NAME(_name, _type) {_name, _type, true, false }
    #define DISPLAY_NAME(_name, _type) {_name, _type, false, true }
    #define INIT_BOOL_FIELDS    , false, false
#else
    #define DRAW_NAME(_name, _type) {_name, _type }
    #define DISPLAY_NAME(_name, _type) {_name, _type }
    #define INIT_BOOL_FIELDS
#endif

const TypeNames gTypeNames[] = {
    // unknown
    { "Math", SkType_Math                       INIT_BOOL_FIELDS },
    { "Number", SkType_Number                   INIT_BOOL_FIELDS },
    { "add", SkType_Add                         INIT_BOOL_FIELDS },
    { "addCircle", SkType_AddCircle             INIT_BOOL_FIELDS },
    // addgeom
    // addmode
    { "addOval", SkType_AddOval                 INIT_BOOL_FIELDS },
    { "addPath", SkType_AddPath                 INIT_BOOL_FIELDS },
    { "addRect", SkType_AddRect                 INIT_BOOL_FIELDS },
    { "addRoundRect", SkType_AddRoundRect       INIT_BOOL_FIELDS },
    // align
    { "animate", SkType_Animate                 INIT_BOOL_FIELDS },
    // animateBase
    { "apply", SkType_Apply                     INIT_BOOL_FIELDS },
    // applymode
    // applytransition
    { "array", SkType_Array                     INIT_BOOL_FIELDS },
    // argb
    // base64
    // basebitmap
    // baseclassinfo
    DRAW_NAME("bitmap", SkType_Bitmap),
    // bitmapencoding
    // bitmapformat
    DRAW_NAME("bitmapShader", SkType_BitmapShader),
    DRAW_NAME("blur", SkType_Blur),
    { "boolean", SkType_Boolean                 INIT_BOOL_FIELDS },
    // boundable
    DISPLAY_NAME("bounds", SkType_Bounds),
    // cap
    { "clear", SkType_Clear                     INIT_BOOL_FIELDS },
    DRAW_NAME("clip", SkType_Clip),
    { "close", SkType_Close                     INIT_BOOL_FIELDS },
    DRAW_NAME("color", SkType_Color),
    { "cubicTo", SkType_CubicTo                 INIT_BOOL_FIELDS },
    { "dash", SkType_Dash                       INIT_BOOL_FIELDS },
    { "data", SkType_DataInput                  INIT_BOOL_FIELDS },
    { "discrete", SkType_Discrete               INIT_BOOL_FIELDS },
    // displayable
    // drawable
    { "drawTo", SkType_DrawTo                   INIT_BOOL_FIELDS },
    { "dump", SkType_Dump                       INIT_BOOL_FIELDS },
    // dynamicstring
    DRAW_NAME("emboss", SkType_Emboss),
    DISPLAY_NAME("event", SkType_Event),
    // eventcode
    // eventkind
    // eventmode
    // filltype
    // filtertype
    { "float", SkType_Float                     INIT_BOOL_FIELDS },
    { "fromPath", SkType_FromPath               INIT_BOOL_FIELDS },
    // frompathmode
    { "full", SkType_Full                       INIT_BOOL_FIELDS },
    // gradient
    { "group", SkType_Group                     INIT_BOOL_FIELDS },
    { "hitClear", SkType_HitClear               INIT_BOOL_FIELDS },
    { "hitTest", SkType_HitTest                 INIT_BOOL_FIELDS },
    { "image", SkType_Image                     INIT_BOOL_FIELDS },
    { "include", SkType_Include                 INIT_BOOL_FIELDS },
    { "input", SkType_Input                     INIT_BOOL_FIELDS },
    { "int", SkType_Int                         INIT_BOOL_FIELDS },
    // join
    { "line", SkType_Line                       INIT_BOOL_FIELDS },
    { "lineTo", SkType_LineTo                   INIT_BOOL_FIELDS },
    { "linearGradient", SkType_LinearGradient   INIT_BOOL_FIELDS },
    { "maskFilter", SkType_MaskFilter           INIT_BOOL_FIELDS },
    // maskfilterblurstyle
    // maskfilterlight
    DRAW_NAME("matrix", SkType_Matrix),
    // memberfunction
    // memberproperty
    { "move", SkType_Move                       INIT_BOOL_FIELDS },
    { "moveTo", SkType_MoveTo                   INIT_BOOL_FIELDS },
    { "movie", SkType_Movie                     INIT_BOOL_FIELDS },
    // msec
    { "oval", SkType_Oval                       INIT_BOOL_FIELDS },
    DRAW_NAME("paint", SkType_Paint),
    DRAW_NAME("path", SkType_Path),
    // pathdirection
    { "pathEffect", SkType_PathEffect           INIT_BOOL_FIELDS },
    // point
    DRAW_NAME("point", SkType_DrawPoint),
    { "polyToPoly", SkType_PolyToPoly           INIT_BOOL_FIELDS },
    { "polygon", SkType_Polygon                 INIT_BOOL_FIELDS },
    { "polyline", SkType_Polyline               INIT_BOOL_FIELDS },
    { "post", SkType_Post                       INIT_BOOL_FIELDS },
    { "quadTo", SkType_QuadTo                   INIT_BOOL_FIELDS },
    { "rCubicTo", SkType_RCubicTo               INIT_BOOL_FIELDS },
    { "rLineTo", SkType_RLineTo                 INIT_BOOL_FIELDS },
    { "rMoveTo", SkType_RMoveTo                 INIT_BOOL_FIELDS },
    { "rQuadTo", SkType_RQuadTo                 INIT_BOOL_FIELDS },
    { "radialGradient", SkType_RadialGradient   INIT_BOOL_FIELDS },
    DISPLAY_NAME("random", SkType_Random),
    { "rect", SkType_Rect                       INIT_BOOL_FIELDS },
    { "rectToRect", SkType_RectToRect           INIT_BOOL_FIELDS },
    { "remove", SkType_Remove                   INIT_BOOL_FIELDS },
    { "replace", SkType_Replace                 INIT_BOOL_FIELDS },
    { "rotate", SkType_Rotate                   INIT_BOOL_FIELDS },
    { "roundRect", SkType_RoundRect             INIT_BOOL_FIELDS },
    { "save", SkType_Save                       INIT_BOOL_FIELDS },
    { "saveLayer", SkType_SaveLayer             INIT_BOOL_FIELDS },
    { "scale", SkType_Scale                     INIT_BOOL_FIELDS },
    // screenplay
    { "set", SkType_Set                         INIT_BOOL_FIELDS },
    { "shader", SkType_Shader                   INIT_BOOL_FIELDS },
    { "skew", SkType_Skew                       INIT_BOOL_FIELDS },
    { "skia3d:camera", SkType_3D_Camera         INIT_BOOL_FIELDS },
    { "skia3d:patch", SkType_3D_Patch           INIT_BOOL_FIELDS },
    // point
    { "snapshot", SkType_Snapshot               INIT_BOOL_FIELDS },
    { "string", SkType_String                   INIT_BOOL_FIELDS },
    // style
    { "text", SkType_Text                       INIT_BOOL_FIELDS },
    { "textBox", SkType_TextBox                 INIT_BOOL_FIELDS },
    // textboxalign
    // textboxmode
    { "textOnPath", SkType_TextOnPath           INIT_BOOL_FIELDS },
    { "textToPath", SkType_TextToPath           INIT_BOOL_FIELDS },
    // tilemode
    { "translate", SkType_Translate             INIT_BOOL_FIELDS },
    DRAW_NAME("transparentShader", SkType_TransparentShader),
    { "typeface", SkType_Typeface               INIT_BOOL_FIELDS }
    // xfermode
    // knumberoftypes
};

const int kTypeNamesSize = SK_ARRAY_COUNT(gTypeNames);

SkDisplayTypes SkDisplayType::Find(SkAnimateMaker* maker, const SkMemberInfo* match) {
    for (int index = 0; index < kTypeNamesSize; index++) {
        SkDisplayTypes type = gTypeNames[index].fType;
        const SkMemberInfo* info = SkDisplayType::GetMembers(maker, type, NULL);
        if (info == match)
            return type;
    }
    return (SkDisplayTypes) -1;
}

// !!! optimize this by replacing function with a byte-sized lookup table
SkDisplayTypes SkDisplayType::GetParent(SkAnimateMaker* maker, SkDisplayTypes base) {
    if (base == SkType_Group || base == SkType_Save || base == SkType_SaveLayer)        //!!! cheat a little until we have a lookup table
        return SkType_Displayable;
    if (base == SkType_Set)
        return SkType_Animate;  // another cheat until we have a lookup table
    const SkMemberInfo* info = GetMembers(maker, base, NULL); // get info for this type
    SkASSERT(info);
    if (info->fType != SkType_BaseClassInfo)
        return SkType_Unknown; // if no base, done
    // !!! could change SK_MEMBER_INHERITED macro to take type, stuff in offset, so that 
    // this (and table builder) could know type without the following steps:
    const SkMemberInfo* inherited = info->getInherited();
    SkDisplayTypes result = (SkDisplayTypes) (SkType_Unknown + 1);
    for (; result <= SkType_Xfermode; result = (SkDisplayTypes) (result + 1)) {
        const SkMemberInfo* match = GetMembers(maker, result, NULL);
        if (match == inherited)
            break;
    }
    SkASSERT(result <= SkType_Xfermode);
    return result;
}

SkDisplayTypes SkDisplayType::GetType(SkAnimateMaker* maker, const char match[], size_t len ) {
    int index = SkStrSearch(&gTypeNames[0].fName, kTypeNamesSize, match, 
        len, sizeof(gTypeNames[0]));
    if (index >= 0 && index < kTypeNamesSize)
        return gTypeNames[index].fType;
    SkExtras** end = maker->fExtras.end();
    for (SkExtras** extraPtr = maker->fExtras.begin(); extraPtr < end; extraPtr++) {
        SkDisplayTypes result = (*extraPtr)->getType(match, len);
        if (result != SkType_Unknown)
            return result;
    }
    return (SkDisplayTypes) -1;
}

bool SkDisplayType::IsEnum(SkAnimateMaker* , SkDisplayTypes type) {
    switch (type) {
        case SkType_AddMode:
        case SkType_Align:
        case SkType_ApplyMode:
        case SkType_ApplyTransition:
        case SkType_BitmapEncoding:
        case SkType_BitmapFormat:
        case SkType_Boolean:
        case SkType_Cap:
        case SkType_EventCode:
        case SkType_EventKind:
        case SkType_EventMode:
        case SkType_FillType:
        case SkType_FilterType:
        case SkType_FontStyle:
        case SkType_FromPathMode:
        case SkType_Join:
        case SkType_MaskFilterBlurStyle:
        case SkType_PathDirection:
        case SkType_Style:
        case SkType_TextBoxAlign:
        case SkType_TextBoxMode:
        case SkType_TileMode:
        case SkType_Xfermode:
            return true;
        default:    // to avoid warnings
            break;
    }
    return false;
}

bool SkDisplayType::IsDisplayable(SkAnimateMaker* , SkDisplayTypes type) {
    switch (type) {
        case SkType_Add:
        case SkType_AddCircle:
        case SkType_AddOval:
        case SkType_AddPath:
        case SkType_AddRect:
        case SkType_AddRoundRect:
        case SkType_Animate:
        case SkType_AnimateBase:
        case SkType_Apply:
        case SkType_BaseBitmap:
        case SkType_Bitmap:
        case SkType_BitmapShader:
        case SkType_Blur:
        case SkType_Clear:
        case SkType_Clip:
        case SkType_Close:
        case SkType_Color:
        case SkType_CubicTo:
        case SkType_Dash:
        case SkType_DataInput:
        case SkType_Discrete:
        case SkType_Displayable:
        case SkType_Drawable:
        case SkType_DrawTo:
        case SkType_Emboss:
        case SkType_Event:
        case SkType_FromPath:
        case SkType_Full:
        case SkType_Group:
        case SkType_Image:
        case SkType_Input:
        case SkType_Line:
        case SkType_LineTo:
        case SkType_LinearGradient:
        case SkType_Matrix:
        case SkType_Move:
        case SkType_MoveTo:
        case SkType_Movie:
        case SkType_Oval:
        case SkType_Paint:
        case SkType_Path:
        case SkType_PolyToPoly:
        case SkType_Polygon:
        case SkType_Polyline:
        case SkType_Post:
        case SkType_QuadTo:
        case SkType_RCubicTo:
        case SkType_RLineTo:
        case SkType_RMoveTo:
        case SkType_RQuadTo:
        case SkType_RadialGradient:
        case SkType_Random:
        case SkType_Rect:
        case SkType_RectToRect:
        case SkType_Remove:
        case SkType_Replace:
        case SkType_Rotate:
        case SkType_RoundRect:
        case SkType_Save:
        case SkType_SaveLayer:
        case SkType_Scale:
        case SkType_Set:
        case SkType_Shader:
        case SkType_Skew:
        case SkType_3D_Camera:
        case SkType_3D_Patch:
        case SkType_Snapshot:
        case SkType_Text:
        case SkType_TextBox:
        case SkType_TextOnPath:
        case SkType_TextToPath:
        case SkType_Translate:
        case SkType_TransparentShader:
            return true;
        default:    // to avoid warnings
            break;
    }
    return false;
}

bool SkDisplayType::IsStruct(SkAnimateMaker* , SkDisplayTypes type) {
    switch (type) {
        case SkType_Point:
        case SkType_3D_Point:
            return true;
        default:    // to avoid warnings
            break;
    }
    return false;
}


SkDisplayTypes SkDisplayType::RegisterNewType() {
    gNewTypes = (SkDisplayTypes) (gNewTypes + 1);
    return gNewTypes;
}



#ifdef SK_DEBUG
const char* SkDisplayType::GetName(SkAnimateMaker* maker, SkDisplayTypes type) {
    for (int index = 0; index < kTypeNamesSize - 1; index++) {
        if (gTypeNames[index].fType == type)
            return gTypeNames[index].fName;
    }
    SkExtras** end = maker->fExtras.end();
    for (SkExtras** extraPtr = maker->fExtras.begin(); extraPtr < end; extraPtr++) {
        const char* result = (*extraPtr)->getName(type);
        if (result != NULL)
            return result;
    }
    return NULL;
}
#endif

#ifdef SK_SUPPORT_UNITTEST
void SkDisplayType::UnitTest() {
    SkAnimator animator;
    SkAnimateMaker* maker = animator.fMaker;
    int index;
    for (index = 0; index < kTypeNamesSize - 1; index++) {
        SkASSERT(strcmp(gTypeNames[index].fName, gTypeNames[index + 1].fName) < 0);
        SkASSERT(gTypeNames[index].fType < gTypeNames[index + 1].fType);
    }
    for (index = 0; index < kTypeNamesSize; index++) {
        SkDisplayable* test = CreateInstance(maker, gTypeNames[index].fType);
        if (test == NULL)
            continue;
#if defined _WIN32 && _MSC_VER >= 1300  && defined _INC_CRTDBG // only on windows, only if using "crtdbg.h"
    // we know that crtdbg puts 0xfdfdfdfd at the end of the block
    // look for unitialized memory, signature 0xcdcdcdcd prior to that
        int* start = (int*) test;
        while (*start != 0xfdfdfdfd) {
            SkASSERT(*start != 0xcdcdcdcd);
            start++;
        }
#endif
        delete test;
    }
    for (index = 0; index < kTypeNamesSize; index++) {
        int infoCount;
        const SkMemberInfo* info = GetMembers(maker, gTypeNames[index].fType, &infoCount);
        if (info == NULL)
            continue;
#if SK_USE_CONDENSED_INFO == 0
        for (int inner = 0; inner < infoCount - 1; inner++) {
            if (info[inner].fType == SkType_BaseClassInfo)
                continue;
            SkASSERT(strcmp(info[inner].fName, info[inner + 1].fName) < 0);
        }
#endif
    }
#if defined SK_DEBUG || defined SK_BUILD_CONDENSED
    BuildCondensedInfo(maker);
#endif
}
#endif
