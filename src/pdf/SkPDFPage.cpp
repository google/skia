
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFCatalog.h"
#include "SkPDFDevice.h"
#include "SkPDFPage.h"
#include "SkStream.h"

SkPDFPage::SkPDFPage(SkPDFDevice* content)
    : SkPDFDict("Page"),
      fDevice(content) {
}

SkPDFPage::~SkPDFPage() {}

void SkPDFPage::finalizePage(SkPDFCatalog* catalog, bool firstPage,
                             SkTDArray<SkPDFObject*>* resourceObjects) {
    if (fContentStream.get() == NULL) {
        insert("Resources", fDevice->getResourceDict());
        insert("MediaBox", fDevice->getMediaBox().get());

        SkRefPtr<SkStream> content = fDevice->content();
        content->unref();  // SkRefPtr and content() both took a reference.
        fContentStream = new SkPDFStream(content.get());
        fContentStream->unref();  // SkRefPtr and new both took a reference.
        insert("Contents", new SkPDFObjRef(fContentStream.get()))->unref();
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
void SkPDFPage::GeneratePageTree(const SkTDArray<SkPDFPage*>& pages,
                                 SkPDFCatalog* catalog,
                                 SkTDArray<SkPDFDict*>* pageTree,
                                 SkPDFDict** rootNode) {
    // PDF wants a tree describing all the pages in the document.  We arbitrary
    // choose 8 (kNodeSize) as the number of allowed children.  The internal
    // nodes have type "Pages" with an array of children, a parent pointer, and
    // the number of leaves below the node as "Count."  The leaves are passed
    // into the method, have type "Page" and need a parent pointer. This method
    // builds the tree bottom up, skipping internal nodes that would have only
    // one child.
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
        SkSafeRef(pages[i]);
        curNodes.push(pages[i]);
    }

    // nextRoundNodes passes its references to nodes on to curNodes.
    SkTDArray<SkPDFDict*> nextRoundNodes;
    nextRoundNodes.setReserve((pages.count() + kNodeSize - 1)/kNodeSize);

    int treeCapacity = kNodeSize;
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
                kids->append(new SkPDFObjRef(curNodes[i]))->unref();

                // TODO(vandebo): put the objects in strict access order.
                // Probably doesn't matter because they are so small.
                if (curNodes[i] != pages[0]) {
                    pageTree->push(curNodes[i]);  // Transfer reference.
                    catalog->addObject(curNodes[i], false);
                } else {
                    SkSafeUnref(curNodes[i]);
                    catalog->addObject(curNodes[i], true);
                }
            }

            newNode->insert(kidsName.get(), kids.get());
            int pageCount = treeCapacity;
            if (count < kNodeSize) {
                pageCount = pages.count() % treeCapacity;
            }
            newNode->insert(countName.get(), new SkPDFInt(pageCount))->unref();
            nextRoundNodes.push(newNode);  // Transfer reference.
        }

        curNodes = nextRoundNodes;
        nextRoundNodes.rewind();
        treeCapacity *= kNodeSize;
    } while (curNodes.count() > 1);

    pageTree->push(curNodes[0]);  // Transfer reference.
    catalog->addObject(curNodes[0], false);
    if (rootNode) {
        *rootNode = curNodes[0];
    }
}

const SkTDArray<SkPDFFont*>& SkPDFPage::getFontResources() const {
    return fDevice->getFontResources();
}

const SkPDFGlyphSetMap& SkPDFPage::getFontGlyphUsage() const {
    return fDevice->getFontGlyphUsage();
}
