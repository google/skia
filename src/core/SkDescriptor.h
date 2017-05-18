/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDescriptor_DEFINED
#define SkDescriptor_DEFINED

#include "SkOpts.h"
#include "SkTypes.h"
#include <memory>

class SkDescriptor : SkNoncopyable {
public:
    static size_t ComputeOverhead(int entryCount) {
        SkASSERT(entryCount >= 0);
        return sizeof(SkDescriptor) + entryCount * sizeof(Entry);
    }

    static std::unique_ptr<SkDescriptor> Alloc(size_t length) {
        SkASSERT(SkAlign4(length) == length);
        return std::unique_ptr<SkDescriptor>(static_cast<SkDescriptor*>(::operator new (length)));
    }

    // Ensure the unsized delete is called.
    void operator delete(void* p) { ::operator delete(p); }

    void init() {
        fLength = sizeof(SkDescriptor);
        fCount  = 0;
    }

    uint32_t getLength() const { return fLength; }

    void* addEntry(uint32_t tag, size_t length, const void* data = nullptr) {
        SkASSERT(tag);
        SkASSERT(SkAlign4(length) == length);
        SkASSERT(this->findEntry(tag, nullptr) == nullptr);

        Entry* entry = (Entry*)((char*)this + fLength);
        entry->fTag = tag;
        entry->fLen = SkToU32(length);
        if (data) {
            memcpy(entry + 1, data, length);
        }

        fCount += 1;
        fLength = SkToU32(fLength + sizeof(Entry) + length);
        return (entry + 1); // return its data
    }

    void computeChecksum() {
        fChecksum = SkDescriptor::ComputeChecksum(this);
    }

#ifdef SK_DEBUG
    void assertChecksum() const {
        SkASSERT(SkDescriptor::ComputeChecksum(this) == fChecksum);
    }
#endif

    const void* findEntry(uint32_t tag, uint32_t* length) const {
        const Entry* entry = (const Entry*)(this + 1);
        int          count = fCount;

        while (--count >= 0) {
            if (entry->fTag == tag) {
                if (length) {
                    *length = entry->fLen;
                }
                return entry + 1;
            }
            entry = (const Entry*)((const char*)(entry + 1) + entry->fLen);
        }
        return nullptr;
    }

    std::unique_ptr<SkDescriptor> copy() const {
        std::unique_ptr<SkDescriptor> desc = SkDescriptor::Alloc(fLength);
        memcpy(desc.get(), this, fLength);
        return desc;
    }

    bool operator==(const SkDescriptor& other) const {
        // probe to see if we have a good checksum algo
//        SkASSERT(a.fChecksum != b.fChecksum || memcmp(&a, &b, a.fLength) == 0);

        // the first value we should look at is the checksum, so this loop
        // should terminate early if they descriptors are different.
        // NOTE: if we wrote a sentinel value at the end of each, we chould
        //       remove the aa < stop test in the loop...
        const uint32_t* aa = (const uint32_t*)this;
        const uint32_t* bb = (const uint32_t*)&other;
        const uint32_t* stop = (const uint32_t*)((const char*)aa + fLength);
        do {
            if (*aa++ != *bb++)
                return false;
        } while (aa < stop);
        return true;
    }
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
    uint32_t fChecksum;  // must be first
    uint32_t fLength;    // must be second
    uint32_t fCount;

    static uint32_t ComputeChecksum(const SkDescriptor* desc) {
        const uint32_t* ptr = (const uint32_t*)desc + 1; // skip the checksum field
        size_t len = desc->fLength - sizeof(uint32_t);
        return SkOpts::hash(ptr, len);
    }

    // private so no one can create one except our factories
    SkDescriptor() {}
};

#include "SkScalerContext.h"

class SkAutoDescriptor : SkNoncopyable {
public:
    SkAutoDescriptor() : fDesc(nullptr) {}
    SkAutoDescriptor(size_t size) : fDesc(nullptr) { this->reset(size); }
    SkAutoDescriptor(const SkDescriptor& desc) : fDesc(nullptr) {
        size_t size = desc.getLength();
        this->reset(size);
        memcpy(fDesc, &desc, size);
    }

    ~SkAutoDescriptor() { this->free(); }

    void reset(size_t size) {
        this->free();
        if (size <= sizeof(fStorage)) {
            fDesc = (SkDescriptor*)(void*)fStorage;
        } else {
            fDesc = SkDescriptor::Alloc(size).release();
        }
    }

    SkDescriptor* getDesc() const { SkASSERT(fDesc); return fDesc; }
private:
    void free() {
        if (fDesc != (SkDescriptor*)(void*)fStorage) {
            delete fDesc;
        }
    }

    enum {
        kStorageSize =  sizeof(SkDescriptor)
                        + sizeof(SkDescriptor::Entry) + sizeof(SkScalerContext::Rec)    // for rec
                        + sizeof(SkDescriptor::Entry) + sizeof(void*)                   // for typeface
                        + 32   // slop for occational small extras
    };
    SkDescriptor*   fDesc;
    uint32_t        fStorage[(kStorageSize + 3) >> 2];
};
#define SkAutoDescriptor(...) SK_REQUIRE_LOCAL_VAR(SkAutoDescriptor)


#endif
