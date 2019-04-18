// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFType1Font_DEFINED
#define SkPDFType1Font_DEFINED

#include "SkPDFDocumentPriv.h"
#include "SkPDFFont.h"

#define SK_PDF_SUPPORT_TYPE_1_FONTS

#ifdef SK_PDF_SUPPORT_TYPE_1_FONTS
void SkPDFEmitType1Font(const SkPDFFont&, SkPDFDocument*);
#endif  // SK_PDF_SUPPORT_TYPE_1_FONTS

#endif  // SkPDFType1Font_DEFINED
