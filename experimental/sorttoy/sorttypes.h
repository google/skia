// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SortTypes_DEFINED
#define SortTypes_DEFINED

#include "include/gpu/GrTypes.h"

enum class Shape {
    kRect,
    kOval
};

// This is strictly used to check if we get the order of draw operations we expected. It is
// pretty much the same as painters order though.
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

    int toInt() const { return fID; }

private:
    ID() : fID(-1) {}

    int fID;
};

// This class just serves to strictly differentiate between painter's order and the sort/draw Zs
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
