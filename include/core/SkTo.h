/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTo_DEFINED
#define SkTo_DEFINED

#include "SkTypes.h"
#include "../private/SkTFitsIn.h"

template <typename D, typename S> constexpr D SkTo(S s) {
    return SkASSERT(SkTFitsIn<D>(s)),
           static_cast<D>(s);
}

#define SkToS8(x)    SkTo<int8_t>(x)
#define SkToU8(x)    SkTo<uint8_t>(x)
#define SkToS16(x)   SkTo<int16_t>(x)
#define SkToU16(x)   SkTo<uint16_t>(x)
#define SkToS32(x)   SkTo<int32_t>(x)
#define SkToU32(x)   SkTo<uint32_t>(x)
#define SkToInt(x)   SkTo<int>(x)
#define SkToUInt(x)  SkTo<unsigned>(x)
#define SkToSizeT(x) SkTo<size_t>(x)

#endif  // SkTo_DEFINED
