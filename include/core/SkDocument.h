/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDocument_DEFINED
#define SkDocument_DEFINED

#include "SkRect.h"
#include "SkRefCnt.h"

class SkCanvas;
class SkWStream;

/**
 *  High-level API for creating a document-based canvas. To use..
 *
 *  1. Create a document, specifying a stream to store the output.
 *  2. For each "page" of content:
 *      a. canvas = doc->beginPage(...)
 *      b. draw_my_content(canvas);
 *      c. doc->endPage();
 *  3. Close the document with doc->close().
 */
class SkDocument : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkDocument)

    /**
     *  Create a PDF-backed document, writing the results into a file.
     *  If there is an error trying to create the doc, returns NULL.
     */
    static SkDocument* CreatePDF(const char filename[]);

    /**
     *  Create a PDF-backed document, writing the results into a stream.
     *  If there is an error trying to create the doc, returns NULL.
     *
     *  The document may write to the stream at anytime during its lifetime,
     *  until either close() is called or the document is deleted. Once close()
     *  has been called, and all of the data has been written to the stream,
     *  if there is a Done proc provided, it will be called with the stream.
     *  The proc can delete the stream, or whatever it needs to do.
     */
    static SkDocument* CreatePDF(SkWStream*, void (*Done)(SkWStream*) = NULL);

    /**
     *  Begin a new page for the document, returning the canvas that will draw
     *  into the page. The document owns this canvas, and it will go out of
     *  scope when endPage() or close() is called, or the document is deleted.
     */
    SkCanvas* beginPage(SkScalar width, SkScalar height,
                        const SkRect* content = NULL);

    /**
     *  Call endPage() when the content for the current page has been drawn
     *  (into the canvas returned by beginPage()). After this call the canvas
     *  returned by beginPage() will be out-of-scope.
     */
    void endPage();

    /**
     *  Call close() when all pages have been drawn. This will close the file
     *  or stream holding the document's contents. After close() the document
     *  can no longer add new pages. Deleting the document will automatically
     *  call close() if need be.
     */
    void close();

protected:
    SkDocument(SkWStream*, void (*)(SkWStream*));
    // note: subclasses must call close() in their destructor, as the base class
    // cannot do this for them.
    virtual ~SkDocument();

    virtual SkCanvas* onBeginPage(SkScalar width, SkScalar height,
                                  const SkRect& content) = 0;
    virtual void onEndPage() = 0;
    virtual void onClose(SkWStream*) = 0;

    enum State {
        kBetweenPages_State,
        kInPage_State,
        kClosed_State
    };
    State getState() const { return fState; }

private:
    SkWStream* fStream;
    void       (*fDoneProc)(SkWStream*);
    State      fState;

    typedef SkRefCnt INHERITED;
};

#endif
