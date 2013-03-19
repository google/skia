
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkData_DEFINED
#define SkData_DEFINED

#include "SkFlattenable.h"

/**
 *  SkData holds an immutable data buffer. Not only is the data immutable,
 *  but the actual ptr that is returned (by data() or bytes()) is guaranteed
 *  to always be the same for the life of this instance.
 */
class SK_API SkData : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkData)

    /**
     *  Returns the number of bytes stored.
     */
    size_t size() const { return fSize; }

    bool isEmpty() const { return 0 == fSize; }

    /**
     *  Returns the ptr to the data.
     */
    const void* data() const { return fPtr; }

    /**
     *  Like data(), returns a read-only ptr into the data, but in this case
     *  it is cast to uint8_t*, to make it easy to add an offset to it.
     */
    const uint8_t* bytes() const {
        return reinterpret_cast<const uint8_t*>(fPtr);
    }

    /**
     *  Helper to copy a range of the data into a caller-provided buffer.
     *  Returns the actual number of bytes copied, after clamping offset and
     *  length to the size of the data. If buffer is NULL, it is ignored, and
     *  only the computed number of bytes is returned.
     */
    size_t copyRange(size_t offset, size_t length, void* buffer) const;

    /**
     *  Returns true if these two objects have the same length and contents,
     *  effectively returning 0 == memcmp(...)
     */
    bool equals(const SkData* other) const;

    /**
     *  Function that, if provided, will be called when the SkData goes out
     *  of scope, allowing for custom allocation/freeing of the data.
     */
    typedef void (*ReleaseProc)(const void* ptr, size_t length, void* context);

    /**
     *  Create a new dataref by copying the specified data
     */
    static SkData* NewWithCopy(const void* data, size_t length);

    /**
     *  Create a new dataref by copying the specified c-string
     *  (a null-terminated array of bytes). The returned SkData will have size()
     *  equal to strlen(cstr) + 1. If cstr is NULL, it will be treated the same
     *  as "".
     */
    static SkData* NewWithCString(const char cstr[]);

    /**
     *  Create a new dataref, taking the data ptr as is, and using the
     *  releaseproc to free it. The proc may be NULL.
     */
    static SkData* NewWithProc(const void* data, size_t length,
                               ReleaseProc proc, void* context);

    /**
     *  Create a new dataref from a pointer allocated by malloc. The Data object
     *  takes ownership of that allocation, and will handling calling sk_free.
     */
    static SkData* NewFromMalloc(const void* data, size_t length);

    /**
     *  Create a new dataref from a pointer allocated by mmap. The Data object
     *  will handle calling munmap().
     */
    static SkData* NewFromMMap(const void* data, size_t length);

    /**
     *  Create a new dataref using a subset of the data in the specified
     *  src dataref.
     */
    static SkData* NewSubset(const SkData* src, size_t offset, size_t length);

    /**
     *  Returns a new empty dataref (or a reference to a shared empty dataref).
     *  New or shared, the caller must see that unref() is eventually called.
     */
    static SkData* NewEmpty();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkData)

protected:
    SkData(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    ReleaseProc fReleaseProc;
    void*       fReleaseProcContext;

    const void* fPtr;
    size_t      fSize;

    SkData(const void* ptr, size_t size, ReleaseProc, void* context);
    virtual ~SkData();

    // This is here because SkAutoTUnref creates an internal helper class
    // that derives from SkData (i.e., BlockRef) to prevent refs\unrefs.
    // This helper class generates a compiler warning on Windows since the
    // SkData's destructor is private. This friending gives the helper class
    // access to the destructor.
    friend class SkAutoTUnref<SkData>::BlockRef<SkData>;

    typedef SkFlattenable INHERITED;
};

/**
 *  Specialized version of SkAutoTUnref<SkData> for automatically unref-ing a
 *  SkData.
 */
class SkAutoDataUnref : SkNoncopyable {
public:
    SkAutoDataUnref(SkData* data) : fRef(data) {}
    ~SkAutoDataUnref() {
        SkSafeUnref(fRef);
    }

    SkData* get() const { return fRef; }

    void release() {
        if (fRef) {
            fRef->unref();
            fRef = NULL;
        }
    }

    SkData *operator->() const { return fRef; }
    operator SkData*() { return fRef; }

private:
    SkData*     fRef;
};

#endif
