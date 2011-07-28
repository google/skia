
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayMath.h"

enum SkDisplayMath_Properties {
    SK_PROPERTY(E),
    SK_PROPERTY(LN10),
    SK_PROPERTY(LN2),
    SK_PROPERTY(LOG10E),
    SK_PROPERTY(LOG2E),
    SK_PROPERTY(PI),
    SK_PROPERTY(SQRT1_2),
    SK_PROPERTY(SQRT2)
};

const SkScalar SkDisplayMath::gConstants[] = {
#ifdef SK_SCALAR_IS_FLOAT
    2.718281828f,   // E
    2.302585093f,   // LN10
    0.693147181f,   // LN2
    0.434294482f,   // LOG10E
    1.442695041f,   // LOG2E
    3.141592654f,   // PI
    0.707106781f,   // SQRT1_2
    1.414213562f        // SQRT2 
#else
    0x2B7E1,    // E
    0x24D76,    // LN10
    0xB172,     // LN2
    0x6F2E,     // LOG10E
    0x17154,    // LOG2E
    0x3243F,    // PI
    0xB505,     // SQRT1_2
    0x16A0A // SQRT2
#endif
};

enum SkDisplayMath_Functions {
    SK_FUNCTION(abs),
    SK_FUNCTION(acos),
    SK_FUNCTION(asin),
    SK_FUNCTION(atan),
    SK_FUNCTION(atan2),
    SK_FUNCTION(ceil),
    SK_FUNCTION(cos),
    SK_FUNCTION(exp),
    SK_FUNCTION(floor),
    SK_FUNCTION(log),
    SK_FUNCTION(max),
    SK_FUNCTION(min),
    SK_FUNCTION(pow),
    SK_FUNCTION(random),
    SK_FUNCTION(round),
    SK_FUNCTION(sin),
    SK_FUNCTION(sqrt),
    SK_FUNCTION(tan)
};

const SkFunctionParamType SkDisplayMath::fFunctionParameters[] = {
    (SkFunctionParamType) SkType_Float, // abs
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // acos
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // asin
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // atan
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // atan2
    (SkFunctionParamType) SkType_Float,
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // ceil
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // cos
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // exp
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // floor
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // log
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Array, // max
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Array, // min
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // pow
    (SkFunctionParamType) SkType_Float,
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // random
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // round
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // sin
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // sqrt
    (SkFunctionParamType) 0,
    (SkFunctionParamType) SkType_Float, // tan
    (SkFunctionParamType) 0
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayMath::fInfo[] = {
    SK_MEMBER_PROPERTY(E, Float),
    SK_MEMBER_PROPERTY(LN10, Float),
    SK_MEMBER_PROPERTY(LN2, Float),
    SK_MEMBER_PROPERTY(LOG10E, Float),
    SK_MEMBER_PROPERTY(LOG2E, Float),
    SK_MEMBER_PROPERTY(PI, Float),
    SK_MEMBER_PROPERTY(SQRT1_2, Float),
    SK_MEMBER_PROPERTY(SQRT2, Float),
    SK_MEMBER_FUNCTION(abs, Float),
    SK_MEMBER_FUNCTION(acos, Float),
    SK_MEMBER_FUNCTION(asin, Float),
    SK_MEMBER_FUNCTION(atan, Float),
    SK_MEMBER_FUNCTION(atan2, Float),
    SK_MEMBER_FUNCTION(ceil, Float),
    SK_MEMBER_FUNCTION(cos, Float),
    SK_MEMBER_FUNCTION(exp, Float),
    SK_MEMBER_FUNCTION(floor, Float),
    SK_MEMBER_FUNCTION(log, Float),
    SK_MEMBER_FUNCTION(max, Float),
    SK_MEMBER_FUNCTION(min, Float),
    SK_MEMBER_FUNCTION(pow, Float),
    SK_MEMBER_FUNCTION(random, Float),
    SK_MEMBER_FUNCTION(round, Float),
    SK_MEMBER_FUNCTION(sin, Float),
    SK_MEMBER_FUNCTION(sqrt, Float),
    SK_MEMBER_FUNCTION(tan, Float)
};

#endif

DEFINE_GET_MEMBER(SkDisplayMath);

void SkDisplayMath::executeFunction(SkDisplayable* target, int index, 
        SkTDArray<SkScriptValue>& parameters, SkDisplayTypes type,
        SkScriptValue* scriptValue) {
    if (scriptValue == NULL)
        return;
    SkASSERT(target == this);
    SkScriptValue* array = parameters.begin();
    SkScriptValue* end = parameters.end();
    SkScalar input = parameters[0].fOperand.fScalar;
    SkScalar scalarResult;
    switch (index) {
        case SK_FUNCTION(abs):
            scalarResult = SkScalarAbs(input); 
            break;
        case SK_FUNCTION(acos):
            scalarResult = SkScalarACos(input);
            break;
        case SK_FUNCTION(asin):
            scalarResult = SkScalarASin(input);
            break;
        case SK_FUNCTION(atan):
            scalarResult = SkScalarATan2(input, SK_Scalar1);
            break;
        case SK_FUNCTION(atan2):
            scalarResult = SkScalarATan2(input, parameters[1].fOperand.fScalar);
            break;
        case SK_FUNCTION(ceil):
            scalarResult = SkIntToScalar(SkScalarCeil(input)); 
            break;
        case SK_FUNCTION(cos):
            scalarResult = SkScalarCos(input);
            break;
        case SK_FUNCTION(exp):
            scalarResult = SkScalarExp(input);
            break;
        case SK_FUNCTION(floor):
            scalarResult = SkIntToScalar(SkScalarFloor(input)); 
            break;
        case SK_FUNCTION(log):
            scalarResult = SkScalarLog(input);
            break;
        case SK_FUNCTION(max):
            scalarResult = -SK_ScalarMax;
            while (array < end) {
                scalarResult = SkMaxScalar(scalarResult, array->fOperand.fScalar);
                array++;
            }
            break;
        case SK_FUNCTION(min):
            scalarResult = SK_ScalarMax;
            while (array < end) {
                scalarResult = SkMinScalar(scalarResult, array->fOperand.fScalar);
                array++;
            }
            break;
        case SK_FUNCTION(pow):
            // not the greatest -- but use x^y = e^(y * ln(x))
            scalarResult = SkScalarLog(input);
            scalarResult = SkScalarMul(parameters[1].fOperand.fScalar, scalarResult);
            scalarResult = SkScalarExp(scalarResult);
            break;
        case SK_FUNCTION(random):
            scalarResult = fRandom.nextUScalar1();
            break;
        case SK_FUNCTION(round):
            scalarResult = SkIntToScalar(SkScalarRound(input)); 
            break;
        case SK_FUNCTION(sin):
            scalarResult = SkScalarSin(input);
            break;
        case SK_FUNCTION(sqrt): {
            SkASSERT(parameters.count() == 1);
            SkASSERT(type == SkType_Float);
            scalarResult = SkScalarSqrt(input); 
            } break;
        case SK_FUNCTION(tan):
            scalarResult = SkScalarTan(input);
            break;
        default:
            SkASSERT(0);
            scalarResult = SK_ScalarNaN;
    }
    scriptValue->fOperand.fScalar = scalarResult;
    scriptValue->fType = SkType_Float;
}

const SkFunctionParamType* SkDisplayMath::getFunctionsParameters() {
    return fFunctionParameters;
}

bool SkDisplayMath::getProperty(int index, SkScriptValue* value) const {
    if ((unsigned)index < SK_ARRAY_COUNT(gConstants)) {
        value->fOperand.fScalar = gConstants[index];
        value->fType = SkType_Float;
        return true;
    }
    SkASSERT(0);
    return false;
}
