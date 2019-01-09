/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFBitmap_DEFINED
#define SkPDFBitmap_DEFINED

class SkImage;
class SkPDFDocument;
struct SkPDFIndirectReference;

/**
 * Serialize a SkImage as an Image Xobject.
 *  quality > 100 means lossless
 */
SkPDFIndirectReference SkPDFSerializeImage(const SkImage* img,
                                           SkPDFDocument* doc,
                                           int encodingQuality = 101);

#endif  // SkPDFBitmap_DEFINED
