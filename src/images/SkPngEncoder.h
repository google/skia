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

    static constexpr int kNone_FilterFlag   = 0x08;
    static constexpr int kSub_FilterFlag    = 0x10;
    static constexpr int kUp_FilterFlag     = 0x20;
    static constexpr int kAvg_FilterFlag    = 0x40;
    static constexpr int kPaeth_FilterFlag  = 0x80;
    static constexpr int kAll_FilterFlag    = kNone_FilterFlag | kSub_FilterFlag | kUp_FilterFlag |
                                              kAvg_FilterFlag | kPaeth_FilterFlag;

    struct Options {
        /**
         *  Selects which filtering strategies to use.
         *
         *  If a single filter is chosen, libpng will use that filter for every row.
         *
         *  If multiple filters are chosen, libpng will use a heuristic to guess which filter
         *  will encode smallest, then apply that filter.  This happens on a per row basis,
         *  different rows can use different filters.
         *
         *  Using a single filter (or less filters) is typically faster.  Trying all of the
         *  filters may help minimize the output file size.
         */
        int fFilterFlags = kAll_FilterFlag;

        /**
         *  Must be in [0, 9] where 9 corresponds to maximal compression.  This value is passed
         *  directly to zlib.
         */
        int fZLibLevel = 6;

        /**
         *  If the input is premultiplied, this controls the unpremultiplication behavior.
         *  The encoder can convert to linear before unpremultiplying or ignore the transfer
         *  function and unpremultiply the input as is.
         */
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
