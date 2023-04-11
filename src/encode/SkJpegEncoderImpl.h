/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegEncoderImpl_DEFINED
#define SkJpegEncoderImpl_DEFINED

#include "include/encode/SkEncoder.h"

#include <memory>

class SkJpegEncoderMgr;
class SkPixmap;
class SkYUVAPixmaps;

class SkJpegEncoderImpl : public SkEncoder {
public:
    SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr>, const SkPixmap& src);
    SkJpegEncoderImpl(std::unique_ptr<SkJpegEncoderMgr>, const SkYUVAPixmaps* srcYUVA);

    ~SkJpegEncoderImpl() override;

protected:
    bool onEncodeRows(int numRows) override;

private:
    std::unique_ptr<SkJpegEncoderMgr> fEncoderMgr;
    const SkYUVAPixmaps* fSrcYUVA = nullptr;
};

#endif
