/* libs/graphics/sgl/SkGlobals.cpp
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

#include "SkGlobals.h"
#include "SkThread.h"

SkGlobals::Rec::~Rec()
{
}

SkGlobals::Rec* SkGlobals::Find(uint32_t tag, Rec* (*create_proc)())
{
    SkGlobals::BootStrap&   bootstrap = SkGlobals::GetBootStrap();

    Rec* rec = bootstrap.fHead;
    while (rec)
    {
        if (rec->fTag == tag)
            return rec;
        rec = rec->fNext;
    }

    if (create_proc == NULL) // no create proc, just return not found
        return NULL;

    // if we get here, we may need to create one. First grab the mutex, and
    // search again, creating one if its not found the 2nd time.

    bootstrap.fMutex.acquire();

    // search again, now that we have the mutex. Odds are it won't be there, but we check again
    // just in case it was added by another thread before we grabbed the mutex

    Rec*& head = bootstrap.fHead;
    rec = head;
    while (rec)
    {
        if (rec->fTag == tag)
            break;
        rec = rec->fNext;
    }

    if (rec == NULL && (rec = create_proc()) != NULL)
    {
        rec->fTag = tag;
        rec->fNext = head;
        bootstrap.fHead = rec;
    }

    bootstrap.fMutex.release();
    return rec;
}

void SkGlobals::Init()
{
}

void SkGlobals::Term()
{
    SkGlobals::BootStrap&   bootstrap = SkGlobals::GetBootStrap();

    bootstrap.fMutex.acquire();

    Rec*&   head = bootstrap.fHead;
    Rec*    rec = head;

    while (rec)
    {
        Rec* next = rec->fNext;
        SkDELETE(rec);
        rec = next;
    }

    bootstrap.fHead = NULL;
    bootstrap.fMutex.release();
}


