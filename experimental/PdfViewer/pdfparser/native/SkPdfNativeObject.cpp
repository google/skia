/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfNativeObject.h"

#include "SkBitmap.h"
#include "SkFlate.h"
#include "SkPdfFont.h"
#include "SkPdfNativeTokenizer.h"
#include "SkPdfReporter.h"
#include "SkStream.h"

// TODO(edisonn): mac builder does not find the header ... but from headers is ok
//#include "SkPdfStreamCommonDictionary_autogen.h"
#include "SkPdfHeaders_autogen.h"


SkPdfNativeObject SkPdfNativeObject::kNull = SkPdfNativeObject::makeNull();

bool SkPdfNativeObject::applyFlateDecodeFilter() {
    const unsigned char* old = fStr.fBuffer;
    bool deleteOld = isStreamOwned();

    SkMemoryStream skstream(fStr.fBuffer, fStr.fBytes >> 2, false);
    SkDynamicMemoryWStream uncompressedData;

    if (SkFlate::Inflate(&skstream, &uncompressedData)) {
        fStr.fBytes = (uncompressedData.bytesWritten() << 2) + kOwnedStreamBit +
                      kUnfilteredStreamBit;
        fStr.fBuffer = (const unsigned char*)new unsigned char[uncompressedData.bytesWritten()];
        uncompressedData.copyTo((void*)fStr.fBuffer);

        if (deleteOld) {
            delete[] old;
        }

        return true;
    } else {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kBadStream_SkPdfIssue, "inflate failed", this, NULL);
        return false;
    }
}

bool SkPdfNativeObject::applyDCTDecodeFilter() {
    // applyDCTDecodeFilter will fail, and it won't allow any more filters.
    // technically, it would be possible, but not a real world scenario.
    // in this way we create the image from the DCT stream directly.
    return false;
}

bool SkPdfNativeObject::applyFilter(const char* name) {
    if (strcmp(name, "FlateDecode") == 0) {
        return applyFlateDecodeFilter();
    } else if (strcmp(name, "DCTDecode") == 0) {
        return applyDCTDecodeFilter();
    }
    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "filter not supported", this,
                NULL);
    return false;
}

bool SkPdfNativeObject::filterStream() {
    SkPdfMarkObjectUsed();

    if (!hasStream()) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kBadStream_SkPdfIssue, "No Stream", this,
                    NULL);
        return false;
    }

    if (isStreamFiltered()) {
        return true;
    }

    SkPdfStreamCommonDictionary* stream = (SkPdfStreamCommonDictionary*)this;

    if (!stream->has_Filter()) {
        fStr.fBytes = ((fStr.fBytes >> 1) << 1) + kFilteredStreamBit;
    } else if (stream->isFilterAName(NULL)) {
        SkString filterName = stream->getFilterAsName(NULL);
        applyFilter(filterName.c_str());
    } else if (stream->isFilterAArray(NULL)) {
        const SkPdfArray* filters = stream->getFilterAsArray(NULL);
        int cnt = (int) filters->size();
        for (int i = cnt - 1; i >= 0; i--) {
            const SkPdfNativeObject* filterName = filters->objAtAIndex(i);
            if (filterName != NULL && filterName->isName()) {
                if (!applyFilter(filterName->nameValue())) {
                    break;
                }
            } else {
                SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kIncositentSyntax_SkPdfIssue,
                            "filter name should be a Name", this, NULL);
            }
        }
    }

    return true;
}

void SkPdfNativeObject::releaseData() {
#ifdef PDF_TRACK_OBJECT_USAGE
    SkPdfReportIf(!fUsed, kInfo_SkPdfIssueSeverity, kUnusedObject_SkPdfIssue,
                  "Unused object in rendering", this, NULL);
#endif  // PDF_TRACK_OBJECT_USAGE

    SkPdfMarkObjectUnused();

    if (fData) {
        switch (fDataType) {
            case kFont_Data:
                delete (SkPdfFont*)fData;
                break;
            case kBitmap_Data:
                delete (SkBitmap*)fData;
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
    fData = NULL;
    fDataType = kEmpty_Data;
}
