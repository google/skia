/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDescriptor_DEFINED
#define SkDescriptor_DEFINED

#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/core/SkScalerContext.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

class SkReadBuffer;
class SkWriteBuffer;

class SkDescriptor : SkNoncopyable {
public:
    static size_t ComputeOverhead(int entryCount) {
        SkASSERT(entryCount >= 0);
        return sizeof(SkDescriptor) + entryCount * sizeof(Entry);
    }

    static std::unique_ptr<SkDescriptor> Alloc(size_t length);

    //
    // Ensure the unsized delete is called.
    void operator delete(void* p);
    void* operator new(size_t);
    void* operator new(size_t, void* p) { return p; }

    void flatten(SkWriteBuffer& buffer) const;

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

    uint32_t getCount() const { return fCount; }

    SkString dumpRec() const;

private:
    SkDescriptor() = default;
    friend class SkDescriptorTestHelper;
    friend class SkAutoDescriptor;

    static uint32_t ComputeChecksum(const SkDescriptor* desc);

    uint32_t fChecksum{0};  // must be first
    uint32_t fLength{sizeof(SkDescriptor)};    // must be second
    uint32_t fCount{0};
};

class SkAutoDescriptor {
public:
    SkAutoDescriptor();
    explicit SkAutoDescriptor(size_t size);
    explicit SkAutoDescriptor(const SkDescriptor&);
    SkAutoDescriptor(const SkAutoDescriptor&);
    SkAutoDescriptor& operator=(const SkAutoDescriptor&);
    SkAutoDescriptor(SkAutoDescriptor&&);
    SkAutoDescriptor& operator=(SkAutoDescriptor&&);
    ~SkAutoDescriptor();

    // Returns no value if there is an error.
    static std::optional<SkAutoDescriptor> MakeFromBuffer(SkReadBuffer& buffer);

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
    alignas(uint32_t) char fStorage[kStorageSize];
};

#endif  //SkDescriptor_DEFINED
