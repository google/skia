/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkFlattenableBuffers.h"

SK_DEFINE_INST_COUNT(SkData)

SkData::SkData(const void* ptr, size_t size, ReleaseProc proc, void* context) {
    fPtr = ptr;
    fSize = size;
    fReleaseProc = proc;
    fReleaseProcContext = context;
}

SkData::~SkData() {
    if (fReleaseProc) {
        fReleaseProc(fPtr, fSize, fReleaseProcContext);
    }
}

bool SkData::equals(const SkData* other) const {
    if (NULL == other) {
        return false;
    }

    return fSize == other->fSize && !memcmp(fPtr, other->fPtr, fSize);
}

size_t SkData::copyRange(size_t offset, size_t length, void* buffer) const {
    size_t available = fSize;
    if (offset >= available || 0 == length) {
        return 0;
    }
    available -= offset;
    if (length > available) {
        length = available;
    }
    SkASSERT(length > 0);

    memcpy(buffer, this->bytes() + offset, length);
    return length;
}

///////////////////////////////////////////////////////////////////////////////

SkData* SkData::NewEmpty() {
    static SkData* gEmptyRef;
    if (NULL == gEmptyRef) {
        gEmptyRef = new SkData(NULL, 0, NULL, NULL);
    }
    gEmptyRef->ref();
    return gEmptyRef;
}

// assumes fPtr was allocated via sk_malloc
static void sk_free_releaseproc(const void* ptr, size_t, void*) {
    sk_free((void*)ptr);
}

SkData* SkData::NewFromMalloc(const void* data, size_t length) {
    return new SkData(data, length, sk_free_releaseproc, NULL);
}

SkData* SkData::NewWithCopy(const void* data, size_t length) {
    if (0 == length) {
        return SkData::NewEmpty();
    }

    void* copy = sk_malloc_throw(length); // balanced in sk_free_releaseproc
    memcpy(copy, data, length);
    return new SkData(copy, length, sk_free_releaseproc, NULL);
}

SkData* SkData::NewWithProc(const void* data, size_t length,
                            ReleaseProc proc, void* context) {
    return new SkData(data, length, proc, context);
}

// assumes context is a SkData
static void sk_dataref_releaseproc(const void*, size_t, void* context) {
    SkData* src = reinterpret_cast<SkData*>(context);
    src->unref();
}

SkData* SkData::NewSubset(const SkData* src, size_t offset, size_t length) {
    /*
        We could, if we wanted/need to, just make a deep copy of src's data,
        rather than referencing it. This would duplicate the storage (of the
        subset amount) but would possibly allow src to go out of scope sooner.
     */

    size_t available = src->size();
    if (offset >= available || 0 == length) {
        return SkData::NewEmpty();
    }
    available -= offset;
    if (length > available) {
        length = available;
    }
    SkASSERT(length > 0);

    src->ref(); // this will be balanced in sk_dataref_releaseproc
    return new SkData(src->bytes() + offset, length, sk_dataref_releaseproc,
                         const_cast<SkData*>(src));
}

SkData* SkData::NewWithCString(const char cstr[]) {
    size_t size;
    if (NULL == cstr) {
        cstr = "";
        size = 1;
    } else {
        size = strlen(cstr) + 1;
    }
    return NewWithCopy(cstr, size);
}

///////////////////////////////////////////////////////////////////////////////

void SkData::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.writeByteArray(fPtr, fSize);
}

SkData::SkData(SkFlattenableReadBuffer& buffer) {
    fSize = buffer.getArrayCount();
    fReleaseProcContext = NULL;

    if (fSize > 0) {
        fPtr = sk_malloc_throw(fSize);
        fReleaseProc = sk_free_releaseproc;
    } else {
        fPtr = NULL;
        fReleaseProc = NULL;
    }

    buffer.readByteArray(const_cast<void*>(fPtr));
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkData)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkDataSet.h"
#include "SkFlattenable.h"
#include "SkStream.h"

static SkData* dupdata(SkData* data) {
    if (data) {
        data->ref();
    } else {
        data = SkData::NewEmpty();
    }
    return data;
}

static SkData* findValue(const char key[], const SkDataSet::Pair array[], int n) {
    for (int i = 0; i < n; ++i) {
        if (!strcmp(key, array[i].fKey)) {
            return array[i].fValue;
        }
    }
    return NULL;
}

static SkDataSet::Pair* allocatePairStorage(int count, size_t storage) {
    size_t size = count * sizeof(SkDataSet::Pair) + storage;
    return (SkDataSet::Pair*)sk_malloc_throw(size);
}

SkDataSet::SkDataSet(const char key[], SkData* value) {
    size_t keyLen = strlen(key);

    fCount = 1;
    fKeySize = keyLen + 1;
    fPairs = allocatePairStorage(1, keyLen + 1);

    fPairs[0].fKey = (char*)(fPairs + 1);
    memcpy(const_cast<char*>(fPairs[0].fKey), key, keyLen + 1);

    fPairs[0].fValue = dupdata(value);
}

SkDataSet::SkDataSet(const Pair array[], int count) {
    if (count < 1) {
        fCount = 0;
        fKeySize = 0;
        fPairs = NULL;
        return;
    }

    int i;
    size_t keySize = 0;
    for (i = 0; i < count; ++i) {
        keySize += strlen(array[i].fKey) + 1;
    }

    Pair* pairs = fPairs = allocatePairStorage(count, keySize);
    char* keyStorage = (char*)(pairs + count);

    keySize = 0;    // reset this, so we can compute the size for unique keys
    int uniqueCount = 0;
    for (int i = 0; i < count; ++i) {
        if (!findValue(array[i].fKey, pairs, uniqueCount)) {
            size_t len = strlen(array[i].fKey);
            memcpy(keyStorage, array[i].fKey, len + 1);
            pairs[uniqueCount].fKey = keyStorage;
            keyStorage += len + 1;
            keySize += len + 1;

            pairs[uniqueCount].fValue = dupdata(array[i].fValue);
            uniqueCount += 1;
        }
    }
    fCount = uniqueCount;
    fKeySize = keySize;
}

SkDataSet::~SkDataSet() {
    for (int i = 0; i < fCount; ++i) {
        fPairs[i].fValue->unref();
    }
    sk_free(fPairs);    // this also frees the key storage
}

SkData* SkDataSet::find(const char key[]) const {
    return findValue(key, fPairs, fCount);
}

void SkDataSet::writeToStream(SkWStream* stream) const {
    stream->write32(fCount);
    if (fCount > 0) {
        stream->write32(fKeySize);
        // our first key points to all the key storage
        stream->write(fPairs[0].fKey, fKeySize);
        for (int i = 0; i < fCount; ++i) {
            stream->writeData(fPairs[i].fValue);
        }
    }
}

void SkDataSet::flatten(SkFlattenableWriteBuffer& buffer) const {
    buffer.writeInt(fCount);
    if (fCount > 0) {
        buffer.writeByteArray(fPairs[0].fKey, fKeySize);
        for (int i = 0; i < fCount; ++i) {
            buffer.writeFlattenable(fPairs[i].fValue);
        }
    }
}

SkDataSet::SkDataSet(SkStream* stream) {
    fCount = stream->readU32();
    if (fCount > 0) {
        fKeySize = stream->readU32();
        fPairs = allocatePairStorage(fCount, fKeySize);
        char* keyStorage = (char*)(fPairs + fCount);

        stream->read(keyStorage, fKeySize);

        for (int i = 0; i < fCount; ++i) {
            fPairs[i].fKey = keyStorage;
            keyStorage += strlen(keyStorage) + 1;
            fPairs[i].fValue = stream->readData();
        }
    } else {
        fKeySize = 0;
        fPairs = NULL;
    }
}

SkDataSet::SkDataSet(SkFlattenableReadBuffer& buffer) {
    fCount = buffer.readInt();
    if (fCount > 0) {
        fKeySize = buffer.getArrayCount();
        fPairs = allocatePairStorage(fCount, fKeySize);
        char* keyStorage = (char*)(fPairs + fCount);

        buffer.readByteArray(keyStorage);

        for (int i = 0; i < fCount; ++i) {
            fPairs[i].fKey = keyStorage;
            keyStorage += strlen(keyStorage) + 1;
            fPairs[i].fValue = buffer.readFlattenableT<SkData>();
        }
    } else {
        fKeySize = 0;
        fPairs = NULL;
    }
}

SkDataSet* SkDataSet::NewEmpty() {
    static SkDataSet* gEmptySet;
    if (NULL == gEmptySet) {
        gEmptySet = SkNEW_ARGS(SkDataSet, (NULL, 0));
    }
    gEmptySet->ref();
    return gEmptySet;
}

SK_DEFINE_FLATTENABLE_REGISTRAR(SkDataSet)
