// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef NGATypes_DEFINED
#define NGATypes_DEFINED

#include "include/gpu/GrTypes.h"

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

#endif


