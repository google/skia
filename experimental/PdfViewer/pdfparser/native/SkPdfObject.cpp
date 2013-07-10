
#include "SkPdfObject.h"
#include "SkPdfStreamCommonDictionary_autogen.h"

#include "SkFlate.h"
#include "SkStream.h"
#include "SkPdfNativeTokenizer.h"

SkPdfObject SkPdfObject::kNull = SkPdfObject::makeNull();

bool SkPdfObject::applyFlateDecodeFilter(SkPdfAllocator* allocator) {
    if (!SkFlate::HaveFlate()) {
        // TODO(edisonn): warn, make callers handle it
        return false;
    }

    SkMemoryStream skstream(fStr.fBuffer, fStr.fBytes >> 1, false);
    SkDynamicMemoryWStream uncompressedData;

    if (SkFlate::Inflate(&skstream, &uncompressedData)) {
        fStr.fBytes = (uncompressedData.bytesWritten() << 1) + kUnfilteredStreamBit;
        fStr.fBuffer = (unsigned char*)allocator->alloc(uncompressedData.bytesWritten());
        uncompressedData.copyTo(fStr.fBuffer);
        return true;
    } else {
        // TODO(edisonn): warn, make callers handle it
        return false;
    }
}

bool SkPdfObject::applyDCTDecodeFilter(SkPdfAllocator* allocator) {
    // this would fail, and it won't allow any more filters.
    // technically, it would be possible, but not a real world scenario
    // TODO(edisonn): or get the image here and store it for fast retrieval?
    return false;
}

bool SkPdfObject::applyFilter(const char* name, SkPdfAllocator* allocator) {
    if (strcmp(name, "FlateDecode") == 0) {
        return applyFlateDecodeFilter(allocator);
    } else if (strcmp(name, "DCTDecode") == 0) {
        return applyDCTDecodeFilter(allocator);
    }
    // TODO(edisonn): allert, not supported, but should be implemented asap
    return false;
}

bool SkPdfObject::filterStream(SkPdfAllocator* allocator) {
    if (!hasStream()) {
        return false;
    }

    if (isStreamFiltered()) {
        return true;
    }

    SkPdfStreamCommonDictionary* stream = (SkPdfStreamCommonDictionary*)this;

    if (!stream->has_Filter()) {
        fStr.fBytes = ((fStr.fBytes >> 1) << 1) + kFilteredStreamBit;
        return true;
    }

    if (stream->isFilterAName(NULL)) {
        std::string filterName = stream->getFilterAsName(NULL);
        applyFilter(filterName.c_str(), allocator);
    } else if (stream->isFilterAArray(NULL)) {
        const SkPdfArray* filters = stream->getFilterAsArray(NULL);
        int cnt = filters->size();
        for (int i = cnt - 1; i >= 0; i--) {
            const SkPdfObject* filterName = filters->objAtAIndex(i);
            if (filterName != NULL && filterName->isName()) {
                if (!applyFilter(filterName->nameValue(), allocator)) {
                    break;
                }
            } else {
                // TODO(edisonn): report warning
            }
        }
    }

    fStr.fBytes = ((fStr.fBytes >> 1) << 1) + kFilteredStreamBit;

    return true;
}
