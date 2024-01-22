/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFTag.h"

using namespace skia_private;

// The struct parent tree consists of one entry per page, followed by
// entries for individual struct tree nodes corresponding to
// annotations.  Each entry is a key/value pair with an integer key
// and an indirect reference key.
//
// The page entries get consecutive keys starting at 0. Since we don't
// know the total number of pages in the document at the time we start
// processing annotations, start the key for annotations with a large
// number, which effectively becomes the maximum number of pages in a
// PDF we can handle.
const int kFirstAnnotationStructParentKey = 100000;

namespace {
struct Location {
    SkPoint fPoint{SK_ScalarNaN, SK_ScalarNaN};
    unsigned fPageIndex{0};

    void accumulate(Location const& child) {
        if (!child.fPoint.isFinite()) {
            return;
        }
        if (!fPoint.isFinite()) {
            *this = child;
            return;
        }
        if (child.fPageIndex < fPageIndex) {
            *this = child;
            return;
        }
        if (child.fPageIndex == fPageIndex) {
            fPoint.fX = std::min(child.fPoint.fX, fPoint.fX);
            fPoint.fY = std::max(child.fPoint.fY, fPoint.fY); // PDF y-up
            return;
        }
    }
};
} // namespace

struct SkPDFTagNode {
    // Structure element nodes need a unique alphanumeric ID,
    // and we need to be able to output them sorted in lexicographic
    // order. This helper function takes one of our node IDs and
    // builds an ID string that zero-pads the digits so that lexicographic
    // order matches numeric order.
    static SkString nodeIdToString(int nodeId) {
        SkString idString;
        idString.printf("node%08d", nodeId);
        return idString;
    }

    SkPDFTagNode* fChildren = nullptr;
    size_t fChildCount = 0;
    struct MarkedContentInfo {
        Location fLocation;
        int fMarkId;
    };
    TArray<MarkedContentInfo> fMarkedContent;
    int fNodeId;
    bool fWantTitle;
    SkString fTypeString;
    SkString fTitle;
    SkString fAlt;
    SkString fLang;
    SkPDFIndirectReference fRef;
    enum State {
        kUnknown,
        kYes,
        kNo,
    } fCanDiscard = kUnknown;
    std::unique_ptr<SkPDFArray> fAttributes;
    struct AnnotationInfo {
        unsigned fPageIndex;
        SkPDFIndirectReference fAnnotationRef;
    };
    std::vector<AnnotationInfo> fAnnotations;
};

SkPDF::AttributeList::AttributeList() = default;

SkPDF::AttributeList::~AttributeList() = default;

void SkPDF::AttributeList::appendInt(
        const char* owner, const char* name, int value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertInt(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendFloat(
        const char* owner, const char* name, float value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertScalar(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendName(
        const char* owner, const char* name, const char* value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertName(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendFloatArray(
        const char* owner, const char* name, const std::vector<float>& value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (float element : value) {
        pdfArray->appendScalar(element);
    }
    attrDict->insertObject(name, std::move(pdfArray));
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendNodeIdArray(
        const char* owner,
        const char* name,
        const std::vector<int>& nodeIds) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (int nodeId : nodeIds) {
        SkString idString = SkPDFTagNode::nodeIdToString(nodeId);
        pdfArray->appendByteString(idString);
    }
    attrDict->insertObject(name, std::move(pdfArray));
    fAttrs->appendObject(std::move(attrDict));
}

SkPDFTagTree::SkPDFTagTree() : fArena(4 * sizeof(SkPDFTagNode)) {}

SkPDFTagTree::~SkPDFTagTree() = default;

// static
void SkPDFTagTree::Copy(SkPDF::StructureElementNode& node,
                        SkPDFTagNode* dst,
                        SkArenaAlloc* arena,
                        THashMap<int, SkPDFTagNode*>* nodeMap,
                        bool wantTitle) {
    nodeMap->set(node.fNodeId, dst);
    for (int nodeId : node.fAdditionalNodeIds) {
        SkASSERT(!nodeMap->find(nodeId));
        nodeMap->set(nodeId, dst);
    }
    dst->fNodeId = node.fNodeId;

    // Accumulate title text, need to be in sync with create_outline_from_headers
    const char* type = node.fTypeString.c_str();
    wantTitle |= fOutline == SkPDF::Metadata::Outline::StructureElementHeaders &&
                 type[0] == 'H' && '1' <= type[1] && type[1] <= '6';
    dst->fWantTitle = wantTitle;

    dst->fTypeString = node.fTypeString;
    dst->fAlt = node.fAlt;
    dst->fLang = node.fLang;

    size_t childCount = node.fChildVector.size();
    SkPDFTagNode* children = arena->makeArray<SkPDFTagNode>(childCount);
    dst->fChildCount = childCount;
    dst->fChildren = children;
    for (size_t i = 0; i < childCount; ++i) {
        Copy(*node.fChildVector[i], &children[i], arena, nodeMap, wantTitle);
    }

    dst->fAttributes = std::move(node.fAttributes.fAttrs);
}

void SkPDFTagTree::init(SkPDF::StructureElementNode* node, SkPDF::Metadata::Outline outline) {
    if (node) {
        fRoot = fArena.make<SkPDFTagNode>();
        fOutline = outline;
        Copy(*node, fRoot, &fArena, &fNodeMap, false);
    }
}

int SkPDFTagTree::Mark::id() {
    return fNode ? fNode->fMarkedContent[fMarkIndex].fMarkId : -1;
}

SkPoint& SkPDFTagTree::Mark::point() {
    return fNode->fMarkedContent[fMarkIndex].fLocation.fPoint;
}

auto SkPDFTagTree::createMarkIdForNodeId(int nodeId, unsigned pageIndex, SkPoint point) -> Mark {
    if (!fRoot) {
        return Mark();
    }
    SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
    if (!tagPtr) {
        return Mark();
    }
    SkPDFTagNode* tag = *tagPtr;
    SkASSERT(tag);
    while (SkToUInt(fMarksPerPage.size()) < pageIndex + 1) {
        fMarksPerPage.push_back();
    }
    TArray<SkPDFTagNode*>& pageMarks = fMarksPerPage[pageIndex];
    int markId = pageMarks.size();
    tag->fMarkedContent.push_back({{point, pageIndex}, markId});
    pageMarks.push_back(tag);
    return Mark(tag, tag->fMarkedContent.size() - 1);
}

int SkPDFTagTree::createStructParentKeyForNodeId(int nodeId, unsigned pageIndex) {
    if (!fRoot) {
        return -1;
    }
    SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
    if (!tagPtr) {
        return -1;
    }
    SkPDFTagNode* tag = *tagPtr;
    SkASSERT(tag);

    tag->fCanDiscard = SkPDFTagNode::kNo;

    int nextStructParentKey = kFirstAnnotationStructParentKey +
        static_cast<int>(fParentTreeAnnotationNodeIds.size());
    fParentTreeAnnotationNodeIds.push_back(nodeId);
    return nextStructParentKey;
}

static bool can_discard(SkPDFTagNode* node) {
    if (node->fCanDiscard == SkPDFTagNode::kYes) {
        return true;
    }
    if (node->fCanDiscard == SkPDFTagNode::kNo) {
        return false;
    }
    if (!node->fMarkedContent.empty()) {
        node->fCanDiscard = SkPDFTagNode::kNo;
        return false;
    }
    for (size_t i = 0; i < node->fChildCount; ++i) {
        if (!can_discard(&node->fChildren[i])) {
            node->fCanDiscard = SkPDFTagNode::kNo;
            return false;
        }
    }
    node->fCanDiscard = SkPDFTagNode::kYes;
    return true;
}

SkPDFIndirectReference SkPDFTagTree::PrepareTagTreeToEmit(SkPDFIndirectReference parent,
                                                          SkPDFTagNode* node,
                                                          SkPDFDocument* doc) {
    SkPDFIndirectReference ref = doc->reserveRef();
    std::unique_ptr<SkPDFArray> kids = SkPDFMakeArray();
    SkPDFTagNode* children = node->fChildren;
    size_t childCount = node->fChildCount;
    for (size_t i = 0; i < childCount; ++i) {
        SkPDFTagNode* child = &children[i];
        if (!(can_discard(child))) {
            kids->appendRef(PrepareTagTreeToEmit(ref, child, doc));
        }
    }
    for (const SkPDFTagNode::MarkedContentInfo& info : node->fMarkedContent) {
        std::unique_ptr<SkPDFDict> mcr = SkPDFMakeDict("MCR");
        mcr->insertRef("Pg", doc->getPage(info.fLocation.fPageIndex));
        mcr->insertInt("MCID", info.fMarkId);
        kids->appendObject(std::move(mcr));
    }
    for (const SkPDFTagNode::AnnotationInfo& annotationInfo : node->fAnnotations) {
        std::unique_ptr<SkPDFDict> annotationDict = SkPDFMakeDict("OBJR");
        annotationDict->insertRef("Obj", annotationInfo.fAnnotationRef);
        annotationDict->insertRef("Pg", doc->getPage(annotationInfo.fPageIndex));
        kids->appendObject(std::move(annotationDict));
    }
    node->fRef = ref;
    SkPDFDict dict("StructElem");
    dict.insertName("S", node->fTypeString.isEmpty() ? "NonStruct" : node->fTypeString.c_str());
    if (!node->fAlt.isEmpty()) {
        dict.insertTextString("Alt", node->fAlt);
    }
    if (!node->fLang.isEmpty()) {
        dict.insertTextString("Lang", node->fLang);
    }
    dict.insertRef("P", parent);
    dict.insertObject("K", std::move(kids));
    if (node->fAttributes) {
        dict.insertObject("A", std::move(node->fAttributes));
    }

    // Each node has a unique ID that also needs to be referenced
    // in a separate IDTree node, along with the lowest and highest
    // unique ID string.
    SkString idString = SkPDFTagNode::nodeIdToString(node->fNodeId);
    dict.insertByteString("ID", idString.c_str());
    IDTreeEntry idTreeEntry = {node->fNodeId, ref};
    fIdTreeEntries.push_back(idTreeEntry);

    return doc->emit(dict, ref);
}

void SkPDFTagTree::addNodeAnnotation(int nodeId, SkPDFIndirectReference annotationRef, unsigned pageIndex) {
    if (!fRoot) {
        return;
    }
    SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
    if (!tagPtr) {
        return;
    }
    SkPDFTagNode* tag = *tagPtr;
    SkASSERT(tag);

    SkPDFTagNode::AnnotationInfo annotationInfo = {pageIndex, annotationRef};
    tag->fAnnotations.push_back(annotationInfo);
}

void SkPDFTagTree::addNodeTitle(int nodeId, SkSpan<const char> title) {
    if (!fRoot) {
        return;
    }
    SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
    if (!tagPtr) {
        return;
    }
    SkPDFTagNode* tag = *tagPtr;
    SkASSERT(tag);

    if (tag->fWantTitle) {
        tag->fTitle.append(title.data(), title.size());
        // Arbitrary cutoff for size.
        if (tag->fTitle.size() > 1023) {
            tag->fWantTitle = false;
        }
    }
}

SkPDFIndirectReference SkPDFTagTree::makeStructTreeRoot(SkPDFDocument* doc) {
    if (!fRoot || can_discard(fRoot)) {
        return SkPDFIndirectReference();
    }

    SkPDFIndirectReference ref = doc->reserveRef();

    unsigned pageCount = SkToUInt(doc->pageCount());

    // Build the StructTreeRoot.
    SkPDFDict structTreeRoot("StructTreeRoot");
    structTreeRoot.insertRef("K", PrepareTagTreeToEmit(ref, fRoot, doc));
    structTreeRoot.insertInt("ParentTreeNextKey", SkToInt(pageCount));

    // Build the parent tree, which consists of two things:
    // (1) For each page, a mapping from the marked content IDs on
    // each page to their corresponding tags
    // (2) For each annotation, an indirect reference to that
    // annotation's struct tree element.
    SkPDFDict parentTree("ParentTree");
    auto parentTreeNums = SkPDFMakeArray();

    // First, one entry per page.
    SkASSERT(SkToUInt(fMarksPerPage.size()) <= pageCount);
    for (int j = 0; j < fMarksPerPage.size(); ++j) {
        const TArray<SkPDFTagNode*>& pageMarks = fMarksPerPage[j];
        SkPDFArray markToTagArray;
        for (SkPDFTagNode* mark : pageMarks) {
            SkASSERT(mark->fRef);
            markToTagArray.appendRef(mark->fRef);
        }
        parentTreeNums->appendInt(j);
        parentTreeNums->appendRef(doc->emit(markToTagArray));
    }

    // Then, one entry per annotation.
    for (size_t j = 0; j < fParentTreeAnnotationNodeIds.size(); ++j) {
        int nodeId = fParentTreeAnnotationNodeIds[j];
        int structParentKey = kFirstAnnotationStructParentKey + static_cast<int>(j);

        SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
        if (!tagPtr) {
            continue;
        }
        SkPDFTagNode* tag = *tagPtr;
        parentTreeNums->appendInt(structParentKey);
        parentTreeNums->appendRef(tag->fRef);
    }

    parentTree.insertObject("Nums", std::move(parentTreeNums));
    structTreeRoot.insertRef("ParentTree", doc->emit(parentTree));

    // Build the IDTree, a mapping from every unique ID string to
    // a reference to its corresponding structure element node.
    if (!fIdTreeEntries.empty()) {
        std::sort(fIdTreeEntries.begin(), fIdTreeEntries.end(),
                  [](const IDTreeEntry& a, const IDTreeEntry& b) {
                    return a.nodeId < b.nodeId;
                  });

        SkPDFDict idTree;
        SkPDFDict idTreeLeaf;
        auto limits = SkPDFMakeArray();
        SkString lowestNodeIdString = SkPDFTagNode::nodeIdToString(
            fIdTreeEntries.begin()->nodeId);
        limits->appendByteString(lowestNodeIdString);
        SkString highestNodeIdString = SkPDFTagNode::nodeIdToString(
            fIdTreeEntries.rbegin()->nodeId);
        limits->appendByteString(highestNodeIdString);
        idTreeLeaf.insertObject("Limits", std::move(limits));
        auto names = SkPDFMakeArray();
        for (const IDTreeEntry& entry : fIdTreeEntries) {
          SkString idString = SkPDFTagNode::nodeIdToString(entry.nodeId);
            names->appendByteString(idString);
            names->appendRef(entry.ref);
        }
        idTreeLeaf.insertObject("Names", std::move(names));
        auto idTreeKids = SkPDFMakeArray();
        idTreeKids->appendRef(doc->emit(idTreeLeaf));
        idTree.insertObject("Kids", std::move(idTreeKids));
        structTreeRoot.insertRef("IDTree", doc->emit(idTree));
    }

    return doc->emit(structTreeRoot, ref);
}

namespace {
struct OutlineEntry {
    struct Content {
        SkString fText;
        Location fLocation;
        void accumulate(Content const& child) {
            fText += child.fText;
            fLocation.accumulate(child.fLocation);
        }
    };

    Content fContent;
    int fHeaderLevel;
    SkPDFIndirectReference fRef;
    SkPDFIndirectReference fStructureRef;
    std::vector<OutlineEntry> fChildren = {};
    size_t fDescendentsEmitted = 0;

    void emitDescendents(SkPDFDocument* const doc) {
        fDescendentsEmitted = fChildren.size();
        for (size_t i = 0; i < fChildren.size(); ++i) {
            auto&& child = fChildren[i];
            child.emitDescendents(doc);
            fDescendentsEmitted += child.fDescendentsEmitted;

            SkPDFDict entry;
            entry.insertTextString("Title", child.fContent.fText);

            auto destination = SkPDFMakeArray();
            destination->appendRef(doc->getPage(child.fContent.fLocation.fPageIndex));
            destination->appendName("XYZ");
            destination->appendScalar(child.fContent.fLocation.fPoint.x());
            destination->appendScalar(child.fContent.fLocation.fPoint.y());
            destination->appendInt(0);
            entry.insertObject("Dest", std::move(destination));

            entry.insertRef("Parent", child.fRef);
            if (child.fStructureRef) {
                entry.insertRef("SE", child.fStructureRef);
            }
            if (0 < i) {
                entry.insertRef("Prev", fChildren[i-1].fRef);
            }
            if (i < fChildren.size()-1) {
                entry.insertRef("Next", fChildren[i+1].fRef);
            }
            if (!child.fChildren.empty()) {
                entry.insertRef("First", child.fChildren.front().fRef);
                entry.insertRef("Last", child.fChildren.back().fRef);
                entry.insertInt("Count", child.fDescendentsEmitted);
            }
            doc->emit(entry, child.fRef);
        }
    }
};

OutlineEntry::Content create_outline_entry_content(SkPDFTagNode* const node) {
    SkString text;
    if (!node->fTitle.isEmpty()) {
        text = node->fTitle;
    } else if (!node->fAlt.isEmpty()) {
        text = node->fAlt;
    }

    // The uppermost/leftmost point on the earliest page of this node's marks.
    Location markPoint;
    for (auto&& mark : node->fMarkedContent) {
        markPoint.accumulate(mark.fLocation);
    }

    OutlineEntry::Content content{std::move(text), std::move(markPoint)};

    // Accumulate children
    SkSpan<SkPDFTagNode> children(node->fChildren, node->fChildCount);
    for (auto&& child : children) {
        if (can_discard(&child)) {
            continue;
        }
        content.accumulate(create_outline_entry_content(&child));
    }
    return content;
}
void create_outline_from_headers(SkPDFDocument* const doc, SkPDFTagNode* const node,
                                 STArray<7, OutlineEntry*>& stack) {
    char const *type = node->fTypeString.c_str();
    if (type[0] == 'H' && '1' <= type[1] && type[1] <= '6') {
        int level = type[1] - '0';
        while (level <= stack.back()->fHeaderLevel) {
            stack.pop_back();
        }
        OutlineEntry::Content content = create_outline_entry_content(node);
        if (!content.fText.isEmpty()) {
            OutlineEntry e{std::move(content), level, doc->reserveRef(), node->fRef};
            stack.push_back(&stack.back()->fChildren.emplace_back(std::move(e)));
            return;
        }
    }

    SkSpan<SkPDFTagNode> children(node->fChildren, node->fChildCount);
    for (auto&& child : children) {
        if (can_discard(&child)) {
            continue;
        }
        create_outline_from_headers(doc, &child, stack);
    }
}

} // namespace

SkPDFIndirectReference SkPDFTagTree::makeOutline(SkPDFDocument* doc) {
    if (!fRoot || can_discard(fRoot) ||
        fOutline != SkPDF::Metadata::Outline::StructureElementHeaders)
    {
        return SkPDFIndirectReference();
    }

    STArray<7, OutlineEntry*> stack;
    OutlineEntry top{{SkString(), Location()}, 0, {}, {}};
    stack.push_back(&top);
    create_outline_from_headers(doc, fRoot, stack);
    if (top.fChildren.empty()) {
        return SkPDFIndirectReference();
    }
    top.emitDescendents(doc);
    SkPDFIndirectReference outlineRef = doc->reserveRef();
    SkPDFDict outline("Outlines");
    outline.insertRef("First", top.fChildren.front().fRef);
    outline.insertRef("Last", top.fChildren.back().fRef);
    outline.insertInt("Count", top.fDescendentsEmitted);

    return doc->emit(outline, outlineRef);
}

SkString SkPDFTagTree::getRootLanguage() {
    return fRoot ? fRoot->fLang : SkString();
}
