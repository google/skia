/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkJpegPriv_DEFINED
#define SkJpegPriv_DEFINED

#include "include/core/SkStream.h"
#include "include/private/base/SkTArray.h"

#include <setjmp.h>
// stdio is needed for jpeglib
#include <stdio.h>

extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
}

static constexpr uint8_t kJpegSig[] = {0xFF, 0xD8, 0xFF};

static constexpr uint32_t kICCMarker = JPEG_APP0 + 2;
static constexpr uint32_t kICCMarkerHeaderSize = 14;
static constexpr uint32_t kICCMarkerIndexSize = 1;
static constexpr uint8_t kICCSig[] = {
        'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', '\0',
};

static constexpr uint32_t kGainmapMarker = JPEG_APP0 + 15;
static constexpr uint32_t kGainmapMarkerIndexSize = 2;
static constexpr uint8_t kGainmapSig[] = {
        'H', 'D', 'R', '_', 'G', 'A', 'I', 'N', '_', 'M', 'A', 'P', '\0',
};

static constexpr uint32_t kXMPMarker = JPEG_APP0 + 1;

static constexpr uint8_t kXMPStandardSig[] = {
        'h', 't', 't', 'p', ':', '/', '/', 'n', 's', '.', 'a', 'd', 'o', 'b', 'e', '.', 'c', 'o',
        'm', '/', 'x', 'a', 'p', '/', '1', '.', '0', '/', '\0'};

static constexpr uint8_t kXMPExtendedSig[] = {
        'h', 't', 't', 'p', ':', '/', '/', 'n', 's', '.', 'a', 'd', 'o', 'b', 'e', '.', 'c', 'o',
        'm', '/', 'x', 'm', 'p', '/', 'e', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', '/', '\0'};

static constexpr uint32_t kExifMarker = JPEG_APP0 + 1;
static constexpr uint32_t kExifHeaderSize = 14;
constexpr uint8_t kExifSig[] = {'E', 'x', 'i', 'f', '\0'};

static constexpr uint32_t kMpfMarker = JPEG_APP0 + 2;
static constexpr uint8_t kMpfSig[] = {'M', 'P', 'F', '\0'};

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
