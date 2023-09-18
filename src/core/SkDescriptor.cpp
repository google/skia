/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDescriptor.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <cstring>

std::unique_ptr<SkDescriptor> SkDescriptor::Alloc(size_t length) {
    SkASSERT(length >= sizeof(SkDescriptor) && SkAlign4(length) == length);
    void* allocation = ::operator new(length);
    return std::unique_ptr<SkDescriptor>(new (allocation) SkDescriptor{});
}

void SkDescriptor::operator delete(void* p) { ::operator delete(p); }
void* SkDescriptor::operator new(size_t) {
    SK_ABORT("Descriptors are created with placement new.");
}

void SkDescriptor::flatten(SkWriteBuffer& buffer) const {
    buffer.writePad32(static_cast<const void*>(this), this->fLength);
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
    return (entry + 1);  // return its data
}

void SkDescriptor::computeChecksum() {
    fChecksum = SkDescriptor::ComputeChecksum(this);
}

const void* SkDescriptor::findEntry(uint32_t tag, uint32_t* length) const {
    const Entry* entry = (const Entry*)(this + 1);
    int count = fCount;

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

SkString SkDescriptor::dumpRec() const {
    const SkScalerContextRec* rec = static_cast<const SkScalerContextRec*>(
            this->findEntry(kRec_SkDescriptorTag, nullptr));

    SkString result;
    result.appendf("    Checksum: %x\n", fChecksum);
    if (rec != nullptr) {
        result.append(rec->dump());
    }
    return result;
}

uint32_t SkDescriptor::ComputeChecksum(const SkDescriptor* desc) {
    const uint32_t* ptr = (const uint32_t*)desc + 1;  // skip the checksum field
    size_t len = desc->fLength - sizeof(uint32_t);
    return SkChecksum::Hash32(ptr, len);
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
SkAutoDescriptor::SkAutoDescriptor(const SkAutoDescriptor& that) {
    this->reset(*that.getDesc());
}
SkAutoDescriptor& SkAutoDescriptor::operator=(const SkAutoDescriptor& that) {
    this->reset(*that.getDesc());
    return *this;
}
SkAutoDescriptor::SkAutoDescriptor(SkAutoDescriptor&& that) {
    if (that.fDesc == (SkDescriptor*)&that.fStorage) {
        this->reset(*that.getDesc());
    } else {
        fDesc = that.fDesc;
        that.fDesc = nullptr;
    }
}
SkAutoDescriptor& SkAutoDescriptor::operator=(SkAutoDescriptor&& that) {
    if (that.fDesc == (SkDescriptor*)&that.fStorage) {
        this->reset(*that.getDesc());
    } else {
        this->free();
        fDesc = that.fDesc;
        that.fDesc = nullptr;
    }
    return *this;
}

SkAutoDescriptor::~SkAutoDescriptor() { this->free(); }

std::optional<SkAutoDescriptor> SkAutoDescriptor::MakeFromBuffer(SkReadBuffer& buffer) {
    SkDescriptor descriptorHeader;
    if (!buffer.readPad32(&descriptorHeader, sizeof(SkDescriptor))) { return {}; }

    // Basic bounds check on header length to make sure that bodyLength calculation does not
    // underflow.
    if (descriptorHeader.getLength() < sizeof(SkDescriptor)) { return {}; }
    uint32_t bodyLength = descriptorHeader.getLength() - sizeof(SkDescriptor);

    // Make sure the fLength makes sense with respect to the incoming data.
    if (bodyLength > buffer.available()) {
        return {};
    }

    SkAutoDescriptor ad{descriptorHeader.getLength()};
    memcpy(ad.fDesc, &descriptorHeader, sizeof(SkDescriptor));
    if (!buffer.readPad32(SkTAddOffset<void>(ad.fDesc, sizeof(SkDescriptor)), bodyLength)) {
        return {};
    }

// If the fuzzer produces data but the checksum does not match, let it continue. This will boost
// fuzzing speed. We leave the actual checksum computation in for fuzzing builds to make sure
// the ComputeChecksum function is covered.
#if defined(SK_BUILD_FOR_FUZZER)
    SkDescriptor::ComputeChecksum(ad.getDesc());
#else
    if (SkDescriptor::ComputeChecksum(ad.getDesc()) != ad.getDesc()->fChecksum) { return {}; }
#endif
    if (!ad.getDesc()->isValid()) { return {}; }

    return {ad};
}

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
    if (fDesc == (SkDescriptor*)&fStorage) {
        fDesc->~SkDescriptor();
    } else {
        delete fDesc;
    }
}


