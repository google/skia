/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPngEncoderImpl_DEFINED
#define SkPngEncoderImpl_DEFINED

#include "include/encode/SkEncoder.h"

#include <memory>

class SkPixmap;
class SkPngEncoderMgr;

class SkPngEncoderImpl : public SkEncoder {
public:
    // public so it can be called from SkPngEncoder namespace. It should only be made
    // via SkPngEncoder::Make
    SkPngEncoderImpl(std::unique_ptr<SkPngEncoderMgr>, const SkPixmap& src);
    ~SkPngEncoderImpl() override;

protected:
    bool onEncodeRows(int numRows) override;
    std::unique_ptr<SkPngEncoderMgr> fEncoderMgr;
};
#endif
