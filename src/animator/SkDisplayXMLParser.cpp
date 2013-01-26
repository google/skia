
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayXMLParser.h"
#include "SkAnimateMaker.h"
#include "SkDisplayApply.h"
#include "SkUtils.h"
#ifdef SK_DEBUG
#include "SkTime.h"
#endif

static char const* const gErrorStrings[] = {
    "unknown error ",
    "apply scopes itself",
    "display tree too deep (circular reference?) ",
    "element missing parent ",
    "element type not allowed in parent ",
    "error adding <data> to <post> ",
    "error adding to <matrix> ",
    "error adding to <paint> ",
    "error adding to <path> ",
    "error in attribute value ",
    "error in script ",
    "expected movie in sink attribute ",
    "field not in target ",
    "number of offsets in gradient must match number of colors",
    "no offset in gradient may be greater than one",
    "last offset in gradient must be one",
    "offsets in gradient must be increasing",
    "first offset in gradient must be zero",
    "gradient attribute \"points\" must have length of four",
    "in include ",
    "in movie ",
    "include name unknown or missing ",
    "index out of range ",
    "movie name unknown or missing ",
    "no parent available to resolve sink attribute ",
    "parent element can't contain ",
    "saveLayer must specify a bounds",
    "target id not found ",
    "unexpected type "
};

SkDisplayXMLParserError::~SkDisplayXMLParserError() {
}

void SkDisplayXMLParserError::getErrorString(SkString* str) const {
    if (fCode > kUnknownError)
        str->set(gErrorStrings[fCode - kUnknownError]);
    else
        str->reset();
    INHERITED::getErrorString(str);
}

void SkDisplayXMLParserError::setInnerError(SkAnimateMaker* parent, const SkString& src) {
    SkString inner;
    getErrorString(&inner);
    inner.prepend(": ");
    inner.prependS32(getLineNumber());
    inner.prepend(", line ");
    inner.prepend(src);
    parent->setErrorNoun(inner);
}


SkDisplayXMLParser::SkDisplayXMLParser(SkAnimateMaker& maker)
    : INHERITED(&maker.fError), fMaker(maker), fInInclude(maker.fInInclude),
        fInSkia(maker.fInInclude), fCurrDisplayable(NULL)
{
}

SkDisplayXMLParser::~SkDisplayXMLParser() {
    if (fCurrDisplayable && fMaker.fChildren.find(fCurrDisplayable) < 0)
        delete fCurrDisplayable;
    for (Parent* parPtr = fParents.begin() + 1; parPtr < fParents.end(); parPtr++) {
        SkDisplayable* displayable = parPtr->fDisplayable;
        if (displayable == fCurrDisplayable)
            continue;
        SkASSERT(fMaker.fChildren.find(displayable) < 0);
        if (fMaker.fHelpers.find(displayable) < 0)
            delete displayable;
    }
}



bool SkDisplayXMLParser::onAddAttribute(const char name[], const char value[]) {
    return onAddAttributeLen(name, value, strlen(value));
}

bool SkDisplayXMLParser::onAddAttributeLen(const char attrName[], const char attrValue[],
                                        size_t attrValueLen)
{
    if (fCurrDisplayable == NULL)    // this signals we should ignore attributes for this element
        return strncmp(attrName, "xmlns", sizeof("xmlns") - 1) != 0;
    SkDisplayable*  displayable = fCurrDisplayable;
    SkDisplayTypes  type = fCurrType;

    if (strcmp(attrName, "id") == 0) {
        if (fMaker.find(attrValue, attrValueLen, NULL)) {
            fError->setNoun(attrValue, attrValueLen);
            fError->setCode(SkXMLParserError::kDuplicateIDs);
            return true;
        }
#ifdef SK_DEBUG
        displayable->_id.set(attrValue, attrValueLen);
        displayable->id = displayable->_id.c_str();
#endif
        fMaker.idsSet(attrValue, attrValueLen, displayable);
        int parentIndex = fParents.count() - 1;
        if (parentIndex > 0) {
            SkDisplayable* parent = fParents[parentIndex - 1].fDisplayable;
            parent->setChildHasID();
        }
        return false;
    }
    const char* name = attrName;
    const SkMemberInfo* info = SkDisplayType::GetMember(&fMaker, type, &name);
    if (info == NULL) {
        fError->setNoun(name);
        fError->setCode(SkXMLParserError::kUnknownAttributeName);
        return true;
    }
    if (info->setValue(fMaker, NULL, 0, info->getCount(), displayable, info->getType(), attrValue,
            attrValueLen))
        return false;
    if (fMaker.fError.hasError()) {
        fError->setNoun(attrValue, attrValueLen);
        return true;
    }
    SkDisplayable* ref = NULL;
    if (fMaker.find(attrValue, attrValueLen, &ref) == false) {
        ref = fMaker.createInstance(attrValue, attrValueLen);
        if (ref == NULL) {
            fError->setNoun(attrValue, attrValueLen);
            fError->setCode(SkXMLParserError::kErrorInAttributeValue);
            return true;
        } else
            fMaker.helperAdd(ref);
    }
    if (info->fType != SkType_MemberProperty) {
        fError->setNoun(name);
        fError->setCode(SkXMLParserError::kUnknownAttributeName);
        return true;
    }
    SkScriptValue scriptValue;
    scriptValue.fOperand.fDisplayable = ref;
    scriptValue.fType = ref->getType();
    displayable->setProperty(info->propertyIndex(), scriptValue);
    return false;
}

#if defined(SK_BUILD_FOR_WIN32)
    #define SK_strcasecmp   _stricmp
    #define SK_strncasecmp  _strnicmp
#else
    #define SK_strcasecmp   strcasecmp
    #define SK_strncasecmp  strncasecmp
#endif

bool SkDisplayXMLParser::onEndElement(const char elem[])
{
    int parentIndex = fParents.count() - 1;
    if (parentIndex >= 0) {
        Parent& container = fParents[parentIndex];
        SkDisplayable* displayable = container.fDisplayable;
        fMaker.fEndDepth = parentIndex;
        displayable->onEndElement(fMaker);
        if (fMaker.fError.hasError())
            return true;
        if (parentIndex > 0) {
            SkDisplayable* parent = fParents[parentIndex - 1].fDisplayable;
            bool result = parent->addChild(fMaker, displayable);
            if (fMaker.hasError())
                return true;
            if (result == false) {
                int infoCount;
                const SkMemberInfo* info =
                    SkDisplayType::GetMembers(&fMaker, fParents[parentIndex - 1].fType, &infoCount);
                const SkMemberInfo* foundInfo;
                if ((foundInfo = searchContainer(info, infoCount)) != NULL) {
                    parent->setReference(foundInfo, displayable);
        //          if (displayable->isHelper() == false)
                        fMaker.helperAdd(displayable);
                } else {
                    fMaker.setErrorCode(SkDisplayXMLParserError::kElementTypeNotAllowedInParent);
                    return true;
                }
            }
            if (parent->childrenNeedDisposing())
                delete displayable;
        }
        fParents.remove(parentIndex);
    }
    fCurrDisplayable = NULL;
    if (fInInclude == false && SK_strcasecmp(elem, "screenplay") == 0) {
        if (fMaker.fInMovie == false) {
            fMaker.fEnableTime = fMaker.getAppTime();
#if defined SK_DEBUG && defined SK_DEBUG_ANIMATION_TIMING
            if (fMaker.fDebugTimeBase == (SkMSec) -1)
                fMaker.fDebugTimeBase = fMaker.fEnableTime;
            SkString debugOut;
            SkMSec time = fMaker.getAppTime();
            debugOut.appendS32(time - fMaker.fDebugTimeBase);
            debugOut.append(" onLoad enable=");
            debugOut.appendS32(fMaker.fEnableTime - fMaker.fDebugTimeBase);
            SkDebugf("%s\n", debugOut.c_str());
#endif
            fMaker.fEvents.doEvent(fMaker, SkDisplayEvent::kOnload, NULL);
            if (fMaker.fError.hasError())
                return true;
            fMaker.fEvents.removeEvent(SkDisplayEvent::kOnload, NULL);

        }
        fInSkia = false;
    }
    return false;
}

bool SkDisplayXMLParser::onStartElement(const char name[])
{
    return onStartElementLen(name, strlen(name));
}

bool SkDisplayXMLParser::onStartElementLen(const char name[], size_t len) {
    fCurrDisplayable = NULL; // init so we'll ignore attributes if we exit early

    if (SK_strncasecmp(name, "screenplay", len) == 0) {
        fInSkia = true;
        if (fInInclude == false)
            fMaker.idsSet(name, len, &fMaker.fScreenplay);
        return false;
    }
    if (fInSkia == false)
        return false;

    SkDisplayable* displayable = fMaker.createInstance(name, len);
    if (displayable == NULL) {
        fError->setNoun(name, len);
        fError->setCode(SkXMLParserError::kUnknownElement);
        return true;
    }
    SkDisplayTypes type = displayable->getType();
    Parent record = { displayable, type };
    *fParents.append() = record;
    if (fParents.count() == 1)
        fMaker.childrenAdd(displayable);
    else {
        Parent* parent = fParents.end() - 2;
        if (displayable->setParent(parent->fDisplayable)) {
            fError->setNoun(name, len);
            getError()->setCode(SkDisplayXMLParserError::kParentElementCantContain);
            return true;
        }
    }

    // set these for subsequent calls to addAttribute()
    fCurrDisplayable = displayable;
    fCurrType = type;
    return false;
}

const SkMemberInfo* SkDisplayXMLParser::searchContainer(const SkMemberInfo* infoBase,
                                                         int infoCount) {
    const SkMemberInfo* bestDisplayable = NULL;
    const SkMemberInfo* lastResort = NULL;
    for (int index = 0; index < infoCount; index++) {
        const SkMemberInfo* info = &infoBase[index];
        if (info->fType == SkType_BaseClassInfo) {
            const SkMemberInfo* inherited = info->getInherited();
            const SkMemberInfo* result = searchContainer(inherited, info->fCount);
            if (result != NULL)
                return result;
            continue;
        }
        Parent* container = fParents.end() - 1;
        SkDisplayTypes type = (SkDisplayTypes) info->fType;
        if (type == SkType_MemberProperty)
            type = info->propertyType();
        SkDisplayTypes containerType = container->fType;
        if (type == containerType && (type == SkType_Rect || type == SkType_Polygon ||
            type == SkType_Array || type == SkType_Int || type == SkType_Bitmap))
            goto rectNext;
        while (type != containerType) {
            if (containerType == SkType_Displayable)
                goto next;
            containerType = SkDisplayType::GetParent(&fMaker, containerType);
            if (containerType == SkType_Unknown)
                goto next;
        }
        return info;
next:
        if (type == SkType_Drawable || (type == SkType_Displayable &&
            container->fDisplayable->isDrawable())) {
rectNext:
            if (fParents.count() > 1) {
                Parent* parent = fParents.end() - 2;
                if (info == parent->fDisplayable->preferredChild(type))
                    bestDisplayable = info;
                else
                    lastResort = info;
            }
        }
    }
    if (bestDisplayable)
        return bestDisplayable;
    if (lastResort)
        return lastResort;
    return NULL;
}
