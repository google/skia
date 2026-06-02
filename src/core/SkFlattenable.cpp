/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkFlattenable.h"

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkSharedMutex.h"
#include "src/core/SkPtrRecorder.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <utility>

SkNamedFactorySet::SkNamedFactorySet() : fNextAddedFactory(0) {}

uint32_t SkNamedFactorySet::find(SkFlattenable::Factory factory) {
    uint32_t index = fFactorySet.find(factory);
    if (index > 0) {
        return index;
    }
    const char* name = SkFlattenable::FactoryToName(factory);
    if (nullptr == name) {
        return 0;
    }
    *fNames.append() = name;
    return fFactorySet.add(factory);
}

const char* SkNamedFactorySet::getNextAddedFactoryName() {
    if (fNextAddedFactory < fNames.size()) {
        return fNames[fNextAddedFactory++];
    }
    return nullptr;
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

namespace {

struct Entry {
    const char*             fName;
    SkFlattenable::Factory  fFactory;
};

struct EntryComparator {
    bool operator()(const Entry& a, const Entry& b) const {
        return strcmp(a.fName, b.fName) < 0;
    }
    bool operator()(const Entry& a, const char* b) const {
        return strcmp(a.fName, b) < 0;
    }
    bool operator()(const char* a, const Entry& b) const {
        return strcmp(a, b.fName) < 0;
    }
};

int gCount = 0;
Entry gEntries[128] = {};
SkSharedMutex gEntriesMutex;

}  // namespace

void SkFlattenable::Register(const char name[], Factory factory) {
    SkAutoSharedMutexExclusive lock(gEntriesMutex);
    SkASSERT(name);
    SkASSERT(factory);
    SkASSERT(gCount < (int)std::size(gEntries));

    /**
     * We add the new Entry to gEntries using a sorted insertion
     *
     * We first find the position at which we must insert the new Entry (insertion_element =
     * std::upper_bound). Then we add our new Entry to gEntries[gCount]
     *
     * Then we call std::rotate(first, middle, last) to place our new Entry at the beginning of
     * insertion_element
     * - first will be the beginning of the elements we want to shift
     * - middle will be the new Entry we want to insert, the things before it will get shifted
     * - last will be the element after the one we are inserting
     *
     * As an example, let's insert name = 'dragonfruit' into
     * gEntries = ['apple', 'blueberry', 'coconut', 'elderberry', 'fig'] with
     * gCount = 5
     *
     * After upper_bound, insertion_element points to 'elderberry' (the first element that comes
     * after 'dragonfruit')
     *
     * After gEntries[gCount] = name, gEntries + gCount points to 'dragonfruit'
     *
     * now we have gEntries = ['apple', 'blueberry', 'coconut', 'elderberry', 'fig', 'dragonfruit']
     *
     * so std::rotate('elderberry', 'dragonfruit', 'dragonfruit'+1); will result in
     * gEntries = ['apple', 'blueberry', 'coconut', 'dragonfruit', 'elderberry', 'fig']
     *
     * This will effectively move the new Entry to the insertion_element position, while maintaining
     * the order of the other elements
     */
    Entry* insertion_element =
            std::upper_bound(gEntries, gEntries + gCount, name, EntryComparator());
    gEntries[gCount].fName = name;
    gEntries[gCount].fFactory = factory;

    std::rotate(insertion_element, gEntries + gCount, gEntries + gCount + 1);
    gCount += 1;
}

SkFlattenable::Factory SkFlattenable::NameToFactory(const char name[]) {
    SkAutoSharedMutexShared lock(gEntriesMutex);
    RegisterFlattenablesIfNeeded();

    SkASSERT(std::is_sorted(gEntries, gEntries + gCount, EntryComparator()));
    auto pair = std::equal_range(gEntries, gEntries + gCount, name, EntryComparator());
    if (pair.first == pair.second) {
        return nullptr;
    }
    return pair.first->fFactory;
}

const char* SkFlattenable::FactoryToName(Factory fact) {
    SkAutoSharedMutexShared lock(gEntriesMutex);
    RegisterFlattenablesIfNeeded();

    const Entry* entries = gEntries;
    for (int i = gCount - 1; i >= 0; --i) {
        if (entries[i].fFactory == fact) {
            return entries[i].fName;
        }
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkData> SkFlattenable::serialize(const SkSerialProcs* procs) const {
    SkSerialProcs p;
    if (procs) {
        p = *procs;
    }
    SkBinaryWriteBuffer writer(p);

    writer.writeFlattenable(this);
    size_t size = writer.bytesWritten();
    auto data = SkData::MakeUninitialized(size);
    writer.writeToMemory(data->writable_data());
    return data;
}

size_t SkFlattenable::serialize(void* memory, size_t memory_size,
                                const SkSerialProcs* procs) const {
    SkSerialProcs p;
    if (procs) {
        p = *procs;
    }
    SkBinaryWriteBuffer writer(memory, memory_size, p);
    writer.writeFlattenable(this);
    return writer.usingInitialStorage() ? writer.bytesWritten() : 0u;
}

sk_sp<SkFlattenable> SkFlattenable::Deserialize(SkFlattenable::Type type, const void* data,
                                                size_t size, const SkDeserialProcs* procs) {
    SkReadBuffer buffer(data, size);
    if (procs) {
        buffer.setDeserialProcs(*procs);
    }
    return sk_sp<SkFlattenable>(buffer.readFlattenable(type));
}
