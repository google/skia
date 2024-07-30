/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFMakeCIDGlyphWidthsArray_DEFINED
#define SkPDFMakeCIDGlyphWidthsArray_DEFINED

#include <cstdint>
#include <memory>

class SkPDFArray;
class SkPDFGlyphUse;
class SkPDFStrikeSpec;

/* PDF 32000-1:2008, page 270: "The array's elements have a variable
   format that can specify individual widths for consecutive CIDs or
   one width for a range of CIDs". */
std::unique_ptr<SkPDFArray> SkPDFMakeCIDGlyphWidthsArray(const SkPDFStrikeSpec& strikeSpec,
                                                         const SkPDFGlyphUse& subset,
                                                         int32_t* defaultAdvance);

#endif  // SkPDFMakeCIDGlyphWidthsArray_DEFINED
