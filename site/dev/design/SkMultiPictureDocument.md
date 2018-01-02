SkMultiPictureDocument
======================

The executive summary is an analogy:

    Canvas : Picture :: Document : MultiPictureDocument

Motivation
----------

A Document contains some number of Pages, where each Page has a Size and some
number of Canvas commands.  The naive way of serializing a Document might be to
write out a SKP (serialized picture) file for each page, along with additional
data containing the page size.  This would have two downsides:

 1. We end up with a collection of files that refer to a single logical Document;
    this can be a pain to deal with.

 2. There will probably be severe duplication of assets (Images and Typefaces)
    between pages.

To solve both of these issues, I designed a new file format.  Version 2 of this
format is canonically documented in
[SkMultiPictureDocument.cpp](https://skia.googlesource.com/skia/+/chrome/m64/src/utils/SkMultiPictureDocument.cpp).

Format
------

The format contains:
    
  * magic number
  * format version number
  * page count
  * page size for each page
  * a single SKP

All of the page content is concatenated into a single Picture, with a special
annotation between each page.  To play the document back into an array of
Pictures, a custom Canvas intercepts the `onDrawAnnotation()` commands and
starts a new PictureRecorder Canvas for the next page when it sees the special
annotation.

Usage
-----

    void record(const char* outputPath,
                int pageCount,
                const SkSize* pageSizes,
                void (*drawPage)(SkCanvas*, int page)) {
        SkFILEWStream out(outputPath);
        assert(out.isValid());
        sk_sp<SkDocument> doc = SkMakeMultiPictureDocument(&out);
        for (int i = 0; i < pageCount; ++i) {
            drawPage(doc->beginPage(pageSizes[i].width(), pageSizes[i].height()), i);
        }
    }

    bool playback(SkDocument* dst, const char* srcPath) {
        SkFILEStream src(srcPath);
        if (src.isValid()) {
            return false;
        }
        int pageCount = SkMultiPictureDocumentReadPageCount(&src);
        src.rewind();
        if (pageCount < 1) {
            return false;
        }
        std::vector<SkDocumentPage> pages(pageCount);
        if (!SkMultiPictureDocumentRead(&src, &pages[0], pageCount)) {
            return false;
        }
        for (int i = 0; i < pageCount; ++i) {
            SkSize s = pages[i].fSize;
            pages[i].fPicture->playback(doc->beginPage(s.width(), s.height()));
        }
        return true;
    }


