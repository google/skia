/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFMakeCIDGlyphWidthsArray_DEFINED
#define SkPDFMakeCIDGlyphWidthsArray_DEFINED

#include "src/pdf/SkPDFTypes.h"

class SkStrike;
class SkPDFGlyphUse;

/* PDF 32000-1:2008, page 270: "The array's elements have a variable
   format that can specify individual widths for consecutive CIDs or
   one width for a range of CIDs". */
std::unique_ptr<SkPDFArray> SkPDFMakeCIDGlyphWidthsArray(SkStrike* cache,
                                                         const SkPDFGlyphUse* subset,
                                                         uint16_t emSize,
                                                         int16_t* defaultWidth);

#endif  // SkPDFMakeCIDGlyphWidthsArray_DEFINED
