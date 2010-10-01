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

SkPDFCatalog::SkPDFCatalog()
    : fNextObjNum(1),
      fStartedAssigningObjNums(false),
      fAssigningFirstPageObjNums(false) {
}

SkPDFCatalog::~SkPDFCatalog() {}

void SkPDFCatalog::addObject(SkPDFObject* obj, bool onFirstPage) {
    SkASSERT(findObjectIndex(obj) == -1);
    SkASSERT(!fStartedAssigningObjNums);

    struct Rec newEntry(obj, onFirstPage);
    fCatalog.append(1, &newEntry);
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
    SkASSERT(pos >= 0);
    uint32_t currentIndex = pos;
    if (fCatalog[currentIndex].fObjNumAssigned)
        return currentIndex + 1;

    fStartedAssigningObjNums = true;
    if (fCatalog[currentIndex].fOnFirstPage) {
        fAssigningFirstPageObjNums = true;
    } else {
        SkASSERT(!fAssigningFirstPageObjNums);
    }

    // When we assign an object an object number, we put it in that array
    // offset (minus 1 because object number 0 is reserved).
    if (fNextObjNum - 1 != currentIndex) {
        Rec other = fCatalog[fNextObjNum - 1];
        fCatalog[fNextObjNum - 1] = fCatalog[currentIndex];
        fCatalog[currentIndex] = other;
    }
    fCatalog[fNextObjNum - 1].fObjNumAssigned = true;
    fNextObjNum++;
    return fNextObjNum - 1;
}
