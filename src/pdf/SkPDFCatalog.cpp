/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkPDFCatalog.h"
#include "SkPDFTypes.h"
#include "SkStream.h"
#include "SkTypes.h"

SkPDFCatalog::SkPDFCatalog()
    : fFirstPageCount(0),
      fNextObjNum(1),
      fNextFirstPageObjNum(0) {
}

SkPDFCatalog::~SkPDFCatalog() {}

SkPDFObject* SkPDFCatalog::addObject(SkPDFObject* obj, bool onFirstPage) {
    SkASSERT(findObjectIndex(obj) == -1);
    SkASSERT(fNextFirstPageObjNum == 0);
    if (onFirstPage)
        fFirstPageCount++;

    struct Rec newEntry(obj, onFirstPage);
    fCatalog.append(1, &newEntry);
    return obj;
}

size_t SkPDFCatalog::setFileOffset(SkPDFObject* obj, size_t offset) {
    int objIndex = assignObjNum(obj) - 1;
    SkASSERT(fCatalog[objIndex].fObjNumAssigned);
    SkASSERT(fCatalog[objIndex].fFileOffset == 0);
    fCatalog[objIndex].fFileOffset = offset;

    return obj->getOutputSize(this, true);
}

void SkPDFCatalog::emitObjectNumber(SkWStream* stream, SkPDFObject* obj) {
    stream->writeDecAsText(assignObjNum(obj));
    stream->writeText(" 0");  // Generation number is always 0.
}

size_t SkPDFCatalog::getObjectNumberSize(SkPDFObject* obj) {
    SkDynamicMemoryWStream buffer;
    emitObjectNumber(&buffer, obj);
    return buffer.getOffset();
}

int SkPDFCatalog::findObjectIndex(SkPDFObject* obj) const {
    for (int i = 0; i < fCatalog.count(); i++) {
        if (fCatalog[i].fObject == obj)
            return i;
    }
    return -1;
}

int SkPDFCatalog::assignObjNum(SkPDFObject* obj) {
    int pos = findObjectIndex(obj);
    // If this assert fails, it means you probably forgot to add an object
    // to the resource list.
    SkASSERT(pos >= 0);
    uint32_t currentIndex = pos;
    if (fCatalog[currentIndex].fObjNumAssigned)
        return currentIndex + 1;

    // First assignment.
    if (fNextFirstPageObjNum == 0)
        fNextFirstPageObjNum = fCatalog.count() - fFirstPageCount + 1;

    uint32_t objNum;
    if (fCatalog[currentIndex].fOnFirstPage) {
        objNum = fNextFirstPageObjNum;
        fNextFirstPageObjNum++;
    } else {
        objNum = fNextObjNum;
        fNextObjNum++;
    }

    // When we assign an object an object number, we put it in that array
    // offset (minus 1 because object number 0 is reserved).
    SkASSERT(!fCatalog[objNum - 1].fObjNumAssigned);
    if (objNum - 1 != currentIndex)
        SkTSwap(fCatalog[objNum - 1], fCatalog[currentIndex]);
    fCatalog[objNum - 1].fObjNumAssigned = true;
    return objNum;
}

int32_t SkPDFCatalog::emitXrefTable(SkWStream* stream, bool firstPage) {
    int first = -1;
    int last = fCatalog.count() - 1;
    // TODO(vandebo) support linearized format.
    //int last = fCatalog.count() - fFirstPageCount - 1;
    //if (firstPage) {
    //    first = fCatalog.count() - fFirstPageCount;
    //    last = fCatalog.count() - 1;
    //}

    stream->writeText("xref\n");
    stream->writeDecAsText(first + 1);
    stream->writeText(" ");
    stream->writeDecAsText(last - first + 1);
    stream->writeText("\n");

    if (first == -1) {
        stream->writeText("0000000000 65535 f \n");
        first++;
    }
    for (int i = first; i <= last; i++) {
        SkASSERT(fCatalog[i].fFileOffset > 0);
        SkASSERT(fCatalog[i].fFileOffset <= 9999999999LL);
        stream->writeBigDecAsText(fCatalog[i].fFileOffset, 10);
        stream->writeText(" 00000 n \n");
    }

    return fCatalog.count() + 1;
}
