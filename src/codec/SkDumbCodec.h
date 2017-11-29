/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDumbCodec_DEFINED
#define SkDumbCodec_DEFINED

#include "SkCodec.h"
#include "SkImageInfo.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTypes.h"

/*
 *  This class implements a trivial (dumb) codec that doesn't try to compress, but also doesn't
 *  care what the pixel format it, it just memcpys the pixels.
 */
class SkDumbCodec : public SkCodec {
public:
    static bool IsDumb(const void*, size_t);

    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    static bool Encode(SkWStream*, const SkPixmap&);

protected:
    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kDUMB;
    }

    Result onGetPixels(const SkImageInfo&, void* dst, size_t rowBytes, const Options&,
                       int*) override;

private:
    SkDumbCodec(const SkImageInfo&, size_t rowBytes, sk_sp<SkData> pixels);

    const SkPixmap  fPixmap;
    sk_sp<SkData>   fStorage;

    typedef SkCodec INHERITED;
};
#endif
