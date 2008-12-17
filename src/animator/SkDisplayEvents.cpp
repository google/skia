/* libs/graphics/animator/SkDisplayEvents.cpp
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

#include "SkDisplayEvents.h"
#include "SkAnimateMaker.h"
#include "SkAnimator.h"
#include "SkDisplayEvent.h"
#include "SkDisplayMovie.h"
#include "SkDrawable.h"
#ifdef SK_DEBUG
#include "SkDump.h"
#endif

SkEventState::SkEventState() : fCode(0), fDisable(false), fDisplayable(0), fX(0), fY(0) {
}

SkEvents::SkEvents() {
}

SkEvents::~SkEvents() {
}

bool SkEvents::doEvent(SkAnimateMaker& maker, SkDisplayEvent::Kind kind, SkEventState* state) {
/*#ifdef SK_DUMP_ENABLED
    if (maker.fDumpEvents) {
        SkDebugf("doEvent: ");
        SkString str;
        SkDump::GetEnumString(SkType_EventKind, kind, &str);
        SkDebugf("kind=%s ", str.c_str());
        if (state && state->fDisplayable)
            state->fDisplayable->SkDisplayable::dump(&maker);
        else
            SkDebugf("\n");
    }
#endif*/
    bool handled = false;
    SkDisplayable** firstMovie = maker.fMovies.begin();
    SkDisplayable** endMovie = maker.fMovies.end();
    for (SkDisplayable** ptr = firstMovie; ptr < endMovie; ptr++) {
        SkDisplayMovie* movie = (SkDisplayMovie*) *ptr;
        if (kind != SkDisplayEvent::kOnload)
            movie->doEvent(kind, state);
    }
    SkDisplayable* displayable = state ? state->fDisplayable : NULL;
    int keyCode = state ? state->fCode : 0;
    int count = fEvents.count();
    for (int index = 0; index < count; index++) {
        SkDisplayEvent* evt = fEvents[index];
        if (evt->disable)
            continue;
        if (evt->kind != kind)
            continue;
        if (evt->code != (SkKey) -1) {
            if ((int) evt->code > keyCode || (int) (evt->fMax != (SkKey) -1 ? evt->fMax : evt->code) < keyCode)
                continue;
            evt->fLastCode = (SkKey) keyCode;
        }
        if (evt->fTarget != NULL && evt->fTarget != displayable)
            continue;
        if (state == NULL || state->fDisable == 0) {
            if (kind >= SkDisplayEvent::kMouseDown && kind <= SkDisplayEvent::kMouseUp) {
                evt->x = state->fX;
                evt->y = state->fY;
            }
            if (evt->enableEvent(maker))
                fError = true;
        }
        handled = true;
    }
    return handled;
}

#ifdef SK_DUMP_ENABLED
void SkEvents::dump(SkAnimateMaker& maker) {
    int index;
    SkTDDrawableArray& drawArray = maker.fDisplayList.fDrawList;
    int count = drawArray.count();
    for (index = 0; index < count; index++) {
        SkDrawable* drawable = drawArray[index];
        drawable->dumpEvents(); 
    }
    count = fEvents.count();
    for (index = 0; index < count; index++) {
        SkDisplayEvent* evt = fEvents[index];
        evt->dumpEvent(&maker);
    }
}
#endif

// currently this only removes onLoad events
void SkEvents::removeEvent(SkDisplayEvent::Kind kind, SkEventState* state) {
    int keyCode = state ? state->fCode : 0;
    SkDisplayable* displayable = state ? state->fDisplayable : NULL;
    for (SkDisplayEvent** evtPtr = fEvents.begin(); evtPtr < fEvents.end(); evtPtr++) {
        SkDisplayEvent* evt = *evtPtr;
        if (evt->kind != kind)
            continue;
        if (evt->code != (SkKey) -1) {
            if ((int) evt->code > keyCode || (int) (evt->fMax != (SkKey) -1 ? evt->fMax : evt->code) < keyCode)
                continue;
        }
        if (evt->fTarget != NULL && evt->fTarget != displayable)
            continue;
        int index = fEvents.find(evt);
        fEvents.remove(index);
    }
}
