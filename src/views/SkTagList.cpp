/* libs/graphics/views/SkTagList.cpp
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

#include "SkTagList.h"

SkTagList::~SkTagList()
{
}

SkTagList* SkTagList::Find(SkTagList* rec, U8CPU tag)
{
    SkASSERT(tag < kSkTagListCount);

    while (rec != NULL)
    {
        if (rec->fTag == tag)
            break;
        rec = rec->fNext;
    }
    return rec;
}

void SkTagList::DeleteTag(SkTagList** head, U8CPU tag)
{
    SkASSERT(tag < kSkTagListCount);

    SkTagList* rec = *head;
    SkTagList* prev = NULL;

    while (rec != NULL)
    {
        SkTagList* next = rec->fNext;

        if (rec->fTag == tag)
        {
            if (prev)
                prev->fNext = next;
            else
                *head = next;
            delete rec;
            break;
        }
        prev = rec;
        rec = next;
    }
}

void SkTagList::DeleteAll(SkTagList* rec)
{
    while (rec)
    {
        SkTagList* next = rec->fNext;
        delete rec;
        rec = next;
    }
}

