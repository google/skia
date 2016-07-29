/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFFormXObject_DEFINED
#define SkPDFFormXObject_DEFINED

#include "SkPDFDevice.h"
#include "SkPDFTypes.h"

/** A form XObject is a self contained description of a graphics
    object.  A form XObject is a page object with slightly different
    syntax, that can be drawn into a page content stream, just like a
    bitmap XObject can be drawn into a page content stream.
*/
sk_sp<SkPDFObject> SkPDFMakeFormXObject(std::unique_ptr<SkStreamAsset> content,
                                        sk_sp<SkPDFArray> mediaBox,
                                        sk_sp<SkPDFDict> resourceDict,
                                        const SkMatrix& inverseTransform,
                                        const char* colorSpace);
#endif
