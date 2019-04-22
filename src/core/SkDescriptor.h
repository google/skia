/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDescriptor_DEFINED
#define SkDescriptor_DEFINED

#include <memory>

#include "include/private/SkMacros.h"
#include "include/private/SkNoncopyable.h"
#include "src/core/SkScalerContext.h"

class SkDescriptor : SkNoncopyable {
public:
    static size_t ComputeOverhead(int entryCount) {
        SkASSERT(entryCount >= 0);
        return sizeof(SkDescriptor) + entryCount * sizeof(Entry);
    }

    static std::unique_ptr<SkDescriptor> Alloc(size_t length);

    // Ensure the unsized delete is called.
    void operator delete(void* p);
    void init() {
        fLength = sizeof(SkDescriptor);
        fCount  = 0;
    }
    uint32_t getLength() const { return fLength; }
    void* addEntry(uint32_t tag, size_t length, const void* data = nullptr);
    void computeChecksum();

    // Assumes that getLength <= capacity of this SkDescriptor.
    bool isValid() const;

#ifdef SK_DEBUG
    void assertChecksum() const {
        SkASSERT(SkDescriptor::ComputeChecksum(this) == fChecksum);
    }
#endif

    const void* findEntry(uint32_t tag, uint32_t* length) const;

    std::unique_ptr<SkDescriptor> copy() const;

    // This assumes that all memory added has a length that is a multiple of 4. This is checked
    // by the assert in addEntry.
    bool operator==(const SkDescriptor& other) const;
    bool operator!=(const SkDescriptor& other) const { return !(*this == other); }

    uint32_t getChecksum() const { return fChecksum; }

    struct Entry {
        uint32_t fTag;
        uint32_t fLen;
    };

#ifdef SK_DEBUG
    uint32_t getCount() const { return fCount; }
#endif

private:
    // private so no one can create one except our factories
    SkDescriptor() = default;
    friend class SkDescriptorTestHelper;

    static uint32_t ComputeChecksum(const SkDescriptor* desc);

    uint32_t fChecksum;  // must be first
    uint32_t fLength;    // must be second
    uint32_t fCount;
};

class SkAutoDescriptor : SkNoncopyable {
public:
    SkAutoDescriptor();
    SkAutoDescriptor(size_t size);
    SkAutoDescriptor(const SkDescriptor& desc);
    SkAutoDescriptor(SkAutoDescriptor&&) = delete;
    SkAutoDescriptor& operator =(SkAutoDescriptor&&) = delete;

    ~SkAutoDescriptor();

    void reset(size_t size);
    void reset(const SkDescriptor& desc);
    SkDescriptor* getDesc() const { SkASSERT(fDesc); return fDesc; }

private:
    void free();
    static constexpr size_t kStorageSize
            = sizeof(SkDescriptor)
              + sizeof(SkDescriptor::Entry) + sizeof(SkScalerContextRec) // for rec
              + sizeof(SkDescriptor::Entry) + sizeof(void*)              // for typeface
              + 32;   // slop for occasional small extras

    SkDescriptor*   fDesc{nullptr};
    std::aligned_storage<kStorageSize, alignof(uint32_t)>::type fStorage;
};

#endif  //SkDescriptor_DEFINED
