
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkFlattenable.h"
#include "SkTypeface.h"

///////////////////////////////////////////////////////////////////////////////

void SkFlattenable::flatten(SkFlattenableWriteBuffer&) const
{
    /*  we don't write anything at the moment, but this allows our subclasses
        to not know that, since we want them to always call INHERITED::flatten()
        in their code.
    */
}

///////////////////////////////////////////////////////////////////////////////

SkFlattenableReadBuffer::SkFlattenableReadBuffer() {
    fRCArray = NULL;
    fRCCount = 0;

    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
}

///////////////////////////////////////////////////////////////////////////////

SkFlattenableWriteBuffer::SkFlattenableWriteBuffer() {
    fFlags = (Flags)0;
    fRCSet = NULL;
    fTFSet = NULL;
    fFactorySet = NULL;
}

SkFlattenableWriteBuffer::~SkFlattenableWriteBuffer() {
    SkSafeUnref(fRCSet);
    SkSafeUnref(fTFSet);
    SkSafeUnref(fFactorySet);
}

SkRefCntSet* SkFlattenableWriteBuffer::setRefCntRecorder(SkRefCntSet* rec) {
    SkRefCnt_SafeAssign(fRCSet, rec);
    return rec;
}

SkRefCntSet* SkFlattenableWriteBuffer::setTypefaceRecorder(SkRefCntSet* rec) {
    SkRefCnt_SafeAssign(fTFSet, rec);
    return rec;
}

SkFactorySet* SkFlattenableWriteBuffer::setFactoryRecorder(SkFactorySet* rec) {
    SkRefCnt_SafeAssign(fFactorySet, rec);
    return rec;
}

void SkFlattenableWriteBuffer::writeRefCnt(SkRefCnt* obj) {
    SkASSERT(!isCrossProcess());
    if (NULL == obj || NULL == fRCSet) {
        this->write32(0);
    } else {
        this->write32(fRCSet->add(obj));
    }
}

void SkFlattenableWriteBuffer::writeTypeface(SkTypeface* obj) {
    if (NULL == obj || NULL == fTFSet) {
        this->write32(0);
    } else {
        this->write32(fTFSet->add(obj));
    }
}

///////////////////////////////////////////////////////////////////////////////

SkRefCntSet::~SkRefCntSet() {
    // call this now, while our decPtr() is sill in scope
    this->reset();
}

void SkRefCntSet::incPtr(void* ptr) {
    ((SkRefCnt*)ptr)->ref();
}

void SkRefCntSet::decPtr(void* ptr) {
    ((SkRefCnt*)ptr)->unref();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define MAX_PAIR_COUNT  64

struct Pair {
    const char*             fName;
    SkFlattenable::Factory  fFactory;
};

static int gCount;
static Pair gPairs[MAX_PAIR_COUNT];

void SkFlattenable::Register(const char name[], Factory factory) {
    SkASSERT(name);
    SkASSERT(factory);

    static bool gOnce;
    if (!gOnce) {
        gCount = 0;
        gOnce = true;
    }

    SkASSERT(gCount < MAX_PAIR_COUNT);

    gPairs[gCount].fName = name;
    gPairs[gCount].fFactory = factory;
    gCount += 1;
}

#if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS && defined(SK_DEBUG)
static void report_no_entries(const char* functionName) {
    if (!gCount) {
        SkDebugf("%s has no registered name/factory pairs."
                 " Call SkGraphics::Init() at process initialization time.",
                 functionName);
    }
}
#endif

SkFlattenable::Factory SkFlattenable::NameToFactory(const char name[]) {
#if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS && defined(SK_DEBUG)
    report_no_entries(__FUNCTION__);
#endif
    const Pair* pairs = gPairs;
    for (int i = gCount - 1; i >= 0; --i) {
        if (strcmp(pairs[i].fName, name) == 0) {
            return pairs[i].fFactory;
        }
    }
    return NULL;
}

const char* SkFlattenable::FactoryToName(Factory fact) {
#if !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS && defined(SK_DEBUG)
    report_no_entries(__FUNCTION__);
#endif
    const Pair* pairs = gPairs;
    for (int i = gCount - 1; i >= 0; --i) {
        if (pairs[i].fFactory == fact) {
            return pairs[i].fName;
        }
    }
    return NULL;
}
