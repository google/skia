/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDescriptor.h"

#include <new>

#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"
#include "src/core/SkOpts.h"

std::unique_ptr<SkDescriptor> SkDescriptor::Alloc(size_t length) {
    SkASSERT(SkAlign4(length) == length);
    void* allocation = ::operator new (length);
    return std::unique_ptr<SkDescriptor>(new (allocation) SkDescriptor{});
}

void SkDescriptor::operator delete(void* p) { ::operator delete(p); }
void* SkDescriptor::operator new(size_t) {
    SK_ABORT("Descriptors are created with placement new.");
}

void* SkDescriptor::addEntry(uint32_t tag, size_t length, const void* data) {
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

void SkDescriptor::computeChecksum() {
    fChecksum = SkDescriptor::ComputeChecksum(this);
}

const void* SkDescriptor::findEntry(uint32_t tag, uint32_t* length) const {
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

std::unique_ptr<SkDescriptor> SkDescriptor::copy() const {
    std::unique_ptr<SkDescriptor> desc = SkDescriptor::Alloc(fLength);
    memcpy(desc.get(), this, fLength);
    return desc;
}

bool SkDescriptor::operator==(const SkDescriptor& other) const {

    // the first value we should look at is the checksum, so this loop
    // should terminate early if they descriptors are different.
    // NOTE: if we wrote a sentinel value at the end of each, we could
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

uint32_t SkDescriptor::ComputeChecksum(const SkDescriptor* desc) {
    const uint32_t* ptr = (const uint32_t*)desc + 1; // skip the checksum field
    size_t len = desc->fLength - sizeof(uint32_t);
    return SkOpts::hash(ptr, len);
}

bool SkDescriptor::isValid() const {
    uint32_t count = fCount;
    size_t lengthRemaining = this->fLength;
    if (lengthRemaining < sizeof(SkDescriptor)) {
        return false;
    }
    lengthRemaining -= sizeof(SkDescriptor);
    size_t offset = sizeof(SkDescriptor);

    while (lengthRemaining > 0 && count > 0) {
        if (lengthRemaining < sizeof(Entry)) {
            return false;
        }
        lengthRemaining -= sizeof(Entry);

        const Entry* entry = (const Entry*)(reinterpret_cast<const char*>(this) + offset);

        if (lengthRemaining < entry->fLen) {
            return false;
        }
        lengthRemaining -= entry->fLen;

        // rec tags are always a known size.
        if (entry->fTag == kRec_SkDescriptorTag && entry->fLen != sizeof(SkScalerContextRec)) {
            return false;
        }

        offset += sizeof(Entry) + entry->fLen;
        count--;
    }
    return lengthRemaining == 0 && count == 0;
}

SkAutoDescriptor::SkAutoDescriptor() = default;
SkAutoDescriptor::SkAutoDescriptor(size_t size) { this->reset(size); }
SkAutoDescriptor::SkAutoDescriptor(const SkDescriptor& desc) { this->reset(desc); }
SkAutoDescriptor::SkAutoDescriptor(const SkAutoDescriptor& ad) {
    this->reset(*ad.getDesc());
}
SkAutoDescriptor& SkAutoDescriptor::operator=(const SkAutoDescriptor& ad) {
    this->reset(*ad.getDesc());
    return *this;
}

SkAutoDescriptor::~SkAutoDescriptor() { this->free(); }

void SkAutoDescriptor::reset(size_t size) {
    this->free();
    if (size <= sizeof(fStorage)) {
        fDesc = new (&fStorage) SkDescriptor{};
    } else {
        fDesc = SkDescriptor::Alloc(size).release();
    }
}

void SkAutoDescriptor::reset(const SkDescriptor& desc) {
    size_t size = desc.getLength();
    this->reset(size);
    memcpy(fDesc, &desc, size);
}

void SkAutoDescriptor::free() {
    if (fDesc != (SkDescriptor*)&fStorage) {
        delete fDesc;
    }
}


