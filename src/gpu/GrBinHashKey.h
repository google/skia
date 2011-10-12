
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
 *  Hash function class that can take a data chunk of any predetermined
 *  length. The hash function used is the One-at-a-Time Hash
 *  (http://burtleburtle.net/bob/hash/doobs.html).
 */
template<typename Entry, size_t KeySize>
class GrBinHashKey {
public:
    GrBinHashKey()
        : fHash(0)
#if GR_DEBUG
        , fIsValid(false)
#endif
    {}

    GrBinHashKey(const GrBinHashKey<Entry, KeySize>& other) {
        *this = other;
    }
    GrBinHashKey<Entry, KeySize>& operator=(const GrBinHashKey<Entry,
        KeySize>& other) {
        memcpy(this, &other, sizeof(*this));
        return *this;
    }

    ~GrBinHashKey() {
    }

    void setKeyData(const uint32_t *data) {
        GrAssert(GrIsALIGN4(KeySize));
        memcpy(&fData, data, KeySize);

        fHash = 0;
        size_t len = KeySize;
        while (len >= 4) {
            fHash += *data++;
            fHash += (fHash << 10);
            fHash ^= (fHash >> 6);
            len -= 4;
        }
        fHash += (fHash << 3);
        fHash ^= (fHash >> 11);
        fHash += (fHash << 15);
#if GR_DEBUG
        fIsValid = true;
#endif
    }

    int compare(const GrBinHashKey<Entry, KeySize>& key) const {
        GrAssert(fIsValid && key.fIsValid);
        return memcmp(fData, key.fData, KeySize);
    }

    static bool
    EQ(const Entry& entry, const GrBinHashKey<Entry, KeySize>& key) {
        GrAssert(key.fIsValid);
        return 0 == entry.compare(key);
    }

    static bool
    LT(const Entry& entry, const GrBinHashKey<Entry, KeySize>& key) {
        GrAssert(key.fIsValid);
        return entry.compare(key) < 0;
    }

    uint32_t getHash() const {
        GrAssert(fIsValid);
        return fHash;
    }

private:
    uint32_t            fHash;
    uint8_t             fData[KeySize];  //Buffer for key storage

#if GR_DEBUG
public:
    bool                fIsValid;
#endif
};

#endif
