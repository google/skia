
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

class SkPDFObjNumMap;

/** \class SkPDFStream

    A stream object in a PDF.  Note, all streams must be indirect objects (via
    SkObjRef).
*/
class SkPDFStream : public SkPDFDict {
    
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

    // The SkPDFObject interface.
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& objNumMap,
                    const SkPDFSubstituteMap& substitutes) override;

protected:
    enum State {
        kUnused_State,         //!< The stream hasn't been requested yet.
        kNoCompression_State,  //!< The stream's been requested in an
                               //   uncompressed form.
        kCompressed_State,     //!< The stream's already been compressed.
    };

    /* Create a PDF stream with no data.  The setData method must be called to
     * set the data.
     */
    SkPDFStream();

    void setData(SkData* data);
    void setData(SkStream* stream);

    size_t dataSize() const;

    void setState(State state) {
        fState = state;
    }

private:
    // Indicates what form (or if) the stream has been requested.
    State fState;

    SkAutoTDelete<SkStreamRewindable> fDataStream;

    typedef SkPDFDict INHERITED;
};

#endif
