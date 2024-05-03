/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTag_DEFINED
#define SkPDFTag_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/docs/SkPDFDocument.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkTHash.h"
#include "src/pdf/SkPDFTypes.h"

#include <cstddef>
#include <vector>

class SkPDFDocument;
struct SkPDFTagNode;
struct SkPoint;

class SkPDFTagTree {
public:
    SkPDFTagTree();
    ~SkPDFTagTree();
    void init(SkPDF::StructureElementNode*, SkPDF::Metadata::Outline);

    class Mark {
        SkPDFTagNode *const fNode;
        size_t const fMarkIndex;
    public:
        Mark(SkPDFTagNode* node, size_t index) : fNode(node), fMarkIndex(index) {}
        Mark() : Mark(nullptr, 0) {}
        Mark(const Mark&) = delete;
        Mark& operator=(const Mark&) = delete;
        Mark(Mark&&) = default;
        Mark& operator=(Mark&&) = delete;

        explicit operator bool() const { return fNode; }
        int id();
        SkPoint& point();
    };
    // Used to allow marked content to refer to its corresponding structure
    // tree node, via a page entry in the parent tree. Returns a false mark if
    // nodeId is 0.
    Mark createMarkIdForNodeId(int nodeId, unsigned pageIndex, SkPoint);
    // Used to allow annotations to refer to their corresponding structure
    // tree node, via the struct parent tree. Returns -1 if no struct parent
    // key.
    int createStructParentKeyForNodeId(int nodeId, unsigned pageIndex);

    void addNodeAnnotation(int nodeId, SkPDFIndirectReference annotationRef, unsigned pageIndex);
    void addNodeTitle(int nodeId, SkSpan<const char>);
    SkPDFIndirectReference makeStructTreeRoot(SkPDFDocument* doc);
    SkPDFIndirectReference makeOutline(SkPDFDocument* doc);
    SkString getRootLanguage();

private:
    // An entry in a map from a node ID to an indirect reference to its
    // corresponding structure element node.
    struct IDTreeEntry {
        int nodeId;
        SkPDFIndirectReference ref;
    };

    void Copy(SkPDF::StructureElementNode& node,
              SkPDFTagNode* dst,
              SkArenaAlloc* arena,
              skia_private::THashMap<int, SkPDFTagNode*>* nodeMap,
              bool wantTitle);
    SkPDFIndirectReference PrepareTagTreeToEmit(SkPDFIndirectReference parent,
                                                SkPDFTagNode* node,
                                                SkPDFDocument* doc);

    SkArenaAlloc fArena;
    skia_private::THashMap<int, SkPDFTagNode*> fNodeMap;
    SkPDFTagNode* fRoot = nullptr;
    SkPDF::Metadata::Outline fOutline;
    skia_private::TArray<skia_private::TArray<SkPDFTagNode*>> fMarksPerPage;
    std::vector<IDTreeEntry> fIdTreeEntries;
    std::vector<int> fParentTreeAnnotationNodeIds;

    SkPDFTagTree(const SkPDFTagTree&) = delete;
    SkPDFTagTree& operator=(const SkPDFTagTree&) = delete;
};

#endif
