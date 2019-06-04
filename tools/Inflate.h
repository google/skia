// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef Inflate_DEFINED
#define Inflate_DEFINED

#include "include/core/SkStream.h"
#include "include/core/SkString.h"

#include <memory>

/** Use the un-deflate compression algorithm to decompress the data in src,
    returning the result.  Returns nullptr and set error string if an error occurs.
    (Used to test SkDeflate)
*/
std::unique_ptr<SkStreamAsset> InflateStream(SkStream* src, SkString* error);

#endif  // Inflate_DEFINED
