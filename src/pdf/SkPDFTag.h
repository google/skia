/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTag_DEFINED
#define SkPDFTag_DEFINED

#include "include/docs/SkPDFDocument.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkTHash.h"

class SkPDFDocument;
struct SkPDFIndirectReference;
struct SkPDFTagNode;

class SkPDFTagTree {
public:
    SkPDFTagTree();
    ~SkPDFTagTree();
    void init(SkPDF::StructureElementNode*);
    // Used to allow marked content to refer to its corresponding structure
    // tree node, via a page entry in the parent tree. Returns -1 if no
    // mark ID.
    int createMarkIdForNodeId(int nodeId, unsigned pageIndex);
    // Used to allow annotations to refer to their corresponding structure
    // tree node, via the struct parent tree. Returns -1 if no struct parent
    // key.
    int createStructParentKeyForNodeId(int nodeId, unsigned pageIndex);

    void addNodeAnnotation(int nodeId, SkPDFIndirectReference annotationRef, unsigned pageIndex);
    SkPDFIndirectReference makeStructTreeRoot(SkPDFDocument* doc);

private:
    // An entry in a map from a node ID to an indirect reference to its
    // corresponding structure element node.
    struct IDTreeEntry {
        int nodeId;
        SkPDFIndirectReference ref;
    };

    static void Copy(SkPDF::StructureElementNode& node,
                     SkPDFTagNode* dst,
                     SkArenaAlloc* arena,
                     skia_private::THashMap<int, SkPDFTagNode*>* nodeMap);
    SkPDFIndirectReference PrepareTagTreeToEmit(SkPDFIndirectReference parent,
                                                SkPDFTagNode* node,
                                                SkPDFDocument* doc);

    SkArenaAlloc fArena;
    skia_private::THashMap<int, SkPDFTagNode*> fNodeMap;
    SkPDFTagNode* fRoot = nullptr;
    skia_private::TArray<skia_private::TArray<SkPDFTagNode*>> fMarksPerPage;
    std::vector<IDTreeEntry> fIdTreeEntries;
    std::vector<int> fParentTreeAnnotationNodeIds;

    SkPDFTagTree(const SkPDFTagTree&) = delete;
    SkPDFTagTree& operator=(const SkPDFTagTree&) = delete;
};

#endif
