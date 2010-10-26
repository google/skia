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
#include "SkPDFDevice.h"
#include "SkPDFPage.h"
#include "SkStream.h"

SkPDFPage::SkPDFPage(const SkRefPtr<SkPDFDevice>& content)
    : SkPDFDict("Page"),
      fDevice(content) {
}

SkPDFPage::~SkPDFPage() {}

void SkPDFPage::finalizePage(SkPDFCatalog* catalog, bool firstPage,
                             SkTDArray<SkPDFObject*>* resourceObjects) {
    if (fContentStream.get() == NULL) {
        insert("Resources", fDevice->getResourceDict().get());
        insert("MediaBox", fDevice->getMediaBox().get());

        fContent = fDevice->content(true);
        SkRefPtr<SkMemoryStream> contentStream = new SkMemoryStream(
                fContent.c_str(), fContent.size());
        contentStream->unref();  // SkRefPtr and new both took a reference.
        fContentStream = new SkPDFStream(contentStream.get());
        fContentStream->unref();  // SkRefPtr and new both took a reference.
        SkRefPtr<SkPDFObjRef> contentRef =
            new SkPDFObjRef(fContentStream.get());
        contentRef->unref();  // SkRefPtr and new both took a reference.
        insert("Contents", contentRef.get());
    }
    catalog->addObject(fContentStream.get(), firstPage);
    fDevice->getResources(resourceObjects);
}

off_t SkPDFPage::getPageSize(SkPDFCatalog* catalog, off_t fileOffset) {
    SkASSERT(fContentStream.get() != NULL);
    catalog->setFileOffset(fContentStream.get(), fileOffset);
    return fContentStream->getOutputSize(catalog, true);
}

void SkPDFPage::emitPage(SkWStream* stream, SkPDFCatalog* catalog) {
    SkASSERT(fContentStream.get() != NULL);
    fContentStream->emitObject(stream, catalog, true);
}

// static
void SkPDFPage::generatePageTree(const SkTDArray<SkPDFPage*>& pages,
                                 SkPDFCatalog* catalog,
                                 SkTDArray<SkPDFDict*>* pageTree,
                                 SkPDFDict** rootNode) {
    static const int kNodeSize = 8;

    SkRefPtr<SkPDFName> kidsName = new SkPDFName("Kids");
    kidsName->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFName> countName = new SkPDFName("Count");
    countName->unref();  // SkRefPtr and new both took a reference.
    SkRefPtr<SkPDFName> parentName = new SkPDFName("Parent");
    parentName->unref();  // SkRefPtr and new both took a reference.

    // curNodes takes a reference to its items, which it passes to pageTree.
    SkTDArray<SkPDFDict*> curNodes;
    curNodes.setReserve(pages.count());
    for (int i = 0; i < pages.count(); i++) {
        pages[i]->safeRef();
        curNodes.push(pages[i]);
    }

    // nextRoundNodes passes its references to nodes on to curNodes.
    SkTDArray<SkPDFDict*> nextRoundNodes;
    nextRoundNodes.setReserve((pages.count() + kNodeSize - 1)/kNodeSize);

    do {
        for (int i = 0; i < curNodes.count(); ) {
            if (i > 0 && i + 1 == curNodes.count()) {
                nextRoundNodes.push(curNodes[i]);
                break;
            }

            SkPDFDict* newNode = new SkPDFDict("Pages");
            SkRefPtr<SkPDFObjRef> newNodeRef = new SkPDFObjRef(newNode);
            newNodeRef->unref();  // SkRefPtr and new both took a reference.

            SkRefPtr<SkPDFArray> kids = new SkPDFArray;
            kids->unref();  // SkRefPtr and new both took a reference.
            kids->reserve(kNodeSize);

            int count = 0;
            for (; i < curNodes.count() && count < kNodeSize; i++, count++) {
                curNodes[i]->insert(parentName.get(), newNodeRef.get());
                SkRefPtr<SkPDFObjRef> nodeRef = new SkPDFObjRef(curNodes[i]);
                nodeRef->unref();  // SkRefPtr and new both took a reference.
                kids->append(nodeRef.get());

                // TODO(vandebo) put the objects in strict access order.
                // Probably doesn't matter because they are so small.
                if (curNodes[i] != pages[0]) {
                    pageTree->push(curNodes[i]); // Transfer reference.
                    catalog->addObject(curNodes[i], false);
                } else {
                    curNodes[i]->safeUnref();
                }
            }

            newNode->insert(kidsName.get(), kids.get());
            SkRefPtr<SkPDFInt> countVal = new SkPDFInt(count);
            countVal->unref();  // SkRefPtr and new both took a reference.
            newNode->insert(countName.get(), countVal.get());
            nextRoundNodes.push(newNode);  // Transfer reference.
        }

        curNodes = nextRoundNodes;
        nextRoundNodes.rewind();
    } while(curNodes.count() > 1);

    pageTree->push(curNodes[0]); // Transfer reference.
    catalog->addObject(curNodes[0], false);
    if (rootNode)
        *rootNode = curNodes[0];
}
