/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFTag.h"

#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkZip.h"
#include "src/pdf/SkPDFDocumentPriv.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

using namespace skia_private;

// The /StructTreeRoot /ParentTree is a number tree which consists of one entry for each page
// (Page::StructParents -> StructElemRef[mcid]) and entries for individual content items
// (?::StructParent -> StructElemRef).
//
// This is implemented with page entries getting consecutive keys starting at 0. Since the total
// number of pages in the document is not known when content items are processed, start the key for
// content items with a large number, which effectively becomes the maximum number of pages in a
// PDF we can handle.
const int kFirstContentItemStructParentKey = 100000;

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

struct SkPDFStructElem {
    // Structure elements (/StructElem) may have an element identifier (/ID) which is a byte string.
    // Element identifiers are used by attributes (/StructElem /A) to refer to structure elements.
    // The mapping from element identifier to structure element is emitted in the /IDTree.
    // Element identifiers are stored as an integer (elemId) and this method creates a byte string.
    // Since the /IDTree is a name tree the element identifier keys must be ordered;
    // the digits are zero-padded so that lexicographic order matches numeric order.
    static SkString StringFromElemId(int elemId) {
        SkString elemIdString;
        elemIdString.printf("node%08d", elemId);
        return elemIdString;
    }

    SkPDFStructElem* fParent = nullptr;
    SkSpan<SkPDFStructElem> fChildren;
    struct MarkedContentInfo {
        Location fLocation;
        int fMcid;
    };
    TArray<MarkedContentInfo> fMarkedContent;
    int fElemId = 0;
    bool fWantTitle = false;
    bool fUsed = false;
    bool fUsedInIDTree = false;
    SkString fStructType;
    SkString fTitle;
    SkString fAlt;
    SkString fLang;
    SkPDFIndirectReference fRef;
    std::unique_ptr<SkPDFArray> fAttributes;
    std::vector<int> fAttributeElemIds;
    struct ContentItemInfo {
        unsigned fPageIndex;
        SkPDFIndirectReference fContentItemRef;
    };
    std::vector<ContentItemInfo> fContentItems;

    void setUsed(const THashMap<int, SkPDFStructElem*>& structElemForElemId) {
        if (fUsed) {
            return;
        }
        // First to avoid possible cycles.
        fUsed = true;
        // Any StructElem referenced by an attribute is used.
        for (int elemId : fAttributeElemIds) {
            SkPDFStructElem** structElemPtr = structElemForElemId.find(elemId);
            if (!structElemPtr) {
                continue;
            }
            SkPDFStructElem* structElem = *structElemPtr;
            SkASSERT(structElem);
            structElem->setUsed(structElemForElemId);
            structElem->fUsedInIDTree = true;
        }
        // The parent StructElem is used.
        if (fParent) {
            fParent->setUsed(structElemForElemId);
        }
    }

    SkPDFIndirectReference emitStructElem(SkPDFIndirectReference parent,
                                          std::vector<SkPDFStructTree::IDTreeEntry>* idTree,
                                          SkPDFDocument* doc);
};

SkPDF::AttributeList::AttributeList() = default;

SkPDF::AttributeList::~AttributeList() = default;

void SkPDF::AttributeList::appendInt(const char* owner, const char* name, int value) {
    if (!fAttrs) {
        fAttrs = SkPDFMakeArray();
    }
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertInt(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendFloat(const char* owner, const char* name, float value) {
    if (!fAttrs) {
        fAttrs = SkPDFMakeArray();
    }
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertScalar(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendName(const char* owner, const char* name, const char* value) {
    if (!fAttrs) {
        fAttrs = SkPDFMakeArray();
    }
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertName(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendFloatArray(const char* owner, const char* name,
                                            const std::vector<float>& value) {
    if (!fAttrs) {
        fAttrs = SkPDFMakeArray();
    }
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (float element : value) {
        pdfArray->appendScalar(element);
    }
    attrDict->insertObject(name, std::move(pdfArray));
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendNodeIdArray(const char* owner, const char* name,
                                             const std::vector<int>& elemIds) {
    if (!fAttrs) {
        fAttrs = SkPDFMakeArray();
    }
    // Keep the element identifiers so we can mark their targets as used (and needing /ID) later.
    fElemIds.insert(fElemIds.end(), elemIds.begin(), elemIds.end());
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (int elemId : elemIds) {
        pdfArray->appendByteString(SkPDFStructElem::StringFromElemId(elemId));
    }
    attrDict->insertObject(name, std::move(pdfArray));
    fAttrs->appendObject(std::move(attrDict));
}

SkPDFStructTree::SkPDFStructTree(SkPDF::StructureElementNode* node,
                                 SkPDF::Metadata::Outline outline)
    : fArena(4 * sizeof(SkPDFStructElem))
{
    if (node) {
        fRoot = fArena.make<SkPDFStructElem>();
        fOutline = outline;
        this->move(*node, fRoot, false);
    }
}

SkPDFStructTree::~SkPDFStructTree() = default;

void SkPDFStructTree::move(SkPDF::StructureElementNode& node,
                           SkPDFStructElem* structElem,
                           bool wantTitle) {
    structElem->fElemId = node.fNodeId;
    fStructElemForElemId.set(structElem->fElemId, structElem);

    // Accumulate title text, need to be in sync with create_outline_from_headers
    const SkString& type = node.fTypeString;
    wantTitle |= fOutline == SkPDF::Metadata::Outline::StructureElementHeaders &&
                 type.size() == 2 && type[0] == 'H' && '1' <= type[1] && type[1] <= '6';
    structElem->fWantTitle = wantTitle;

    static SkString nonStruct("NonStruct");
    structElem->fStructType = node.fTypeString.isEmpty() ? nonStruct : std::move(node.fTypeString);
    structElem->fAlt = std::move(node.fAlt);
    structElem->fLang = std::move(node.fLang);

    size_t childCount = node.fChildVector.size();
    structElem->fChildren = SkSpan(fArena.makeArray<SkPDFStructElem>(childCount), childCount);
    for (auto&& [nodeChild, elemChild] : SkMakeZip(node.fChildVector, structElem->fChildren)) {
        elemChild.fParent = structElem;
        this->move(*nodeChild, &elemChild, wantTitle);
    }

    structElem->fAttributes = std::move(node.fAttributes.fAttrs);
    structElem->fAttributeElemIds = std::move(node.fAttributes.fElemIds);
}

int SkPDFStructTree::Mark::elemId() const {
    return fStructElem ? fStructElem->fElemId : 0;
}

SkString SkPDFStructTree::Mark::structType() const {
    SkASSERT(bool(*this));
    return fStructElem->fStructType;
}

int SkPDFStructTree::Mark::mcid() const {
    return fStructElem ? fStructElem->fMarkedContent[fMarkIndex].fMcid : -1;
}

void SkPDFStructTree::Mark::accumulate(SkPoint point) {
    SkASSERT(bool(*this));
    Location& location = fStructElem->fMarkedContent[fMarkIndex].fLocation;
    return location.accumulate({{point}, location.fPageIndex});
}

auto SkPDFStructTree::createMarkForElemId(int elemId, unsigned pageIndex) -> Mark {
    if (!fRoot) {
        return Mark();
    }
    SkPDFStructElem** structElemPtr = fStructElemForElemId.find(elemId);
    if (!structElemPtr) {
        return Mark();
    }
    SkPDFStructElem* structElem = *structElemPtr;
    SkASSERT(structElem);

    structElem->setUsed(fStructElemForElemId);

    while (SkToUInt(fStructElemForMcidForPage.size()) < pageIndex + 1) {
        fStructElemForMcidForPage.push_back();
    }
    TArray<SkPDFStructElem*>& structElemForMcid = fStructElemForMcidForPage[pageIndex];
    int mcid = structElemForMcid.size();
    SkASSERT(structElem->fMarkedContent.empty() ||
             structElem->fMarkedContent.back().fLocation.fPageIndex <= pageIndex);
    structElem->fMarkedContent.push_back({{{SK_ScalarNaN, SK_ScalarNaN}, pageIndex}, mcid});
    structElemForMcid.push_back(structElem);
    return Mark(structElem, structElem->fMarkedContent.size() - 1);
}

int SkPDFStructTree::createStructParentKeyForElemId(int elemId,
                                                    SkPDFIndirectReference contentItemRef,
                                                    unsigned pageIndex) {
    if (!fRoot) {
        return -1;
    }
    SkPDFStructElem** structElemPtr = fStructElemForElemId.find(elemId);
    if (!structElemPtr) {
        return -1;
    }
    SkPDFStructElem* structElem = *structElemPtr;
    SkASSERT(structElem);

    structElem->setUsed(fStructElemForElemId);

    SkPDFStructElem::ContentItemInfo contentItemInfo = {pageIndex, contentItemRef};
    structElem->fContentItems.push_back(contentItemInfo);

    int structParentKey = kFirstContentItemStructParentKey + fStructElemForContentItem.size();
    fStructElemForContentItem.push_back(structElem);
    return structParentKey;
}

SkPDFIndirectReference SkPDFStructElem::emitStructElem(
        SkPDFIndirectReference parent,
        std::vector<SkPDFStructTree::IDTreeEntry>* idTree,
        SkPDFDocument* doc)
{
    fRef = doc->reserveRef();

    SkPDFDict dict("StructElem");
    dict.insertName("S", fStructType);

    if (!fAlt.isEmpty()) {
        dict.insertTextString("Alt", fAlt);
    }
    if (!fLang.isEmpty()) {
        dict.insertTextString("Lang", fLang);
    }
    dict.insertRef("P", parent);

    { // K
        std::unique_ptr<SkPDFArray> kids = SkPDFMakeArray();
        for (auto&& child : fChildren) {
            if (child.fUsed) {
                kids->appendRef(child.emitStructElem(fRef, idTree, doc));
            }
        }
        if (!fMarkedContent.empty()) {
            // Use the mode page as /Pg and use integer mcid for marks on that page.
            // SkPDFStructElem::fMarkedContent is already sorted by page, since it is append only in
            // createMarkForElemId where pageIndex is the monotonically increasing current page.
            size_t longestRun = 0;
            unsigned longestPage = 0;
            size_t currentRun = 0;
            unsigned currentPage = 0;
            for (const SkPDFStructElem::MarkedContentInfo& info : fMarkedContent) {
                unsigned thisPage = info.fLocation.fPageIndex;
                if (currentPage != thisPage) {
                    SkASSERT(currentPage < thisPage);
                    currentPage = thisPage;
                    currentRun = 0;
                }
                ++currentRun;
                if (longestRun < currentRun) {
                    longestRun = currentRun;
                    longestPage = currentPage;
                }
            }
            for (const SkPDFStructElem::MarkedContentInfo& info : fMarkedContent) {
                if (info.fLocation.fPageIndex == longestPage) {
                    kids->appendInt(info.fMcid);
                } else {
                    std::unique_ptr<SkPDFDict> mcr = SkPDFMakeDict("MCR");
                    mcr->insertRef("Pg", doc->getPage(info.fLocation.fPageIndex));
                    mcr->insertInt("MCID", info.fMcid);
                    kids->appendObject(std::move(mcr));
                }
            }
            dict.insertRef("Pg", doc->getPage(longestPage));
        }
        for (const SkPDFStructElem::ContentItemInfo& contentItemInfo : fContentItems) {
            std::unique_ptr<SkPDFDict> contentItemDict = SkPDFMakeDict("OBJR");
            contentItemDict->insertRef("Obj", contentItemInfo.fContentItemRef);
            contentItemDict->insertRef("Pg", doc->getPage(contentItemInfo.fPageIndex));
            kids->appendObject(std::move(contentItemDict));
        }
        dict.insertObject("K", std::move(kids));
    }

    if (fAttributes) {
        dict.insertObject("A", std::move(fAttributes));
    }

    // If this StructElem ID was referenced, add /ID and add it to the IDTree.
    if (fUsedInIDTree) {
        dict.insertByteString("ID", SkPDFStructElem::StringFromElemId(fElemId));
        idTree->push_back({fElemId, fRef});
    }

    return doc->emit(dict, fRef);
}

void SkPDFStructTree::addStructElemTitle(int elemId, SkSpan<const char> title) {
    if (!fRoot) {
        return;
    }
    SkPDFStructElem** structElemPtr = fStructElemForElemId.find(elemId);
    if (!structElemPtr) {
        return;
    }
    SkPDFStructElem* structElem = *structElemPtr;
    SkASSERT(structElem);

    if (structElem->fWantTitle) {
        structElem->fTitle.append(title.data(), title.size());
        // Arbitrary cutoff for size.
        if (structElem->fTitle.size() > 1023) {
            structElem->fWantTitle = false;
        }
    }
}

SkPDFIndirectReference SkPDFStructTree::emitStructTreeRoot(SkPDFDocument* doc) const {
    if (!fRoot || !fRoot->fUsed) {
        return SkPDFIndirectReference();
    }

    SkPDFIndirectReference structTreeRootRef = doc->reserveRef();

    unsigned pageCount = SkToUInt(doc->pageCount());

    // Build the StructTreeRoot.
    SkPDFDict structTreeRoot("StructTreeRoot");
    std::vector<IDTreeEntry> idTree;
    structTreeRoot.insertRef("K", fRoot->emitStructElem(structTreeRootRef, &idTree, doc));
    structTreeRoot.insertInt("ParentTreeNextKey", SkToInt(pageCount));

    // Build the parent tree, a number tree which consists of two things:
    // For each page:
    //   key: Page::StructParents
    //   value: array of structure element ref indexed by the page's marked-content identifiers
    // For each content item (usually an annotation)
    //   key: ?::StructParent
    //   value: structure element ref
    SkPDFDict parentTree("ParentTree");
    auto parentTreeNums = SkPDFMakeArray();

    // First, one entry per page.
    SkASSERT(SkToUInt(fStructElemForMcidForPage.size()) <= pageCount);
    for (int j = 0; j < fStructElemForMcidForPage.size(); ++j) {
        const TArray<SkPDFStructElem*>& structElemForMcid = fStructElemForMcidForPage[j];
        SkPDFArray structElemForMcidArray;
        for (SkPDFStructElem* structElem : structElemForMcid) {
            SkASSERT(structElem->fRef);
            structElemForMcidArray.appendRef(structElem->fRef);
        }
        parentTreeNums->appendInt(j); // /Page /StructParents
        parentTreeNums->appendRef(doc->emit(structElemForMcidArray));
    }

    // Then, one entry per content item.
    for (int j = 0; j < fStructElemForContentItem.size(); ++j) {
        int structParentKey = kFirstContentItemStructParentKey + j;
        SkPDFStructElem* structElem = fStructElemForContentItem[j];
        parentTreeNums->appendInt(structParentKey); // /<content-item> /StructParent
        parentTreeNums->appendRef(structElem->fRef);
    }

    parentTree.insertObject("Nums", std::move(parentTreeNums));
    structTreeRoot.insertRef("ParentTree", doc->emit(parentTree));

    // Build the IDTree, a mapping from every unique element identifier byte string to
    // a reference to its corresponding structure element.
    if (!idTree.empty()) {
        std::sort(idTree.begin(), idTree.end(),
                  [](const IDTreeEntry& a, const IDTreeEntry& b) {
                    return a.elemId < b.elemId;
                  });

        SkPDFDict idTreeLeaf;
        auto limits = SkPDFMakeArray();
        SkString lowestElemIdString = SkPDFStructElem::StringFromElemId(idTree.begin()->elemId);
        limits->appendByteString(lowestElemIdString);
        SkString highestElemIdString = SkPDFStructElem::StringFromElemId(idTree.rbegin()->elemId);
        limits->appendByteString(highestElemIdString);
        idTreeLeaf.insertObject("Limits", std::move(limits));
        auto names = SkPDFMakeArray();
        for (const IDTreeEntry& entry : idTree) {
            names->appendByteString(SkPDFStructElem::StringFromElemId(entry.elemId));
            names->appendRef(entry.structElemRef);
        }
        idTreeLeaf.insertObject("Names", std::move(names));
        auto idTreeKids = SkPDFMakeArray();
        idTreeKids->appendRef(doc->emit(idTreeLeaf));

        SkPDFDict idTreeRoot;
        idTreeRoot.insertObject("Kids", std::move(idTreeKids));
        structTreeRoot.insertRef("IDTree", doc->emit(idTreeRoot));
    }

    return doc->emit(structTreeRoot, structTreeRootRef);
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

OutlineEntry::Content create_outline_entry_content(SkPDFStructElem* const structElem) {
    SkString text;
    if (!structElem->fTitle.isEmpty()) {
        text = structElem->fTitle;
    } else if (!structElem->fAlt.isEmpty()) {
        text = structElem->fAlt;
    }

    // The uppermost/leftmost point on the earliest page of this StructElem's marks.
    Location structElemLocation;
    for (auto&& mark : structElem->fMarkedContent) {
        structElemLocation.accumulate(mark.fLocation);
    }

    OutlineEntry::Content content{std::move(text), std::move(structElemLocation)};

    // Accumulate children
    for (auto&& child : structElem->fChildren) {
        if (child.fUsed) {
            content.accumulate(create_outline_entry_content(&child));
        }
    }
    return content;
}
void create_outline_from_headers(SkPDFDocument* const doc, SkPDFStructElem* const structElem,
                                 STArray<7, OutlineEntry*>& stack) {
    const SkString& type = structElem->fStructType;
    if (type.size() == 2 && type[0] == 'H' && '1' <= type[1] && type[1] <= '6') {
        int level = type[1] - '0';
        while (level <= stack.back()->fHeaderLevel) {
            stack.pop_back();
        }
        OutlineEntry::Content content = create_outline_entry_content(structElem);
        if (!content.fText.isEmpty()) {
            OutlineEntry e{std::move(content), level, doc->reserveRef(), structElem->fRef};
            stack.push_back(&stack.back()->fChildren.emplace_back(std::move(e)));
            return;
        }
    }

    for (auto&& child : structElem->fChildren) {
        if (child.fUsed) {
            create_outline_from_headers(doc, &child, stack);
        }
    }
}

} // namespace

SkPDFIndirectReference SkPDFStructTree::makeOutline(SkPDFDocument* doc) const {
    if (!fRoot || !fRoot->fUsed ||
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

SkString SkPDFStructTree::getRootLanguage() {
    return fRoot ? fRoot->fLang : SkString();
}
