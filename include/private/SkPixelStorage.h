/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelStorage_DEFINED
#define SkPixelStorage_DEFINED

#include <cstdint>
#include <atomic>
#include "include/core/SkRefCnt.h"

/**
 * SkPixelStorage is the class representing the storage block for pixel data. It serves as the
 * abstract source of pixel memory—the 2D array of data—that is read from and written to by
 * higher-level Skia objects like SkImage and SkSurface.
 */
class SkPixelStorage : public SkRefCnt {
public:
    enum Type {
        kTextureProxy,
        kPixelRef,
    };

    struct Key {
        uint32_t fStorageID = 0;
        uint32_t fContentID = 0;

        bool isValid() const { return fStorageID != 0; }
        bool operator==(const Key& o) const {
            return fStorageID == o.fStorageID && fContentID == o.fContentID;
        }
        bool operator!=(const Key& o) const { return !(*this == o); }
    };

    struct Provenance {
        enum class Source : uint8_t {
            kUnknown = 0,
            kClientRequest = 1, // Requested through factory functions on recorder (makeSurface)
            kCopy = 2, // Created from copying other pixels
        };

        Source fSource = Source::kUnknown;

        // Parent data is only considered in kCopy case.
        Key fParentKey;

        static Provenance ClientRequest() {
            return {Source::kClientRequest, {}};
        }

        static Provenance Copy(Key parentKey) {
            return {Source::kCopy, parentKey};
        }

        static Provenance Copy(uint32_t parentStorageID, uint32_t parentContentID) {
            return {Source::kCopy, {parentStorageID, parentContentID}};
        }

        bool isClientRequest() const { return fSource == Source::kClientRequest; }
        bool isCopy() const { return fSource == Source::kCopy; }
        bool hasParent() const { return isCopy() && fParentKey.isValid(); }
    };

    SkPixelStorage(Type type);
    ~SkPixelStorage() override = default;

    uint32_t getPixelStorageId() const { return fID; }
    Type type() const { return fType; }

    uint32_t getContentId() const { return fContentID.load(std::memory_order_relaxed); }
    uint32_t incrementContentId() { return fContentID.fetch_add(1, std::memory_order_relaxed) + 1; }

    void setProvenance(Provenance provenance) { fProvenance = provenance; }
    Provenance provenance() const { return fProvenance; }

private:
    Type fType;
    /**
     * fID is a static identifier that differenciates PixelStorages. This is assigned at
     * construction and does not change.
     */
    uint32_t fID;
    /**
     * fContentID is an identifier that tracks the current state of a PixelStorage. Gets incremented
     * and read from other objects that want to track the state of a PixelStorage at a given time.
     */
    std::atomic<uint32_t> fContentID = 0;

    Provenance fProvenance;

    static uint32_t NextId();
};

#endif
