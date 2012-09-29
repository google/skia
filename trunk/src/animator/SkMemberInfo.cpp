
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkMemberInfo.h"
#include "SkAnimateMaker.h"
#include "SkAnimatorScript.h"
#include "SkBase64.h"
#include "SkCamera.h"
#include "SkDisplayable.h"
#include "SkDisplayTypes.h"
#include "SkDraw3D.h"
#include "SkDrawColor.h"
#include "SkParse.h"
#include "SkScript.h"
#include "SkTSearch.h"
#include "SkTypedArray.h"

size_t SkMemberInfo::GetSize(SkDisplayTypes type) { // size of simple types only
    size_t byteSize;
    switch (type) {
        case SkType_ARGB:
            byteSize = sizeof(SkColor);
            break;
        case SkType_AddMode:
        case SkType_Align:
        case SkType_ApplyMode:
        case SkType_ApplyTransition:
        case SkType_BitmapEncoding:
        case SkType_Boolean:
        case SkType_Cap:
        case SkType_EventCode:
        case SkType_EventKind:
        case SkType_EventMode:
        case SkType_FilterType:
        case SkType_FontStyle:
        case SkType_FromPathMode:
        case SkType_Join:
        case SkType_MaskFilterBlurStyle:
        case SkType_PathDirection:
        case SkType_Style:
        case SkType_TileMode:
        case SkType_Xfermode:
            byteSize = sizeof(int);
            break;
        case SkType_Base64: // assume base64 data is always const, copied by ref
        case SkType_Displayable:
        case SkType_Drawable:
        case SkType_Matrix:
            byteSize = sizeof(void*);
            break;
        case SkType_MSec:
            byteSize = sizeof(SkMSec);
            break;
        case SkType_Point:
            byteSize = sizeof(SkPoint);
            break;
        case SkType_3D_Point:
            byteSize = sizeof(Sk3D_Point);
            break;
        case SkType_Int:
            byteSize = sizeof(int32_t);
            break;
        case SkType_Float:
            byteSize = sizeof(SkScalar);
            break;
        case SkType_DynamicString:
        case SkType_String:
            byteSize = sizeof(SkString);    // assume we'll copy by reference, not value
            break;
        default:
//          SkASSERT(0);
            byteSize = 0;
    }
    return byteSize;
}

bool SkMemberInfo::getArrayValue(const SkDisplayable* displayable, int index, SkOperand* value) const {
    SkASSERT(fType != SkType_String && fType != SkType_MemberProperty);
    char* valuePtr = (char*) *(SkOperand**) memberData(displayable);
    SkDisplayTypes type = (SkDisplayTypes) 0;
    if (displayable->getType() == SkType_Array) {
        SkDisplayArray* dispArray = (SkDisplayArray*) displayable;
        if (dispArray->values.count() <= index)
            return false;
        type = dispArray->values.getType();
    } else {
        SkASSERT(0); // incomplete
    }
    size_t byteSize = GetSize(type);
    memcpy(value, valuePtr + index * byteSize, byteSize);
    return true;
}

size_t SkMemberInfo::getSize(const SkDisplayable* displayable) const {
    size_t byteSize;
    switch (fType) {
        case SkType_MemberProperty:
            byteSize = GetSize(propertyType());
            break;
        case SkType_Array: {
            SkDisplayTypes type;
            if (displayable == NULL)
                return sizeof(int);
            if (displayable->getType() == SkType_Array) {
                SkDisplayArray* dispArray = (SkDisplayArray*) displayable;
                type = dispArray->values.getType();
            } else
                type = propertyType();
            SkTDOperandArray* array = (SkTDOperandArray*) memberData(displayable);
            byteSize = GetSize(type) * array->count();
            } break;
        default:
            byteSize = GetSize((SkDisplayTypes) fType);
    }
    return byteSize;
}

void SkMemberInfo::getString(const SkDisplayable* displayable, SkString** string) const {
    if (fType == SkType_MemberProperty) {
        SkScriptValue value;
        displayable->getProperty(propertyIndex(), &value);
        SkASSERT(value.fType == SkType_String);
        *string = value.fOperand.fString;
        return;
    }
    SkASSERT(fCount == sizeof(SkString) / sizeof(SkScalar));
    SkASSERT(fType == SkType_String || fType == SkType_DynamicString);
    void* valuePtr = memberData(displayable);
    *string = (SkString*) valuePtr;
}

void SkMemberInfo::getValue(const SkDisplayable* displayable, SkOperand value[], int count) const {
    SkASSERT(fType != SkType_String && fType != SkType_MemberProperty);
    SkASSERT(count == fCount);
    void* valuePtr = memberData(displayable);
    size_t byteSize = getSize(displayable);
    SkASSERT(sizeof(value[0].fScalar) == sizeof(value[0])); // no support for 64 bit pointers, yet
    memcpy(value, valuePtr, byteSize);
}

void SkMemberInfo::setString(SkDisplayable* displayable, SkString* value) const {
    SkString* string = (SkString*) memberData(displayable);
    string->set(*value);
    displayable->dirty();
}

void SkMemberInfo::setValue(SkDisplayable* displayable, const SkOperand values[],
                            int count) const {
    SkASSERT(sizeof(values[0].fScalar) == sizeof(values[0]));   // no support for 64 bit pointers, yet
    char* dst = (char*) memberData(displayable);
    if (fType == SkType_Array) {
        SkTDScalarArray* array = (SkTDScalarArray* ) dst;
        array->setCount(count);
        dst = (char*) array->begin();
    }
    memcpy(dst, values, count * sizeof(SkOperand));
    displayable->dirty();
}


static inline bool is_between(int c, int min, int max)
{
    return (unsigned)(c - min) <= (unsigned)(max - min);
}

static inline bool is_hex(int c)
{
    if (is_between(c, '0', '9'))
        return true;
    c |= 0x20;  // make us lower-case
    if (is_between(c, 'a', 'f'))
        return true;
    return false;
}


bool SkMemberInfo::setValue(SkAnimateMaker& maker, SkTDOperandArray* arrayStorage,
    int storageOffset, int maxStorage, SkDisplayable* displayable, SkDisplayTypes outType,
    const char rawValue[], size_t rawValueLen) const
{
    SkString valueStr(rawValue, rawValueLen);
    SkScriptValue scriptValue;
    scriptValue.fType = SkType_Unknown;
    scriptValue.fOperand.fS32 = 0;
    SkDisplayTypes type = getType();
    SkAnimatorScript engine(maker, displayable, type);
    if (arrayStorage)
        displayable = NULL;
    bool success = true;
    void* untypedStorage = NULL;
    if (displayable && fType != SkType_MemberProperty && fType != SkType_MemberFunction)
        untypedStorage = (SkTDOperandArray*) memberData(displayable);

    if (type == SkType_ARGB) {
        // for both SpiderMonkey and SkiaScript, substitute any #xyz or #xxyyzz first
            // it's enough to expand the colors into 0xFFxxyyzz
        const char* poundPos;
        while ((poundPos = strchr(valueStr.c_str(), '#')) != NULL) {
            size_t offset = poundPos - valueStr.c_str();
            if (valueStr.size() - offset < 4)
                break;
            char r = poundPos[1];
            char g = poundPos[2];
            char b = poundPos[3];
            if (is_hex(r) == false || is_hex(g) == false || is_hex(b) == false)
                break;
            char hex = poundPos[4];
            if (is_hex(hex) == false) {
                valueStr.insertUnichar(offset + 1, r);
                valueStr.insertUnichar(offset + 3, g);
                valueStr.insertUnichar(offset + 5, b);
            }
            *(char*) poundPos = '0'; // overwrite '#'
            valueStr.insert(offset + 1, "xFF");
        }
    }
    if (SkDisplayType::IsDisplayable(&maker, type) || SkDisplayType::IsEnum(&maker, type) || type == SkType_ARGB)
        goto scriptCommon;
    switch (type) {
        case SkType_String:
#if 0
            if (displayable && displayable->isAnimate()) {

                goto noScriptString;
            }
            if (strncmp(rawValue, "#string:", sizeof("#string:") - 1) == 0) {
                SkASSERT(sizeof("string") == sizeof("script"));
                char* stringHeader = valueStr.writable_str();
                memcpy(&stringHeader[1], "script", sizeof("script") - 1);
                rawValue = valueStr.c_str();
                goto noScriptString;
            } else
#endif
            if (strncmp(rawValue, "#script:", sizeof("#script:") - 1) != 0)
                goto noScriptString;
            valueStr.remove(0, 8);
        case SkType_Unknown:
        case SkType_Int:
        case SkType_MSec:  // for the purposes of script, MSec is treated as a Scalar
        case SkType_Point:
        case SkType_3D_Point:
        case SkType_Float:
        case SkType_Array:
scriptCommon: {
                const char* script = valueStr.c_str();
                success = engine.evaluateScript(&script, &scriptValue);
                if (success == false) {
                    maker.setScriptError(engine);
                    return false;
                }
            }
            SkASSERT(success);
            if (scriptValue.fType == SkType_Displayable) {
                if (type == SkType_String) {
                    const char* charPtr = NULL;
                    maker.findKey(scriptValue.fOperand.fDisplayable, &charPtr);
                    scriptValue.fOperand.fString = new SkString(charPtr);
                    scriptValue.fType = SkType_String;
                    engine.SkScriptEngine::track(scriptValue.fOperand.fString);
                    break;
                }
                SkASSERT(SkDisplayType::IsDisplayable(&maker, type));
                if (displayable)
                    displayable->setReference(this, scriptValue.fOperand.fDisplayable);
                else
                    arrayStorage->begin()[0].fDisplayable = scriptValue.fOperand.fDisplayable;
                return true;
            }
            if (type != scriptValue.fType) {
                if (scriptValue.fType == SkType_Array) {
                    engine.forget(scriptValue.getArray());
                    goto writeStruct; // real structs have already been written by script
                }
                switch (type) {
                    case SkType_String:
                        success = engine.convertTo(SkType_String, &scriptValue);
                        break;
                    case SkType_MSec:
                    case SkType_Float:
                        success = engine.convertTo(SkType_Float, &scriptValue);
                        break;
                    case SkType_Int:
                        success = engine.convertTo(SkType_Int, &scriptValue);
                        break;
                    case SkType_Array:
                        success = engine.convertTo(arrayType(), &scriptValue);
                        // !!! incomplete; create array of appropriate type and add scriptValue to it
                        SkASSERT(0);
                        break;
                    case SkType_Displayable:
                    case SkType_Drawable:
                        return false;   // no way to convert other types to this
                    default:    // to avoid warnings
                        break;
                }
                if (success == false)
                    return false;
            }
            if (type == SkType_MSec)
                scriptValue.fOperand.fMSec = SkScalarMulRound(scriptValue.fOperand.fScalar, 1000);
            scriptValue.fType = type;
        break;
        noScriptString:
        case SkType_DynamicString:
            if (fType == SkType_MemberProperty && displayable) {
                SkString string(rawValue, rawValueLen);
                SkScriptValue scriptValue;
                scriptValue.fOperand.fString = &string;
                scriptValue.fType = SkType_String;
                displayable->setProperty(propertyIndex(), scriptValue);
            } else if (displayable) {
                SkString* string = (SkString*) memberData(displayable);
                string->set(rawValue, rawValueLen);
            } else {
                SkASSERT(arrayStorage->count() == 1);
                arrayStorage->begin()->fString->set(rawValue, rawValueLen);
            }
            goto dirty;
        case SkType_Base64: {
            SkBase64 base64;
            base64.decode(rawValue, rawValueLen);
            *(SkBase64* ) untypedStorage = base64;
            } goto dirty;
        default:
            SkASSERT(0);
            break;
    }
//  if (SkDisplayType::IsStruct(type) == false)
    {
writeStruct:
        if (writeValue(displayable, arrayStorage, storageOffset, maxStorage,
                untypedStorage, outType, scriptValue)) {
                    maker.setErrorCode(SkDisplayXMLParserError::kUnexpectedType);
            return false;
        }
    }
dirty:
    if (displayable)
        displayable->dirty();
    return true;
}

bool SkMemberInfo::setValue(SkAnimateMaker& maker, SkTDOperandArray* arrayStorage,
        int storageOffset, int maxStorage, SkDisplayable* displayable, SkDisplayTypes outType,
        SkString& raw) const {
    return setValue(maker, arrayStorage, storageOffset, maxStorage, displayable, outType, raw.c_str(),
        raw.size());
}

bool SkMemberInfo::writeValue(SkDisplayable* displayable, SkTDOperandArray* arrayStorage,
    int storageOffset, int maxStorage, void* untypedStorage, SkDisplayTypes outType,
    SkScriptValue& scriptValue) const
{
    SkOperand* storage = untypedStorage ? (SkOperand*) untypedStorage : arrayStorage ?
        arrayStorage->begin() : NULL;
    if (storage)
        storage += storageOffset;
    SkDisplayTypes type = getType();
    if (fType == SkType_MemberProperty) {
        if(displayable)
            displayable->setProperty(propertyIndex(), scriptValue);
        else {
            SkASSERT(storageOffset < arrayStorage->count());
            switch (scriptValue.fType) {
                case SkType_Boolean:
                case SkType_Float:
                case SkType_Int:
                    memcpy(&storage->fScalar, &scriptValue.fOperand.fScalar, sizeof(SkScalar));
                    break;
                case SkType_Array:
                    memcpy(&storage->fScalar, scriptValue.fOperand.fArray->begin(), scriptValue.fOperand.fArray->count() * sizeof(SkScalar));
                    break;
                case SkType_String:
                    storage->fString->set(*scriptValue.fOperand.fString);
                    break;
                default:
                    SkASSERT(0);    // type isn't handled yet
            }
        }
    } else if (fType == SkType_MemberFunction) {
        SkASSERT(scriptValue.fType == SkType_Array);
        if (displayable)
            displayable->executeFunction(displayable, this, scriptValue.fOperand.fArray, NULL);
        else {
            int count = scriptValue.fOperand.fArray->count();
    //      SkASSERT(maxStorage == 0 || count == maxStorage);
            if (arrayStorage->count() == 2)
                arrayStorage->setCount(2 * count);
            else {
                storageOffset *= count;
                SkASSERT(count + storageOffset <= arrayStorage->count());
            }
            memcpy(&(*arrayStorage)[storageOffset], scriptValue.fOperand.fArray->begin(), count * sizeof(SkOperand));
        }

    } else if (fType == SkType_Array) {
        SkTypedArray* destArray = (SkTypedArray*) (untypedStorage ? untypedStorage : arrayStorage);
        SkASSERT(destArray);
    //  destArray->setCount(0);
        if (scriptValue.fType != SkType_Array) {
            SkASSERT(type == scriptValue.fType);
    //      SkASSERT(storageOffset + 1 <= maxStorage);
            destArray->setCount(storageOffset + 1);
            (*destArray)[storageOffset] = scriptValue.fOperand;
        } else {
            if (type == SkType_Unknown) {
                type = scriptValue.fOperand.fArray->getType();
                destArray->setType(type);
            }
            SkASSERT(type == scriptValue.fOperand.fArray->getType());
            int count = scriptValue.fOperand.fArray->count();
    //      SkASSERT(storageOffset + count <= maxStorage);
            destArray->setCount(storageOffset + count);
            memcpy(destArray->begin() + storageOffset, scriptValue.fOperand.fArray->begin(), sizeof(SkOperand) * count);
        }
    } else if (type == SkType_String) {
        SkString* string = untypedStorage ? (SkString*) untypedStorage : (*arrayStorage)[storageOffset].fString;
        string->set(*scriptValue.fOperand.fString);
    } else if (type == SkType_ARGB && outType == SkType_Float) {
        SkTypedArray* array = scriptValue.fOperand.fArray;
        SkASSERT(scriptValue.fType == SkType_Int || scriptValue.fType == SkType_ARGB ||
            scriptValue.fType == SkType_Array);
        SkASSERT(scriptValue.fType != SkType_Array || (array != NULL &&
            array->getType() == SkType_Int));
        int numberOfColors = scriptValue.fType == SkType_Array ? array->count() : 1;
        int numberOfComponents = numberOfColors * 4;
    //  SkASSERT(maxStorage == 0 || maxStorage == numberOfComponents);
        if (maxStorage == 0)
            arrayStorage->setCount(numberOfComponents);
        for (int index = 0; index < numberOfColors; index++) {
            SkColor color = scriptValue.fType == SkType_Array ?
                (SkColor) array->begin()[index].fS32 : (SkColor) scriptValue.fOperand.fS32;
            storage[0].fScalar = SkIntToScalar(SkColorGetA(color));
            storage[1].fScalar = SkIntToScalar(SkColorGetR(color));
            storage[2].fScalar = SkIntToScalar(SkColorGetG(color));
            storage[3].fScalar = SkIntToScalar(SkColorGetB(color));
            storage += 4;
        }
    } else if (SkDisplayType::IsStruct(NULL /* !!! maker*/, type)) {
        if (scriptValue.fType != SkType_Array)
            return true;    // error
        SkASSERT(sizeof(SkScalar) == sizeof(SkOperand)); // !!! no 64 bit pointer support yet
        int count = scriptValue.fOperand.fArray->count();
        if (count > 0) {
            SkASSERT(fCount == count);
            memcpy(storage, scriptValue.fOperand.fArray->begin(), count * sizeof(SkOperand));
        }
    } else if (scriptValue.fType == SkType_Array) {
        SkASSERT(scriptValue.fOperand.fArray->getType() == type);
        SkASSERT(scriptValue.fOperand.fArray->count() == getCount());
        memcpy(storage, scriptValue.fOperand.fArray->begin(), getCount() * sizeof(SkOperand));
    } else {
        memcpy(storage, &scriptValue.fOperand, sizeof(SkOperand));
    }
    return false;
}


//void SkMemberInfo::setValue(SkDisplayable* displayable, const char value[], const char name[]) const {
//  void* valuePtr = (void*) ((char*) displayable + fOffset);
//  switch (fType) {
//      case SkType_Point3D: {
//          static const char xyz[] = "x|y|z";
//          int index = find_one(xyz, name);
//          SkASSERT(index >= 0);
//          valuePtr = (void*) ((char*) valuePtr + index * sizeof(SkScalar));
//          } break;
//      default:
//          SkASSERT(0);
//  }
//  SkParse::FindScalar(value, (SkScalar*) valuePtr);
//  displayable->dirty();
//}

#if SK_USE_CONDENSED_INFO == 0

// Find Nth memberInfo
const SkMemberInfo* SkMemberInfo::Find(const SkMemberInfo info[], int count, int* index) {
    SkASSERT(*index >= 0);
    if (info->fType == SkType_BaseClassInfo) {
        const SkMemberInfo* inherited = (SkMemberInfo*) info->fName;
        const SkMemberInfo* result = SkMemberInfo::Find(inherited, info->fCount, index);
        if (result != NULL)
            return result;
        if (--count == 0)
            return NULL;
        info++;
    }
    SkASSERT(info->fName);
    SkASSERT(info->fType != SkType_BaseClassInfo);
    if (*index >= count) {
        *index -= count;
        return NULL;
    }
    return &info[*index];
}

// Find named memberinfo
const SkMemberInfo* SkMemberInfo::Find(const SkMemberInfo info[], int count, const char** matchPtr) {
    const char* match = *matchPtr;
    if (info->fType == SkType_BaseClassInfo) {
        const SkMemberInfo* inherited = (SkMemberInfo*) info->fName;
        const SkMemberInfo* result = SkMemberInfo::Find(inherited, info->fCount, matchPtr);
        if (result != NULL)
            return result;
        if (--count == 0)
            return NULL;
        info++;
    }
    SkASSERT(info->fName);
    SkASSERT(info->fType != SkType_BaseClassInfo);
    int index = SkStrSearch(&info->fName, count, match, sizeof(*info));
    if (index < 0 || index >= count)
        return NULL;
    return &info[index];
}

const SkMemberInfo* SkMemberInfo::getInherited() const {
    return (SkMemberInfo*) fName;
}

#endif // SK_USE_CONDENSED_INFO == 0

#if 0
bool SkMemberInfo::SetValue(void* valuePtr, const char value[], SkDisplayTypes type,
                            int count) {
    switch (type) {
        case SkType_Animate:
        case SkType_BaseBitmap:
        case SkType_Bitmap:
        case SkType_Dash:
        case SkType_Displayable:
        case SkType_Drawable:
        case SkType_Matrix:
        case SkType_Path:
        case SkType_Text:
        case SkType_3D_Patch:
            return false; // ref to object; caller must resolve
        case SkType_MSec: {
            SkParse::FindMSec(value, (SkMSec*) valuePtr);
            } break;
        case SkType_3D_Point:
        case SkType_Point:
    //  case SkType_PointArray:
        case SkType_ScalarArray:
            SkParse::FindScalars(value, (SkScalar*) valuePtr, count);
            break;
        default:
            SkASSERT(0);
    }
    return true;
}
#endif


