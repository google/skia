
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrMurmur3HashKey_DEFINED
#define GrMurmur3HashKey_DEFINED

#include "SkChecksum.h"
#include "GrTypes.h"

/**
 *  GrMurmur3HashKey is a hash key class that can take a data chunk of any predetermined
 *  length. It uses the Murmur3 hash function. It is intended to be used with
 *  SkTDynamicHash (where GrBinHashKey is for GrTHashTable).
 */
template<size_t KEY_SIZE_IN_BYTES>
class GrMurmur3HashKey {
public:
    GrMurmur3HashKey() {
        this->reset();
    }

    void reset() {
        fHash = 0;
#ifdef SK_DEBUG
        fIsValid = false;
#endif
    }

    void setKeyData(const uint32_t* data) {
        SK_COMPILE_ASSERT(KEY_SIZE_IN_BYTES % 4 == 0, key_size_mismatch);
        memcpy(fData, data, KEY_SIZE_IN_BYTES);

        fHash = SkChecksum::Murmur3(fData, KEY_SIZE_IN_BYTES);
#ifdef SK_DEBUG
        fIsValid = true;
#endif
    }

    bool operator==(const GrMurmur3HashKey& other) const {
        if (fHash != other.fHash) {
            return false;
        }

        return !memcmp(fData, other.fData, KEY_SIZE_IN_BYTES);
    }

    uint32_t getHash() const {
        SkASSERT(fIsValid);
        return fHash;
    }

    const uint8_t* getData() const {
        SkASSERT(fIsValid);
        return reinterpret_cast<const uint8_t*>(fData);
    }

private:
    uint32_t fHash;
    uint32_t fData[KEY_SIZE_IN_BYTES / sizeof(uint32_t)];  // Buffer for key storage.

#ifdef SK_DEBUG
public:
    bool                fIsValid;
#endif
};

#endif
