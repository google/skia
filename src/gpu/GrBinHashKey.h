
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrBinHashKey_DEFINED
#define GrBinHashKey_DEFINED

#include "GrTypes.h"

/**
 *  Hash function class that can take a data chunk of any predetermined length. The hash function
 *  used is the One-at-a-Time Hash (http://burtleburtle.net/bob/hash/doobs.html).
 *
 *  Keys are computed from ENTRY objects. ENTRY must be fully ordered by a member:
 *      int compare(const GrTBinHashKey<ENTRY, ..>& k);
 *  which returns negative if the ENTRY < k, 0 if it equals k, and positive if k < the ENTRY.
 *  Additionally, ENTRY must be flattenable into the key using setKeyData.
 *
 *  This class satisfies the requirements to be a key for a GrTHashTable.
 */
template<typename ENTRY, size_t KEY_SIZE>
class GrTBinHashKey {
public:
    enum { kKeySize = KEY_SIZE };

    GrTBinHashKey() {
        this->reset();
    }

    GrTBinHashKey(const GrTBinHashKey<ENTRY, KEY_SIZE>& other) {
        *this = other;
    }

    GrTBinHashKey<ENTRY, KEY_SIZE>& operator=(const GrTBinHashKey<ENTRY, KEY_SIZE>& other) {
        memcpy(this, &other, sizeof(*this));
        return *this;
    }

    ~GrTBinHashKey() {
    }

    void reset() {
        fHash = 0;
#if GR_DEBUG
        fIsValid = false;
#endif
    }

    void setKeyData(const uint32_t* SK_RESTRICT data) {
        GrAssert(GrIsALIGN4(KEY_SIZE));
        memcpy(&fData, data, KEY_SIZE);

        uint32_t hash = 0;
        size_t len = KEY_SIZE;
        while (len >= 4) {
            hash += *data++;
            hash += (fHash << 10);
            hash ^= (hash >> 6);
            len -= 4;
        }
        hash += (fHash << 3);
        hash ^= (fHash >> 11);
        hash += (fHash << 15);
#if GR_DEBUG
        fIsValid = true;
#endif
        fHash = hash;
    }

    int compare(const GrTBinHashKey<ENTRY, KEY_SIZE>& key) const {
        GrAssert(fIsValid && key.fIsValid);
        return memcmp(fData, key.fData, KEY_SIZE);
    }

    static bool EQ(const ENTRY& entry, const GrTBinHashKey<ENTRY, KEY_SIZE>& key) {
        GrAssert(key.fIsValid);
        return 0 == entry.compare(key);
    }

    static bool LT(const ENTRY& entry, const GrTBinHashKey<ENTRY, KEY_SIZE>& key) {
        GrAssert(key.fIsValid);
        return entry.compare(key) < 0;
    }

    uint32_t getHash() const {
        GrAssert(fIsValid);
        return fHash;
    }

    const uint8_t* getData() const {
        GrAssert(fIsValid);
        return fData;
    }

private:
    uint32_t            fHash;
    uint8_t             fData[KEY_SIZE];  // Buffer for key storage

#if GR_DEBUG
public:
    bool                fIsValid;
#endif
};

#endif
