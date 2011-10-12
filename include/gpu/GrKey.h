
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrKey_DEFINED
#define GrKey_DEFINED

#include "GrRefCnt.h"

class GrKey : public GrRefCnt {
public:
    typedef intptr_t Hash;

    explicit GrKey(Hash hash) : fHash(hash) {}

    intptr_t getHash() const { return fHash; }

    bool operator<(const GrKey& rh) const {
        return fHash < rh.fHash || (fHash == rh.fHash && this->lt(rh));
    }
    bool operator==(const GrKey& rh) const {
        return fHash == rh.fHash && this->eq(rh);
    }

protected:
    virtual bool lt(const GrKey& rh) const = 0;
    virtual bool eq(const GrKey& rh) const = 0;

private:
    const Hash fHash;
};

#endif

