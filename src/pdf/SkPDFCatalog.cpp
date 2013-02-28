
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFCatalog.h"
#include "SkPDFTypes.h"
#include "SkStream.h"
#include "SkTypes.h"

SkPDFCatalog::SkPDFCatalog(SkPDFDocument::Flags flags)
    : fFirstPageCount(0),
      fNextObjNum(1),
      fNextFirstPageObjNum(0),
      fDocumentFlags(flags) {
}

SkPDFCatalog::~SkPDFCatalog() {
    fSubstituteResourcesRemaining.safeUnrefAll();
    fSubstituteResourcesFirstPage.safeUnrefAll();
}

SkPDFObject* SkPDFCatalog::addObject(SkPDFObject* obj, bool onFirstPage) {
    if (findObjectIndex(obj) != -1) {  // object already added
        return obj;
    }
    SkASSERT(fNextFirstPageObjNum == 0);
    if (onFirstPage) {
        fFirstPageCount++;
    }

    struct Rec newEntry(obj, onFirstPage);
    fCatalog.append(1, &newEntry);
    return obj;
}

size_t SkPDFCatalog::setFileOffset(SkPDFObject* obj, off_t offset) {
    int objIndex = assignObjNum(obj) - 1;
    SkASSERT(fCatalog[objIndex].fObjNumAssigned);
    SkASSERT(fCatalog[objIndex].fFileOffset == 0);
    fCatalog[objIndex].fFileOffset = offset;

    return getSubstituteObject(obj)->getOutputSize(this, true);
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
        if (fCatalog[i].fObject == obj) {
            return i;
        }
    }
    // If it's not in the main array, check if it's a substitute object.
    for (int i = 0; i < fSubstituteMap.count(); ++i) {
        if (fSubstituteMap[i].fSubstitute == obj) {
            return findObjectIndex(fSubstituteMap[i].fOriginal);
        }
    }
    return -1;
}

int SkPDFCatalog::assignObjNum(SkPDFObject* obj) {
    int pos = findObjectIndex(obj);
    // If this assert fails, it means you probably forgot to add an object
    // to the resource list.
    SkASSERT(pos >= 0);
    uint32_t currentIndex = pos;
    if (fCatalog[currentIndex].fObjNumAssigned) {
        return currentIndex + 1;
    }

    // First assignment.
    if (fNextFirstPageObjNum == 0) {
        fNextFirstPageObjNum = fCatalog.count() - fFirstPageCount + 1;
    }

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
    if (objNum - 1 != currentIndex) {
        SkTSwap(fCatalog[objNum - 1], fCatalog[currentIndex]);
    }
    fCatalog[objNum - 1].fObjNumAssigned = true;
    return objNum;
}

int32_t SkPDFCatalog::emitXrefTable(SkWStream* stream, bool firstPage) {
    int first = -1;
    int last = fCatalog.count() - 1;
    // TODO(vandebo): Support linearized format.
    // int last = fCatalog.count() - fFirstPageCount - 1;
    // if (firstPage) {
    //     first = fCatalog.count() - fFirstPageCount;
    //     last = fCatalog.count() - 1;
    // }

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

void SkPDFCatalog::setSubstitute(SkPDFObject* original,
                                 SkPDFObject* substitute) {
#if defined(SK_DEBUG)
    // Sanity check: is the original already in substitute list?
    for (int i = 0; i < fSubstituteMap.count(); ++i) {
        if (original == fSubstituteMap[i].fSubstitute ||
            original == fSubstituteMap[i].fOriginal) {
            SkASSERT(false);
            return;
        }
    }
#endif
    // Check if the original is on first page.
    bool onFirstPage = false;
    for (int i = 0; i < fCatalog.count(); ++i) {
        if (fCatalog[i].fObject == original) {
            onFirstPage = fCatalog[i].fOnFirstPage;
            break;
        }
#if defined(SK_DEBUG)
        if (i == fCatalog.count() - 1) {
            SkASSERT(false);  // original not in catalog
            return;
        }
#endif
    }

    SubstituteMapping newMapping(original, substitute);
    fSubstituteMap.append(1, &newMapping);

    // Add resource objects of substitute object to catalog.
    SkTDArray<SkPDFObject*>* targetList = getSubstituteList(onFirstPage);
    int existingSize = targetList->count();
    newMapping.fSubstitute->getResources(targetList);
    for (int i = existingSize; i < targetList->count(); ++i) {
        addObject((*targetList)[i], onFirstPage);
    }
}

SkPDFObject* SkPDFCatalog::getSubstituteObject(SkPDFObject* object) {
    for (int i = 0; i < fSubstituteMap.count(); ++i) {
        if (object == fSubstituteMap[i].fOriginal) {
            return fSubstituteMap[i].fSubstitute;
        }
    }
    return object;
}

off_t SkPDFCatalog::setSubstituteResourcesOffsets(off_t fileOffset,
                                                  bool firstPage) {
    SkTDArray<SkPDFObject*>* targetList = getSubstituteList(firstPage);
    off_t offsetSum = fileOffset;
    for (int i = 0; i < targetList->count(); ++i) {
        offsetSum += setFileOffset((*targetList)[i], offsetSum);
    }
    return offsetSum - fileOffset;
}

void SkPDFCatalog::emitSubstituteResources(SkWStream *stream, bool firstPage) {
    SkTDArray<SkPDFObject*>* targetList = getSubstituteList(firstPage);
    for (int i = 0; i < targetList->count(); ++i) {
        (*targetList)[i]->emit(stream, this, true);
    }
}

SkTDArray<SkPDFObject*>* SkPDFCatalog::getSubstituteList(bool firstPage) {
    return firstPage ? &fSubstituteResourcesFirstPage :
                       &fSubstituteResourcesRemaining;
}
