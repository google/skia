/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFStream_DEFINED
#define SkPDFStream_DEFINED

#include "SkPDFTypes.h"
#include "SkStream.h"

/** \class SkPDFStream

    A stream object in a PDF.  Note, all streams must be indirect objects (via
    SkObjRef).
*/
class SkPDFStream : public SkPDFDict {

public:
    /** Create a PDF stream. A Length entry is automatically added to the
     *  stream dictionary.
     *  @param data   The data part of the stream. */
    explicit SkPDFStream(sk_sp<SkData> data) {
        this->setData(std::unique_ptr<SkStreamAsset>(
                              new SkMemoryStream(std::move(data))));
    }

    /** Create a PDF stream. A Length entry is automatically added to the
     *  stream dictionary.
     *  @param stream The data part of the stream.
     */
    explicit SkPDFStream(std::unique_ptr<SkStreamAsset> stream) {
        this->setData(std::move(stream));
    }

    virtual ~SkPDFStream();

    // The SkPDFObject interface.
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& objNumMap,
                    const SkPDFSubstituteMap& substitutes) const override;
    void drop() override;

protected:
    /* Create a PDF stream with no data.  The setData method must be called to
     * set the data.
     */
    SkPDFStream() {}

    /** Only call this function once. */
    void setData(std::unique_ptr<SkStreamAsset> stream);

private:
    std::unique_ptr<SkStreamAsset> fCompressedData;

    typedef SkPDFDict INHERITED;
};

#endif
