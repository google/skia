// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4486d0c0b22ad2931db130f42da4c80c
REG_FIDDLE(Canvas_015, 256, 256, true, 0) {
static void DeleteCallback(void*, void* context) {
    delete (char*) context;
}
class CustomAllocator : public SkRasterHandleAllocator {
public:
    bool allocHandle(const SkImageInfo& info, Rec* rec) override {
        char* context = new char[4]{'s', 'k', 'i', 'a'};
        rec->fReleaseProc = DeleteCallback;
        rec->fReleaseCtx = context;
        rec->fHandle = context;
        rec->fPixels = context;
        rec->fRowBytes = 4;
        return true;
    }
    void updateHandle(Handle handle, const SkMatrix& ctm, const SkIRect& clip_bounds) override {
        // apply canvas matrix and clip to custom environment
    }
};

void draw(SkCanvas* canvas) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    std::unique_ptr<SkCanvas> c2 =
            SkRasterHandleAllocator::MakeCanvas(std::unique_ptr<CustomAllocator>(
            new CustomAllocator()), info);
    char* context = (char*) c2->accessTopRasterHandle();
    SkDebugf("context = %.4s\n", context);
}
}  // END FIDDLE
