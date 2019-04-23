/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegPriv_DEFINED
#define SkJpegPriv_DEFINED

#include "include/core/SkStream.h"
#include "include/private/SkTArray.h"

#include <setjmp.h>
// stdio is needed for jpeglib
#include <stdio.h>

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

static constexpr uint32_t kICCMarker = JPEG_APP0 + 2;
static constexpr uint32_t kICCMarkerHeaderSize = 14;
static constexpr uint8_t kICCSig[] = {
        'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', '\0',
};

/*
 * Error handling struct
 */
struct skjpeg_error_mgr : jpeg_error_mgr {
    class AutoPushJmpBuf {
    public:
        AutoPushJmpBuf(skjpeg_error_mgr* mgr) : fMgr(mgr) {
            fMgr->fJmpBufStack.push_back(&fJmpBuf);
        }
        ~AutoPushJmpBuf() {
            SkASSERT(fMgr->fJmpBufStack.back() == &fJmpBuf);
            fMgr->fJmpBufStack.pop_back();
        }
        operator jmp_buf&() { return fJmpBuf; }

    private:
        skjpeg_error_mgr* const fMgr;
        jmp_buf fJmpBuf;
    };

    SkSTArray<4, jmp_buf*> fJmpBufStack;
};

#endif
