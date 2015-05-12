
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkScriptRuntime.h"
#include "SkScript2.h"
#include "SkMath.h"
#include "SkParse.h"
#include "SkScriptCallBack.h"
#include "SkString.h"
#include "SkOpArray.h"

// script tokenizer

// turn text into token string
// turn number literals into inline UTF8-style values
// process operators to turn standard notation into stack notation

// defer processing until the tokens can all be resolved
// then, turn token strings into indices into the appropriate tables / dictionaries

// consider: const evaluation?

// replace script string with script tokens preceeded by special value

// need second version of script plugins that return private index of found value?
    // then would need in script index of plugin, private index

// encode brace stack push/pop as opcodes

// should token script enocde type where possible?

// current flow:
    // strip whitespace
    // if in array brace [ recurse, continue
    // if token, handle function, or array, or property (continue)
    // parse number, continue
    // parse token, continue
    // parse string literal, continue
    // if dot operator, handle dot, continue
    // if [ , handle array literal or accessor, continue
    // if ), pop (if function, break)
    // if ], pop ; if ',' break
    // handle logical ops
    // or, handle arithmetic ops
    // loop

// !!! things to do
    // add separate processing loop to advance while suppressed
    // or, include jump offset to skip suppressed code?

SkScriptRuntime::~SkScriptRuntime() {
    for (SkString** stringPtr = fTrackString.begin(); stringPtr < fTrackString.end(); stringPtr++)
        delete *stringPtr;
    for (SkOpArray** arrayPtr = fTrackArray.begin(); arrayPtr < fTrackArray.end(); arrayPtr++)
        delete *arrayPtr;
}

bool SkScriptRuntime::executeTokens(unsigned char* opCode) {
    SkOperand2 operand[2];    // 1=accumulator and 2=operand
    SkScriptEngine2::TypeOp op;
    size_t ref;
    int index, size;
    int registerLoad;
    SkScriptCallBack* callBack SK_INIT_TO_AVOID_WARNING;
    do {
    switch ((op = (SkScriptEngine2::TypeOp) *opCode++)) {
        case SkScriptEngine2::kArrayToken:    // create an array
            operand[0].fArray = new SkOpArray(SkOperand2::kNoType /*fReturnType*/);
            break;
        case SkScriptEngine2::kArrayIndex:    // array accessor
            index = operand[1].fS32;
            if (index >= operand[0].fArray->count()) {
                fError = kArrayIndexOutOfBounds;
                return false;
            }
            operand[0] = operand[0].fArray->begin()[index];
            break;
        case SkScriptEngine2::kArrayParam:    // array initializer, or function param
            *operand[0].fArray->append() = operand[1];
            break;
        case SkScriptEngine2::kCallback:
            memcpy(&index, opCode, sizeof(index));
            opCode += sizeof(index);
            callBack = fCallBackArray[index];
            break;
        case SkScriptEngine2::kFunctionCall: {
            memcpy(&ref, opCode, sizeof(ref));
            opCode += sizeof(ref);
            SkScriptCallBackFunction* callBackFunction = (SkScriptCallBackFunction*) callBack;
            if (callBackFunction->invoke(ref, operand[0].fArray, /* params */
                    &operand[0] /* result */) == false) {
                fError = kFunctionCallFailed;
                return false;
            }
            } break;
        case SkScriptEngine2::kMemberOp: {
            memcpy(&ref, opCode, sizeof(ref));
            opCode += sizeof(ref);
            SkScriptCallBackMember* callBackMember = (SkScriptCallBackMember*) callBack;
            if (callBackMember->invoke(ref, operand[0].fObject, &operand[0]) == false) {
                fError = kMemberOpFailed;
                return false;
            }
            } break;
        case SkScriptEngine2::kPropertyOp: {
            memcpy(&ref, opCode, sizeof(ref));
            opCode += sizeof(ref);
            SkScriptCallBackProperty* callBackProperty = (SkScriptCallBackProperty*) callBack;
            if (callBackProperty->getResult(ref, &operand[0])== false) {
                fError = kPropertyOpFailed;
                return false;
            }
            } break;
        case SkScriptEngine2::kAccumulatorPop:
            fRunStack.pop(&operand[0]);
            break;
        case SkScriptEngine2::kAccumulatorPush:
            *fRunStack.push() = operand[0];
            break;
        case SkScriptEngine2::kIntegerAccumulator:
        case SkScriptEngine2::kIntegerOperand:
            registerLoad = op - SkScriptEngine2::kIntegerAccumulator;
            memcpy(&operand[registerLoad].fS32, opCode, sizeof(int32_t));
            opCode += sizeof(int32_t);
            break;
        case SkScriptEngine2::kScalarAccumulator:
        case SkScriptEngine2::kScalarOperand:
            registerLoad = op - SkScriptEngine2::kScalarAccumulator;
            memcpy(&operand[registerLoad].fScalar, opCode, sizeof(SkScalar));
            opCode += sizeof(SkScalar);
            break;
        case SkScriptEngine2::kStringAccumulator:
        case SkScriptEngine2::kStringOperand: {
            SkString* strPtr = new SkString();
            track(strPtr);
            registerLoad = op - SkScriptEngine2::kStringAccumulator;
            memcpy(&size, opCode, sizeof(size));
            opCode += sizeof(size);
            strPtr->set((char*) opCode, size);
            opCode += size;
            operand[registerLoad].fString = strPtr;
            } break;
        case SkScriptEngine2::kStringTrack: // call after kObjectToValue
            track(operand[0].fString);
            break;
        case SkScriptEngine2::kBoxToken: {
            SkOperand2::OpType type;
            memcpy(&type, opCode, sizeof(type));
            opCode += sizeof(type);
            SkScriptCallBackConvert* callBackBox = (SkScriptCallBackConvert*) callBack;
            if (callBackBox->convert(type, &operand[0]) == false)
                return false;
            } break;
        case SkScriptEngine2::kUnboxToken:
        case SkScriptEngine2::kUnboxToken2: {
            SkScriptCallBackConvert* callBackUnbox = (SkScriptCallBackConvert*) callBack;
            if (callBackUnbox->convert(SkOperand2::kObject, &operand[0]) == false)
                return false;
            } break;
        case SkScriptEngine2::kIfOp:
        case SkScriptEngine2::kLogicalAndInt:
            memcpy(&size, opCode, sizeof(size));
            opCode += sizeof(size);
            if (operand[0].fS32 == 0)
                opCode += size; // skip to else (or end of if predicate)
            break;
        case SkScriptEngine2::kElseOp:
            memcpy(&size, opCode, sizeof(size));
            opCode += sizeof(size);
            opCode += size; // if true: after predicate, always skip to end of else
            break;
        case SkScriptEngine2::kLogicalOrInt:
            memcpy(&size, opCode, sizeof(size));
            opCode += sizeof(size);
            if (operand[0].fS32 != 0)
                opCode += size; // skip to kToBool opcode after || predicate
            break;
        // arithmetic conversion ops
        case SkScriptEngine2::kFlipOpsOp:
            SkTSwap(operand[0], operand[1]);
            break;
        case SkScriptEngine2::kIntToString:
        case SkScriptEngine2::kIntToString2:
        case SkScriptEngine2::kScalarToString:
        case SkScriptEngine2::kScalarToString2:{
            SkString* strPtr = new SkString();
            track(strPtr);
            if (op == SkScriptEngine2::kIntToString || op == SkScriptEngine2::kIntToString2)
                strPtr->appendS32(operand[op - SkScriptEngine2::kIntToString].fS32);
            else
                strPtr->appendScalar(operand[op - SkScriptEngine2::kScalarToString].fScalar);
            operand[0].fString = strPtr;
            } break;
        case SkScriptEngine2::kIntToScalar:
        case SkScriptEngine2::kIntToScalar2:
            operand[0].fScalar = SkScriptEngine2::IntToScalar(operand[op - SkScriptEngine2::kIntToScalar].fS32);
            break;
        case SkScriptEngine2::kStringToInt:
            if (SkParse::FindS32(operand[0].fString->c_str(), &operand[0].fS32) == NULL)
                return false;
            break;
        case SkScriptEngine2::kStringToScalar:
        case SkScriptEngine2::kStringToScalar2:
            if (SkParse::FindScalar(operand[0].fString->c_str(),
                    &operand[op - SkScriptEngine2::kStringToScalar].fScalar) == NULL)
                return false;
            break;
        case SkScriptEngine2::kScalarToInt:
            operand[0].fS32 = SkScalarFloorToInt(operand[0].fScalar);
            break;
        // arithmetic ops
        case SkScriptEngine2::kAddInt:
            operand[0].fS32 += operand[1].fS32;
            break;
        case SkScriptEngine2::kAddScalar:
            operand[0].fScalar += operand[1].fScalar;
            break;
        case SkScriptEngine2::kAddString:
//            if (fTrackString.find(operand[1].fString) < 0) {
//                operand[1].fString = SkNEW_ARGS(SkString, (*operand[1].fString));
//                track(operand[1].fString);
//            }
            operand[0].fString->append(*operand[1].fString);
            break;
        case SkScriptEngine2::kBitAndInt:
            operand[0].fS32 &= operand[1].fS32;
            break;
        case SkScriptEngine2::kBitNotInt:
            operand[0].fS32 = ~operand[0].fS32;
            break;
        case SkScriptEngine2::kBitOrInt:
            operand[0].fS32 |= operand[1].fS32;
            break;
        case SkScriptEngine2::kDivideInt:
            SkASSERT(operand[1].fS32 != 0);
            if (operand[1].fS32 == 0)
                operand[0].fS32 = operand[0].fS32 == 0 ? SK_NaN32 :
                    operand[0].fS32 > 0 ? SK_MaxS32 : -SK_MaxS32;
            else
            if (operand[1].fS32 != 0) // throw error on divide by zero?
                operand[0].fS32 /= operand[1].fS32;
            break;
        case SkScriptEngine2::kDivideScalar:
            if (operand[1].fScalar == 0)
                operand[0].fScalar = operand[0].fScalar == 0 ? SK_ScalarNaN :
                    operand[0].fScalar > 0 ? SK_ScalarMax : -SK_ScalarMax;
            else
                operand[0].fScalar = SkScalarDiv(operand[0].fScalar, operand[1].fScalar);
            break;
        case SkScriptEngine2::kEqualInt:
            operand[0].fS32 = operand[0].fS32 == operand[1].fS32;
            break;
        case SkScriptEngine2::kEqualScalar:
            operand[0].fS32 = operand[0].fScalar == operand[1].fScalar;
            break;
        case SkScriptEngine2::kEqualString:
            operand[0].fS32 = *operand[0].fString == *operand[1].fString;
            break;
        case SkScriptEngine2::kGreaterEqualInt:
            operand[0].fS32 = operand[0].fS32 >= operand[1].fS32;
            break;
        case SkScriptEngine2::kGreaterEqualScalar:
            operand[0].fS32 = operand[0].fScalar >= operand[1].fScalar;
            break;
        case SkScriptEngine2::kGreaterEqualString:
            operand[0].fS32 = strcmp(operand[0].fString->c_str(), operand[1].fString->c_str()) >= 0;
            break;
        case SkScriptEngine2::kToBool:
            operand[0].fS32 = !! operand[0].fS32;
            break;
        case SkScriptEngine2::kLogicalNotInt:
            operand[0].fS32 = ! operand[0].fS32;
            break;
        case SkScriptEngine2::kMinusInt:
            operand[0].fS32 = -operand[0].fS32;
            break;
        case SkScriptEngine2::kMinusScalar:
            operand[0].fScalar = -operand[0].fScalar;
            break;
        case SkScriptEngine2::kModuloInt:
            operand[0].fS32 %= operand[1].fS32;
            break;
        case SkScriptEngine2::kModuloScalar:
            operand[0].fScalar = SkScalarMod(operand[0].fScalar, operand[1].fScalar);
            break;
        case SkScriptEngine2::kMultiplyInt:
            operand[0].fS32 *= operand[1].fS32;
            break;
        case SkScriptEngine2::kMultiplyScalar:
            operand[0].fScalar = SkScalarMul(operand[0].fScalar, operand[1].fScalar);
            break;
        case SkScriptEngine2::kShiftLeftInt:
            operand[0].fS32 <<= operand[1].fS32;
            break;
        case SkScriptEngine2::kShiftRightInt:
            operand[0].fS32 >>= operand[1].fS32;
            break;
        case SkScriptEngine2::kSubtractInt:
            operand[0].fS32 -= operand[1].fS32;
            break;
        case SkScriptEngine2::kSubtractScalar:
            operand[0].fScalar -= operand[1].fScalar;
            break;
        case SkScriptEngine2::kXorInt:
            operand[0].fS32 ^= operand[1].fS32;
            break;
        case SkScriptEngine2::kEnd:
            goto done;
        case SkScriptEngine2::kNop:
                SkASSERT(0);
    default:
        break;
    }
    } while (true);
done:
    fRunStack.push(operand[0]);
    return true;
}

bool SkScriptRuntime::getResult(SkOperand2* result) {
    if (fRunStack.count() == 0)
        return false;
    fRunStack.pop(result);
    return true;
}

void SkScriptRuntime::track(SkOpArray* array) {
    SkASSERT(fTrackArray.find(array) < 0);
    *fTrackArray.append() = array;
}

void SkScriptRuntime::track(SkString* string) {
    SkASSERT(fTrackString.find(string) < 0);
    *fTrackString.append() = string;
}

void SkScriptRuntime::untrack(SkOpArray* array) {
    int index = fTrackArray.find(array);
    SkASSERT(index >= 0);
    fTrackArray.begin()[index] = NULL;
}

void SkScriptRuntime::untrack(SkString* string) {
    int index = fTrackString.find(string);
    SkASSERT(index >= 0);
    fTrackString.begin()[index] = NULL;
}
