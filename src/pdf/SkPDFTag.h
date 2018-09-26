/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTag_DEFINED
#define SkPDFTag_DEFINED

#include "SkDocument.h"
#include "SkPDFTypes.h"
#include "SkRefCnt.h"

class SkPDFDocument;

/** \class SkPDFTag

    A PDF Tag represents a semantic tag in the tag tree for an
    accessible tagged PDF. Documents can create an accessible PDF by
    creating a tree of SkPDFTags representing the semantic tree
    structure of the overall document, and then calling
    SkPDF::SetNodeId with the SkCanvas used to draw to the page and
    the same corresponding node IDs to mark the content for each
    page. It's allowed for the marked content for one tag to span
    multiple pages.
*/
class SkPDFTag final : public SkPDFDict {
public:
    SkPDFTag(int nodeId, SkPDF::DocumentStructureType type, sk_sp<SkPDFTag> parent);
    ~SkPDFTag() override;

    void appendChild(sk_sp<SkPDFTag> child);

private:
    friend class SkPDFDocument;

    void drop() override;

    void addMarkedContent(int pageIndex, int markId);

    // Should be called after all content has been emitted. Fills in
    // all of the SkPDFDict fields in this tag and all descendants.
    // Returns true if this tag is valid, and false if no tag in this
    // subtree was referred to by any marked content.
    bool prepareTagTreeToEmit(const SkPDFDocument& document);

    struct MarkedContentInfo {
        int pageIndex;
        int markId;
    };

    // This tag's node ID, which must correspond to the node ID set
    // on the SkCanvas when content inside this tag is drawn.
    // The node IDs are arbitrary and are not output to the PDF.
    int fNodeId;

    // The children of this tag. Some tags like lists and tables require
    // a particular hierarchical structure, similar to HTML.
    SkTArray<sk_sp<SkPDFTag>> fChildren;

    // An array consisting of a [page index, mark ID] pair for each piece
    // of marked content associated with this tag.
    SkTArray<MarkedContentInfo> fMarkedContent;
};

#endif
