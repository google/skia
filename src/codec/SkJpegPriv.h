/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegPriv_DEFINED
#define SkJpegPriv_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTArray.h"

#include <setjmp.h>
// stdio is needed for jpeglib
#include <stdio.h>

extern "C" {
    #include "jpeglib.h"  // NO_G3_REWRITE
    #include "jerror.h"   // NO_G3_REWRITE
}

/*
 * Error handling struct
 */
struct skjpeg_error_mgr : jpeg_error_mgr {
    class AutoPushJmpBuf {
    public:
        AutoPushJmpBuf(skjpeg_error_mgr* mgr) : fMgr(mgr) { fMgr->push(&fJmpBuf); }
        ~AutoPushJmpBuf()                                 { fMgr->pop(&fJmpBuf); }
        operator jmp_buf&()                               { return fJmpBuf; }

    private:
        skjpeg_error_mgr* const fMgr;
        jmp_buf fJmpBuf;
    };

    void push(jmp_buf* j) {
        SkASSERT(fStack[3] == nullptr);  // if we assert here, the stack has overflowed
        fStack[3] = fStack[2];
        fStack[2] = fStack[1];
        fStack[1] = fStack[0];
        fStack[0] = j;
    }

    void pop(jmp_buf* j) {
        SkASSERT(fStack[0] == j);  // if we assert here, the pushes and pops were unbalanced
        fStack[0] = fStack[1];
        fStack[1] = fStack[2];
        fStack[2] = fStack[3];
        fStack[3] = nullptr;
    }

    jmp_buf* fStack[4] = {};
};

namespace SkJpegPriv {
SkEncodedOrigin get_exif_orientation(jpeg_decompress_struct* dinfo);
}

#endif
