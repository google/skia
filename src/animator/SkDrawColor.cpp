
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDrawColor.h"
#ifdef SK_DEBUG
#include "SkDisplayList.h"
#endif
#include "SkDrawPaint.h"
#include "SkParse.h"
#include "SkScript.h"

enum HSV_Choice {
    kGetHue,
    kGetSaturation,
    kGetValue
};

static SkScalar RGB_to_HSV(SkColor color, HSV_Choice choice) {
    SkScalar red = SkIntToScalar(SkColorGetR(color));
    SkScalar green = SkIntToScalar(SkColorGetG(color));
    SkScalar blue = SkIntToScalar(SkColorGetB(color));
    SkScalar min = SkMinScalar(SkMinScalar(red, green), blue);
    SkScalar value = SkMaxScalar(SkMaxScalar(red, green), blue);
    if (choice == kGetValue)
        return value/255;
    SkScalar delta = value - min;
    SkScalar saturation = value == 0 ? 0 : SkScalarDiv(delta, value);
    if (choice == kGetSaturation)
        return saturation;
    SkScalar hue;
    if (saturation == 0)
        hue = 0;
    else {
        SkScalar part60 = SkScalarDiv(60 * SK_Scalar1, delta);
        if (red == value) {
            hue = SkScalarMul(green - blue, part60);
            if (hue < 0)
                hue += 360 * SK_Scalar1;
        }
        else if (green == value)
            hue = 120 * SK_Scalar1 + SkScalarMul(blue - red, part60);
        else  // blue == value
            hue = 240 * SK_Scalar1 + SkScalarMul(red - green, part60);
    }
    SkASSERT(choice == kGetHue);
    return hue;
}

#if defined _WIN32 && _MSC_VER >= 1300  // disable 'red', etc. may be used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

static SkColor HSV_to_RGB(SkColor color, HSV_Choice choice, SkScalar hsv) {
    SkScalar hue = choice == kGetHue ? hsv : RGB_to_HSV(color, kGetHue);
    SkScalar saturation = choice == kGetSaturation ? hsv : RGB_to_HSV(color, kGetSaturation);
    SkScalar value = choice == kGetValue ? hsv : RGB_to_HSV(color, kGetValue);
    value *= 255;
    SkScalar red SK_INIT_TO_AVOID_WARNING;
    SkScalar green SK_INIT_TO_AVOID_WARNING;
    SkScalar blue SK_INIT_TO_AVOID_WARNING;
    if (saturation == 0)    // color is on black-and-white center line
        red = green = blue = value;
    else {
        //SkScalar fraction = SkScalarMod(hue, 60 * SK_Scalar1);
        int sextant = SkScalarFloor(hue / 60);
        SkScalar fraction = hue / 60 - SkIntToScalar(sextant);
        SkScalar p = SkScalarMul(value , SK_Scalar1 - saturation);
        SkScalar q = SkScalarMul(value, SK_Scalar1 - SkScalarMul(saturation, fraction));
        SkScalar t = SkScalarMul(value, SK_Scalar1 -
            SkScalarMul(saturation, SK_Scalar1 - fraction));
        switch (sextant % 6) {
            case 0: red = value; green = t; blue = p; break;
            case 1: red = q; green = value; blue = p; break;
            case 2: red = p; green = value; blue = t; break;
            case 3: red = p; green = q; blue = value; break;
            case 4: red = t;  green = p; blue = value; break;
            case 5: red = value; green = p; blue = q; break;
        }
    }
    //used to say SkToU8((U8CPU) red) etc
    return SkColorSetARGB(SkColorGetA(color), SkScalarRound(red),
        SkScalarRound(green), SkScalarRound(blue));
}

#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( pop )
#endif

enum SkDrawColor_Properties {
    SK_PROPERTY(alpha),
    SK_PROPERTY(blue),
    SK_PROPERTY(green),
    SK_PROPERTY(hue),
    SK_PROPERTY(red),
    SK_PROPERTY(saturation),
    SK_PROPERTY(value)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDrawColor::fInfo[] = {
    SK_MEMBER_PROPERTY(alpha, Float),
    SK_MEMBER_PROPERTY(blue, Float),
    SK_MEMBER(color, ARGB),
    SK_MEMBER_PROPERTY(green, Float),
    SK_MEMBER_PROPERTY(hue, Float),
    SK_MEMBER_PROPERTY(red, Float),
    SK_MEMBER_PROPERTY(saturation, Float),
    SK_MEMBER_PROPERTY(value, Float),
};

#endif

DEFINE_GET_MEMBER(SkDrawColor);

SkDrawColor::SkDrawColor() : fDirty(false) {
    color = SK_ColorBLACK;
    fHue = fSaturation = fValue = SK_ScalarNaN;
}

bool SkDrawColor::add() {
    if (fPaint->color != NULL)
        return true; // error (probably color in paint as attribute as well)
    fPaint->color = this;
    fPaint->fOwnsColor = true;
    return false;
}

SkDisplayable* SkDrawColor::deepCopy(SkAnimateMaker*) {
    SkDrawColor* copy = new SkDrawColor();
    copy->color = color;
    copy->fHue = fHue;
    copy->fSaturation = fSaturation;
    copy->fValue = fValue;
    copy->fDirty = fDirty;
    return copy;
}

void SkDrawColor::dirty(){
    fDirty = true;
}

#ifdef SK_DUMP_ENABLED
void SkDrawColor::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkDebugf("alpha=\"%d\" red=\"%d\" green=\"%d\" blue=\"%d\" />\n",
        SkColorGetA(color)/255, SkColorGetR(color),
        SkColorGetG(color), SkColorGetB(color));
}
#endif

SkColor SkDrawColor::getColor() {
    if (fDirty) {
        if (SkScalarIsNaN(fValue) == false)
            color = HSV_to_RGB(color, kGetValue, fValue);
        if (SkScalarIsNaN(fSaturation) == false)
            color = HSV_to_RGB(color, kGetSaturation, fSaturation);
        if (SkScalarIsNaN(fHue) == false)
            color = HSV_to_RGB(color, kGetHue, fHue);
        fDirty = false;
    }
    return color;
}

SkDisplayable* SkDrawColor::getParent() const {
    return fPaint;
}

bool SkDrawColor::getProperty(int index, SkScriptValue* value) const {
    value->fType = SkType_Float;
    SkScalar result;
    switch(index) {
        case SK_PROPERTY(alpha):
            result = SkIntToScalar(SkColorGetA(color)) / 255;
            break;
        case SK_PROPERTY(blue):
            result = SkIntToScalar(SkColorGetB(color));
            break;
        case SK_PROPERTY(green):
            result = SkIntToScalar(SkColorGetG(color));
            break;
        case SK_PROPERTY(hue):
            result = RGB_to_HSV(color, kGetHue);
            break;
        case SK_PROPERTY(red):
            result = SkIntToScalar(SkColorGetR(color));
            break;
        case SK_PROPERTY(saturation):
            result = RGB_to_HSV(color, kGetSaturation);
            break;
        case SK_PROPERTY(value):
            result = RGB_to_HSV(color, kGetValue);
            break;
        default:
            SkASSERT(0);
            return false;
    }
    value->fOperand.fScalar = result;
    return true;
}

void SkDrawColor::onEndElement(SkAnimateMaker&) {
    fDirty = true;
}

bool SkDrawColor::setParent(SkDisplayable* parent) {
    SkASSERT(parent != NULL);
    if (parent->getType() == SkType_DrawLinearGradient || parent->getType() == SkType_DrawRadialGradient)
        return false;
    if (parent->isPaint() == false)
        return true;
    fPaint = (SkDrawPaint*) parent;
    return false;
}

bool SkDrawColor::setProperty(int index, SkScriptValue& value) {
    SkASSERT(value.fType == SkType_Float);
    SkScalar scalar = value.fOperand.fScalar;
    switch (index) {
        case SK_PROPERTY(alpha):
            uint8_t alpha;
        #ifdef SK_SCALAR_IS_FLOAT
            alpha = scalar == SK_Scalar1 ? 255 : SkToU8((U8CPU) (scalar * 256));
        #else
            alpha = SkToU8((scalar - (scalar >= SK_ScalarHalf)) >> 8);
        #endif
            color = SkColorSetARGB(alpha, SkColorGetR(color),
                SkColorGetG(color), SkColorGetB(color));
            break;
        case SK_PROPERTY(blue):
            scalar = SkScalarClampMax(scalar, 255 * SK_Scalar1);
            color = SkColorSetARGB(SkColorGetA(color), SkColorGetR(color),
                SkColorGetG(color), SkToU8((U8CPU) scalar));
            break;
        case SK_PROPERTY(green):
            scalar = SkScalarClampMax(scalar, 255 * SK_Scalar1);
            color = SkColorSetARGB(SkColorGetA(color), SkColorGetR(color),
                SkToU8((U8CPU) scalar), SkColorGetB(color));
            break;
        case SK_PROPERTY(hue):
            fHue = scalar;//RGB_to_HSV(color, kGetHue);
            fDirty = true;
            break;
        case SK_PROPERTY(red):
            scalar = SkScalarClampMax(scalar, 255 * SK_Scalar1);
            color = SkColorSetARGB(SkColorGetA(color), SkToU8((U8CPU) scalar),
                SkColorGetG(color), SkColorGetB(color));
        break;
        case SK_PROPERTY(saturation):
            fSaturation = scalar;//RGB_to_HSV(color, kGetSaturation);
            fDirty = true;
            break;
        case SK_PROPERTY(value):
            fValue = scalar;//RGB_to_HSV(color, kGetValue);
            fDirty = true;
            break;
        default:
            SkASSERT(0);
            return false;
    }
    return true;
}
