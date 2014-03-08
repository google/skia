/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOffsetTable_DEFINED
#define SkOffsetTable_DEFINED

#include "SkRefCnt.h"
#include "SkTDArray.h"

// A 2D table of skp offsets. Each row is indexed by an int. This is used
// to store the command offsets that reference a particular bitmap using
// the bitmap's index in the bitmap heap as the 'id' here. It has to be
// ref-countable so SkPicturePlayback can take ownership of it.
// Note that this class assumes that the ids are densely packed.

// TODO: This needs to be sped up. We could replace the offset table with
// a hash table.
class SkOffsetTable : public SkRefCnt {
public:
    SkOffsetTable() {}
    ~SkOffsetTable() {
        fOffsetArrays.deleteAll();
    }

    // Record that this 'id' is used by the command starting at this 'offset'.
    // Offsets for a given 'id' should always be added in increasing order.
    void add(int id, size_t offset) {
        if (id >= fOffsetArrays.count()) {
            int oldCount = fOffsetArrays.count();
            fOffsetArrays.setCount(id+1);
            for (int i = oldCount; i <= id; ++i) {
                fOffsetArrays[i] = NULL;
            }
        }

        if (NULL == fOffsetArrays[id]) {
            fOffsetArrays[id] = SkNEW(OffsetArray);
        }
        fOffsetArrays[id]->add(offset);
    }

    int numIDs() const {
        return fOffsetArrays.count();
    }

    // Do the offsets of any commands referencing this ID fall in the
    // range [min, max] (both inclusive)
    bool overlap(int id, size_t min, size_t max) {
        SkASSERT(id < fOffsetArrays.count());

        if (NULL == fOffsetArrays[id]) {
            return false;
        }

        // If this id has an offset array it should have at least one use
        SkASSERT(fOffsetArrays[id]->count() > 0);
        if (max < fOffsetArrays[id]->min() || min > fOffsetArrays[id]->max()) {
            return false;
        }

        return true;
    }

    bool includes(int id, size_t offset) {
        SkASSERT(id < fOffsetArrays.count());

        OffsetArray* array = fOffsetArrays[id];

        for (int i = 0; i < array->fOffsets.count(); ++i) {
            if (array->fOffsets[i] == offset) {
                return true;
            } else if (array->fOffsets[i] > offset) {
                return false;
            }
        }

        // Calls to 'includes' should be gaurded by an overlap() call, so we
        // should always find something.
        SkASSERT(0);
        return false;
    }

protected:
    class OffsetArray {
    public:
        void add(size_t offset) {
            SkASSERT(fOffsets.count() == 0 || offset > this->max());
            *fOffsets.append() = offset;
        }
        size_t min() const {
            SkASSERT(fOffsets.count() > 0);
            return fOffsets[0];
        }
        size_t max() const {
            SkASSERT(fOffsets.count() > 0);
            return fOffsets[fOffsets.count()-1];
        }
        int count() const {
            return fOffsets.count();
        }

        SkTDArray<size_t> fOffsets;
    };

    SkTDArray<OffsetArray*> fOffsetArrays;

private:
    typedef SkRefCnt INHERITED;
};

#endif
