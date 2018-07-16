/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDocument.h"
#include "SkPDFTag.h"

namespace {

// Table 333 in PDF 32000-1:2008
const char* tagNameFromType(SkDocumentStructureType type) {
    switch (type) {
        case SkDocumentStructureType::kDocument:
            return "Document";
        case SkDocumentStructureType::kPart:
            return "Part";
        case SkDocumentStructureType::kArt:
            return "Art";
        case SkDocumentStructureType::kSect:
            return "Sect";
        case SkDocumentStructureType::kDiv:
            return "Div";
        case SkDocumentStructureType::kBlockQuote:
            return "BlockQuote";
        case SkDocumentStructureType::kCaption:
            return "Caption";
        case SkDocumentStructureType::kTOC:
            return "TOC";
        case SkDocumentStructureType::kTOCI:
            return "TOCI";
        case SkDocumentStructureType::kIndex:
            return "Index";
        case SkDocumentStructureType::kNonStruct:
            return "NonStruct";
        case SkDocumentStructureType::kPrivate:
            return "Private";
        case SkDocumentStructureType::kH:
            return "H";
        case SkDocumentStructureType::kH1:
            return "H1";
        case SkDocumentStructureType::kH2:
            return "H2";
        case SkDocumentStructureType::kH3:
            return "H3";
        case SkDocumentStructureType::kH4:
            return "H4";
        case SkDocumentStructureType::kH5:
            return "H5";
        case SkDocumentStructureType::kH6:
            return "H6";
        case SkDocumentStructureType::kP:
            return "P";
        case SkDocumentStructureType::kL:
            return "L";
        case SkDocumentStructureType::kLI:
            return "LI";
        case SkDocumentStructureType::kLbl:
            return "Lbl";
        case SkDocumentStructureType::kLBody:
            return "LBody";
        case SkDocumentStructureType::kTable:
            return "Table";
        case SkDocumentStructureType::kTR:
            return "TR";
        case SkDocumentStructureType::kTH:
            return "TH";
        case SkDocumentStructureType::kTD:
            return "TD";
        case SkDocumentStructureType::kTHead:
            return "THead";
        case SkDocumentStructureType::kTBody:
            return "TBody";
        case SkDocumentStructureType::kTFoot:
            return "TFoot";
        case SkDocumentStructureType::kSpan:
            return "Span";
        case SkDocumentStructureType::kQuote:
            return "Quote";
        case SkDocumentStructureType::kNote:
            return "Note";
        case SkDocumentStructureType::kReference:
            return "Reference";
        case SkDocumentStructureType::kBibEntry:
            return "BibEntry";
        case SkDocumentStructureType::kCode:
            return "Code";
        case SkDocumentStructureType::kLink:
            return "Link";
        case SkDocumentStructureType::kAnnot:
            return "Annot";
        case SkDocumentStructureType::kRuby:
            return "Ruby";
        case SkDocumentStructureType::kWarichu:
            return "Warichu";
        case SkDocumentStructureType::kFigure:
            return "Figure";
        case SkDocumentStructureType::kFormula:
            return "Formula";
        case SkDocumentStructureType::kForm:
            return "Form";
    }
}

}  // namespace

SkPDFTag::SkPDFTag(int64_t nodeId, SkDocumentStructureType type)
    : SkPDFDict("StructElem")
    , fNodeId(nodeId)
    , fType(type) {
}

SkPDFTag::~SkPDFTag() {
}

void SkPDFTag::appendChild(sk_sp<SkPDFTag> child) {
    fChildren.emplace_back(child);
    child->setParent(sk_ref_sp(this));
}

void SkPDFTag::drop() {
    // Unref the parent so that it doesn't cause a refcount loop.
    SkSafeUnref(fParent.get());

    SkPDFDict::drop();
}

void SkPDFTag::setParent(sk_sp<SkPDFTag> parent) {
    fParent = parent;
}

void SkPDFTag::addMarkedContent(int pageIndex, int markId) {
    MarkedContentInfo mark;
    mark.pageIndex = pageIndex;
    mark.markId = markId;
    fMarkedContent.emplace_back(mark);
}

bool SkPDFTag::prepareTagTreeToEmit(sk_sp<SkPDFDocument> document) {
    // Set the type.
    insertName("S", tagNameFromType(fType));

    // Set the parent.
    if (fParent)
        insertObjRef("P", fParent);

    // Scan the marked content. If it's all on the page, output a
    // Pg to the dict. If not, we'll use MCR dicts, below.
    bool allSamePage = true;
    if (fMarkedContent.count() > 0) {
        int firstPageIndex = fMarkedContent[0].pageIndex;
        for (int i = 1; i < fMarkedContent.count(); i++) {
            if (fMarkedContent[i].pageIndex != firstPageIndex) {
                allSamePage = false;
                break;
            }
        }

        if (allSamePage)
            insertObjRef("Pg", document->getPage(firstPageIndex));
    }

    // Recursively prepare all child tags of this node.
    SkTArray<sk_sp<SkPDFTag>> validChildren;
    for (int i = 0; i < fChildren.count(); i++) {
        if (fChildren[i]->prepareTagTreeToEmit(document))
            validChildren.push_back(fChildren[i]);
    }

    // Now set the kids of this node, which includes both child tags
    // and marked content IDs.
    if (validChildren.count() + fMarkedContent.count() == 1) {
        // If there's just one valid kid, or one marked content,
        // we can just output the reference directly with no array.
        if (validChildren.count() == 1)
            insertObjRef("K", validChildren[0]);
        else
            insertInt("K", fMarkedContent[0].markId);
        return true;
    } else if (validChildren.count() + fMarkedContent.count() > 1) {
        // If there's more than one kid, output them in an array.
        auto kids = sk_make_sp<SkPDFArray>();
        for (int i = 0; i < validChildren.count(); i++)
            kids->appendObjRef(validChildren[i]);
        for (int i = 0; i < fMarkedContent.count(); i++) {
            if (allSamePage) {
                kids->appendInt(fMarkedContent[i].markId);
            } else {
                auto mcr = sk_make_sp<SkPDFDict>("MCR");
                mcr->insertObjRef("Pg", document->getPage(fMarkedContent[i].pageIndex));
                mcr->insertInt("MCID", fMarkedContent[i].markId);
                kids->appendObject(mcr);
            }
        }
        insertObject("K", kids);
        return true;
    }

    // This tag didn't have any marked content or any children with
    // marked content, so return false. This subtree will be omitted
    // from the structure tree.
    return false;
}
