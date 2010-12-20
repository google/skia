/* libs/graphics/views/SkEvent.cpp
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

#include "SkEvent.h"

void SkEvent::initialize(const char* type, size_t typeLen) {
    fType = NULL;
    setType(type, typeLen);
    f32 = 0;
#ifdef SK_DEBUG
    fTargetID = 0;
    fTime = 0;
    fNextEvent = NULL;
#endif
    SkDEBUGCODE(fDebugTrace = false;)
}

SkEvent::SkEvent()
{
    initialize("", 0);
}

SkEvent::SkEvent(const SkEvent& src)
{
    *this = src;
    if (((size_t) fType & 1) == 0)
        setType(src.fType);
}

SkEvent::SkEvent(const SkString& type)
{
    initialize(type.c_str(), type.size());
}

SkEvent::SkEvent(const char type[])
{
    SkASSERT(type);
    initialize(type, strlen(type));
}

SkEvent::~SkEvent()
{
    if (((size_t) fType & 1) == 0)
        sk_free((void*) fType);
}

static size_t makeCharArray(char* buffer, size_t compact)
{
    size_t bits = (size_t) compact >> 1;
    memcpy(buffer, &bits, sizeof(compact));
    buffer[sizeof(compact)] = 0;
    return strlen(buffer);
}

#if 0
const char* SkEvent::getType() const 
{ 
    if ((size_t) fType & 1) {   // not a pointer
        char chars[sizeof(size_t) + 1];
        size_t len = makeCharArray(chars, (size_t) fType);
        fType = (char*) sk_malloc_throw(len);
        SkASSERT(((size_t) fType & 1) == 0);
        memcpy(fType, chars, len);
    }
    return fType; 
}
#endif

void SkEvent::getType(SkString* str) const 
{ 
    if (str) 
    {
        if ((size_t) fType & 1) // not a pointer
        {
            char chars[sizeof(size_t) + 1];
            size_t len = makeCharArray(chars, (size_t) fType);
            str->set(chars, len);
        }
        else
            str->set(fType);
    }
}

bool SkEvent::isType(const SkString& str) const 
{
    return this->isType(str.c_str(), str.size()); 
}

bool SkEvent::isType(const char type[], size_t typeLen) const 
{ 
    if (typeLen == 0)
        typeLen = strlen(type);
    if ((size_t) fType & 1) {   // not a pointer
        char chars[sizeof(size_t) + 1];
        size_t len = makeCharArray(chars, (size_t) fType);
        return len == typeLen && strncmp(chars, type, typeLen) == 0;
    }
    return strncmp(fType, type, typeLen) == 0 && fType[typeLen] == 0; 
}

void SkEvent::setType(const char type[], size_t typeLen)
{
    if (typeLen == 0)
        typeLen = strlen(type);
    if (typeLen <= sizeof(fType)) {
        size_t slot = 0;
        memcpy(&slot, type, typeLen);
        if (slot << 1 >> 1 != slot)
            goto useCharStar;
        slot <<= 1;
        slot |= 1;
        fType = (char*) slot;
    } else {
useCharStar:
        fType = (char*) sk_malloc_throw(typeLen + 1);
        SkASSERT(((size_t) fType & 1) == 0);
        memcpy(fType, type, typeLen);
        fType[typeLen] = 0;
    }
}

void SkEvent::setType(const SkString& type)
{
    setType(type.c_str());
}

////////////////////////////////////////////////////////////////////////////

#include "SkParse.h"

void SkEvent::inflate(const SkDOM& dom, const SkDOM::Node* node)
{
    const char* name = dom.findAttr(node, "type");
    if (name)
        this->setType(name);

    const char* value;
    if ((value = dom.findAttr(node, "fast32")) != NULL)
    {
        int32_t n;
        if (SkParse::FindS32(value, &n))
            this->setFast32(n);
    }

    for (node = dom.getFirstChild(node); node; node = dom.getNextSibling(node))
    {
        if (strcmp(dom.getName(node), "data"))
        {
            SkDEBUGCODE(SkDebugf("SkEvent::inflate unrecognized subelement <%s>\n", dom.getName(node));)
            continue;
        }

        name = dom.findAttr(node, "name");
        if (name == NULL)
        {
            SkDEBUGCODE(SkDebugf("SkEvent::inflate missing required \"name\" attribute in <data> subelement\n");)
            continue;
        }

        if ((value = dom.findAttr(node, "s32")) != NULL)
        {
            int32_t n;
            if (SkParse::FindS32(value, &n))
                this->setS32(name, n);
        }
        else if ((value = dom.findAttr(node, "scalar")) != NULL)
        {
            SkScalar x;
            if (SkParse::FindScalar(value, &x))
                this->setScalar(name, x);
        }
        else if ((value = dom.findAttr(node, "string")) != NULL)
            this->setString(name, value);
#ifdef SK_DEBUG
        else
        {
            SkDebugf("SkEvent::inflate <data name=\"%s\"> subelement missing required type attribute [S32 | scalar | string]\n", name);
        }
#endif
    }
}

#ifdef SK_DEBUG

    #ifndef SkScalarToFloat
        #define SkScalarToFloat(x)  ((x) / 65536.f)
    #endif

    void SkEvent::dump(const char title[])
    {
        if (title)
            SkDebugf("%s ", title);
            
        SkString    etype;
        this->getType(&etype);
        SkDebugf("event<%s> fast32=%d", etype.c_str(), this->getFast32());

        const SkMetaData&   md = this->getMetaData();
        SkMetaData::Iter    iter(md);
        SkMetaData::Type    mtype;
        int                 count;
        const char*         name;
        
        while ((name = iter.next(&mtype, &count)) != NULL)
        {
            SkASSERT(count > 0);

            SkDebugf(" <%s>=", name);
            switch (mtype) {
            case SkMetaData::kS32_Type:     // vector version???
                {
                    int32_t value;
                    md.findS32(name, &value);
                    SkDebugf("%d ", value);
                }
                break;
            case SkMetaData::kScalar_Type:
                {
                    const SkScalar* values = md.findScalars(name, &count, NULL);
                    SkDebugf("%f", SkScalarToFloat(values[0]));
                    for (int i = 1; i < count; i++)
                        SkDebugf(", %f", SkScalarToFloat(values[i]));
                    SkDebugf(" ");
                }
                break;
            case SkMetaData::kString_Type:
                {
                    const char* value = md.findString(name);
                    SkASSERT(value);
                    SkDebugf("<%s> ", value);
                }
                break;
            case SkMetaData::kPtr_Type:     // vector version???
                {
                    void*   value;
                    md.findPtr(name, &value);
                    SkDebugf("%p ", value);
                }
                break;
            case SkMetaData::kBool_Type:    // vector version???
                {
                    bool    value;
                    md.findBool(name, &value);
                    SkDebugf("%s ", value ? "true" : "false");
                }
                break;
            default:
                SkASSERT(!"unknown metadata type returned from iterator");
                break;
            }
        }
        SkDebugf("\n");
    }
#endif

///////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
// #define SK_TRACE_EVENTSx
#endif

#ifdef SK_TRACE_EVENTS
    static void event_log(const char s[])
    {
        SkDEBUGF(("%s\n", s));
    }

    #define EVENT_LOG(s)        event_log(s)
    #define EVENT_LOGN(s, n)    do { SkString str(s); str.append(" "); str.appendS32(n); event_log(str.c_str()); } while (0)
#else
    #define EVENT_LOG(s)
    #define EVENT_LOGN(s, n)
#endif

#include "SkGlobals.h"
#include "SkThread.h"
#include "SkTime.h"

#define SK_Event_GlobalsTag     SkSetFourByteTag('e', 'v', 'n', 't')

class SkEvent_Globals : public SkGlobals::Rec {
public:
    SkMutex     fEventMutex;
    SkEvent*    fEventQHead, *fEventQTail;
    SkEvent*    fDelayQHead;
    SkDEBUGCODE(int fEventCounter;)
};

static SkGlobals::Rec* create_globals()
{
    SkEvent_Globals* rec = new SkEvent_Globals;
    rec->fEventQHead = NULL;
    rec->fEventQTail = NULL;
    rec->fDelayQHead = NULL;
    SkDEBUGCODE(rec->fEventCounter = 0;)
    return rec;
}

bool SkEvent::Post(SkEvent* evt, SkEventSinkID sinkID, SkMSec delay)
{
    if (delay)
        return SkEvent::PostTime(evt, sinkID, SkTime::GetMSecs() + delay);

    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);

    evt->fTargetID = sinkID;

#ifdef SK_TRACE_EVENTS
    {
        SkString    str("SkEvent::Post(");
        str.append(evt->getType());
        str.append(", 0x");
        str.appendHex(sinkID);
        str.append(", ");
        str.appendS32(delay);
        str.append(")");
        event_log(str.c_str());
    }
#endif

    globals.fEventMutex.acquire();
    bool wasEmpty = SkEvent::Enqueue(evt);
    globals.fEventMutex.release();

    // call outside of us holding the mutex
    if (wasEmpty)
        SkEvent::SignalNonEmptyQueue();
    return true;
}

#if defined(SK_SIMULATE_FAILED_MALLOC) && defined(SK_FIND_MEMORY_LEAKS)
SkMSec gMaxDrawTime;
#endif

bool SkEvent::PostTime(SkEvent* evt, SkEventSinkID sinkID, SkMSec time)
{
#if defined(SK_SIMULATE_FAILED_MALLOC) && defined(SK_FIND_MEMORY_LEAKS)
    gMaxDrawTime = time;
#endif
    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);

    evt->fTargetID = sinkID;

#ifdef SK_TRACE_EVENTS
    {
        SkString    str("SkEvent::Post(");
        str.append(evt->getType());
        str.append(", 0x");
        str.appendHex(sinkID);
        str.append(", ");
        str.appendS32(time);
        str.append(")");
        event_log(str.c_str());
    }
#endif

    globals.fEventMutex.acquire();
    SkMSec queueDelay = SkEvent::EnqueueTime(evt, time);
    globals.fEventMutex.release();

    // call outside of us holding the mutex
    if ((int32_t)queueDelay != ~0)
        SkEvent::SignalQueueTimer(queueDelay);
    return true;
}

bool SkEvent::Enqueue(SkEvent* evt)
{
    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);
    //  gEventMutex acquired by caller

    SkASSERT(evt);

    bool wasEmpty = globals.fEventQHead == NULL;

    if (globals.fEventQTail)
        globals.fEventQTail->fNextEvent = evt;
    globals.fEventQTail = evt;
    if (globals.fEventQHead == NULL)
        globals.fEventQHead = evt;
    evt->fNextEvent = NULL;

    SkDEBUGCODE(++globals.fEventCounter);
//  SkDebugf("Enqueue: count=%d\n", gEventCounter);

    return wasEmpty;
}

SkEvent* SkEvent::Dequeue(SkEventSinkID* sinkID)
{
    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);
    globals.fEventMutex.acquire();

    SkEvent* evt = globals.fEventQHead;
    if (evt)
    {
        SkDEBUGCODE(--globals.fEventCounter);

        if (sinkID)
            *sinkID = evt->fTargetID;

        globals.fEventQHead = evt->fNextEvent;
        if (globals.fEventQHead == NULL)
            globals.fEventQTail = NULL;
    }
    globals.fEventMutex.release();

//  SkDebugf("Dequeue: count=%d\n", gEventCounter);

    return evt;
}

bool SkEvent::QHasEvents()
{
    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);

    // this is not thread accurate, need a semaphore for that
    return globals.fEventQHead != NULL;
}

#ifdef SK_TRACE_EVENTS
    static int gDelayDepth;
#endif

SkMSec SkEvent::EnqueueTime(SkEvent* evt, SkMSec time)
{
#ifdef SK_TRACE_EVENTS
    SkDebugf("enqueue-delay %s %d (%d)", evt->getType(), time, gDelayDepth);
    const char* idStr = evt->findString("id");
    if (idStr)
        SkDebugf(" (%s)", idStr);
    SkDebugf("\n");
    ++gDelayDepth;
#endif

    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);
    //  gEventMutex acquired by caller

    SkEvent* curr = globals.fDelayQHead;
    SkEvent* prev = NULL;

    while (curr)
    {
        if (SkMSec_LT(time, curr->fTime))
            break;
        prev = curr;
        curr = curr->fNextEvent;
    }

    evt->fTime = time;
    evt->fNextEvent = curr;
    if (prev == NULL)
        globals.fDelayQHead = evt;
    else
        prev->fNextEvent = evt;

    SkMSec delay = globals.fDelayQHead->fTime - SkTime::GetMSecs();
    if ((int32_t)delay <= 0)
        delay = 1;
    return delay;
}

//////////////////////////////////////////////////////////////////////////////

#include "SkEventSink.h"

bool SkEvent::ProcessEvent()
{
    SkEventSinkID   sinkID;
    SkEvent*        evt = SkEvent::Dequeue(&sinkID);
    SkAutoTDelete<SkEvent>  autoDelete(evt);
    bool            again = false;

    EVENT_LOGN("ProcessEvent", (int32_t)evt);

    if (evt)
    {
        (void)SkEventSink::DoEvent(*evt, sinkID);
        again = SkEvent::QHasEvents();
    }
    return again;
}

void SkEvent::ServiceQueueTimer()
{
    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);

    globals.fEventMutex.acquire();

    bool        wasEmpty = false;
    SkMSec      now = SkTime::GetMSecs();
    SkEvent*    evt = globals.fDelayQHead;

    while (evt)
    {
        if (SkMSec_LT(now, evt->fTime))
            break;

#ifdef SK_TRACE_EVENTS
        --gDelayDepth;
        SkDebugf("dequeue-delay %s (%d)", evt->getType(), gDelayDepth);
        const char* idStr = evt->findString("id");
        if (idStr)
            SkDebugf(" (%s)", idStr);
        SkDebugf("\n");
#endif

        SkEvent* next = evt->fNextEvent;
        if (SkEvent::Enqueue(evt))
            wasEmpty = true;
        evt = next;
    }
    globals.fDelayQHead = evt;

    SkMSec time = evt ? evt->fTime - now : 0;

    globals.fEventMutex.release();

    if (wasEmpty)
        SkEvent::SignalNonEmptyQueue();

    SkEvent::SignalQueueTimer(time);
}

int SkEvent::CountEventsOnQueue() {
    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);
    globals.fEventMutex.acquire();
    
    int count = 0;
    const SkEvent* evt = globals.fEventQHead;
    while (evt) {
        count += 1;
        evt = evt->fNextEvent;
    }
    globals.fEventMutex.release();

    return count;
}

////////////////////////////////////////////////////////////////

void SkEvent::Init()
{
}

void SkEvent::Term()
{
    SkEvent_Globals& globals = *(SkEvent_Globals*)SkGlobals::Find(SK_Event_GlobalsTag, create_globals);

    SkEvent* evt = globals.fEventQHead;
    while (evt)
    {
        SkEvent* next = evt->fNextEvent;
        delete evt;
        evt = next;
    }

    evt = globals.fDelayQHead;
    while (evt)
    {
        SkEvent* next = evt->fNextEvent;
        delete evt;
        evt = next;
    }
}

