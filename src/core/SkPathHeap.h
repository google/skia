
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathHeap_DEFINED
#define SkPathHeap_DEFINED

#include "SkRefCnt.h"
#include "SkChunkAlloc.h"
#include "SkTDArray.h"

class SkPath;
class SkReadBuffer;
class SkWriteBuffer;

class SkPathHeap : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkPathHeap)

    SkPathHeap();
    SkPathHeap(SkReadBuffer&);
    virtual ~SkPathHeap();

    /** Copy the path into the heap, and return the new total number of paths.
        Thus, the returned value will be index+1, where index is the index of
        this newly added (copied) path.
     */
    int append(const SkPath&);

    /** Add the specified path to the heap using its gen ID to de-duplicate.
        Returns the path's index in the heap + 1.
     */
    int insert(const SkPath&);

    // called during picture-playback
    int count() const { return fPaths.count(); }
    const SkPath& operator[](int index) const {
        return *fPaths[index];
    }

    void flatten(SkWriteBuffer&) const;

private:
    // we store the paths in the heap (placement new)
    SkChunkAlloc        fHeap;
    // we just store ptrs into fHeap here
    SkTDArray<SkPath*>  fPaths;

    class LookupEntry {
    public:
        LookupEntry(const SkPath& path);

        int storageSlot() const { return fStorageSlot; }
        void setStorageSlot(int storageSlot) { fStorageSlot = storageSlot; }

        static bool Less(const LookupEntry& a, const LookupEntry& b) {
            return a.fGenerationID < b.fGenerationID;
        }

    private:
        uint32_t fGenerationID;     // the SkPath's generation ID
        // the path's index in the heap + 1. It is 0 if the path is not yet in the heap.
        int      fStorageSlot;
    };

    SkTDArray<LookupEntry> fLookupTable;

    SkPathHeap::LookupEntry* addIfNotPresent(const SkPath& path);

    typedef SkRefCnt INHERITED;
};

#endif
