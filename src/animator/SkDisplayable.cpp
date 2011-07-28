
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayable.h"
#include "SkDisplayApply.h"
#include "SkParse.h"
#ifdef SK_DEBUG
#include "SkDisplayList.h"
#endif
#include "SkDisplayTypes.h"

#ifdef SK_FIND_LEAKS
// int SkDisplayable::fAllocationCount;
SkTDDisplayableArray SkDisplayable::fAllocations;
#endif

#ifdef SK_DEBUG
SkDisplayable::SkDisplayable() { 
    id = _id.c_str();
#ifdef SK_FIND_LEAKS
    // fAllocationCount++;
    *fAllocations.append() = this;
#endif
}
#endif

SkDisplayable::~SkDisplayable() {
#ifdef SK_FIND_LEAKS
    //  fAllocationCount--;
    int index = fAllocations.find(this);
    SkASSERT(index >= 0);
    fAllocations.remove(index);
#endif
}

bool SkDisplayable::add(SkAnimateMaker& , SkDisplayable* child) {
    return false; 
}

//void SkDisplayable::apply(SkAnimateMaker& , const SkMemberInfo* , 
//      SkDisplayable* , SkScalar [], int count) { 
//  SkASSERT(0); 
//}

bool SkDisplayable::canContainDependents() const {
    return false; 
}
 
bool SkDisplayable::childrenNeedDisposing() const { 
    return false; 
}

void SkDisplayable::clearBounder() {
}

bool SkDisplayable::contains(SkDisplayable* ) {
    return false;
}

SkDisplayable* SkDisplayable::contains(const SkString& ) {
    return NULL;
}

SkDisplayable* SkDisplayable::deepCopy(SkAnimateMaker* maker) {
    SkDisplayTypes type = getType();
    if (type == SkType_Unknown) {
        SkASSERT(0);
        return NULL;
    }
    SkDisplayable* copy = SkDisplayType::CreateInstance(maker, type);
    int index = -1;
    int propIndex = 0;
    const SkMemberInfo* info;
    do {
        info = copy->getMember(++index);
        if (info == NULL)
            break;
        if (info->fType == SkType_MemberProperty) {
            SkScriptValue value;
            if (getProperty(propIndex, &value))
                copy->setProperty(propIndex, value);
            propIndex++;
            continue;
        }
        if (info->fType == SkType_MemberFunction)
            continue;
        if (info->fType == SkType_Array) {
            SkTDOperandArray* array = (SkTDOperandArray*) info->memberData(this);
            int arrayCount;
            if (array == NULL || (arrayCount = array->count()) == 0)
                continue;
            SkTDOperandArray* copyArray = (SkTDOperandArray*) info->memberData(copy);
            copyArray->setCount(arrayCount);
            SkDisplayTypes elementType;
            if (type == SkType_Array) {
                SkDisplayArray* dispArray = (SkDisplayArray*) this;
                elementType = dispArray->values.getType();
            } else
                elementType = info->arrayType();
            size_t elementSize = SkMemberInfo::GetSize(elementType);
            size_t byteSize = elementSize * arrayCount;
            memcpy(copyArray->begin(), array->begin(), byteSize);
            continue;
        }
        if (SkDisplayType::IsDisplayable(maker, info->fType)) {
            SkDisplayable** displayable = (SkDisplayable**) info->memberData(this);
            if (*displayable == NULL || *displayable == (SkDisplayable*) -1)
                continue;
            SkDisplayable* deeper = (*displayable)->deepCopy(maker);
            info->setMemberData(copy, deeper, sizeof(deeper));
            continue;
        }
        if (info->fType == SkType_String || info->fType == SkType_DynamicString) {
            SkString* string;
            info->getString(this, &string);
            info->setString(copy, string);
            continue;
        }
        void* data = info->memberData(this);
        size_t size = SkMemberInfo::GetSize(info->fType);
        info->setMemberData(copy, data, size);
    } while (true);
    copy->dirty();
    return copy;
}

void SkDisplayable::dirty() {
}

#ifdef SK_DUMP_ENABLED
void SkDisplayable::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
#if SK_USE_CONDENSED_INFO == 0
    this->dumpAttrs(maker);
    this->dumpChildren(maker);
#endif
}

void SkDisplayable::dumpAttrs(SkAnimateMaker* maker) {
    SkDisplayTypes type = getType();
    if (type == SkType_Unknown) {
        //SkDebugf("/>\n");
        return;
    }
    SkDisplayable* blankCopy = SkDisplayType::CreateInstance(maker, type);

    int index = -1;
    int propIndex = 0;
    const SkMemberInfo* info;
    const SkMemberInfo* blankInfo;
    SkScriptValue value;
    SkScriptValue blankValue;
    SkOperand values[2];
    SkOperand blankValues[2];
    do {
        info = this->getMember(++index);
        if (NULL == info) {
            //SkDebugf("\n");
            break;
        }
        if (SkType_MemberProperty == info->fType) {
            if (getProperty(propIndex, &value)) {
                blankCopy->getProperty(propIndex, &blankValue);
                //last two are dummies
                dumpValues(info, value.fType, value.fOperand, blankValue.fOperand, value.fOperand, blankValue.fOperand);
                }
            
            propIndex++;
            continue;
        }
        if (SkDisplayType::IsDisplayable(maker, info->fType)) {
            continue;
        }
        
        if (info->fType == SkType_MemberFunction)
            continue;
            
            
        if (info->fType == SkType_Array) {
            SkTDOperandArray* array = (SkTDOperandArray*) info->memberData(this);
            int arrayCount;
            if (array == NULL || (arrayCount = array->count()) == 0)
                continue;
            SkDisplayTypes elementType;
            if (type == SkType_Array) {
                SkDisplayArray* dispArray = (SkDisplayArray*) this;
                elementType = dispArray->values.getType();
            } else
                elementType = info->arrayType();
            bool firstElem = true;
            SkDebugf("%s=\"[", info->fName);
            for (SkOperand* op = array->begin(); op < array->end(); op++) {
                if (!firstElem) SkDebugf(",");
                switch (elementType) {
                        case SkType_Displayable:
                            SkDebugf("%s", op->fDisplayable->id);
                            break;
                        case SkType_Int:                            
                            SkDebugf("%d", op->fS32);
                            break;
                        case SkType_Float:
#ifdef SK_CAN_USE_FLOAT
                            SkDebugf("%g", SkScalarToFloat(op->fScalar));
#else
                            SkDebugf("%x", op->fScalar);
#endif
                            break;
                        case SkType_String:
                        case SkType_DynamicString:    
                            SkDebugf("%s", op->fString->c_str());
                            break;
                        default:
                            break;
                }
                firstElem = false;
            }
            SkDebugf("]\" ");
            continue;
        }
        
        if (info->fType == SkType_String || info->fType == SkType_DynamicString) {
            SkString* string;
            info->getString(this, &string);
            if (string->isEmpty() == false)
                SkDebugf("%s=\"%s\"\t", info->fName, string->c_str()); 
            continue;
        }
        
        
        blankInfo = blankCopy->getMember(index);
        int i = info->fCount;
        info->getValue(this, values, i);
        blankInfo->getValue(blankCopy, blankValues, i);
        dumpValues(info, info->fType, values[0], blankValues[0], values[1], blankValues[1]);
    } while (true);
    delete blankCopy;
}

void SkDisplayable::dumpBase(SkAnimateMaker* maker) {
    SkDisplayTypes type = getType();
    const char* elementName = "(unknown)";
    if (type != SkType_Unknown && type != SkType_Screenplay)
        elementName = SkDisplayType::GetName(maker, type);
    SkDebugf("%*s", SkDisplayList::fIndent, "");
    if (SkDisplayList::fDumpIndex != 0 && SkDisplayList::fIndent == 0)
        SkDebugf("%d: ", SkDisplayList::fDumpIndex);
    SkDebugf("<%s ", elementName);
    if (strcmp(id,"") != 0)
        SkDebugf("id=\"%s\" ", id);
}

void SkDisplayable::dumpChildren(SkAnimateMaker* maker, bool closedAngle) {
    
    int index = -1;
    const SkMemberInfo* info;
    index = -1;
    SkDisplayList::fIndent += 4;
    do {
        info = this->getMember(++index);
        if (NULL == info) {
            break;
        }
        if (SkDisplayType::IsDisplayable(maker, info->fType)) {
            SkDisplayable** displayable = (SkDisplayable**) info->memberData(this);
            if (*displayable == NULL || *displayable == (SkDisplayable*) -1)
                continue;
            if (closedAngle == false) {
                SkDebugf(">\n");
                closedAngle = true;
            }
            (*displayable)->dump(maker);
        }
    } while (true);
    SkDisplayList::fIndent -= 4;
    if (closedAngle)
        dumpEnd(maker);
    else
        SkDebugf("/>\n");
}

void SkDisplayable::dumpEnd(SkAnimateMaker* maker) {
    SkDisplayTypes type = getType();
    const char* elementName = "(unknown)";
    if (type != SkType_Unknown && type != SkType_Screenplay)
        elementName = SkDisplayType::GetName(maker, type);
    SkDebugf("%*s", SkDisplayList::fIndent, "");
    SkDebugf("</%s>\n", elementName);
}

void SkDisplayable::dumpEvents() {
}

void SkDisplayable::dumpValues(const SkMemberInfo* info, SkDisplayTypes type, SkOperand op, SkOperand blankOp,
    SkOperand op2, SkOperand blankOp2) {
    switch (type) {
    case SkType_BitmapEncoding:
        switch (op.fS32) {
            case 0 : SkDebugf("type=\"jpeg\" ");
                break;
            case 1 : SkDebugf("type=\"png\" ");
                break;
            default: SkDebugf("type=\"UNDEFINED\" ");
        }
        break;
    //should make this a separate case in dump attrs, rather than make dump values have a larger signature
    case SkType_Point:
        if (op.fScalar != blankOp.fScalar || op2.fScalar != blankOp.fScalar) {
#ifdef SK_CAN_USE_FLOAT
            SkDebugf("%s=\"[%g,%g]\" ", info->fName, SkScalarToFloat(op.fScalar), SkScalarToFloat(op2.fScalar));
#else
            SkDebugf("%s=\"[%x,%x]\" ", info->fName, op.fScalar, op2.fScalar);
#endif
        }
        break;
    case SkType_FromPathMode:
        switch (op.fS32) {
            case 0:
                //don't want to print anything for 0, just adding it to remove it from default:
                break;
            case 1:
                SkDebugf("%s=\"%s\" ", info->fName, "angle");
                break;
            case 2:
                SkDebugf("%s=\"%s\" ", info->fName, "position");
                break;
            default:
                SkDebugf("%s=\"INVALID\" ", info->fName);
        }
        break;
    case SkType_MaskFilterBlurStyle:
        switch (op.fS32) {
            case 0:
                break;
            case 1:
                SkDebugf("%s=\"%s\" ", info->fName, "solid");
                break;
            case 2:
                SkDebugf("%s=\"%s\" ", info->fName, "outer");
                break;
            case 3:
                SkDebugf("%s=\"%s\" ", info->fName, "inner");
                break;
            default:
                SkDebugf("%s=\"INVALID\" ", info->fName);
        }
        break;
    case SkType_FilterType:
        if (op.fS32 == 1)
            SkDebugf("%s=\"%s\" ", info->fName, "bilinear");
        break;
    case SkType_PathDirection:
        SkDebugf("%s=\"%s\" ", info->fName, op.fS32 == 0 ? "cw" : "ccw");
        break;
    case SkType_FillType:
        SkDebugf("%s=\"%s\" ", info->fName, op.fS32 == 0 ? "winding" : "evenOdd");
        break;
    case SkType_TileMode:
        //correct to look at the S32?
        if (op.fS32 != blankOp.fS32) 
            SkDebugf("%s=\"%s\" ", info->fName, op.fS32 == 0 ? "clamp" : op.fS32 == 1 ? "repeat" : "mirror");
        break;
    case SkType_Boolean:
        if (op.fS32 != blankOp.fS32)
            SkDebugf("%s=\"%s\" ", info->fName, op.fS32 == 0 ? "false" : "true");
        break;
    case SkType_Int:
        if (op.fS32 != blankOp.fS32)
            SkDebugf(" %s=\"%d\"  ", info->fName, op.fS32);
        break;
    case SkType_Float:
        if (op.fScalar != blankOp.fScalar) { //or /65536?
#ifdef SK_CAN_USE_FLOAT
            SkDebugf("%s=\"%g\"  ", info->fName, SkScalarToFloat(op.fScalar));
#else
            SkDebugf("%s=\"%x\"  ", info->fName, op.fScalar);
#endif
        }
        break;
    case SkType_String:
    case SkType_DynamicString:
        if (op.fString->size() > 0) 
            SkDebugf("%s=\"%s\" ", info->fName, op.fString->c_str());
        break;
    case SkType_MSec:
        if (op.fS32 != blankOp.fS32) {
#ifdef SK_CAN_USE_FLOAT
            SkDebugf(" %s=\"%g\"  ", info->fName, SkScalarToFloat(SkScalarDiv(op.fS32, 1000)));
#else
            SkDebugf(" %s=\"%x\"  ", info->fName, SkScalarDiv(op.fS32, 1000));
#endif
        }
    default:
        SkDebugf("");
    }    
}

#endif

bool SkDisplayable::enable( SkAnimateMaker& ) { 
    return false;
}

void SkDisplayable::enableBounder() {
}

void SkDisplayable::executeFunction(SkDisplayable* , int index, 
        SkTDArray<SkScriptValue>& , SkDisplayTypes, SkScriptValue*  ) {
    SkASSERT(0); 
}

void SkDisplayable::executeFunction(SkDisplayable* target, 
        const SkMemberInfo* info, SkTypedArray* values, SkScriptValue* value) {
    SkTDArray<SkScriptValue> typedValues;
    for (SkOperand* op = values->begin(); op < values->end(); op++) {
        SkScriptValue temp;
        temp.fType = values->getType();
        temp.fOperand = *op;
        *typedValues.append() = temp;
    }
    executeFunction(target, info->functionIndex(), typedValues, info->getType(), value);
}

void SkDisplayable::executeFunction2(SkDisplayable* , int index, 
        SkOpArray* params, SkDisplayTypes, SkOperand2*  ) {
    SkASSERT(0); 
}

void SkDisplayable::getBounds(SkRect* rect) {
    SkASSERT(rect);
    rect->fLeft = rect->fTop = SK_ScalarMax;
    rect->fRight= rect->fBottom = -SK_ScalarMax;
}

const SkFunctionParamType* SkDisplayable::getFunctionsParameters() {
    return NULL;
}

const SkMemberInfo* SkDisplayable::getMember(int index) { 
    return NULL; 
}

const SkMemberInfo* SkDisplayable::getMember(const char name[]) { 
    return NULL; 
}

const SkFunctionParamType* SkDisplayable::getParameters(const SkMemberInfo* info, 
        int* paramCount) {
    const SkFunctionParamType* params = getFunctionsParameters();
    SkASSERT(params != NULL);
    int funcIndex = info->functionIndex();
    // !!! eventually break traversing params into an external function (maybe this whole function)
    int index = funcIndex;
    int offset = 0;
    while (--index >= 0) {
        while (params[offset] != 0)
            offset++;
        offset++;
    }
    int count = 0;
    while (params[offset] != 0) {
        count++;
        offset++;
    }
    *paramCount = count;
    return &params[offset - count];
}

SkDisplayable* SkDisplayable::getParent() const {
    return NULL;
}

bool SkDisplayable::getProperty(int index, SkScriptValue* ) const {
//  SkASSERT(0); 
    return false; 
}

bool SkDisplayable::getProperty2(int index, SkOperand2* value) const {
    SkASSERT(0); 
    return false; 
}

SkDisplayTypes SkDisplayable::getType() const { 
    return SkType_Unknown; 
}

bool SkDisplayable::hasEnable() const {
    return false;
}

bool SkDisplayable::isDrawable() const {
    return false; 
}

void SkDisplayable::onEndElement(SkAnimateMaker& ) {}

const SkMemberInfo* SkDisplayable::preferredChild(SkDisplayTypes type) {
    return NULL;
}

bool SkDisplayable::resolveIDs(SkAnimateMaker& maker, SkDisplayable* original, SkApply* apply) {
    return false;
}

//SkDisplayable* SkDisplayable::resolveTarget(SkAnimateMaker& ) { 
//  return this; 
//}

void SkDisplayable::setChildHasID() {
}

bool SkDisplayable::setParent(SkDisplayable* ) {
    return false;
}

bool SkDisplayable::setProperty(int index, SkScriptValue& ) {
    //SkASSERT(0); 
    return false; 
}

void SkDisplayable::setReference(const SkMemberInfo* info, SkDisplayable* displayable) {
    if (info->fType == SkType_MemberProperty) {
        SkScriptValue scriptValue;
        scriptValue.fOperand.fDisplayable = displayable;
        scriptValue.fType = displayable->getType();
        setProperty(info->propertyIndex(), scriptValue);
    } else if (info->fType == SkType_Array) {
        SkASSERT(displayable->getType() == SkType_Array);
        SkDisplayArray* dispArray = (SkDisplayArray*) displayable;
        SkTDScalarArray* array = (SkTDScalarArray* ) info->memberData(this);
        array->setCount(dispArray->values.count());
        memcpy(array->begin(), dispArray->values.begin(), dispArray->values.count() * sizeof(int));
        //

        // !!! need a way for interpreter engine to own array
        // !!! probably need to replace all scriptable arrays with single bigger array
        // that has operand and type on every element -- or
        // when array is dirtied, need to get parent to reparse to local array
    } else {
        void* storage = info->memberData(this);
        memcpy(storage, &displayable, sizeof(SkDisplayable*));
    }
// !!! unclear why displayable is dirtied here
// if this is called, this breaks fromPath.xml
//  displayable->dirty();
}

#ifdef SK_DEBUG
void SkDisplayable::validate() {
}
#endif


