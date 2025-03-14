/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_UniquePaintParamsID_DEFINED
#define skgpu_graphite_UniquePaintParamsID_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu::graphite {

// This class boils down to a unique uint that can be used instead of a variable length
// key derived from a PaintParams.
class UniquePaintParamsID {
public:
    explicit constexpr UniquePaintParamsID(uint32_t id) : fID(id) {}

    constexpr UniquePaintParamsID() : fID(SK_InvalidUniqueID) {}

    static constexpr UniquePaintParamsID Invalid() { return UniquePaintParamsID(); }

    bool operator==(const UniquePaintParamsID &that) const { return fID == that.fID; }
    bool operator!=(const UniquePaintParamsID &that) const { return !(*this == that); }

    bool isValid() const { return fID != SK_InvalidUniqueID; }
    uint32_t asUInt() const { return fID; }

private:
    uint32_t fID;
};

} // skgpu::graphite

#endif // skgpu_graphite_UniquePaintParamsID_DEFINED
