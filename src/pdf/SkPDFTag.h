/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTag_DEFINED
#define SkPDFTag_DEFINED

#include "SkRefCnt.h"
#include "SkPDFTypes.h"

// Table 333 in PDF 32000-1:2008
enum SkPDFStructureType {
  kDocument_StructureType,
  kPart_StructureType,
  kArt_StructureType,         // Article
  kSect_StructureType,        // Section
  kDiv_StructureType,
  kBlockQuote_StructureType,
  kCaption_StructureType,
  kTOC_StructureType,         // Table of Contents
  kTOCI_StructureType,        // Table of Contents Item
  kIndex_StructureType,
  kNonStruct_StructureType,
  kPrivate_StructureType,
  kH_StructureType,           // Heading
  kH1_StructureType,          // Heading level 1
  kH2_StructureType,
  kH3_StructureType,
  kH4_StructureType,
  kH5_StructureType,
  kH6_StructureType,          // Heading level 6
  kP_StructureType,           // Paragraph
  kL_StructureType,           // List
  kLI_StructureType,          // List item
  kLbl_StructureType,         // List item label
  kLBody_StructureType,       // List item body
  kTable_StructureType,
  kTR_StructureType,
  kTH_StructureType,
  kTD_StructureType,
  kTHead_StructureType,
  kTBody_StructureType,
  kTFoot_StructureType,
  kSpan_StructureType,
  kQuote_StructureType,
  kNote_StructureType,
  kReference_StructureType,
  kBibEntry_StructureType,
  kCode_StructureType,
  kLink_StructureType,
  kAnnot_StructureType,
  kRuby_StructureType,
  kWarichu_StructureType,
  kFigure_StructureType,
  kFormula_StructureType,
  kForm_StructureType,        // Form control (not like an HTML FORM element)
};

class SkPDFDocument;

/** \class SkPDFTag

    A PDF Tag represents a semantic tag in the tag tree for an
    accessible tagged PDF. Documents can create an accessible
    PDF by creating a tree of SkPDFTags representing the semantic
    tree structure of the overall document, and then calling
    SkCanvas::setNodeId with the same corresponding node IDs
    to mark the content for each page. It's allowed for the
    marked content for one tag to span multiple pages.
*/
class SkPDFTag final : public SkPDFDict {
public:
    SkPDFTag(int64_t nodeId, SkPDFStructureType type);
    ~SkPDFTag() override;

    void appendChild(sk_sp<SkPDFTag> child);

private:
    friend class SkPDFDocument;

    void setParent(sk_sp<SkPDFTag> parent);
    void addMarkedContent(int pageIndex, int markId);

    // Should be called after all content has been emitted. Fills in
    // all of the SkPDFDict fields in this tag and all descendants.
    // Returns true if this tag is valid, and false if no tag in this
    // subtree was referred to by any marked content.
    bool prepareTagTreeToEmit(sk_sp<SkPDFDocument> document);

    struct MarkedContentInfo {
      int pageIndex;
      int markId;
    };

    // This tag's node ID, which must correspond to the node ID set
    // on the SkCanvas when content inside this tag is drawn.
    // The node IDs are arbitrary and are not output to the PDF.
    int64_t fNodeId;

    // The structure type for this node.
    SkPDFStructureType fType;

    // The children of this tag. Some tags like lists and tables require
    // a particular hierarchical structure, similar to HTML.
    SkTArray<sk_sp<SkPDFTag>> fChildren;

    // An array consisting of a [page index, mark ID] pair for each piece
    // of marked content associated with this tag.
    SkTArray<MarkedContentInfo> fMarkedContent;

    // The parent node of this tag. SkPDFDocument sets the parent of
    // the root tag to the StructTreeRoot.
    sk_sp<SkPDFTag> fParent;
};

#endif
