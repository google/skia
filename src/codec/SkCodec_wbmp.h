/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_wbmp_DEFINED
#define SkCodec_wbmp_DEFINED

#include "SkCodec.h"

class SkWbmpCodec final : public SkCodec {
public:
    static bool IsWbmp(SkStream*);
    static SkCodec* NewFromStream(SkStream*);
protected:
    SkEncodedFormat onGetEncodedFormat() const override;
    Result onGetPixels(const SkImageInfo&, void*, size_t,
                       const Options&, SkPMColor[], int*) override;
private:
    SkWbmpCodec(const SkImageInfo&, SkStream*);
    typedef SkCodec INHERITED;
};

#endif  // SkCodec_wbmp_DEFINED
