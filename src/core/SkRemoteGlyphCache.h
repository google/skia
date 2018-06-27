/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemoteGlyphCache_DEFINED
#define SkRemoteGlyphCache_DEFINED

#include <memory>
#include "SkData.h"
#include "SkDescriptor.h"
#include "SkSerialProcs.h"
#include "SkTHash.h"
#include "SkTypeface.h"
#include "SkTypeface_remote.h"

class SkScalerContextRecDescriptor {
public:
    SkScalerContextRecDescriptor() {}
    explicit SkScalerContextRecDescriptor(const SkScalerContextRec& rec) {
        auto desc = reinterpret_cast<SkDescriptor*>(&fDescriptor);
        desc->init();
        desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
        SkASSERT(sizeof(fDescriptor) == desc->getLength());
    }

    SkScalerContextRecDescriptor& operator=(const SkScalerContextRecDescriptor& rhs) {
        std::memcpy(&fDescriptor, &rhs.fDescriptor, rhs.desc().getLength());
        return *this;
    }

    const SkDescriptor& desc() const {
        return *reinterpret_cast<const SkDescriptor*>(&fDescriptor);
    }

    struct Hash {
        uint32_t operator()(SkScalerContextRecDescriptor const& s) const {
            return s.desc().getChecksum();
        }
    };

    friend bool operator==(const SkScalerContextRecDescriptor& lhs,
                           const SkScalerContextRecDescriptor& rhs ) {
        return lhs.desc() == rhs.desc();
    }

private:
    // The system only passes descriptors without effects. That is why it uses a fixed size
    // descriptor. storageFor is needed because some of the constructors below are private.
    template <typename T>
    using storageFor = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    struct {
        storageFor<SkDescriptor>        dummy1;
        storageFor<SkDescriptor::Entry> dummy2;
        storageFor<SkScalerContextRec>  dummy3;
    } fDescriptor;
};

class SkRemoteGlyphCacheRenderer {
public:
    void prepareSerializeProcs(SkSerialProcs* procs);

    SkScalerContext* generateScalerContext(
        const SkScalerContextRecDescriptor& desc, SkFontID typefaceId);

private:
    sk_sp<SkData> encodeTypeface(SkTypeface* tf);

    SkTHashMap<SkFontID, sk_sp<SkTypeface>> fTypefaceMap;

    using DescriptorToContextMap =
    SkTHashMap<
    SkScalerContextRecDescriptor,
    std::unique_ptr<SkScalerContext>,
    SkScalerContextRecDescriptor::Hash>;

    DescriptorToContextMap fScalerContextMap;
};

class SkRemoteGlyphCacheGPU {
public:
    explicit SkRemoteGlyphCacheGPU(std::unique_ptr<SkRemoteScalerContext> remoteScalerContext);

    void prepareDeserializeProcs(SkDeserialProcs* procs);

private:
    sk_sp<SkTypeface> decodeTypeface(const void* buf, size_t len);

    std::unique_ptr<SkRemoteScalerContext> fRemoteScalerContext;
    // TODO: Figure out how to manage the entries for the following maps.
    SkTHashMap<SkFontID, sk_sp<SkTypefaceProxy>> fMapIdToTypeface;
};

#endif  // SkRemoteGlyphCache_DEFINED
