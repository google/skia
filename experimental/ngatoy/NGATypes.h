// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef NGATypes_DEFINED
#define NGATypes_DEFINED

#include "include/gpu/GrTypes.h"

class ID {
public:
    explicit ID(int id) : fID(id) {
        SkASSERT(id != -1);
    }

    static ID Invalid() {
        return ID();
    }

    bool isValid() const { return fID != -1; }

    bool operator==(ID other) const { return fID == other.fID; }

private:
    ID() : fID(-1) {}

    int fID;
};

class PaintersOrder {
public:
    PaintersOrder() : fPaintersOrder(0) {}

    explicit PaintersOrder(uint32_t paintersOrder) : fPaintersOrder(paintersOrder) {
        SkASSERT(paintersOrder != 0);
    }

    static PaintersOrder Invalid() {
        return PaintersOrder();
    }

    bool isValid() const { return fPaintersOrder != 0; }

    uint32_t toUInt() const { return fPaintersOrder; }

private:

    uint32_t fPaintersOrder = 0;
};


#endif


