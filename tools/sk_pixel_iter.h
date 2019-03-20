/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_pixel_iter_DEFINED
#define sk_pixel_iter_DEFINED

#include "SkPixmap.h"
#include "SkSurface.h"

namespace ToolUtils {

class PixelIter {
public:
    PixelIter();
    PixelIter(SkSurface* surf) {
        SkPixmap pm;
        if (!surf->peekPixels(&pm)) {
            pm.reset();
        }
        this->reset(pm);
    }

    void reset(const SkPixmap& pm) {
        fPM  = pm;
        fLoc = {-1, 0};
    }

    void* next(SkIPoint* loc = nullptr) {
        if (!fPM.addr()) {
            return nullptr;
        }
        fLoc.fX += 1;
        if (fLoc.fX >= fPM.width()) {
            fLoc.fX = 0;
            if (++fLoc.fY >= fPM.height()) {
                this->setDone();
                return nullptr;
            }
        }
        if (loc) {
            *loc = fLoc;
        }
        return fPM.writable_addr(fLoc.fX, fLoc.fY);
    }

    void setDone() { fPM.reset(); }

private:
    SkPixmap fPM;
    SkIPoint fLoc;
};

}  // namespace ToolUtils

#endif  // ToolUtils_DEFINED
