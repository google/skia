/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTag_DEFINED
#define SkPDFTag_DEFINED

#include "include/docs/SkPDFDocument.h"
#include "include/private/SkArenaAlloc.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"

class SkPDFDocument;
struct SkPDFTagNode;

class SkPDFTagTree {
public:
    SkPDFTagTree();
    ~SkPDFTagTree();
    void init(const SkPDF::StructureElementNode*);
    void reset();
    int getMarkIdForNodeId(int nodeId, unsigned pageIndex);
    SkPDFIndirectReference makeStructTreeRoot(SkPDFDocument* doc);

private:
    SkArenaAlloc fArena;
    SkTHashMap<int, SkPDFTagNode*> fNodeMap;
    SkPDFTagNode* fRoot = nullptr;
    SkTArray<SkTArray<SkPDFTagNode*>> fMarksPerPage;

    SkPDFTagTree(const SkPDFTagTree&) = delete;
    SkPDFTagTree& operator=(const SkPDFTagTree&) = delete;
};

#endif
