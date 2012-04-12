
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOrderedReadBuffer.h"
#include "SkTypeface.h"


SkOrderedReadBuffer::SkOrderedReadBuffer(const void* data, size_t size)
        : INHERITED() {
    fReader.setMemory(data, size);
}

SkTypeface* SkOrderedReadBuffer::readTypeface() {
    uint32_t index = fReader.readU32();
    if (0 == index || index > (unsigned)fTFCount) {
        if (index) {
            SkDebugf("====== typeface index %d\n", index);
        }
        return NULL;
    } else {
        SkASSERT(fTFArray);
        return fTFArray[index - 1];
    }
}

SkRefCnt* SkOrderedReadBuffer::readRefCnt() {
    uint32_t index = fReader.readU32();
    if (0 == index || index > (unsigned)fRCCount) {
        return NULL;
    } else {
        SkASSERT(fRCArray);
        return fRCArray[index - 1];
    }
}

SkFlattenable* SkOrderedReadBuffer::readFlattenable() {
    SkFlattenable::Factory factory = NULL;

    if (fFactoryCount > 0) {
        int32_t index = fReader.readU32();
        if (0 == index) {
            return NULL; // writer failed to give us the flattenable
        }
        index = -index; // we stored the negative of the index
        index -= 1;     // we stored the index-base-1
        SkASSERT(index < fFactoryCount);
        factory = fFactoryArray[index];
    } else if (fFactoryTDArray) {
        const int32_t* peek = (const int32_t*)fReader.peek();
        if (*peek <= 0) {
            int32_t index = fReader.readU32();
            if (0 == index) {
                return NULL; // writer failed to give us the flattenable
            }
            index = -index; // we stored the negative of the index
            index -= 1;     // we stored the index-base-1
            factory = (*fFactoryTDArray)[index];
        } else {
            const char* name = fReader.readString();
            factory = SkFlattenable::NameToFactory(name);
            if (factory) {
                SkASSERT(fFactoryTDArray->find(factory) < 0);
                *fFactoryTDArray->append() = factory;
            } else {
//                SkDebugf("can't find factory for [%s]\n", name);
            }
            // if we didn't find a factory, that's our failure, not the writer's,
            // so we fall through, so we can skip the sizeRecorded data.
        }
    } else {
        factory = (SkFlattenable::Factory)readFunctionPtr();
        if (NULL == factory) {
            return NULL; // writer failed to give us the flattenable
        }
    }

    // if we get here, factory may still be null, but if that is the case, the
    // failure was ours, not the writer.
    SkFlattenable* obj = NULL;
    uint32_t sizeRecorded = fReader.readU32();
    if (factory) {
        uint32_t offset = fReader.offset();
        obj = (*factory)(*this);
        // check that we read the amount we expected
        uint32_t sizeRead = fReader.offset() - offset;
        if (sizeRecorded != sizeRead) {
            // we could try to fix up the offset...
            sk_throw();
        }
    } else {
        // we must skip the remaining data
        fReader.skip(sizeRecorded);
    }
    return obj;
}

void* SkOrderedReadBuffer::readFunctionPtr() {
    void* proc;
    fReader.read(&proc, sizeof(proc));
    return proc;
}
