/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrTArray_DEFINED
#define GrTArray_DEFINED

#include <new>
#include "GrTypes.h"

// TODO: convert from uint32_t to int.

// DATA_TYPE indicates that T has a trivial cons, destructor
// and can be shallow-copied
template <typename T, bool DATA_TYPE = false> class GrTArray {
public:
    GrTArray() {
        fCount = 0;
        fReserveCount = MIN_ALLOC_COUNT;
        fAllocCount = 0;
        fMemArray = NULL;
        fPreAllocMemArray = NULL;
    }

    GrTArray(uint32_t reserveCount) {
        fCount = 0;
        fReserveCount = GrMax(reserveCount, (uint32_t)MIN_ALLOC_COUNT);
        fAllocCount = fReserveCount;
        fMemArray = GrMalloc(sizeof(T) * fReserveCount);
        fPreAllocMemArray = NULL;
    }

    GrTArray(void* preAllocStorage, uint32_t preAllocCount) {
        // we allow NULL,0 args and revert to the default cons. behavior
        // this makes it possible for a owner-object to use same constructor
        // to get either prealloc or nonprealloc behavior based using same line
        GrAssert((NULL == preAllocStorage) == !preAllocCount);

        fCount              = 0;
        fReserveCount       = preAllocCount > 0 ? preAllocCount :
                                                  MIN_ALLOC_COUNT;
        fAllocCount         = preAllocCount;
        fMemArray           = preAllocStorage;
        fPreAllocMemArray   = preAllocStorage;
    }

    GrTArray(const GrTArray& array) {
        fCount              = array.count();
        fReserveCount       = MIN_ALLOC_COUNT;
        fAllocCount         = GrMax(fReserveCount, fCount);
        fMemArray           = GrMalloc(sizeof(T) * fAllocCount);
        fPreAllocMemArray   = NULL;
        if (DATA_TYPE) {
            memcpy(fMemArray, array.fMemArray, sizeof(T) * fCount);
        } else {
            for (uint32_t i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
    }

    GrTArray(const T* array, uint32_t count) {
        fCount              = count;
        fReserveCount       = MIN_ALLOC_COUNT;
        fAllocCount         = GrMax(fReserveCount, fCount);
        fMemArray           = GrMalloc(sizeof(T) * fAllocCount);
        fPreAllocMemArray   = NULL;
        if (DATA_TYPE) {
            memcpy(fMemArray, array, sizeof(T) * fCount);
        } else {
            for (uint32_t i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
    }

    GrTArray(const GrTArray& array,
             void* preAllocStorage, uint32_t preAllocCount) {

        // for same reason as non-copying cons we allow NULL, 0 for prealloc
        GrAssert((NULL == preAllocStorage) == !preAllocCount);

        fCount              = array.count();
        fReserveCount       = preAllocCount > 0 ? preAllocCount :
                                                  MIN_ALLOC_COUNT;
        fPreAllocMemArray   = preAllocStorage;

        if (fReserveCount >= fCount && preAllocCount) {
            fAllocCount = fReserveCount;
            fMemArray = preAllocStorage;
        } else {
            fAllocCount = GrMax(fCount, fReserveCount);
            fMemArray = GrMalloc(fAllocCount * sizeof(T));
        }

        if (DATA_TYPE) {
            memcpy(fMemArray, array.fMemArray, sizeof(T) * fCount);
        } else {
            for (uint32_t i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
    }

    GrTArray(const T* array, uint32_t count,
             void* preAllocStorage, uint32_t preAllocCount) {

        // for same reason as non-copying cons we allow NULL, 0 for prealloc
        GrAssert((NULL == preAllocStorage) == !preAllocCount);

        fCount              = count;
        fReserveCount       = (preAllocCount > 0) ? preAllocCount :
                                                    MIN_ALLOC_COUNT;
        fPreAllocMemArray   = preAllocStorage;

        if (fReserveCount >= fCount && preAllocCount) {
            fAllocCount = fReserveCount;
            fMemArray = preAllocStorage;
        } else {
            fAllocCount = GrMax(fCount, fReserveCount);
            fMemArray = GrMalloc(fAllocCount * sizeof(T));
        }

        if (DATA_TYPE) {
            memcpy(fMemArray, array, sizeof(T) * fCount);
        } else {
            for (uint32_t i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
    }

    GrTArray& operator =(const GrTArray& array) {
        for (uint32_t i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        fCount = 0;
        checkRealloc((int)array.count());
        fCount = array.count();
        if (DATA_TYPE) {
            memcpy(fMemArray, array.fMemArray, sizeof(T) * fCount);
        } else {
            for (uint32_t i = 0; i < fCount; ++i) {
                new (fItemArray + i) T(array[i]);
            }
        }
        return *this;
    }

    ~GrTArray() {
        for (uint32_t i = 0; i < fCount; ++i) {
            fItemArray[i].~T();
        }
        if (fMemArray != fPreAllocMemArray) {
            GrFree(fMemArray);
        }
    }

    uint32_t count() const { return fCount; }

    bool empty() const { return !fCount; }

    T& push_back() {
        checkRealloc(1);
        new ((char*)fMemArray+sizeof(T)*fCount) T;
        ++fCount;
        return fItemArray[fCount-1];
    }

    void push_back_n(uint32_t n) {
        checkRealloc(n);
        for (uint32_t i = 0; i < n; ++i) {
            new (fItemArray + fCount + i) T;
        }
        fCount += n;
    }

    void pop_back() {
        GrAssert(0 != fCount);
        --fCount;
        fItemArray[fCount].~T();
        checkRealloc(0);
    }

    void pop_back_n(uint32_t n) {
        GrAssert(fCount >= n);
        fCount -= n;
        for (uint32_t i = 0; i < n; ++i) {
            fItemArray[i].~T();
        }
        checkRealloc(0);
    }

    // pushes or pops from the back to resize
    void resize_back(uint32_t newCount) {
        if (newCount > fCount) {
            push_back_n(newCount - fCount);
        } else if (newCount < fCount) {
            pop_back_n(fCount - newCount);
        }
    }

    T& operator[] (uint32_t i) {
        GrAssert(i < fCount);
        return fItemArray[i];
    }

    const T& operator[] (uint32_t i) const {
        GrAssert(i < fCount);
        return fItemArray[i];
    }

    T& front() { GrAssert(fCount); return fItemArray[0];}

    const T& front() const { GrAssert(fCount); return fItemArray[0];}

    T& back() { GrAssert(fCount); return fItemArray[fCount - 1];}

    const T& back() const { GrAssert(fCount); return fItemArray[fCount - 1];}

    T& fromBack(uint32_t i) {
        GrAssert(i < fCount);
        return fItemArray[fCount - i - 1];
    }

    const T& fromBack(uint32_t i) const {
        GrAssert(i < fCount);
        return fItemArray[fCount - i - 1];
    }

private:
    static const uint32_t MIN_ALLOC_COUNT = 8;

    inline void checkRealloc(int32_t delta) {
        GrAssert(-delta <= (int32_t)fCount);

        uint32_t newCount = fCount + delta;
        uint32_t fNewAllocCount = fAllocCount;

        if (newCount > fAllocCount) {
            fNewAllocCount = GrMax(newCount + ((newCount + 1) >> 1),
                                   fReserveCount);
        } else if (newCount < fAllocCount / 3) {
            fNewAllocCount = GrMax(fAllocCount / 2, fReserveCount);
        }

        if (fNewAllocCount != fAllocCount) {

            fAllocCount = fNewAllocCount;
            char* fNewMemArray;

            if (fAllocCount == fReserveCount && NULL != fPreAllocMemArray) {
                fNewMemArray = (char*) fPreAllocMemArray;
            } else {
                fNewMemArray = (char*) GrMalloc(fAllocCount*sizeof(T));
            }

            if (DATA_TYPE) {
                memcpy(fNewMemArray, fMemArray, fCount * sizeof(T));
            } else {
                for (uint32_t i = 0; i < fCount; ++i) {
                    new (fNewMemArray + sizeof(T) * i) T(fItemArray[i]);
                    fItemArray[i].~T();
                }
            }

            if (fMemArray != fPreAllocMemArray) {
                GrFree(fMemArray);
            }
            fMemArray = fNewMemArray;
        }
    }

    uint32_t fReserveCount;
    uint32_t fCount;
    uint32_t fAllocCount;
    void*    fPreAllocMemArray;
    union {
        T*       fItemArray;
        void*    fMemArray;
    };
};

#endif

