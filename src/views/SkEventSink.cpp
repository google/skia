/* libs/graphics/views/SkEventSink.cpp
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

#include "SkEventSink.h"
#include "SkTagList.h"
#include "SkThread.h"

#include "SkGlobals.h"
#include "SkThread.h"
#include "SkTime.h"

#define SK_EventSink_GlobalsTag     SkSetFourByteTag('e', 'v', 's', 'k')

class SkEventSink_Globals : public SkGlobals::Rec {
public:
    SkMutex         fSinkMutex;
    SkEventSinkID   fNextSinkID;
    SkEventSink*    fSinkHead;
};

static SkGlobals::Rec* create_globals()
{
    SkEventSink_Globals* rec = new SkEventSink_Globals;
    rec->fNextSinkID = 0;
    rec->fSinkHead = NULL;
    return rec;
}

SkEventSink::SkEventSink() : fTagHead(NULL)
{
    SkEventSink_Globals& globals = *(SkEventSink_Globals*)SkGlobals::Find(SK_EventSink_GlobalsTag, create_globals);

    globals.fSinkMutex.acquire();

    fID = ++globals.fNextSinkID;
    fNextSink = globals.fSinkHead;
    globals.fSinkHead = this;

    globals.fSinkMutex.release();
}

SkEventSink::~SkEventSink()
{
    SkEventSink_Globals& globals = *(SkEventSink_Globals*)SkGlobals::Find(SK_EventSink_GlobalsTag, create_globals);

    if (fTagHead)
        SkTagList::DeleteAll(fTagHead);

    globals.fSinkMutex.acquire();

    SkEventSink* sink = globals.fSinkHead;
    SkEventSink* prev = NULL;

    for (;;)
    {
        SkEventSink* next = sink->fNextSink;
        if (sink == this)
        {
            if (prev)
                prev->fNextSink = next;
            else
                globals.fSinkHead = next;
            break;
        }
        prev = sink;
        sink = next;
    }
    globals.fSinkMutex.release();
}

bool SkEventSink::doEvent(const SkEvent& evt)
{
    return this->onEvent(evt);
}

bool SkEventSink::doQuery(SkEvent* evt)
{
    SkASSERT(evt);
    return this->onQuery(evt);
}

bool SkEventSink::onEvent(const SkEvent&)
{
    return false;
}

bool SkEventSink::onQuery(SkEvent*)
{
    return false;
}

///////////////////////////////////////////////////////////////////////////////

SkTagList* SkEventSink::findTagList(U8CPU tag) const
{
    return fTagHead ? SkTagList::Find(fTagHead, tag) : NULL;
}

void SkEventSink::addTagList(SkTagList* rec)
{
    SkASSERT(rec);
    SkASSERT(fTagHead == NULL || SkTagList::Find(fTagHead, rec->fTag) == NULL);

    rec->fNext = fTagHead;
    fTagHead = rec;
}

void SkEventSink::removeTagList(U8CPU tag)
{
    if (fTagHead)
        SkTagList::DeleteTag(&fTagHead, tag);
}

///////////////////////////////////////////////////////////////////////////////

struct SkListenersTagList : SkTagList {
    SkListenersTagList(U16CPU count) : SkTagList(kListeners_SkTagList)
    {
        fExtra16 = SkToU16(count);
        fIDs = (SkEventSinkID*)sk_malloc_throw(count * sizeof(SkEventSinkID));
    }
    virtual ~SkListenersTagList()
    {
        sk_free(fIDs);
    }

    int countListners() const { return fExtra16; }

    int find(SkEventSinkID id) const
    {
        const SkEventSinkID* idptr = fIDs;
        for (int i = fExtra16 - 1; i >= 0; --i)
            if (idptr[i] == id)
                return i;
        return -1;
    }

    SkEventSinkID*  fIDs;
};

void SkEventSink::addListenerID(SkEventSinkID id)
{
    if (id == 0)
        return;

    SkListenersTagList* prev = (SkListenersTagList*)this->findTagList(kListeners_SkTagList);
    int                 count = 0;

    if (prev)
    {
        if (prev->find(id) >= 0)
            return;
        count = prev->countListners();
    }

    SkListenersTagList* next = SkNEW_ARGS(SkListenersTagList, (count + 1));

    if (prev)
    {
        memcpy(next->fIDs, prev->fIDs, count * sizeof(SkEventSinkID));
        this->removeTagList(kListeners_SkTagList);
    }
    next->fIDs[count] = id;
    this->addTagList(next);
}

void SkEventSink::copyListeners(const SkEventSink& sink) 
{
    SkListenersTagList* sinkList = (SkListenersTagList*)sink.findTagList(kListeners_SkTagList);
    if (sinkList == NULL)
        return;
    SkASSERT(sinkList->countListners() > 0);
    const SkEventSinkID* iter = sinkList->fIDs;
    const SkEventSinkID* stop = iter + sinkList->countListners();
    while (iter < stop)
        addListenerID(*iter++);
}

void SkEventSink::removeListenerID(SkEventSinkID id)
{
    if (id == 0)
        return;

    SkListenersTagList* list = (SkListenersTagList*)this->findTagList(kListeners_SkTagList);

    if (list == NULL)
        return;

    int index = list->find(id);
    if (index >= 0)
    {
        int count = list->countListners();
        SkASSERT(count > 0);
        if (count == 1)
            this->removeTagList(kListeners_SkTagList);
        else
        {
            // overwrite without resize/reallocating our struct (for speed)
            list->fIDs[index] = list->fIDs[count - 1];
            list->fExtra16 = SkToU16(count - 1);
        }
    }
}

bool SkEventSink::hasListeners() const
{
    return this->findTagList(kListeners_SkTagList) != NULL;
}

void SkEventSink::postToListeners(const SkEvent& evt, SkMSec delay)
{
    SkListenersTagList* list = (SkListenersTagList*)this->findTagList(kListeners_SkTagList);
    if (list)
    {
        SkASSERT(list->countListners() > 0);
        const SkEventSinkID* iter = list->fIDs;
        const SkEventSinkID* stop = iter + list->countListners();
        while (iter < stop)
            (SkNEW_ARGS(SkEvent, (evt)))->post(*iter++, delay);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkEventSink::EventResult SkEventSink::DoEvent(const SkEvent& evt, SkEventSinkID sinkID)
{
    SkEventSink* sink = SkEventSink::FindSink(sinkID);

    if (sink)
    {
#ifdef SK_DEBUG
        if (evt.isDebugTrace())
        {
            SkString    etype;
            evt.getType(&etype);
            SkDebugf("SkEventTrace: dispatching event <%s> to 0x%x", etype.c_str(), sinkID);
            const char* idStr = evt.findString("id");
            if (idStr)
                SkDebugf(" (%s)", idStr);
            SkDebugf("\n");
        }
#endif
        return sink->doEvent(evt) ? kHandled_EventResult : kNotHandled_EventResult;
    }
    else
    {
#ifdef SK_DEBUG
        if (sinkID)
            SkDebugf("DoEvent: Can't find sink for ID(%x)\n", sinkID);
        else
            SkDebugf("Event sent to 0 sinkID\n");

        if (evt.isDebugTrace())
        {
            SkString    etype;
            evt.getType(&etype);
            SkDebugf("SkEventTrace: eventsink not found <%s> for 0x%x\n", etype.c_str(), sinkID);
        }
#endif
        return kSinkNotFound_EventResult;
    }
}

SkEventSink* SkEventSink::FindSink(SkEventSinkID sinkID)
{
    if (sinkID == 0)
        return 0;

    SkEventSink_Globals&    globals = *(SkEventSink_Globals*)SkGlobals::Find(SK_EventSink_GlobalsTag, create_globals);
    SkAutoMutexAcquire      ac(globals.fSinkMutex);
    SkEventSink*            sink = globals.fSinkHead;

    while (sink)
    {
        if (sink->getSinkID() == sinkID)
            return sink;
        sink = sink->fNextSink;
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#if 0   // experimental, not tested

#include "SkThread.h"
#include "SkTDict.h"

#define kMinStringBufferSize    128
static SkMutex                  gNamedSinkMutex;
static SkTDict<SkEventSinkID>   gNamedSinkIDs(kMinStringBufferSize);

/** Register a name/id pair with the system. If the name already exists,
    replace its ID with the new id. This pair will persist until UnregisterNamedSink()
    is called.
*/
void SkEventSink::RegisterNamedSinkID(const char name[], SkEventSinkID id)
{
    if (id && name && *name)
    {
        SkAutoMutexAcquire  ac(gNamedSinkMutex);
        gNamedSinkIDs.set(name, id);
    }
}

/** Return the id that matches the specified name (from a previous call to
    RegisterNamedSinkID(). If no match is found, return 0
*/
SkEventSinkID SkEventSink::FindNamedSinkID(const char name[])
{
    SkEventSinkID id = 0;

    if (name && *name)
    {
        SkAutoMutexAcquire  ac(gNamedSinkMutex);
        (void)gNamedSinkIDs.find(name, &id);
    }
    return id;
}

/** Remove all name/id pairs from the system. This is call internally
    on shutdown, to ensure no memory leaks. It should not be called
    before shutdown.
*/
void SkEventSink::RemoveAllNamedSinkIDs()
{
    SkAutoMutexAcquire  ac(gNamedSinkMutex);
    (void)gNamedSinkIDs.reset();
}
#endif
