
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayEvent.h"
#include "SkAnimateMaker.h"
#include "SkDisplayApply.h"
#include "SkDisplayInput.h"
#include "SkDisplayList.h"
#ifdef SK_DEBUG
#include "SkDump.h"
#endif
#include "SkEvent.h"
#include "SkDisplayInput.h"
#include "SkKey.h"
#include "SkMetaData.h"
#include "SkScript.h"
#include "SkUtils.h"

enum SkDisplayEvent_Properties {
    SK_PROPERTY(key),
    SK_PROPERTY(keys)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkDisplayEvent::fInfo[] = {
    SK_MEMBER(code, EventCode),
    SK_MEMBER(disable, Boolean),
    SK_MEMBER_PROPERTY(key, String), // a single key (also last key pressed)
    SK_MEMBER_PROPERTY(keys, String), // a single key or dash-delimited range of keys
    SK_MEMBER(kind, EventKind),
    SK_MEMBER(target, String),
    SK_MEMBER(x, Float),
    SK_MEMBER(y, Float)
};

#endif

DEFINE_GET_MEMBER(SkDisplayEvent);

SkDisplayEvent::SkDisplayEvent() : code((SkKey) -1), disable(false),
    kind(kUser), x(0), y(0), fLastCode((SkKey) -1), fMax((SkKey) -1), fTarget(NULL) {
}

SkDisplayEvent::~SkDisplayEvent() {
    deleteMembers();
}

bool SkDisplayEvent::add(SkAnimateMaker& , SkDisplayable* child) { 
    *fChildren.append() = child; 
    return true; 
}

bool SkDisplayEvent::contains(SkDisplayable* match) {
    for (int index = 0; index < fChildren.count(); index++) {
        if (fChildren[index] == match || fChildren[index]->contains(match))
            return true;
    }
    return false;
}

SkDisplayable* SkDisplayEvent::contains(const SkString& match) {
    for (int index = 0; index < fChildren.count(); index++) {
        SkDisplayable* child = fChildren[index];
        if (child->contains(match))
            return child;
    }
    return NULL;
}

void SkDisplayEvent::deleteMembers() {
    for (int index = 0; index < fChildren.count(); index++) {
        SkDisplayable* evt = fChildren[index];
        delete evt;
    }
}

#ifdef SK_DUMP_ENABLED
void SkDisplayEvent::dumpEvent(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkString str;
    SkDump::GetEnumString(SkType_EventKind, kind, &str);
    SkDebugf("kind=\"%s\" ", str.c_str());
    if (kind == SkDisplayEvent::kKeyPress || kind == SkDisplayEvent::kKeyPressUp) {
        if (code >= 0)
            SkDump::GetEnumString(SkType_EventCode, code, &str);
        else
            str.set("none");
        SkDebugf("code=\"%s\" ", str.c_str());
    }
    if (kind == SkDisplayEvent::kKeyChar) {
        if (fMax != (SkKey) -1 && fMax != code)
            SkDebugf("keys=\"%c - %c\" ", code, fMax);
        else
            SkDebugf("key=\"%c\" ", code);
    }
    if (fTarget != NULL) {
        SkDebugf("target=\"%s\" ", fTarget->id);
    }
    if (kind >= SkDisplayEvent::kMouseDown && kind <= SkDisplayEvent::kMouseUp) {
#ifdef SK_CAN_USE_FLOAT
        SkDebugf("x=\"%g\" y=\"%g\" ", SkScalarToFloat(x), SkScalarToFloat(y));
#else
        SkDebugf("x=\"%x\" y=\"%x\" ", x, y);
#endif
    }
    if (disable)
        SkDebugf("disable=\"true\" ");
    SkDebugf("/>\n");
}
#endif

bool SkDisplayEvent::enableEvent(SkAnimateMaker& maker) 
{
    maker.fActiveEvent = this;
    if (fChildren.count() == 0)
        return false;
    if (disable)
        return false;
#ifdef SK_DUMP_ENABLED
    if (maker.fDumpEvents) {
        SkDebugf("enable: ");
        dumpEvent(&maker);
    }
#endif
    SkDisplayList& displayList = maker.fDisplayList;
    for (int index = 0; index < fChildren.count(); index++) {
        SkDisplayable* displayable = fChildren[index];
        if (displayable->isGroup()) {
            SkTDDrawableArray* parentList = displayList.getDrawList();
            *parentList->append() = (SkDrawable*) displayable;  // make it findable before children are enabled
        }
        if (displayable->enable(maker))
            continue;
        if (maker.hasError()) 
            return true;
        if (displayable->isDrawable() == false)
            return true;    // error
        SkDrawable* drawable = (SkDrawable*) displayable;
        SkTDDrawableArray* parentList = displayList.getDrawList();
        *parentList->append() = drawable;
    }
    return false;
}

bool SkDisplayEvent::getProperty(int index, SkScriptValue* value) const {
    switch (index) {
        case SK_PROPERTY(key):
        case SK_PROPERTY(keys): {
            value->fType = SkType_String;
            char scratch[8];
            SkKey convert = index == SK_PROPERTY(keys) ? code : fLastCode;
            size_t size = convert > 0 ? SkUTF8_FromUnichar(convert, scratch) : 0;
            fKeyString.set(scratch, size);
            value->fOperand.fString = &fKeyString;
            if (index != SK_PROPERTY(keys) || fMax == (SkKey) -1 || fMax == code)
                break;
            value->fOperand.fString->append("-");
            size = SkUTF8_FromUnichar(fMax, scratch);
            value->fOperand.fString->append(scratch, size);
            } break;
        default:
            SkASSERT(0);
            return false;
    }
    return true;
}

void SkDisplayEvent::onEndElement(SkAnimateMaker& maker)
{
    if (kind == kUser)
        return;
    maker.fEvents.addEvent(this);
    if (kind == kOnEnd) {
        bool found = maker.find(target.c_str(), &fTarget);
        SkASSERT(found);
        SkASSERT(fTarget && fTarget->isAnimate());
        SkAnimateBase* animate = (SkAnimateBase*) fTarget;
        animate->setHasEndEvent();
    }
}

void SkDisplayEvent::populateInput(SkAnimateMaker& maker, const SkEvent& fEvent) {
    const SkMetaData& meta = fEvent.getMetaData();
    SkMetaData::Iter iter(meta);
    SkMetaData::Type    type;
    int number;
    const char* name;
    while ((name = iter.next(&type, &number)) != NULL) {
        if (name[0] == '\0')
            continue;
        SkDisplayable* displayable;
        SkInput* input;
        for (int index = 0; index < fChildren.count(); index++) {
            displayable = fChildren[index];
            if (displayable->getType() != SkType_Input)
                continue;
            input = (SkInput*) displayable;
            if (input->name.equals(name))
                goto found;
        }
        if (!maker.find(name, &displayable) || displayable->getType() != SkType_Input)
            continue;
        input = (SkInput*) displayable;
    found:
        switch (type) {
            case SkMetaData::kS32_Type:
                meta.findS32(name, &input->fInt);
                break;
            case SkMetaData::kScalar_Type:
                meta.findScalar(name, &input->fFloat);
                break;
            case SkMetaData::kPtr_Type:
                SkASSERT(0);
                break; // !!! not handled for now
            case SkMetaData::kString_Type:
                input->string.set(meta.findString(name));
                break;
            default:
                SkASSERT(0);
        }
    }
    // re-evaluate all animators that may have built their values from input strings
    for (SkDisplayable** childPtr = fChildren.begin(); childPtr < fChildren.end(); childPtr++) {
        SkDisplayable* displayable = *childPtr;
        if (displayable->isApply() == false)
            continue;
        SkApply* apply = (SkApply*) displayable;
        apply->refresh(maker);
    }
}

bool SkDisplayEvent::setProperty(int index, SkScriptValue& value) {
    SkASSERT(index == SK_PROPERTY(key) || index == SK_PROPERTY(keys));
    SkASSERT(value.fType == SkType_String);
    SkString* string = value.fOperand.fString;
    const char* chars = string->c_str();
    int count = SkUTF8_CountUnichars(chars);
    SkASSERT(count >= 1);
    code = (SkKey) SkUTF8_NextUnichar(&chars);
    fMax = code;
    SkASSERT(count == 1 || index == SK_PROPERTY(keys));
    if (--count > 0) {
        SkASSERT(*chars == '-');
        chars++;
        fMax = (SkKey) SkUTF8_NextUnichar(&chars);
        SkASSERT(fMax >= code);
    }
    return true;
}

#ifdef SK_BUILD_FOR_ANDROID

#include "SkMetaData.h"
#include "SkParse.h"
#include "SkTextBox.h"
#include "SkXMLWriter.h"

void SkMetaData::setPtr(char const*, void*, PtrProc ) {}
void SkMetaData::setS32(char const*, int ) {}
bool SkEventSink::doEvent(SkEvent const& ) { return false; }
bool SkXMLParser::parse(SkStream& ) { return false; }
SkXMLParserError::SkXMLParserError( ) {}
void SkEvent::setType(char const*, size_t ) {}
void SkEvent::postTime(SkMSec) {}
SkEvent::SkEvent(char const*, SkEventSinkID) {}
SkEvent::SkEvent(SkEvent const&) {}
SkEvent::SkEvent( ) {}
SkEvent::~SkEvent( ) {}
bool SkEventSink::onQuery(SkEvent* ) { return false; }
SkEventSink::SkEventSink( ) {}
SkEventSink::~SkEventSink( ) {}
bool SkXMLParser::parse(char const*, size_t ) { return false; }
bool SkXMLParser::parse(SkDOM const&, SkDOMNode const* ) { return false; }
//void SkParse::UnitTest( ) {}
const char* SkMetaData::findString(char const* ) const {return 0;}
bool SkMetaData::findPtr(char const*, void**, PtrProc* ) const {return false;}
bool SkMetaData::findS32(char const*, int* ) const {return false;}
bool SkEvent::isType(char const*, size_t ) const { return false; }
void SkMetaData::setString(char const*, char const* ) {}
const char* SkParse::FindNamedColor(char const*, size_t, SkColor* ) {return false; }
const char* SkMetaData::Iter::next(SkMetaData::Type*, int* ) { return false; }
SkMetaData::Iter::Iter(SkMetaData const& ) {}
bool SkMetaData::findScalar(char const*, SkScalar* ) const {return false;}
void SkMetaData::reset( ) {}
void SkEvent::setType(SkString const& ) {}
bool SkMetaData::findBool(char const*, bool* ) const {return false;}
void SkEvent::getType(SkString*) const {}
bool SkXMLParser::endElement(char const* ) { return false; }
bool SkXMLParser::addAttribute(char const*, char const* ) { return false;}
bool SkXMLParser::startElement(char const* ) { return false;}
bool SkXMLParser::text(char const*, int ) { return false;}
bool SkXMLParser::onText(char const*, int ) { return false;}
SkXMLParser::SkXMLParser(SkXMLParserError* ) {}
SkXMLParser::~SkXMLParser( ) {}
SkXMLParserError::~SkXMLParserError( ) {}
void SkXMLParserError::getErrorString(SkString*) const {}
void SkTextBox::setSpacing(SkScalar, SkScalar ) {}
void SkTextBox::setSpacingAlign(SkTextBox::SpacingAlign ) {}
void SkTextBox::draw(SkCanvas*, char const*, size_t, SkPaint const& ) {}
void SkTextBox::setBox(SkRect const& ) {}
void SkTextBox::setMode(SkTextBox::Mode ) {}
SkTextBox::SkTextBox( ) {}
void SkMetaData::setScalar(char const*, SkScalar ) {}
const char* SkParse::FindScalar(char const*, SkScalar* ) {return 0; }
const char* SkParse::FindScalars(char const*, SkScalar*, int ) {return 0; }
const char* SkParse::FindHex(char const*, unsigned int* ) {return 0; }
const char* SkParse::FindS32(char const*, int* ) {return 0; }
void SkXMLWriter::addAttribute(char const*, char const* ) {}
void SkXMLWriter::startElement(char const* ) {}
void SkXMLWriter::doEnd(SkXMLWriter::Elem* ) {}
SkXMLWriter::Elem* SkXMLWriter::getEnd( ) { return 0; }
bool SkXMLWriter::doStart(char const*, size_t ) { return false; }
SkXMLWriter::SkXMLWriter(bool ) {}
SkXMLWriter::~SkXMLWriter( ) {}
SkMetaData::SkMetaData() {}
SkMetaData::~SkMetaData() {}
bool SkEventSink::onEvent(SkEvent const&) {return false;}
bool SkXMLParser::onEndElement(char const*) {return false;}
bool SkXMLParser::onAddAttribute(char const*, char const*) {return false;}
bool SkXMLParser::onStartElement(char const*) {return false;}
void SkXMLWriter::writeHeader() {}

#endif
