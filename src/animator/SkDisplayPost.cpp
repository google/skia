
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDisplayPost.h"
#include "SkAnimateMaker.h"
#include "SkAnimator.h"
#include "SkDisplayMovie.h"
#include "SkPostParts.h"
#include "SkScript.h"
#ifdef SK_DEBUG
#include "SkDump.h"
#include "SkTime.h"
#endif

enum SkPost_Properties {
    SK_PROPERTY(target),
    SK_PROPERTY(type)
};

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkPost::fInfo[] = {
    SK_MEMBER(delay, MSec),
//  SK_MEMBER(initialized, Boolean),
    SK_MEMBER(mode, EventMode),
    SK_MEMBER(sink, String),
    SK_MEMBER_PROPERTY(target, String),
    SK_MEMBER_PROPERTY(type, String)
};

#endif

DEFINE_GET_MEMBER(SkPost);

SkPost::SkPost() : delay(0), /*initialized(SkBool(-1)), */ mode(kImmediate), fMaker(NULL),
    fSinkID(0), fTargetMaker(NULL), fChildHasID(false), fDirty(false) {
}

SkPost::~SkPost() {
    for (SkDataInput** part = fParts.begin(); part < fParts.end();  part++)
        delete *part;
}

bool SkPost::addChild(SkAnimateMaker& , SkDisplayable* child) {
    SkASSERT(child && child->isDataInput());
    SkDataInput* part = (SkDataInput*) child;
    *fParts.append() = part;
    return true;
}

bool SkPost::childrenNeedDisposing() const {
    return false;
}

void SkPost::dirty() {
    fDirty = true;
}

#ifdef SK_DUMP_ENABLED
void SkPost::dump(SkAnimateMaker* maker) {
    dumpBase(maker);
    SkString* eventType = new SkString();
    fEvent.getType(eventType);
    if (eventType->equals("user")) {
        const char* target = fEvent.findString("id");
        SkDebugf("target=\"%s\" ", target);
    }
    else
        SkDebugf("type=\"%s\" ", eventType->c_str());
    delete eventType;

    if (delay > 0) {
        SkDebugf("delay=\"%g\" ", delay * 0.001);
    }
//  if (initialized == false)
//      SkDebugf("(uninitialized) ");
    SkString string;
    SkDump::GetEnumString(SkType_EventMode, mode, &string);
    if (!string.equals("immediate"))
        SkDebugf("mode=\"%s\" ", string.c_str());
    // !!! could enhance this to search through make hierarchy to show name of sink
    if (sink.size() > 0) {
        SkDebugf("sink=\"%s\" sinkID=\"%d\" ", sink.c_str(), fSinkID);
    } else if (fSinkID != maker->getAnimator()->getSinkID() && fSinkID != 0) {
        SkDebugf("sinkID=\"%d\" ", fSinkID);
    }
    const SkMetaData& meta = fEvent.getMetaData();
    SkMetaData::Iter iter(meta);
    SkMetaData::Type    type;
    int number;
    const char* name;
    bool closedYet = false;
    SkDisplayList::fIndent += 4;
    //this seems to work, but kinda hacky
    //for some reason the last part is id, which i don't want
    //and the parts seem to be in the reverse order from the one in which we find the
    //data itself
    //SkDataInput** ptr = fParts.end();
    //SkDataInput* data;
    //const char* ID;
    while ((name = iter.next(&type, &number)) != NULL) {
        //ptr--;
        if (strcmp(name, "id") == 0)
            continue;
        if (closedYet == false) {
            SkDebugf(">\n");
            closedYet = true;
        }
        //data = *ptr;
        //if (data->id)
        //    ID = data->id;
        //else
        //    ID = "";
        SkDebugf("%*s<data name=\"%s\" ", SkDisplayList::fIndent, "", name);
        switch (type) {
            case SkMetaData::kS32_Type: {
                int32_t s32;
                meta.findS32(name, &s32);
                SkDebugf("int=\"%d\" ", s32);
                } break;
            case SkMetaData::kScalar_Type: {
                SkScalar scalar;
                meta.findScalar(name, &scalar);
                SkDebugf("float=\"%g\" ", SkScalarToFloat(scalar));
                } break;
            case SkMetaData::kString_Type:
                SkDebugf("string=\"%s\" ", meta.findString(name));
                break;
            case SkMetaData::kPtr_Type: {//when do we have a pointer
                    void* ptr;
                    meta.findPtr(name, &ptr);
                    SkDebugf("0x%08x ", ptr);
                } break;
            case SkMetaData::kBool_Type: {
                bool boolean;
                meta.findBool(name, &boolean);
                SkDebugf("boolean=\"%s\" ", boolean ? "true " : "false ");
                } break;
            default:
                break;
        }
        SkDebugf("/>\n");
        //ptr++;
/*      perhaps this should only be done in the case of a pointer?
        SkDisplayable* displayable;
        if (maker->find(name, &displayable))
            displayable->dump(maker);
        else
            SkDebugf("\n");*/
    }
    SkDisplayList::fIndent -= 4;
    if (closedYet)
        dumpEnd(maker);
    else
        SkDebugf("/>\n");

}
#endif

bool SkPost::enable(SkAnimateMaker& maker ) {
    if (maker.hasError())
        return true;
    if (fDirty) {
        if (sink.size() > 0)
            findSinkID();
        if (fChildHasID) {
            SkString preserveID(fEvent.findString("id"));
            fEvent.getMetaData().reset();
            if (preserveID.size() > 0)
                fEvent.setString("id", preserveID);
            for (SkDataInput** part = fParts.begin(); part < fParts.end();  part++) {
                if ((*part)->add())
                    maker.setErrorCode(SkDisplayXMLParserError::kErrorAddingDataToPost);
            }
        }
        fDirty = false;
    }
#ifdef SK_DUMP_ENABLED
    if (maker.fDumpPosts) {
        SkDebugf("post enable: ");
        dump(&maker);
    }
#if defined SK_DEBUG_ANIMATION_TIMING
    SkString debugOut;
    SkMSec time = maker.getAppTime();
    debugOut.appendS32(time - maker.fDebugTimeBase);
    debugOut.append(" post id=");
    debugOut.append(_id);
    debugOut.append(" enable=");
    debugOut.appendS32(maker.fEnableTime - maker.fDebugTimeBase);
    debugOut.append(" delay=");
    debugOut.appendS32(delay);
#endif
#endif
//  SkMSec adjustedDelay = maker.adjustDelay(maker.fEnableTime, delay);
    SkMSec futureTime = maker.fEnableTime + delay;
    fEvent.setFast32(futureTime);
#if defined SK_DEBUG && defined SK_DEBUG_ANIMATION_TIMING
    debugOut.append(" future=");
    debugOut.appendS32(futureTime - maker.fDebugTimeBase);
    SkDebugf("%s\n", debugOut.c_str());
#endif
    SkEventSinkID targetID = fSinkID;
    bool isAnimatorEvent = true;
    SkAnimator* anim = maker.getAnimator();
    if (targetID == 0) {
        isAnimatorEvent = fEvent.findString("id") != NULL;
        if (isAnimatorEvent)
            targetID = anim->getSinkID();
        else if (maker.fHostEventSinkID)
            targetID = maker.fHostEventSinkID;
        else
            return true;
    } else
        anim = fTargetMaker->getAnimator();
    if (delay == 0) {
        if (isAnimatorEvent && mode == kImmediate)
            fTargetMaker->doEvent(fEvent);
        else
            anim->onEventPost(new SkEvent(fEvent), targetID);
    } else
        anim->onEventPostTime(new SkEvent(fEvent), targetID, futureTime);
    return true;
}

void SkPost::findSinkID() {
    // get the next delimiter '.' if any
    fTargetMaker = fMaker;
    const char* ch = sink.c_str();
    do {
        const char* end = strchr(ch, '.');
        size_t len = end ? (size_t) (end - ch) : strlen(ch);
        SkDisplayable* displayable = NULL;
        if (SK_LITERAL_STR_EQUAL("parent", ch, len)) {
            if (fTargetMaker->fParentMaker)
                fTargetMaker = fTargetMaker->fParentMaker;
            else {
                fTargetMaker->setErrorCode(SkDisplayXMLParserError::kNoParentAvailable);
                return;
            }
        } else {
            fTargetMaker->find(ch, len, &displayable);
            if (displayable == NULL || displayable->getType() != SkType_Movie) {
                fTargetMaker->setErrorCode(SkDisplayXMLParserError::kExpectedMovie);
                return;
            }
            SkDisplayMovie* movie = (SkDisplayMovie*) displayable;
            fTargetMaker = movie->fMovie.fMaker;
        }
        if (end == NULL)
            break;
        ch = ++end;
    } while (true);
    SkAnimator* anim = fTargetMaker->getAnimator();
    fSinkID = anim->getSinkID();
}

bool SkPost::hasEnable() const {
    return true;
}

void SkPost::onEndElement(SkAnimateMaker& maker) {
    fTargetMaker = fMaker = &maker;
    if (fChildHasID == false) {
        for (SkDataInput** part = fParts.begin(); part < fParts.end();  part++)
            delete *part;
        fParts.reset();
    }
}

void SkPost::setChildHasID() {
    fChildHasID = true;
}

bool SkPost::setProperty(int index, SkScriptValue& value) {
    SkASSERT(value.fType == SkType_String);
    SkString* string = value.fOperand.fString;
    switch(index) {
        case SK_PROPERTY(target): {
            fEvent.setType("user");
            fEvent.setString("id", *string);
            mode = kImmediate;
            } break;
        case SK_PROPERTY(type):
            fEvent.setType(*string);
            break;
        default:
            SkASSERT(0);
            return false;
    }
    return true;
}
