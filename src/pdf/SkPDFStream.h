
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFStream_DEFINED
#define SkPDFStream_DEFINED

#include "SkPDFTypes.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTemplates.h"

class SkPDFCatalog;

/** \class SkPDFStream

    A stream object in a PDF.  Note, all streams must be indirect objects (via
    SkObjRef).
*/
class SkPDFStream : public SkPDFDict {
    SK_DECLARE_INST_COUNT(SkPDFStream)
public:
    /** Create a PDF stream. A Length entry is automatically added to the
     *  stream dictionary.
     *  @param data   The data part of the stream.  Will be ref()ed.
     */
    explicit SkPDFStream(SkData* data);

    /** Create a PDF stream. A Length entry is automatically added to the
     *  stream dictionary.
     *  @param stream The data part of the stream.  Will be duplicate()d.
     */
    explicit SkPDFStream(SkStream* stream);

    virtual ~SkPDFStream();

    // The SkPDFObject interface.  These two methods use a mutex to
    // allow multiple threads to call at the same time.
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                            bool indirect);
    virtual size_t getOutputSize(SkPDFCatalog* catalog, bool indirect);

protected:
    enum State {
        kUnused_State,         //!< The stream hasn't been requested yet.
        kNoCompression_State,  //!< The stream's been requested in an
                               //   uncompressed form.
        kCompressed_State,     //!< The stream's already been compressed.
    };

    /** Create a PDF stream with the same content and dictionary entries
     *  as the passed one.
     */
    explicit SkPDFStream(const SkPDFStream& pdfStream);

    /* Create a PDF stream with no data.  The setData method must be called to
     * set the data.
     */
    SkPDFStream();

    // Populate the stream dictionary.  This method returns false if
    // fSubstitute should be used.
    virtual bool populate(SkPDFCatalog* catalog);

    void setSubstitute(SkPDFStream* stream) {
        fSubstitute.reset(stream);
    }

    SkPDFStream* getSubstitute() const {
        return fSubstitute.get();
    }

    void setData(SkData* data);
    void setData(SkStream* stream);

    size_t dataSize() const;

    void setState(State state) {
        fState = state;
    }

    State getState() const {
        return fState;
    }

private:
    // Indicates what form (or if) the stream has been requested.
    State fState;

    // Mutex guards fState, fDataStream, and fSubstitute in public interface.
    SkMutex fMutex;

    SkMemoryStream fMemoryStream;  // Used by fDataStream when
                                   // fDataStream needs to be backed
                                   // by SkData.
    SkAutoTUnref<SkStreamRewindable> fDataStream;
    SkAutoTUnref<SkPDFStream> fSubstitute;

    typedef SkPDFDict INHERITED;
};

#endif
