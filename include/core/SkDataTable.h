/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDataTable_DEFINED
#define SkDataTable_DEFINED

#include "SkChunkAlloc.h"
#include "SkData.h"
#include "SkString.h"
#include "SkTDArray.h"

/**
 *  Like SkData, SkDataTable holds an immutable data buffer. The data buffer is
 *  organized into a table of entries, each with a length, so the entries are
 *  not required to all be the same size.
 */
class SK_API SkDataTable : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkDataTable)

    /**
     *  Returns true if the table is empty (i.e. has no entries).
     */
    bool isEmpty() const { return 0 == fCount; }

    /**
     *  Return the number of entries in the table. 0 for an empty table
     */
    int count() const { return fCount; }

    /**
     *  Return the size of the index'th entry in the table. The caller must
     *  ensure that index is valid for this table.
     */
    size_t atSize(int index) const;

    /**
     *  Return a pointer to the data of the index'th entry in the table.
     *  The caller must ensure that index is valid for this table.
     *
     *  @param size If non-null, this returns the byte size of this entry. This
     *              will be the same value that atSize(index) would return.
     */
    const void* at(int index, size_t* size = NULL) const;

    template <typename T>
    const T* atT(int index, size_t* size = NULL) const {
        return reinterpret_cast<const T*>(this->at(index, size));
    }

    /**
     *  Returns the index'th entry as a c-string, and assumes that the trailing
     *  null byte had been copied into the table as well.
     */
    const char* atStr(int index) const {
        size_t size;
        const char* str = this->atT<const char>(index, &size);
        SkASSERT(strlen(str) + 1 == size);
        return str;
    }

    typedef void (*FreeProc)(void* context);

    static SkDataTable* NewEmpty();

    /**
     *  Return a new DataTable that contains a copy of the data stored in each
     *  "array".
     *
     *  @param ptrs array of points to each element to be copied into the table.
     *  @param sizes array of byte-lengths for each entry in the corresponding
     *               ptrs[] array.
     *  @param count the number of array elements in ptrs[] and sizes[] to copy.
     */
    static SkDataTable* NewCopyArrays(const void * const * ptrs,
                                      const size_t sizes[], int count);

    /**
     *  Return a new table that contains a copy of the data in array.
     *
     *  @param array contiguous array of data for all elements to be copied.
     *  @param elemSize byte-length for a given element.
     *  @param count the number of entries to be copied out of array. The number
     *               of bytes that will be copied is count * elemSize.
     */
    static SkDataTable* NewCopyArray(const void* array, size_t elemSize,
                                     int count);

    static SkDataTable* NewArrayProc(const void* array, size_t elemSize,
                                     int count, FreeProc proc, void* context);

private:
    struct Dir {
        const void* fPtr;
        uintptr_t   fSize;
    };

    int         fCount;
    size_t      fElemSize;
    union {
        const Dir*  fDir;
        const char* fElems;
    } fU;

    FreeProc    fFreeProc;
    void*       fFreeProcContext;

    SkDataTable();
    SkDataTable(const void* array, size_t elemSize, int count,
                FreeProc, void* context);
    SkDataTable(const Dir*, int count, FreeProc, void* context);
    virtual ~SkDataTable();

    friend class SkDataTableBuilder;    // access to Dir

    typedef SkRefCnt INHERITED;
};

/**
 *  Helper class that allows for incrementally building up the data needed to
 *  create a SkDataTable.
 */
class SK_API SkDataTableBuilder : SkNoncopyable {
public:
    SkDataTableBuilder(size_t minChunkSize);
    ~SkDataTableBuilder();

    int  count() const { return fDir.count(); }
    size_t minChunkSize() const { return fMinChunkSize; }

    /**
     *  Forget any previously appended entries, setting count() back to 0.
     */
    void reset(size_t minChunkSize);
    void reset() {
        this->reset(fMinChunkSize);
    }

    /**
     *  Copy size-bytes from data, and append it to the growing SkDataTable.
     */
    void append(const void* data, size_t size);

    /**
     *  Helper version of append() passes strlen() + 1 for the size,
     *  so the trailing-zero will be copied as well.
     */
    void appendStr(const char str[]) {
        this->append(str, strlen(str) + 1);
    }

    /**
     *  Helper version of append() passes string.size() + 1 for the size,
     *  so the trailing-zero will be copied as well.
     */
    void appendString(const SkString& string) {
        this->append(string.c_str(), string.size() + 1);
    }

    /**
     *  Return an SkDataTable from the accumulated entries that were added by
     *  calls to append(). This call also clears any accumluated entries from
     *  this builder, so its count() will be 0 after this call.
     */
    SkDataTable* detachDataTable();

private:
    SkTDArray<SkDataTable::Dir> fDir;
    SkChunkAlloc*               fHeap;
    size_t                      fMinChunkSize;
};

#endif
