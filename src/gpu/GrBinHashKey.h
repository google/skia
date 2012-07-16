
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
 *  Keys are computed from Entry objects. Entry must be fully ordered by a member:
 *      int compare(const GrTBinHashKey<Entry, ..>& k);
 *  which returns negative if the Entry < k, 0 if it equals k, and positive if k < the Entry.
 *  Additionally, Entry must be flattenable into the key using setKeyData.
 *
 *  This class satisfies the requirements to be a key for a GrTHashTable.
 */
template<typename Entry, size_t KeySize>
class GrTBinHashKey {
public:
    GrTBinHashKey() {
        this->reset();
    }

    GrTBinHashKey(const GrTBinHashKey<Entry, KeySize>& other) {
        *this = other;
    }

    GrTBinHashKey<Entry, KeySize>& operator=(const GrTBinHashKey<Entry, KeySize>& other) {
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
        GrAssert(GrIsALIGN4(KeySize));
        memcpy(&fData, data, KeySize);

        uint32_t hash = 0;
        size_t len = KeySize;
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

    int compare(const GrTBinHashKey<Entry, KeySize>& key) const {
        GrAssert(fIsValid && key.fIsValid);
        return memcmp(fData, key.fData, KeySize);
    }

    static bool EQ(const Entry& entry, const GrTBinHashKey<Entry, KeySize>& key) {
        GrAssert(key.fIsValid);
        return 0 == entry.compare(key);
    }

    static bool LT(const Entry& entry, const GrTBinHashKey<Entry, KeySize>& key) {
        GrAssert(key.fIsValid);
        return entry.compare(key) < 0;
    }

    uint32_t getHash() const {
        GrAssert(fIsValid);
        return fHash;
    }

private:
    uint32_t            fHash;
    uint8_t             fData[KeySize];  // Buffer for key storage

#if GR_DEBUG
public:
    bool                fIsValid;
#endif
};

#endif
