/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTag_DEFINED
#define SkPDFTag_DEFINED

#include "include/docs/SkPDFDocument.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkArenaAlloc.h"

class SkPDFDocument;
struct SkPDFIndirectReference;
struct SkPDFTagNode;

class SkPDFTagTree {
public:
    SkPDFTagTree();
    ~SkPDFTagTree();
    void init(SkPDF::StructureElementNode*);
    void reset();
    int getMarkIdForNodeId(int nodeId, unsigned pageIndex);
    void addNodeAnnotation(int nodeId, SkPDFIndirectReference annotationRef);
    SkPDFIndirectReference makeStructTreeRoot(SkPDFDocument* doc);

private:
    static void Copy(SkPDF::StructureElementNode& node,
                     SkPDFTagNode* dst,
                     SkArenaAlloc* arena,
                     SkTHashMap<int, SkPDFTagNode*>* nodeMap);

    SkArenaAlloc fArena;
    SkTHashMap<int, SkPDFTagNode*> fNodeMap;
    SkPDFTagNode* fRoot = nullptr;
    SkTArray<SkTArray<SkPDFTagNode*>> fMarksPerPage;

    SkPDFTagTree(const SkPDFTagTree&) = delete;
    SkPDFTagTree& operator=(const SkPDFTagTree&) = delete;
};

#endif
