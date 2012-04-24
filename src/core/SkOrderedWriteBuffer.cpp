
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOrderedWriteBuffer.h"
#include "SkTypeface.h"

SkOrderedWriteBuffer::SkOrderedWriteBuffer(size_t minSize) :
    INHERITED(),
    fWriter(minSize) {
}

SkOrderedWriteBuffer::SkOrderedWriteBuffer(size_t minSize, void* storage, size_t storageSize) :
    INHERITED(),
    fWriter(minSize, storage, storageSize) {
}

void SkOrderedWriteBuffer::writeFlattenable(SkFlattenable* flattenable) {
    /*
     *  If we have a factoryset, then the first 32bits tell us...
     *       0: failure to write the flattenable
     *      <0: we store the negative of the (1-based) index
     *      >0: the length of the name
     *  If we don't have a factoryset, then the first "ptr" is either the
     *  factory, or null for failure.
     *
     *  The distinction is important, since 0-index is 32bits (always), but a
     *  0-functionptr might be 32 or 64 bits.
     */

    SkFlattenable::Factory factory = NULL;
    if (flattenable) {
        factory = flattenable->getFactory();
    }
    if (NULL == factory) {
        if (fFactorySet) {
            this->write32(0);
        } else {
            this->writeFunctionPtr(NULL);
        }
        return;
    }

    /*
     *  We can write 1 of 3 versions of the flattenable:
     *  1.  function-ptr : this is the fastest for the reader, but assumes that
     *      the writer and reader are in the same process.
     *  2.  index into fFactorySet : This is assumes the writer will later
     *      resolve the function-ptrs into strings for its reader. SkPicture
     *      does exactly this, by writing a table of names (matching the indices)
     *      up front in its serialized form.
     *  3.  names : Reuse fFactorySet to store indices, but only after we've
     *      written the name the first time. SkGPipe uses this technique, as it
     *      doesn't require the reader to be told to know the table of names
     *      up front.
     */
    if (fFactorySet) {
        if (this->inlineFactoryNames()) {
            int index = fFactorySet->find(factory);
            if (index) {
                // we write the negative of the index, to distinguish it from
                // the length of a string
                this->write32(-index);
            } else {
                const char* name = SkFlattenable::FactoryToName(factory);
                if (NULL == name) {
                    this->write32(0);
                    return;
                }
                this->writeString(name);
                index = fFactorySet->add(factory);
            }
        } else {
            // we write the negative of the index, to distinguish it from
            // the length of a string
            this->write32(-(int)fFactorySet->add(factory));
        }
    } else {
        this->writeFunctionPtr((void*)factory);
    }

    // make room for the size of the flatttened object
    (void)this->reserve(sizeof(uint32_t));
    // record the current size, so we can subtract after the object writes.
    uint32_t offset = this->size();
    // now flatten the object
    flattenObject(flattenable, *this);
    uint32_t objSize = this->size() - offset;
    // record the obj's size
    *fWriter.peek32(offset - sizeof(uint32_t)) = objSize;
}

void SkOrderedWriteBuffer::writeFunctionPtr(void* proc) {
    *(void**)this->reserve(sizeof(void*)) = proc;
}
