
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFDocument_DEFINED
#define SkPDFDocument_DEFINED

#include "SkAdvancedTypefaceMetrics.h"
#include "SkTDArray.h"

class SkPDFDevice;
class SkWStream;

namespace SkPDFDocument {
/**
 *  Assemble pages together and generate a PDF document file.
 *
 *  Output the PDF to the passed stream.  It is an error to call this (it
 *  will return false and not modify stream) if pageDevices is empty.
 *  No device pointer can be NULL.
 *
 *  @param pageDevices An array of pages, in order.  All pages
 *                     should be created using the same SkPDFCanon.
 *  @param SkWStream   The writable output stream to send the PDF to.
 */
bool EmitPDF(const SkTDArray<const SkPDFDevice*>& pageDevices, SkWStream*);

/** Get the count of unique font types used in the given pages.
 */
void GetCountOfFontTypes(const SkTDArray<SkPDFDevice*>& pageDevices,
                         int counts[SkAdvancedTypefaceMetrics::kOther_Font + 1],
                         int* notSubsettableCount,
                         int* notEmbedddableCount);
}

#endif
