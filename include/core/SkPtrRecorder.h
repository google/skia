/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkPtrRecorder_DEFINED
#define SkPtrRecorder_DEFINED

#include "SkRefCnt.h"
#include "SkTDArray.h"

/**
 *  Maintains a set of ptrs, assigning each a unique ID [1...N]. Duplicate ptrs
 *  return the same ID (since its a set). Subclasses can override inPtr()
 *  and decPtr(). incPtr() is called each time a unique ptr is added ot the
 *  set. decPtr() is called on each ptr when the set is destroyed or reset.
 */
class SkPtrRecorder : public SkRefCnt {
public:
    /**
     *  Add the specified ptr to the set, returning a unique 32bit ID for it
     *  [1...N]. Duplicate ptrs will return the same ID.
     *
     *  If the ptr is NULL, it is not recorded, and 0 is returned.
     */
    uint32_t recordPtr(void*);
    
    /**
     *  Return the number of (non-null) ptrs in the set.
     */
    int count() const { return fList.count(); }

    /**
     *  Copy the ptrs in the set into the specified array (allocated by the
     *  caller). The ptrs are assgined to the array based on their corresponding
     *  ID. e.g. array[ptr.ID - 1] = ptr.
     *
     *  incPtr() and decPtr() are not called during this operation.
     */
    void getPtrs(void* array[]) const;

    /**
     *  Call decPtr() on each ptr in the set, and the reset the size of the set
     *  to 0.
     */
    void reset();

protected:
    virtual void incPtr(void* ptr) {}
    virtual void decPtr(void* ptr) {}

private:
    struct Pair {
        void*       fPtr;   // never NULL
        uint32_t    fIndex; // 1...N
    };

    // we store the ptrs in sorted-order (using Cmp) so that we can efficiently
    // detect duplicates when recordPtr() is called. Hence we need to store the
    // ptr and its ID/fIndex explicitly, since the ptr's position in the array
    // is not related to its "index".
    SkTDArray<Pair>  fList;
    
    static int Cmp(const Pair& a, const Pair& b);
    
    typedef SkRefCnt INHERITED;
};

#endif
