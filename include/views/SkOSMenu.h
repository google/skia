
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

    void    appendItem(const char title[], const char eventType[], int32_t eventData);

    // called by SkOSWindow when it receives an OS menu event
    int         countItems() const;
    const char* getItem(int index, uint32_t* cmdID) const;

    SkEvent* createEvent(uint32_t os_cmd);

private:
    const char* fTitle;

    struct Item {
        const char* fTitle;
        const char* fEventType;
        uint32_t    fEventData;
        uint32_t    fOSCmd; // internal
    };
    SkTDArray<Item> fItems;

    // illegal
    SkOSMenu(const SkOSMenu&);
    SkOSMenu& operator=(const SkOSMenu&);
};

#endif

