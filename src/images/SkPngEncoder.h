/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPngEncoder_DEFINED
#define SkPngEncoder_DEFINED

#include "SkEncoder.h"

class SkPngEncoderMgr;
class SkWStream;

class SkPngEncoder : public SkEncoder {
public:

    // TODO (skbug.com/6409):
    // Add options for png filters and zlib compression.

    struct Options {
        SkTransferFunctionBehavior fUnpremulBehavior = SkTransferFunctionBehavior::kRespect;
    };

    /**
     *  Encode the |src| pixels to the |dst| stream.
     *  |options| may be used to control the encoding behavior.
     *
     *  Returns true on success.  Returns false on an invalid or unsupported |src|.
     */
    static bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options);

    /**
     *  Create a png encoder that will encode the |src| pixels to the |dst| stream.
     *  |options| may be used to control the encoding behavior.
     *
     *  |dst| is unowned but must remain valid for the lifetime of the object.
     *
     *  This returns nullptr on an invalid or unsupported |src|.
     */
    static std::unique_ptr<SkPngEncoder> Make(SkWStream* dst, const SkPixmap& src,
                                               const Options& options);

    ~SkPngEncoder() override;

protected:
    bool onEncodeRows(int numRows) override;

    SkPngEncoder(std::unique_ptr<SkPngEncoderMgr>, const SkPixmap& src);

    std::unique_ptr<SkPngEncoderMgr> fEncoderMgr;
    typedef SkEncoder INHERITED;
};

#endif
