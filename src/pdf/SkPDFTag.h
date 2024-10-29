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

class SkPDFDocument;
struct SkPDFStructElem;
struct SkPoint;

class SkPDFStructTree {
public:
    SkPDFStructTree(SkPDF::StructureElementNode*, SkPDF::Metadata::Outline);
    SkPDFStructTree(const SkPDFStructTree&) = delete;
    SkPDFStructTree& operator=(const SkPDFStructTree&) = delete;
    SkPDFStructTree(SkPDFStructTree&&) = delete;
    SkPDFStructTree& operator=(SkPDFStructTree&&) = delete;
    ~SkPDFStructTree();

    class Mark {
        SkPDFStructElem* fStructElem;
        size_t fMarkIndex;
    public:
        Mark(SkPDFStructElem* structElem, size_t markIndex)
            : fStructElem(structElem), fMarkIndex(markIndex) {}
        Mark() : Mark(nullptr, 0) {}
        Mark(const Mark&) = default;
        Mark& operator=(const Mark&) = default;
        Mark(Mark&&) = default;
        Mark& operator=(Mark&&) = default;

        explicit operator bool() const { return fStructElem; }
        int mcid() const; // mcid < 0 means no active mark, if bool(this) always >= 0
        int elemId() const; // 0 elemId means no active structure element
        SkString structType() const; // only call when bool(this)
        void accumulate(SkPoint); // only call when bool(this)
    };

    // Create a new marked-content identifier (MCID) to be used with a marked-content sequence
    // parented by the structure element (StructElem) with the given element identifier (elemId).
    // The StructTreeRoot::ParentTree[Page::StructParent][mcid] will refer to the structure element.
    // The structure element will add this MCID as its next child (in StructElem::K).
    // Returns a false Mark if if elemId does not refer to a StructElem.
    SkPDFStructTree::Mark createMarkForElemId(int elemId, unsigned pageIndex);

    // Create a key to use with /StructParent in a content item (usually an annotation) which refers
    // to the structure element (StructElem) with the given element identifier (elemId).
    // The StructTreeRoot ParentTree will map from this key to the structure element.
    // The structure element will add the content item as its next child (as StructElem::K::OBJR).
    // Returns -1 if elemId does not refer to a StructElem.
    int createStructParentKeyForElemId(int elemId, SkPDFIndirectReference contentItemRef,
                                       unsigned pageIndex);

    void addStructElemTitle(int elemId, SkSpan<const char>);
    SkPDFIndirectReference emitStructTreeRoot(SkPDFDocument* doc) const;
    SkPDFIndirectReference makeOutline(SkPDFDocument* doc) const;
    SkString getRootLanguage();

    // An entry in an ordered map from an element identifier to an indirect reference to its
    // corresponding structure element.
    struct IDTreeEntry {
        int elemId;
        SkPDFIndirectReference structElemRef;
    };
private:
    void move(SkPDF::StructureElementNode& node, SkPDFStructElem* structElem, bool wantTitle);

    SkArenaAlloc fArena;
    skia_private::THashMap<int, SkPDFStructElem*> fStructElemForElemId;
    SkPDFStructElem* fRoot = nullptr;
    SkPDF::Metadata::Outline fOutline = SkPDF::Metadata::Outline::None;
    // fStructElemForMcidForPage[Page::StructParents][mcid] -> parent StructElem of mcid
    skia_private::TArray<skia_private::TArray<SkPDFStructElem*>> fStructElemForMcidForPage;
    // fStructElemForContentItem[?::StructParent] -> parent StructElem of content-item
    skia_private::TArray<SkPDFStructElem*> fStructElemForContentItem;
};

#endif
