/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGInvalidationController_DEFINED
#define SkSGInvalidationController_DEFINED

#include "SkMatrix.h"
#include "SkTDArray.h"
#include "SkTypes.h"

struct SkRect;

namespace sksg {

/**
 * Receiver for invalidation events.
 *
 * Tracks dirty regions for repaint.
 */
class InvalidationController : public SkNoncopyable {
public:
    InvalidationController();

    void inval(const SkRect&, const SkMatrix& ctm = SkMatrix::I());

    const SkRect& bounds() const { return fBounds;        }
    const SkRect*  begin() const { return fRects.begin(); }
    const SkRect*    end() const { return fRects.end();   }

private:
    SkTDArray<SkRect> fRects;
    SkRect            fBounds;

    typedef SkNoncopyable INHERITED;
};

} // namespace sksg

#endif // SkSGInvalidationController_DEFINED
