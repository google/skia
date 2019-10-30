/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFFormXObject_DEFINED
#define SkPDFFormXObject_DEFINED

#include "src/pdf/SkPDFDevice.h"
#include "src/pdf/SkPDFTypes.h"

class SkPDFDocument;

/** A form XObject is a self contained description of a graphics
    object.  A form XObject is a page object with slightly different
    syntax, that can be drawn into a page content stream, just like a
    bitmap XObject can be drawn into a page content stream.
*/
SkPDFIndirectReference SkPDFMakeFormXObject(SkPDFDocument* doc,
                                            std::unique_ptr<SkStreamAsset> content,
                                            std::unique_ptr<SkPDFArray> mediaBox,
                                            std::unique_ptr<SkPDFDict> resourceDict,
                                            const SkMatrix& inverseTransform,
                                            const char* colorSpace);
#endif
