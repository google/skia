
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrTLList_DEFINED
#define GrTLList_DEFINED

#include "GrNoncopyable.h"

template <typename T> class GrTLList : GrNoncopyable {
public:
    class Entry {
        Entry*  fPrev;
        Entry*  fNext;
    };

    GrTLList() : fHead(NULL), fTail(NULL) {}
#if GR_DEBUG
    ~GrTLList() {
        GrAssert(NULL == fHead);
        GrAssert(NULL == ftail);
    }
#endif

    T*  head() const { return fHead; }
    T*  tail() const { return fTail; }

    void addToHead(T*);
    void addToTail(T*);
    void removeFromList(T*);

private:
    Entry*  fHead;
    Entry*  fTail;

    friend class Entry;
};


class Parent {
    GrTDLList<Child>    fList;
};

class Child : public GrTLList::Entry<Child> {
};

#endif

