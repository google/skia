/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFConvertType1FontStream_DEFINED
#define SkPDFConvertType1FontStream_DEFINED

#include "SkData.h"
#include "SkStream.h"

/*
  "A standard Type 1 font program, as described in the Adobe Type 1
  Font Format specification, consists of three parts: a clear-text
  portion (written using PostScript syntax), an encrypted portion, and
  a fixed-content portion.  The fixed-content portion contains 512
  ASCII zeros followed by a cleartomark operator, and perhaps followed
  by additional data. Although the encrypted portion of a standard
  Type 1 font may be in binary or ASCII hexadecimal format, PDF
  supports only the binary format."
*/
sk_sp<SkData> SkPDFConvertType1FontStream(
        std::unique_ptr<SkStreamAsset> srcStream, size_t* headerLen,
        size_t* dataLen, size_t* trailerLen);

#endif  // SkPDFConvertType1FontStream_DEFINED
