// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkUUID_DEFINED
#define SkUUID_DEFINED

#include <cstdint>
#include <cstring>

struct SkUUID {
    uint8_t fData[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

static inline bool operator==(const SkUUID& u, const SkUUID& v) {
    return 0 == memcmp(u.fData, v.fData, sizeof(u.fData));
}
static inline bool operator!=(const SkUUID& u, const SkUUID& v) { return !(u == v); }

#endif  // SkUUID_DEFINED
