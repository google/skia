/* include/graphics/SkOSMenu.h
**
** Copyright 2006, Google Inc.
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

#ifndef SkOSMenu_DEFINED
#define SkOSMenu_DEFINED

#include "SkEvent.h"
#include "SkTDArray.h"

class SkOSMenu {
public:
    explicit SkOSMenu(const char title[]);
    ~SkOSMenu();

    const char* getTitle() const { return fTitle; }

    void    appendItem(const char title[], const char eventType[], S32 eventData);

    // called by SkOSWindow when it receives an OS menu event
    int     countItems() const;
    const char* getItem(int index, U32* cmdID) const;

    SkEvent* createEvent(U32 os_cmd);

private:
    const char* fTitle;

    struct Item {
        const char* fTitle;
        const char* fEventType;
        U32         fEventData;
        U32         fOSCmd; // internal
    };
    SkTDArray<Item> fItems;

    // illegal
    SkOSMenu(const SkOSMenu&);
    SkOSMenu& operator=(const SkOSMenu&);
};

#endif

