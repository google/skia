
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkScript.h"
#include "SkMath.h"
#include "SkParse.h"
#include "SkString.h"
#include "SkTypedArray.h"

/* things to do
    ? re-enable support for struct literals (e.g., for initializing points or rects)
        {x:1, y:2}
    ? use standard XML / script notation like document.getElementById("canvas");  
    finish support for typed arrays
        ? allow indexing arrays by string
            this could map to the 'name' attribute of a given child of an array
        ? allow multiple types in the array
    remove SkDisplayType.h  // from SkOperand.h
    merge type and operand arrays into scriptvalue array
*/

#ifdef SK_DEBUG
static const char* errorStrings[] = {
        "array index of out bounds", // kArrayIndexOutOfBounds
        "could not find reference id", // kCouldNotFindReferencedID
        "dot operator expects object", // kDotOperatorExpectsObject
        "error in array index", // kErrorInArrrayIndex
        "error in function parameters", // kErrorInFunctionParameters
        "expected array", // kExpectedArray
        "expected boolean expression", // kExpectedBooleanExpression
        "expected field name", // kExpectedFieldName
        "expected hex", // kExpectedHex
        "expected int for condition operator", // kExpectedIntForConditionOperator
        "expected number", // kExpectedNumber
        "expected number for array index", // kExpectedNumberForArrayIndex
        "expected operator", // kExpectedOperator
        "expected token", // kExpectedToken
        "expected token before dot operator", // kExpectedTokenBeforeDotOperator
        "expected value", // kExpectedValue
        "handle member failed", // kHandleMemberFailed
        "handle member function failed", // kHandleMemberFunctionFailed
        "handle unbox failed", // kHandleUnboxFailed
        "index out of range", // kIndexOutOfRange
        "mismatched array brace", // kMismatchedArrayBrace
        "mismatched brackets", // kMismatchedBrackets
        "no function handler found", // kNoFunctionHandlerFound
        "premature end", // kPrematureEnd
        "too many parameters", // kTooManyParameters
        "type conversion failed", // kTypeConversionFailed
        "unterminated string" // kUnterminatedString
};
#endif

const SkScriptEngine::SkOperatorAttributes SkScriptEngine::gOpAttributes[] = {
    { kNoType, kNoType, kNoBias }, //   kUnassigned,
    { SkOpType(kInt | kScalar | kString), SkOpType(kInt | kScalar | kString), kTowardsString }, // kAdd
    // kAddInt = kAdd,
    { kNoType, kNoType, kNoBias },  // kAddScalar,
    { kNoType, kNoType, kNoBias },  // kAddString,
    { kNoType, kNoType, kNoBias },  // kArrayOp,
    { kInt, kInt, kNoBias }, // kBitAnd
    { kNoType, kInt, kNoBias }, // kBitNot
    { kInt, kInt, kNoBias }, // kBitOr
    { SkOpType(kInt | kScalar), SkOpType(kInt | kScalar), kNoBias }, // kDivide
    // kDivideInt = kDivide
    { kNoType, kNoType, kNoBias },  // kDivideScalar
    { kNoType, kNoType, kNoBias },  // kElse
    { SkOpType(kInt | kScalar | kString), SkOpType(kInt | kScalar | kString), kTowardsNumber }, // kEqual
    // kEqualInt = kEqual
    { kNoType, kNoType, kNoBias },  // kEqualScalar
    { kNoType, kNoType, kNoBias },  // kEqualString
    { kInt, kNoType, kNoBias },     // kFlipOps
    { SkOpType(kInt | kScalar | kString), SkOpType(kInt | kScalar | kString), kTowardsNumber }, // kGreaterEqual
    // kGreaterEqualInt = kGreaterEqual
    { kNoType, kNoType, kNoBias },  // kGreaterEqualScalar
    { kNoType, kNoType, kNoBias },  // kGreaterEqualString
    { kNoType, kNoType, kNoBias },  // kIf
    { kNoType, kInt, kNoBias }, // kLogicalAnd  (really, ToBool)
    { kNoType, kInt, kNoBias }, // kLogicalNot
    { kInt, kInt, kNoBias }, // kLogicalOr
    { kNoType, SkOpType(kInt | kScalar), kNoBias }, // kMinus
    // kMinusInt = kMinus
    { kNoType, kNoType, kNoBias },  // kMinusScalar
    { SkOpType(kInt | kScalar), SkOpType(kInt | kScalar), kNoBias }, // kModulo
    // kModuloInt = kModulo
    { kNoType, kNoType, kNoBias },  // kModuloScalar
    { SkOpType(kInt | kScalar), SkOpType(kInt | kScalar), kNoBias }, // kMultiply
    // kMultiplyInt = kMultiply
    { kNoType, kNoType, kNoBias },  // kMultiplyScalar
    { kNoType, kNoType, kNoBias },  // kParen
    { kInt, kInt, kNoBias }, // kShiftLeft
    { kInt, kInt, kNoBias }, // kShiftRight
    { SkOpType(kInt | kScalar), SkOpType(kInt | kScalar), kNoBias }, // kSubtract
    // kSubtractInt = kSubtract
    { kNoType, kNoType, kNoBias },  // kSubtractScalar
    { kInt, kInt, kNoBias } // kXor
};

// Note that the real precedence for () [] is '2'
// but here, precedence means 'while an equal or smaller precedence than the current operator
// is on the stack, process it. This allows 3+5*2 to defer the add until after the multiply
// is preformed, since the add precedence is not smaller than multiply.
// But, (3*4 does not process the '(', since brackets are greater than all other precedences
#define kBracketPrecedence 16
#define kIfElsePrecedence 15

const signed char SkScriptEngine::gPrecedence[] = {
        -1, //  kUnassigned,
        6, // kAdd,
        // kAddInt = kAdd,
        6, // kAddScalar,
        6, // kAddString,   // string concat
        kBracketPrecedence, // kArrayOp,
        10, // kBitAnd,
        4, // kBitNot,
        12, // kBitOr,
        5, // kDivide,
        // kDivideInt = kDivide,
        5, // kDivideScalar,
        kIfElsePrecedence, // kElse,
        9, // kEqual,
        // kEqualInt = kEqual,
        9, // kEqualScalar,
        9, // kEqualString,
        -1, // kFlipOps,
        8, // kGreaterEqual,
        // kGreaterEqualInt = kGreaterEqual,
        8, // kGreaterEqualScalar,
        8, // kGreaterEqualString,
        kIfElsePrecedence, // kIf,
        13, // kLogicalAnd,
        4, // kLogicalNot,
        14, // kLogicalOr,
        4, // kMinus,
        // kMinusInt = kMinus,
        4, // kMinusScalar,
        5, // kModulo,
        // kModuloInt = kModulo,
        5, // kModuloScalar,
        5, // kMultiply,
        // kMultiplyInt = kMultiply,
        5, // kMultiplyScalar,
        kBracketPrecedence, // kParen,
        7, // kShiftLeft,
        7, // kShiftRight,  // signed
        6, // kSubtract,
        // kSubtractInt = kSubtract,
        6, // kSubtractScalar,
        11, // kXor
};

static inline bool is_between(int c, int min, int max)
{
    return (unsigned)(c - min) <= (unsigned)(max - min);
}

static inline bool is_ws(int c)
{
    return is_between(c, 1, 32);
}

static int token_length(const char* start) {
    char ch = start[0];
    if (! is_between(ch, 'a' , 'z') &&  ! is_between(ch, 'A', 'Z') && ch != '_' && ch != '$')
        return -1;
    int length = 0;
    do
        ch = start[++length];
    while (is_between(ch, 'a' , 'z') || is_between(ch, 'A', 'Z') || is_between(ch, '0', '9') ||
        ch == '_' || ch == '$');
    return length;
}

SkScriptEngine::SkScriptEngine(SkOpType returnType) :
    fTokenLength(0), fReturnType(returnType), fError(kNoError)
{
    SkSuppress noInitialSuppress;
    noInitialSuppress.fOperator = kUnassigned;
    noInitialSuppress.fOpStackDepth = 0;
    noInitialSuppress.fSuppress = false;
    noInitialSuppress.fElse = 0;
    fSuppressStack.push(noInitialSuppress);
    *fOpStack.push() = kParen;
    fTrackArray.appendClear();
    fTrackString.appendClear();
}

SkScriptEngine::~SkScriptEngine() {
    for (SkString** stringPtr = fTrackString.begin(); stringPtr < fTrackString.end(); stringPtr++)
        delete *stringPtr;
    for (SkTypedArray** arrayPtr = fTrackArray.begin(); arrayPtr < fTrackArray.end(); arrayPtr++)
        delete *arrayPtr;
}

int SkScriptEngine::arithmeticOp(char ch, char nextChar, bool lastPush) {
    SkOp op = kUnassigned;
    bool reverseOperands = false;
    bool negateResult = false;
    int advance = 1;
    switch (ch) {
        case '+':
            // !!! ignoring unary plus as implemented here has the side effect of
            // suppressing errors like +"hi"
            if (lastPush == false)  // unary plus, don't push an operator
                goto returnAdv;
            op = kAdd;
            break;
        case '-':
            op = lastPush ? kSubtract : kMinus;
            break;
        case '*':
            op = kMultiply;
            break;
        case '/':
            op = kDivide;
            break;
        case '>':
            if (nextChar == '>') {
                op = kShiftRight;
                goto twoChar;
            } 
            op = kGreaterEqual;
            if (nextChar == '=')
                goto twoChar;
            reverseOperands = negateResult = true;
            break;
        case '<':
            if (nextChar == '<') {
                op = kShiftLeft;
                goto twoChar;
            }
            op = kGreaterEqual;
            reverseOperands = nextChar == '=';
            negateResult = ! reverseOperands;
            advance += reverseOperands;
            break;
        case '=':
            if (nextChar == '=') {
                op = kEqual;
                goto twoChar;
            }
            break;
        case '!':
            if (nextChar == '=') {
                op = kEqual;
                negateResult = true;
twoChar:
                advance++;
                break;
            } 
            op = kLogicalNot;
            break;
        case '?':
            op = kIf;
            break;
        case ':':
            op = kElse;
            break;
        case '^':
            op = kXor;
            break;
        case '(':
            *fOpStack.push() = kParen;  // push even if eval is suppressed
            goto returnAdv;
        case '&':
            SkASSERT(nextChar != '&');
            op = kBitAnd;
            break;
        case '|':
            SkASSERT(nextChar != '|');
            op = kBitOr;
            break;
        case '%':
            op = kModulo;
            break;
        case '~':
            op = kBitNot;
            break;
    }
    if (op == kUnassigned)
        return 0;
    if (fSuppressStack.top().fSuppress == false) {
        signed char precedence = gPrecedence[op];
        do {
            int idx = 0;
            SkOp compare;
            do {
                compare = fOpStack.index(idx);
                if ((compare & kArtificialOp) == 0)
                    break;
                idx++;
            } while (true);
            signed char topPrecedence = gPrecedence[compare];
            SkASSERT(topPrecedence != -1);
            if (topPrecedence > precedence || (topPrecedence == precedence && 
                    gOpAttributes[op].fLeftType == kNoType)) {
                break;
            }
            if (processOp() == false)
                return 0;   // error
        } while (true);
        if (negateResult)
            *fOpStack.push() = (SkOp) (kLogicalNot | kArtificialOp);
        fOpStack.push(op);
        if (reverseOperands)
            *fOpStack.push() = (SkOp) (kFlipOps | kArtificialOp);
    }
returnAdv:
    return advance;
}

void SkScriptEngine::boxCallBack(_boxCallBack func, void* userStorage) {
    UserCallBack callBack;
    callBack.fBoxCallBack = func;
    commonCallBack(kBox, callBack, userStorage);
}

void SkScriptEngine::commonCallBack(CallBackType type, UserCallBack& callBack, void* userStorage) {
    callBack.fCallBackType = type;
    callBack.fUserStorage = userStorage;
    *fUserCallBacks.prepend() = callBack;
}

bool SkScriptEngine::convertParams(SkTDArray<SkScriptValue>& params, 
        const SkFunctionParamType* paramTypes, int paramCount) {
    if (params.count() > paramCount) {
        fError = kTooManyParameters;
        return false;   // too many parameters passed
    }
    for (int index = 0; index < params.count(); index++) {
        if (convertTo((SkDisplayTypes) paramTypes[index], &params[index]) == false)
            return false;
    }
    return true;
}

bool SkScriptEngine::convertTo(SkDisplayTypes toType, SkScriptValue* value ) {
    SkDisplayTypes type = value->fType;
    if (type == toType)
        return true;
    if (ToOpType(type) == kObject) {
#if 0   // !!! I want object->string to get string from displaystringtype, not id
        if (ToOpType(toType) == kString) {
            bool success = handleObjectToString(value->fOperand.fObject);
            if (success == false)
                return false;
            SkOpType type;
            fTypeStack.pop(&type);
            value->fType = ToDisplayType(type);
            fOperandStack.pop(&value->fOperand);
            return true;
        }
#endif
        if (handleUnbox(value) == false) {
            fError = kHandleUnboxFailed;
            return false;
        }
        return convertTo(toType, value);
    }
    return ConvertTo(this, toType, value);
}

bool SkScriptEngine::evaluateDot(const char*& script, bool suppressed) { 
    size_t fieldLength = token_length(++script);        // skip dot
    if (fieldLength == 0) {
        fError = kExpectedFieldName;
        return false;
    }
    const char* field = script;
    script += fieldLength;
    bool success = handleProperty(suppressed);
    if (success == false) {
        fError = kCouldNotFindReferencedID; // note: never generated by standard animator plugins
        return false;
    }
    return evaluateDotParam(script, suppressed, field, fieldLength);
}

bool SkScriptEngine::evaluateDotParam(const char*& script, bool suppressed, 
        const char* field, size_t fieldLength) { 
    void* object;
    if (suppressed)
        object = NULL;
    else {
        if (fTypeStack.top() != kObject) {
            fError = kDotOperatorExpectsObject;
            return false;
        }
        object = fOperandStack.top().fObject;
        fTypeStack.pop();
        fOperandStack.pop();
    }
    char ch; // see if it is a simple member or a function
    while (is_ws(ch = script[0])) 
        script++;
    bool success = true;
    if (ch != '(') {
            if (suppressed == false) {
                if ((success = handleMember(field, fieldLength, object)) == false)
                    fError = kHandleMemberFailed;
            }
    } else {
        SkTDArray<SkScriptValue> params;
        *fBraceStack.push() = kFunctionBrace;
        success = functionParams(&script, params);
        if (success && suppressed == false &&
                (success = handleMemberFunction(field, fieldLength, object, params)) == false) 
            fError = kHandleMemberFunctionFailed;       
    }
    return success; 
}

bool SkScriptEngine::evaluateScript(const char** scriptPtr, SkScriptValue* value) {
#ifdef SK_DEBUG
    const char** original = scriptPtr;
#endif
    bool success;
    const char* inner;
    if (strncmp(*scriptPtr, "#script:", sizeof("#script:") - 1) == 0) {
        *scriptPtr += sizeof("#script:") - 1;
        if (fReturnType == kNoType || fReturnType == kString) {
            success = innerScript(scriptPtr, value);
            if (success == false)
                goto end;
            inner = value->fOperand.fString->c_str();
            scriptPtr = &inner;
        }
    }
    {
        success = innerScript(scriptPtr, value);
        if (success == false)
            goto end;
        const char* script = *scriptPtr;
        char ch;
        while (is_ws(ch = script[0]))
            script++;
        if (ch != '\0') {
            // error may trigger on scripts like "50,0" that were intended to be written as "[50, 0]"
            fError = kPrematureEnd;
            success = false;
        }
    }
end:
#ifdef SK_DEBUG
    if (success == false) {
        SkDebugf("script failed: %s", *original);
        if (fError)
            SkDebugf(" %s", errorStrings[fError - 1]);
        SkDebugf("\n");
    }
#endif
    return success;
}

void SkScriptEngine::forget(SkTypedArray* array) {
    if (array->getType() == SkType_String) {
        for (int index = 0; index < array->count(); index++) {
            SkString* string = (*array)[index].fString;
            int found = fTrackString.find(string);
            if (found >= 0)
                fTrackString.remove(found);
        }
        return;
    }
    if (array->getType() == SkType_Array) {
        for (int index = 0; index < array->count(); index++) {
            SkTypedArray* child = (*array)[index].fArray;
            forget(child);  // forgets children of child
            int found = fTrackArray.find(child);
            if (found >= 0)
                fTrackArray.remove(found);
        }
    }
}

void SkScriptEngine::functionCallBack(_functionCallBack func, void* userStorage) {
    UserCallBack callBack;
    callBack.fFunctionCallBack = func;
    commonCallBack(kFunction, callBack, userStorage);
}

bool SkScriptEngine::functionParams(const char** scriptPtr, SkTDArray<SkScriptValue>& params) {
    (*scriptPtr)++; // skip open paren
    *fOpStack.push() = kParen;
    *fBraceStack.push() = kFunctionBrace;
    SkBool suppressed = fSuppressStack.top().fSuppress;
    do {
        SkScriptValue value;
        bool success = innerScript(scriptPtr, suppressed ? NULL : &value);
        if (success == false) {
            fError = kErrorInFunctionParameters;
            return false;
        }
        if (suppressed)
            continue;
        *params.append() = value;
    } while ((*scriptPtr)[-1] == ',');
    fBraceStack.pop();
    fOpStack.pop(); // pop paren
    (*scriptPtr)++; // advance beyond close paren
    return true;
}

#ifdef SK_DEBUG
bool SkScriptEngine::getErrorString(SkString* str) const {
    if (fError)
        str->set(errorStrings[fError - 1]);
    return fError != 0;
}
#endif

bool SkScriptEngine::innerScript(const char** scriptPtr, SkScriptValue* value) {
    const char* script = *scriptPtr;
    char ch;
    bool lastPush = false;
    bool success = true;
    int opBalance = fOpStack.count();
    int baseBrace = fBraceStack.count();
    int suppressBalance = fSuppressStack.count();
    while ((ch = script[0]) != '\0') {
        if (is_ws(ch)) {
            script++;
            continue;
        }
        SkBool suppressed = fSuppressStack.top().fSuppress;
        SkOperand operand;
        const char* dotCheck;
        if (fBraceStack.count() > baseBrace) {
#if 0   // disable support for struct brace
            if (ch == ':') {
                SkASSERT(fTokenLength > 0);
                SkASSERT(fBraceStack.top() == kStructBrace);
                ++script;
                SkASSERT(fDisplayable);
                SkString token(fToken, fTokenLength);
                fTokenLength = 0;
                const char* tokenName = token.c_str();
                const SkMemberInfo* tokenInfo SK_INIT_TO_AVOID_WARNING;
                if (suppressed == false) {
                    SkDisplayTypes type = fInfo->getType();
                    tokenInfo = SkDisplayType::GetMember(type, &tokenName);
                    SkASSERT(tokenInfo);
                }
                SkScriptValue tokenValue;
                success = innerScript(&script, &tokenValue);    // terminate and return on comma, close brace
                SkASSERT(success);
                if (suppressed == false) {
                    if (tokenValue.fType == SkType_Displayable) {
                        SkASSERT(SkDisplayType::IsDisplayable(tokenInfo->getType()));
                        fDisplayable->setReference(tokenInfo, tokenValue.fOperand.fDisplayable);
                    } else {
                        if (tokenValue.fType != tokenInfo->getType()) {
                            if (convertTo(tokenInfo->getType(), &tokenValue) == false)
                                return false;
                        }
                        tokenInfo->writeValue(fDisplayable, NULL, 0, 0, 
                            (void*) ((char*) fInfo->memberData(fDisplayable) + tokenInfo->fOffset + fArrayOffset),
                            tokenInfo->getType(), tokenValue);
                    }
                }
                lastPush = false;
                continue;
            } else 
#endif              
            if (fBraceStack.top() == kArrayBrace) {
                SkScriptValue tokenValue;
                success = innerScript(&script, &tokenValue);    // terminate and return on comma, close brace
                if (success == false) {
                    fError = kErrorInArrrayIndex;
                    return false;
                }
                if (suppressed == false) {
#if 0 // no support for structures for now
                    if (tokenValue.fType == SkType_Structure) {
                        fArrayOffset += (int) fInfo->getSize(fDisplayable);
                    } else 
#endif
                    {
                        SkDisplayTypes type = ToDisplayType(fReturnType);
                        if (fReturnType == kNoType) {
                            // !!! short sighted; in the future, allow each returned array component to carry 
                            // its own type, and let caller do any needed conversions
                            if (value->fOperand.fArray->count() == 0)
                                value->fOperand.fArray->setType(type = tokenValue.fType);
                            else
                                type = value->fOperand.fArray->getType();
                        }
                        if (tokenValue.fType != type) {
                            if (convertTo(type, &tokenValue) == false)
                                return false;
                        }
                        *value->fOperand.fArray->append() = tokenValue.fOperand;
                    }
                }
                lastPush = false;
                continue;
            } else {
                if (token_length(script) == 0) {
                    fError = kExpectedToken;
                    return false;
                }
            }
        }
        if (lastPush != false && fTokenLength > 0) {
            if (ch == '(') {
                *fBraceStack.push() = kFunctionBrace;
                if (handleFunction(&script, SkToBool(suppressed)) == false)
                    return false;
                lastPush = true;
                continue;
            } else if (ch == '[') {
                if (handleProperty(SkToBool(suppressed)) == false)
                    return false;   // note: never triggered by standard animator plugins
                if (handleArrayIndexer(&script, SkToBool(suppressed)) == false)
                    return false;
                lastPush = true;
                continue;
            } else if (ch != '.') {
                if (handleProperty(SkToBool(suppressed)) == false)
                    return false;   // note: never triggered by standard animator plugins
                lastPush = true;
                continue;
            }
        }
        if (ch == '0' && (script[1] & ~0x20) == 'X') {
            if (lastPush != false) {
                fError = kExpectedOperator;
                return false;
            }
            script += 2;
            script = SkParse::FindHex(script, (uint32_t*)&operand.fS32);
            if (script == NULL) {
                fError = kExpectedHex;
                return false;
            }
            goto intCommon;
        }
        if (lastPush == false && ch == '.')
            goto scalarCommon;
        if (ch >= '0' && ch <= '9') {
            if (lastPush != false) {
                fError = kExpectedOperator;
                return false;
            }
            dotCheck = SkParse::FindS32(script, &operand.fS32);
            if (dotCheck[0] != '.') {
                script = dotCheck;
intCommon:
                if (suppressed == false)
                    *fTypeStack.push() = kInt;
            } else {
scalarCommon:
                script = SkParse::FindScalar(script, &operand.fScalar);
                if (suppressed == false)
                    *fTypeStack.push() = kScalar;
            }
            if (suppressed == false)
                fOperandStack.push(operand);
            lastPush = true;
            continue;
        }
        int length = token_length(script);
        if (length > 0) {
            if (lastPush != false) {
                fError = kExpectedOperator;
                return false;
            }
            fToken = script;
            fTokenLength = length;
            script += length;
            lastPush = true;
            continue;
        }
        char startQuote = ch;
        if (startQuote == '\'' || startQuote == '\"') {
            if (lastPush != false) {
                fError = kExpectedOperator;
                return false;
            }
            operand.fString = new SkString();
            track(operand.fString);
            ++script;

            // <mrr> this is a lot of calls to append() one char at at time
            // how hard to preflight script so we know how much to grow fString by?
            do {
                if (script[0] == '\\')
                    ++script;
                operand.fString->append(script, 1);
                ++script;
                if (script[0] == '\0') {
                    fError = kUnterminatedString;
                    return false;
                }
            } while (script[0] != startQuote);
            ++script;
            if (suppressed == false) {
                *fTypeStack.push() = kString;
                fOperandStack.push(operand);
            }
            lastPush = true;
            continue;
        }
        ;
        if (ch ==  '.') {
            if (fTokenLength == 0) {
                SkScriptValue scriptValue;
                SkDEBUGCODE(scriptValue.fOperand.fObject = NULL);
                int tokenLength = token_length(++script);
                const char* token = script;
                script += tokenLength;
                if (suppressed == false) {
                    if (fTypeStack.count() == 0) {
                        fError = kExpectedTokenBeforeDotOperator;
                        return false;
                    }
                    SkOpType topType;
                    fTypeStack.pop(&topType);
                    fOperandStack.pop(&scriptValue.fOperand);
                    scriptValue.fType = ToDisplayType(topType);
                    handleBox(&scriptValue);
                }
                success = evaluateDotParam(script, SkToBool(suppressed), token, tokenLength);
                if (success == false)
                    return false;
                lastPush = true;
                continue; 
            }
            // get next token, and evaluate immediately
            success = evaluateDot(script, SkToBool(suppressed));
            if (success == false)               
                return false;
            lastPush = true;
            continue;
        }
        if (ch == '[') {
            if (lastPush == false) {
                script++;
                *fBraceStack.push() = kArrayBrace;
                if (suppressed)
                    continue;
                operand.fArray = value->fOperand.fArray = new SkTypedArray(ToDisplayType(fReturnType));
                track(value->fOperand.fArray);
                *fTypeStack.push() = (SkOpType) kArray;
                fOperandStack.push(operand);
                continue;
            }
            if (handleArrayIndexer(&script, SkToBool(suppressed)) == false)
                return false;
            lastPush = true;
            continue;
        }
#if 0 // structs not supported for now
        if (ch == '{') {
            if (lastPush == false) {
                script++;
                *fBraceStack.push() = kStructBrace;
                if (suppressed)
                    continue;
                operand.fS32 = 0;
                *fTypeStack.push() = (SkOpType) kStruct;
                fOperandStack.push(operand);
                continue;
            }
            SkASSERT(0); // braces in other contexts aren't supported yet
        }
#endif
        if (ch == ')' && fBraceStack.count() > 0) {
            SkBraceStyle braceStyle = fBraceStack.top(); 
            if (braceStyle == kFunctionBrace) {
                fBraceStack.pop();
                break;
            }
        }
        if (ch == ',' || ch == ']') {
            if (ch != ',') {
                SkBraceStyle match;
                fBraceStack.pop(&match);
                if (match != kArrayBrace) {
                    fError = kMismatchedArrayBrace;
                    return false;
                }
            }
            script++;
            // !!! see if brace or bracket is correct closer
            break;
        }
        char nextChar = script[1];
        int advance = logicalOp(ch, nextChar);
        if (advance < 0)     // error
            return false;
        if (advance == 0) 
            advance = arithmeticOp(ch, nextChar, lastPush);
        if (advance == 0) // unknown token
            return false;
        if (advance > 0)
            script += advance;
        lastPush = ch == ']' || ch == ')';
    }
    bool suppressed = SkToBool(fSuppressStack.top().fSuppress);
    if (fTokenLength > 0) {
        success = handleProperty(suppressed);
        if (success == false)
            return false;   // note: never triggered by standard animator plugins
    }
    while (fOpStack.count() > opBalance) {   // leave open paren
        if ((fError = opError()) != kNoError)
            return false;
        if (processOp() == false)
            return false;
    }   
    SkOpType topType = fTypeStack.count() > 0 ? fTypeStack.top() : kNoType;
    if (suppressed == false && topType != fReturnType &&
            topType == kString && fReturnType != kNoType) { // if result is a string, give handle property a chance to convert it to the property value
        SkString* string = fOperandStack.top().fString;
        fToken = string->c_str();
        fTokenLength = string->size();
        fOperandStack.pop();
        fTypeStack.pop();
        success = handleProperty(SkToBool(fSuppressStack.top().fSuppress));
        if (success == false) { // if it couldn't convert, return string (error?)
            SkOperand operand;
            operand.fS32 = 0;
            *fTypeStack.push() = kString;
            operand.fString = string;
            fOperandStack.push(operand);
        }
    }
    if (value) {
        if (fOperandStack.count() == 0)
            return false;
        SkASSERT(fOperandStack.count() >= 1);
        SkASSERT(fTypeStack.count() >= 1);
        fOperandStack.pop(&value->fOperand);
        SkOpType type;
        fTypeStack.pop(&type);
        value->fType = ToDisplayType(type);
//      SkASSERT(value->fType != SkType_Unknown);
        if (topType != fReturnType && topType == kObject && fReturnType != kNoType) {
            if (convertTo(ToDisplayType(fReturnType), value) == false)
                return false;
        }
    }
    while (fSuppressStack.count() > suppressBalance)
        fSuppressStack.pop();
    *scriptPtr = script;
    return true; // no error
}

void SkScriptEngine::memberCallBack(_memberCallBack member , void* userStorage) {
    UserCallBack callBack;
    callBack.fMemberCallBack = member;
    commonCallBack(kMember, callBack, userStorage);
}

void SkScriptEngine::memberFunctionCallBack(_memberFunctionCallBack func, void* userStorage) {
    UserCallBack callBack;
    callBack.fMemberFunctionCallBack = func;
    commonCallBack(kMemberFunction, callBack, userStorage);
}

#if 0
void SkScriptEngine::objectToStringCallBack(_objectToStringCallBack func, void* userStorage) {
    UserCallBack callBack;
    callBack.fObjectToStringCallBack = func;
    commonCallBack(kObjectToString, callBack, userStorage);
}
#endif

bool SkScriptEngine::handleArrayIndexer(const char** scriptPtr, bool suppressed) {
    SkScriptValue scriptValue;
    (*scriptPtr)++;
    *fOpStack.push() = kParen;
    *fBraceStack.push() = kArrayBrace;
    SkOpType saveType = fReturnType;
    fReturnType = kInt;
    bool success = innerScript(scriptPtr, suppressed == false ? &scriptValue : NULL);
    if (success == false)
        return false;
    fReturnType = saveType;
    if (suppressed == false) {
        if (convertTo(SkType_Int, &scriptValue) == false)
            return false;
        int index = scriptValue.fOperand.fS32;
        SkScriptValue scriptValue;
        SkOpType type;
        fTypeStack.pop(&type);
        fOperandStack.pop(&scriptValue.fOperand);
        scriptValue.fType = ToDisplayType(type);
        if (type == kObject) {
            success = handleUnbox(&scriptValue);
            if (success == false)
                return false;
            if (ToOpType(scriptValue.fType) != kArray) {
                fError = kExpectedArray;
                return false;
            }
        }
        *fTypeStack.push() = scriptValue.fOperand.fArray->getOpType();
//      SkASSERT(index >= 0);
        if ((unsigned) index >= (unsigned) scriptValue.fOperand.fArray->count()) {
            fError = kArrayIndexOutOfBounds;
            return false;
        }
        scriptValue.fOperand = scriptValue.fOperand.fArray->begin()[index];
        fOperandStack.push(scriptValue.fOperand);
    }
    fOpStack.pop(); // pop paren
    return success;
}

bool SkScriptEngine::handleBox(SkScriptValue* scriptValue) {
    bool success = true;
    for (UserCallBack* callBack = fUserCallBacks.begin(); callBack < fUserCallBacks.end(); callBack++) {
        if (callBack->fCallBackType != kBox)
            continue;
        success = (*callBack->fBoxCallBack)(callBack->fUserStorage, scriptValue);
        if (success) {
            fOperandStack.push(scriptValue->fOperand);
            *fTypeStack.push() = ToOpType(scriptValue->fType);
            goto done;
        }
    }
done:
    return success;
}

bool SkScriptEngine::handleFunction(const char** scriptPtr, bool suppressed) {
    SkScriptValue callbackResult;
    SkTDArray<SkScriptValue> params;
    SkString functionName(fToken, fTokenLength);
    fTokenLength = 0;
    bool success = functionParams(scriptPtr, params);
    if (success == false)
        goto done;
    if (suppressed == true)
        return true;
    {
        for (UserCallBack* callBack = fUserCallBacks.begin(); callBack < fUserCallBacks.end(); callBack++) {
            if (callBack->fCallBackType != kFunction)
                continue;
            success = (*callBack->fFunctionCallBack)(functionName.c_str(), functionName.size(), params, 
                callBack->fUserStorage, &callbackResult);
            if (success) {
                fOperandStack.push(callbackResult.fOperand);
                *fTypeStack.push() = ToOpType(callbackResult.fType);
                goto done;
            }
        }
    }
    fError = kNoFunctionHandlerFound;
    return false;
done:
    return success;
}

bool SkScriptEngine::handleMember(const char* field, size_t len, void* object) {
    SkScriptValue callbackResult;
    bool success = true;
    for (UserCallBack* callBack = fUserCallBacks.begin(); callBack < fUserCallBacks.end(); callBack++) {
        if (callBack->fCallBackType != kMember)
            continue;
        success = (*callBack->fMemberCallBack)(field, len, object, callBack->fUserStorage, &callbackResult);
        if (success) {
            if (callbackResult.fType == SkType_String)
                track(callbackResult.fOperand.fString);
            fOperandStack.push(callbackResult.fOperand);
            *fTypeStack.push() = ToOpType(callbackResult.fType);
            goto done;
        }
    }
    return false;
done:
    return success;
}

bool SkScriptEngine::handleMemberFunction(const char* field, size_t len, void* object, SkTDArray<SkScriptValue>& params) {
    SkScriptValue callbackResult;
    bool success = true;
    for (UserCallBack* callBack = fUserCallBacks.begin(); callBack < fUserCallBacks.end(); callBack++) {
        if (callBack->fCallBackType != kMemberFunction)
            continue;
        success = (*callBack->fMemberFunctionCallBack)(field, len, object, params, 
            callBack->fUserStorage, &callbackResult);
        if (success) {
            if (callbackResult.fType == SkType_String)
                track(callbackResult.fOperand.fString);
            fOperandStack.push(callbackResult.fOperand);
            *fTypeStack.push() = ToOpType(callbackResult.fType);
            goto done;
        }
    }
    return false;
done:
    return success;
}

#if 0
bool SkScriptEngine::handleObjectToString(void* object) {
    SkScriptValue callbackResult;
    bool success = true;
    for (UserCallBack* callBack = fUserCallBacks.begin(); callBack < fUserCallBacks.end(); callBack++) {
        if (callBack->fCallBackType != kObjectToString)
            continue;
        success = (*callBack->fObjectToStringCallBack)(object, 
            callBack->fUserStorage, &callbackResult);
        if (success) {
            if (callbackResult.fType == SkType_String)
                track(callbackResult.fOperand.fString);
            fOperandStack.push(callbackResult.fOperand);
            *fTypeStack.push() = ToOpType(callbackResult.fType);
            goto done;
        }
    }
    return false;
done:
    return success;
}
#endif

bool SkScriptEngine::handleProperty(bool suppressed) {
    SkScriptValue callbackResult;
    bool success = true;
    if (suppressed) 
        goto done;
    success = false; // note that with standard animator-script plugins, callback never returns false
    {
        for (UserCallBack* callBack = fUserCallBacks.begin(); callBack < fUserCallBacks.end(); callBack++) {
            if (callBack->fCallBackType != kProperty)
                continue;
            success = (*callBack->fPropertyCallBack)(fToken, fTokenLength, 
                callBack->fUserStorage, &callbackResult);
            if (success) {
                if (callbackResult.fType == SkType_String && callbackResult.fOperand.fString == NULL) {
                    callbackResult.fOperand.fString = new SkString(fToken, fTokenLength);
                    track(callbackResult.fOperand.fString);
                }
                fOperandStack.push(callbackResult.fOperand);
                *fTypeStack.push() = ToOpType(callbackResult.fType);
                goto done;
            }
        }
    }
done:
    fTokenLength = 0;
    return success;
}

bool SkScriptEngine::handleUnbox(SkScriptValue* scriptValue) {
    bool success = true;
    for (UserCallBack* callBack = fUserCallBacks.begin(); callBack < fUserCallBacks.end(); callBack++) {
        if (callBack->fCallBackType != kUnbox)
            continue;
        success = (*callBack->fUnboxCallBack)(callBack->fUserStorage, scriptValue);
        if (success) {
            if (scriptValue->fType == SkType_String)
                track(scriptValue->fOperand.fString);
            goto done;
        }
    }
    return false;
done:
    return success;
}

// note that entire expression is treated as if it were enclosed in parens
// an open paren is always the first thing in the op stack

int SkScriptEngine::logicalOp(char ch, char nextChar) {
    int advance = 1;
    SkOp match;
    signed char precedence;
    switch (ch) {
        case ')':
            match = kParen;
            break;
        case ']':
            match = kArrayOp;
            break;
        case '?':
            match = kIf;
            break;
        case ':':
            match = kElse;
            break;
        case '&':
            if (nextChar != '&')
                goto noMatch;
            match = kLogicalAnd;
            advance = 2;
            break;
        case '|':
            if (nextChar != '|')
                goto noMatch;
            match = kLogicalOr;
            advance = 2;
            break;
        default:
noMatch:
            return 0;
    }
    SkSuppress suppress;
    precedence = gPrecedence[match];
    if (fSuppressStack.top().fSuppress) {
        if (fSuppressStack.top().fOpStackDepth < fOpStack.count()) {
            SkOp topOp = fOpStack.top();
            if (gPrecedence[topOp] <= precedence)
                fOpStack.pop();
            goto goHome;
        }
        bool changedPrecedence = gPrecedence[fSuppressStack.top().fOperator] < precedence;
        if (changedPrecedence)
            fSuppressStack.pop();
        if (precedence == kIfElsePrecedence) {
            if (match == kIf) {
                if (changedPrecedence)
                    fOpStack.pop();
                else
                    *fOpStack.push() = kIf;
            } else {
                if (fSuppressStack.top().fOpStackDepth == fOpStack.count()) {
                    goto flipSuppress;
                }
                fOpStack.pop();
            }
        }
        if (changedPrecedence == false)
            goto goHome;
    }
    while (gPrecedence[fOpStack.top() & ~kArtificialOp] < precedence) {
        if (processOp() == false)
            return false;
    }
    if (fSuppressStack.top().fOpStackDepth > fOpStack.count())
        fSuppressStack.pop();
    switch (match) {
        case kParen:
        case kArrayOp:
            if (fOpStack.count() <= 1 || fOpStack.top() != match) {
                fError = kMismatchedBrackets;
                return -1;
            }
            if (match == kParen) 
                fOpStack.pop();
            else {
                SkOpType indexType;
                fTypeStack.pop(&indexType);
                if (indexType != kInt && indexType != kScalar) {
                    fError = kExpectedNumberForArrayIndex; // (although, could permit strings eventually)
                    return -1;
                }
                SkOperand indexOperand;
                fOperandStack.pop(&indexOperand);
                int index = indexType == kScalar ? SkScalarFloor(indexOperand.fScalar) : 
                    indexOperand.fS32;
                SkOpType arrayType;
                fTypeStack.pop(&arrayType);
                if ((unsigned)arrayType != (unsigned)kArray) {
                    fError = kExpectedArray;
                    return -1;
                }
                SkOperand arrayOperand;
                fOperandStack.pop(&arrayOperand);
                SkTypedArray* array = arrayOperand.fArray;
                SkOperand operand;
                if (array->getIndex(index, &operand) == false) {
                    fError = kIndexOutOfRange;
                    return -1;
                }
                SkOpType resultType = array->getOpType();
                fTypeStack.push(resultType);
                fOperandStack.push(operand);
            }
            break;
        case kIf: {
            SkScriptValue ifValue;
            SkOpType ifType;
            fTypeStack.pop(&ifType);
            ifValue.fType = ToDisplayType(ifType);
            fOperandStack.pop(&ifValue.fOperand);
            if (convertTo(SkType_Int, &ifValue) == false)
                return -1;
            if (ifValue.fType != SkType_Int) {
                fError = kExpectedIntForConditionOperator;
                return -1;
            }
            suppress.fSuppress = ifValue.fOperand.fS32 == 0;
            suppress.fOperator = kIf;
            suppress.fOpStackDepth = fOpStack.count(); 
            suppress.fElse = false;
            fSuppressStack.push(suppress);
            // if left is true, do only up to colon
            // if left is false, do only after colon
            } break;
        case kElse:
flipSuppress:
            if (fSuppressStack.top().fElse)
                fSuppressStack.pop();
            fSuppressStack.top().fElse = true;
            fSuppressStack.top().fSuppress ^= true;
            // flip last do / don't do consideration from last '?'
            break;
        case kLogicalAnd:
        case kLogicalOr: {
            if (fTypeStack.top() != kInt) {
                fError = kExpectedBooleanExpression;
                return -1;
            }
            int32_t topInt = fOperandStack.top().fS32;
            if (fOpStack.top() != kLogicalAnd)
                *fOpStack.push() = kLogicalAnd; // really means 'to bool', and is appropriate for 'or'
            if (match == kLogicalOr ? topInt != 0 : topInt == 0) {
                suppress.fSuppress = true;
                suppress.fOperator = match;
                suppress.fOpStackDepth = fOpStack.count(); 
                suppress.fElse = false;
                fSuppressStack.push(suppress);
            } else {
                fTypeStack.pop();
                fOperandStack.pop();
            }
        }   break;
        default:
            SkASSERT(0);
    }
goHome:
    return advance;
}

SkScriptEngine::Error SkScriptEngine::opError() {
    int opCount = fOpStack.count();
    int operandCount = fOperandStack.count();
    if (opCount == 0) {
        if (operandCount != 1)
            return kExpectedOperator;
        return kNoError;
    }
    SkOp op = (SkOp) (fOpStack.top() & ~kArtificialOp);
    const SkOperatorAttributes* attributes = &gOpAttributes[op];
    if (attributes->fLeftType != kNoType && operandCount < 2)
        return kExpectedValue;
    if (attributes->fLeftType == kNoType && operandCount < 1)
        return kExpectedValue;
    return kNoError;
}

bool SkScriptEngine::processOp() {
    SkOp op;
    fOpStack.pop(&op);
    op = (SkOp) (op & ~kArtificialOp);
    const SkOperatorAttributes* attributes = &gOpAttributes[op];
    SkOpType type2;
    fTypeStack.pop(&type2);
    SkOpType type1 = type2;
    SkOperand operand2;
    fOperandStack.pop(&operand2);
    SkOperand operand1 = operand2; // !!! not really needed, suppresses warning
    if (attributes->fLeftType != kNoType) {
        fTypeStack.pop(&type1);
        fOperandStack.pop(&operand1);
        if (op == kFlipOps) {
            SkTSwap(type1, type2);
            SkTSwap(operand1, operand2);
            fOpStack.pop(&op);
            op = (SkOp) (op & ~kArtificialOp);
            attributes = &gOpAttributes[op];
        }
        if (type1 == kObject && (type1 & attributes->fLeftType) == 0) {
            SkScriptValue val;
            val.fType = ToDisplayType(type1);
            val.fOperand = operand1;
            bool success = handleUnbox(&val);
            if (success == false)
                return false;
            type1 = ToOpType(val.fType);
            operand1 = val.fOperand;
        }
    }
    if (type2 == kObject && (type2 & attributes->fLeftType) == 0) {
        SkScriptValue val;
        val.fType = ToDisplayType(type2);
        val.fOperand = operand2;
        bool success = handleUnbox(&val);
        if (success == false)
            return false;
        type2 = ToOpType(val.fType);
        operand2 = val.fOperand;
    }
    if (attributes->fLeftType != kNoType) {
        if (type1 != type2) {
            if ((attributes->fLeftType & kString) && attributes->fBias & kTowardsString && ((type1 | type2) & kString)) {
                if (type1 == kInt || type1 == kScalar) {
                    convertToString(operand1, type1 == kInt ? SkType_Int : SkType_Float);
                    type1 = kString;
                }
                if (type2 == kInt || type2 == kScalar) {
                    convertToString(operand2, type2 == kInt ? SkType_Int : SkType_Float);
                    type2 = kString;
                }
            } else if (attributes->fLeftType & kScalar && ((type1 | type2) & kScalar)) {
                if (type1 == kInt) {
                    operand1.fScalar = IntToScalar(operand1.fS32);
                    type1 = kScalar;
                }
                if (type2 == kInt) {
                    operand2.fScalar = IntToScalar(operand2.fS32);
                     type2 = kScalar;
                }
            }
        }
        if ((type1 & attributes->fLeftType) == 0 || type1 != type2) {
            if (type1 == kString) {
                const char* result = SkParse::FindScalar(operand1.fString->c_str(), &operand1.fScalar);
                if (result == NULL) {
                    fError = kExpectedNumber;
                    return false;
                }
                type1 = kScalar;
            }
            if (type1 == kScalar && (attributes->fLeftType == kInt || type2 == kInt)) {
                operand1.fS32 = SkScalarFloor(operand1.fScalar);
                type1 = kInt;
            }
        }
    }
    if ((type2 & attributes->fRightType) == 0 || type1 != type2) {
        if (type2 == kString) {
            const char* result = SkParse::FindScalar(operand2.fString->c_str(), &operand2.fScalar);
            if (result == NULL) {
                fError = kExpectedNumber;
                return false;
            }
            type2 = kScalar;
        }
        if (type2 == kScalar && (attributes->fRightType == kInt || type1 == kInt)) {
            operand2.fS32 = SkScalarFloor(operand2.fScalar);
            type2 = kInt;
        }
    }
    if (type2 == kScalar)
        op = (SkOp) (op + 1);
    else if (type2 == kString)
        op = (SkOp) (op + 2);
    switch(op) {
        case kAddInt:
            operand2.fS32 += operand1.fS32;
            break;
        case kAddScalar:
            operand2.fScalar += operand1.fScalar;
            break;
        case kAddString:
            if (fTrackString.find(operand1.fString) < 0) {
                operand1.fString = SkNEW_ARGS(SkString, (*operand1.fString));
                track(operand1.fString);
            }
            operand1.fString->append(*operand2.fString);
            operand2 = operand1;
            break;
        case kBitAnd:
            operand2.fS32 &= operand1.fS32;
            break;
        case kBitNot:
            operand2.fS32 = ~operand2.fS32;
            break;
        case kBitOr:
            operand2.fS32 |= operand1.fS32;
            break;
        case kDivideInt:
            if (operand2.fS32 == 0) {
                operand2.fS32 = operand1.fS32 == 0 ? SK_NaN32 : operand1.fS32 > 0 ? SK_MaxS32 : -SK_MaxS32;
                break;
            } else {
                int32_t original = operand2.fS32;
                operand2.fS32 = operand1.fS32 / operand2.fS32;
                if (original * operand2.fS32 == operand1.fS32)
                    break;    // integer divide was good enough
                operand2.fS32 = original;
                type2 = kScalar;
            }
        case kDivideScalar:
            if (operand2.fScalar == 0)
                operand2.fScalar = operand1.fScalar == 0 ? SK_ScalarNaN : operand1.fScalar > 0 ? SK_ScalarMax : -SK_ScalarMax;
            else
                operand2.fScalar = SkScalarDiv(operand1.fScalar, operand2.fScalar);
            break;
        case kEqualInt:
            operand2.fS32 = operand1.fS32 == operand2.fS32;
            break;
        case kEqualScalar:
            operand2.fS32 = operand1.fScalar == operand2.fScalar;
            type2 = kInt;
            break;
        case kEqualString:
            operand2.fS32 = *operand1.fString == *operand2.fString;
            type2 = kInt;
            break;
        case kGreaterEqualInt:
            operand2.fS32 = operand1.fS32 >= operand2.fS32;
            break;
        case kGreaterEqualScalar:
            operand2.fS32 = operand1.fScalar >= operand2.fScalar;
            type2 = kInt;
            break;
        case kGreaterEqualString:
            operand2.fS32 = strcmp(operand1.fString->c_str(), operand2.fString->c_str()) >= 0;
            type2 = kInt;
            break;
        case kLogicalAnd:
            operand2.fS32 = !! operand2.fS32;   // really, ToBool
            break;
        case kLogicalNot:
            operand2.fS32 = ! operand2.fS32;
            break;
        case kLogicalOr:
            SkASSERT(0);    // should have already been processed
            break;
        case kMinusInt:
            operand2.fS32 = -operand2.fS32;
            break;
        case kMinusScalar:
            operand2.fScalar = -operand2.fScalar;
            break;
        case kModuloInt:
            operand2.fS32 = operand1.fS32 % operand2.fS32;
            break;
        case kModuloScalar:
            operand2.fScalar = SkScalarMod(operand1.fScalar, operand2.fScalar);
            break;
        case kMultiplyInt:
            operand2.fS32 *= operand1.fS32;
            break;
        case kMultiplyScalar:
            operand2.fScalar = SkScalarMul(operand1.fScalar, operand2.fScalar);
            break;
        case kShiftLeft:
            operand2.fS32 = operand1.fS32 << operand2.fS32;
            break;
        case kShiftRight:
            operand2.fS32 = operand1.fS32 >> operand2.fS32;
            break;
        case kSubtractInt:
            operand2.fS32 = operand1.fS32 - operand2.fS32;
            break;
        case kSubtractScalar:
            operand2.fScalar = operand1.fScalar - operand2.fScalar;
            break;
        case kXor:
            operand2.fS32 ^= operand1.fS32;
            break;
        default:
            SkASSERT(0);
    }
    fTypeStack.push(type2);
    fOperandStack.push(operand2);
    return true;
}

void SkScriptEngine::propertyCallBack(_propertyCallBack prop, void* userStorage) {
    UserCallBack callBack;
    callBack.fPropertyCallBack = prop;
    commonCallBack(kProperty, callBack, userStorage);
}

void SkScriptEngine::track(SkTypedArray* array) { 
    SkASSERT(fTrackArray.find(array) < 0);  
    *(fTrackArray.end() - 1) = array; 
    fTrackArray.appendClear(); 
}

void SkScriptEngine::track(SkString* string) { 
    SkASSERT(fTrackString.find(string) < 0);  
    *(fTrackString.end() - 1) = string; 
    fTrackString.appendClear(); 
}

void SkScriptEngine::unboxCallBack(_unboxCallBack func, void* userStorage) {
    UserCallBack callBack;
    callBack.fUnboxCallBack = func;
    commonCallBack(kUnbox, callBack, userStorage);
}

bool SkScriptEngine::ConvertTo(SkScriptEngine* engine, SkDisplayTypes toType, SkScriptValue* value ) {
    SkASSERT(value);
    if (SkDisplayType::IsEnum(NULL /* fMaker */, toType))
        toType = SkType_Int;
    if (toType == SkType_Point || toType == SkType_3D_Point)
        toType = SkType_Float;
    if (toType == SkType_Drawable)
        toType = SkType_Displayable;
    SkDisplayTypes type = value->fType;
    if (type == toType) 
        return true;
    SkOperand& operand = value->fOperand;
    bool success = true;
    switch (toType) {
        case SkType_Int:
            if (type == SkType_Boolean)
                break;
            if (type == SkType_Float)
                operand.fS32 = SkScalarFloor(operand.fScalar);
            else {
                if (type != SkType_String) {
                    success = false;
                    break; // error
                }
                success = SkParse::FindS32(operand.fString->c_str(), &operand.fS32) != NULL;
            }
            break;
        case SkType_Float:
            if (type == SkType_Int) {
                if ((uint32_t)operand.fS32 == SK_NaN32)
                    operand.fScalar = SK_ScalarNaN;
                else if (SkAbs32(operand.fS32) == SK_MaxS32)
                    operand.fScalar = SkSign32(operand.fS32) * SK_ScalarMax;
                else
                    operand.fScalar = SkIntToScalar(operand.fS32);
            } else {
                if (type != SkType_String) {
                    success = false;
                    break; // error
                }
                success = SkParse::FindScalar(operand.fString->c_str(), &operand.fScalar) != NULL;
            }
            break;
        case SkType_String: {
            SkString* strPtr = new SkString();
            SkASSERT(engine);
            engine->track(strPtr);
            if (type == SkType_Int)
                strPtr->appendS32(operand.fS32);
            else if (type == SkType_Displayable) 
                SkASSERT(0); // must call through instance version instead of static version
            else {
                if (type != SkType_Float) {
                    success = false;
                    break;
                }
                strPtr->appendScalar(operand.fScalar);
            }
            operand.fString = strPtr;
            } break;
        case SkType_Array: {
            SkTypedArray* array = new SkTypedArray(type);
            *array->append() = operand;
            engine->track(array);
            operand.fArray = array;
            } break;
        default:
            SkASSERT(0);
    }
    value->fType = toType;
    if (success == false)
        engine->fError = kTypeConversionFailed;
    return success;
}

SkScalar SkScriptEngine::IntToScalar(int32_t s32) {
    SkScalar scalar;
    if ((uint32_t)s32 == SK_NaN32)
        scalar = SK_ScalarNaN;
    else if (SkAbs32(s32) == SK_MaxS32)
        scalar = SkSign32(s32) * SK_ScalarMax;
    else
        scalar = SkIntToScalar(s32);
    return scalar;
}

SkDisplayTypes SkScriptEngine::ToDisplayType(SkOpType type) {
    int val = type;
    switch (val) {
        case kNoType:
            return SkType_Unknown;
        case kInt:
            return SkType_Int;
        case kScalar:
            return SkType_Float;
        case kString:
            return SkType_String;
        case kArray:
            return SkType_Array;
        case kObject:
            return SkType_Displayable;
//      case kStruct:
//          return SkType_Structure;
        default:
            SkASSERT(0);
            return SkType_Unknown;
    }
}

SkScriptEngine::SkOpType SkScriptEngine::ToOpType(SkDisplayTypes type) {
    if (SkDisplayType::IsDisplayable(NULL /* fMaker */, type))
        return (SkOpType) kObject;
    if (SkDisplayType::IsEnum(NULL /* fMaker */, type))
        return kInt;
    switch (type) {
        case SkType_ARGB:
        case SkType_MSec:
        case SkType_Int:
            return kInt;
        case SkType_Float:
        case SkType_Point:
        case SkType_3D_Point:
            return kScalar;
        case SkType_Base64:
        case SkType_DynamicString:
        case SkType_String:
            return kString;
        case SkType_Array:
            return (SkOpType) kArray;
        case SkType_Unknown:
            return kNoType;
        default:
            SkASSERT(0);
            return kNoType;
    }
}

bool SkScriptEngine::ValueToString(SkScriptValue value, SkString* string) {
    switch (value.fType) {
        case kInt:
            string->reset();
            string->appendS32(value.fOperand.fS32);
            break;
        case kScalar:
            string->reset();
            string->appendScalar(value.fOperand.fScalar);
            break;
        case kString:
            string->set(*value.fOperand.fString);
            break;
        default:
            SkASSERT(0);
            return false;
    }
    return true; // no error
}

#ifdef SK_SUPPORT_UNITTEST

#ifdef SK_CAN_USE_FLOAT
    #include "SkFloatingPoint.h"
#endif

#define DEF_SCALAR_ANSWER   0
#define DEF_STRING_ANSWER   NULL

#define testInt(expression) { #expression, SkType_Int, expression, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER }
#ifdef SK_SCALAR_IS_FLOAT
    #define testScalar(expression) { #expression, SkType_Float, 0, (float) expression, DEF_STRING_ANSWER }
    #define testRemainder(exp1, exp2) { #exp1 "%" #exp2, SkType_Float, 0, sk_float_mod(exp1, exp2), DEF_STRING_ANSWER }
#else
    #ifdef SK_CAN_USE_FLOAT
        #define testScalar(expression) { #expression, SkType_Float, 0, (int) ((expression) * 65536.0f), DEF_STRING_ANSWER }
        #define testRemainder(exp1, exp2) { #exp1 "%" #exp2, SkType_Float, 0, (int) (sk_float_mod(exp1, exp2)  * 65536.0f), DEF_STRING_ANSWER }
    #endif
#endif
#define testTrue(expression) { #expression, SkType_Int, 1, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER }
#define testFalse(expression) { #expression, SkType_Int, 0, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER }

static const SkScriptNAnswer scriptTests[]  = {
    testInt(1>1/2),
    testInt((6+7)*8),
    testInt(0&&1?2:3),
    testInt(3*(4+5)),
#ifdef SK_CAN_USE_FLOAT
    testScalar(1.0+2.0), 
    testScalar(1.0+5), 
    testScalar(3.0-1.0), 
    testScalar(6-1.0), 
    testScalar(- -5.5- -1.5), 
    testScalar(2.5*6.), 
    testScalar(0.5*4), 
    testScalar(4.5/.5), 
    testScalar(9.5/19), 
    testRemainder(9.5, 0.5), 
    testRemainder(9.,2), 
    testRemainder(9,2.5),
    testRemainder(-9,2.5),
    testTrue(-9==-9.0),
    testTrue(-9.==-4.0-5),
    testTrue(-9.*1==-4-5),
    testFalse(-9!=-9.0),
    testFalse(-9.!=-4.0-5),
    testFalse(-9.*1!=-4-5),
#endif
    testInt(0x123),
    testInt(0XABC),
    testInt(0xdeadBEEF),
    {   "'123'+\"456\"", SkType_String, 0, 0, "123456" },
    {   "123+\"456\"", SkType_String, 0, 0, "123456" },
    {   "'123'+456", SkType_String, 0, 0, "123456" },
    {   "'123'|\"456\"", SkType_Int, 123|456, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER },
    {   "123|\"456\"", SkType_Int, 123|456, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER },
    {   "'123'|456", SkType_Int, 123|456, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER },
    {   "'2'<11", SkType_Int, 1, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER },
    {   "2<'11'", SkType_Int, 1, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER },
    {   "'2'<'11'", SkType_Int, 0, DEF_SCALAR_ANSWER, DEF_STRING_ANSWER },
    testInt(123),
    testInt(-345),
    testInt(+678),
    testInt(1+2+3),
    testInt(3*4+5),
    testInt(6+7*8),
    testInt(-1-2-8/4),
    testInt(-9%4),
    testInt(9%-4),
    testInt(-9%-4),
    testInt(123|978),
    testInt(123&978),
    testInt(123^978),
    testInt(2<<4),
    testInt(99>>3),
    testInt(~55),
    testInt(~~55),
    testInt(!55),
    testInt(!!55),
    // both int
    testInt(2<2),
    testInt(2<11),
    testInt(20<11),
    testInt(2<=2),
    testInt(2<=11),
    testInt(20<=11),
    testInt(2>2),
    testInt(2>11),
    testInt(20>11),
    testInt(2>=2),
    testInt(2>=11),
    testInt(20>=11),
    testInt(2==2),
    testInt(2==11),
    testInt(20==11),
    testInt(2!=2),
    testInt(2!=11),
    testInt(20!=11),
#ifdef SK_CAN_USE_FLOAT
    // left int, right scalar
    testInt(2<2.),
    testInt(2<11.),
    testInt(20<11.),
    testInt(2<=2.),
    testInt(2<=11.),
    testInt(20<=11.),
    testInt(2>2.),
    testInt(2>11.),
    testInt(20>11.),
    testInt(2>=2.),
    testInt(2>=11.),
    testInt(20>=11.),
    testInt(2==2.),
    testInt(2==11.),
    testInt(20==11.),
    testInt(2!=2.),
    testInt(2!=11.),
    testInt(20!=11.),
    // left scalar, right int
        testInt(2.<2),
    testInt(2.<11),
    testInt(20.<11),
    testInt(2.<=2),
    testInt(2.<=11),
    testInt(20.<=11),
    testInt(2.>2),
    testInt(2.>11),
    testInt(20.>11),
    testInt(2.>=2),
    testInt(2.>=11),
    testInt(20.>=11),
    testInt(2.==2),
    testInt(2.==11),
    testInt(20.==11),
    testInt(2.!=2),
    testInt(2.!=11),
    testInt(20.!=11),
    // both scalar
    testInt(2.<11.),
    testInt(20.<11.),
    testInt(2.<=2.),
    testInt(2.<=11.),
    testInt(20.<=11.),
    testInt(2.>2.),
    testInt(2.>11.),
    testInt(20.>11.),
    testInt(2.>=2.),
    testInt(2.>=11.),
    testInt(20.>=11.),
    testInt(2.==2.),
    testInt(2.==11.),
    testInt(20.==11.),
    testInt(2.!=2.),
    testInt(2.!=11.),
    testInt(20.!=11.),
#endif
    // int, string (string is int)
    testFalse(2<'2'),
    testTrue(2<'11'),
    testFalse(20<'11'),
    testTrue(2<='2'),
    testTrue(2<='11'),
    testFalse(20<='11'),
    testFalse(2>'2'),
    testFalse(2>'11'),
    testTrue(20>'11'),
    testTrue(2>='2'),
    testFalse(2>='11'),
    testTrue(20>='11'),
    testTrue(2=='2'),
    testFalse(2=='11'),
    testFalse(2!='2'),
    testTrue(2!='11'),
    // int, string (string is scalar)
    testFalse(2<'2.'),
    testTrue(2<'11.'),
    testFalse(20<'11.'),
    testTrue(2=='2.'),
    testFalse(2=='11.'),
#ifdef SK_CAN_USE_FLOAT
    // scalar, string
    testFalse(2.<'2.'),
    testTrue(2.<'11.'),
    testFalse(20.<'11.'),
    testTrue(2.=='2.'),
    testFalse(2.=='11.'),
    // string, int
    testFalse('2'<2),
    testTrue('2'<11),
    testFalse('20'<11),
    testTrue('2'==2),
    testFalse('2'==11),
    // string, scalar
    testFalse('2'<2.),
    testTrue('2'<11.),
    testFalse('20'<11.),
    testTrue('2'==2.),
    testFalse('2'==11.),
#endif
    // string, string
    testFalse('2'<'2'),
    testFalse('2'<'11'),
    testFalse('20'<'11'),
    testTrue('2'=='2'),
    testFalse('2'=='11'),
    // logic
    testInt(1?2:3),
    testInt(0?2:3),
    testInt(1&&2||3),
    testInt(1&&0||3),
    testInt(1&&0||0),
    testInt(1||0&&3),
    testInt(0||0&&3),
    testInt(0||1&&3),
    testInt(1?(2?3:4):5),
    testInt(0?(2?3:4):5),
    testInt(1?(0?3:4):5),
    testInt(0?(0?3:4):5),
    testInt(1?2?3:4:5),
    testInt(0?2?3:4:5),
    testInt(1?0?3:4:5),
    testInt(0?0?3:4:5),
    
    testInt(1?2:(3?4:5)),
    testInt(0?2:(3?4:5)),
    testInt(1?0:(3?4:5)),
    testInt(0?0:(3?4:5)),
    testInt(1?2:3?4:5),
    testInt(0?2:3?4:5),
    testInt(1?0:3?4:5),
    testInt(0?0:3?4:5)
#ifdef SK_CAN_USE_FLOAT
    , { "123.5", SkType_Float, 0, SkIntToScalar(123) + SK_Scalar1/2, DEF_STRING_ANSWER }
#endif
};

#define SkScriptNAnswer_testCount   SK_ARRAY_COUNT(scriptTests)

void SkScriptEngine::UnitTest() {
    for (unsigned index = 0; index < SkScriptNAnswer_testCount; index++) {
        SkScriptEngine engine(SkScriptEngine::ToOpType(scriptTests[index].fType));
        SkScriptValue value;
        const char* script = scriptTests[index].fScript;
        SkASSERT(engine.evaluateScript(&script, &value) == true);
        SkASSERT(value.fType == scriptTests[index].fType);
        SkScalar error;
        switch (value.fType) {
            case SkType_Int:
                SkASSERT(value.fOperand.fS32 == scriptTests[index].fIntAnswer);
                break;
            case SkType_Float:
                error = SkScalarAbs(value.fOperand.fScalar - scriptTests[index].fScalarAnswer);
                SkASSERT(error < SK_Scalar1 / 10000);
                break;
            case SkType_String:
                SkASSERT(strcmp(value.fOperand.fString->c_str(), scriptTests[index].fStringAnswer) == 0);
                break;
            default:
                SkASSERT(0);
        }
    }
}
#endif

