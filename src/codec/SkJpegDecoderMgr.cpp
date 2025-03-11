/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/codec/SkJpegDecoderMgr.h"

#include "include/core/SkTypes.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegSourceMgr.h"
#include "src/codec/SkJpegUtility.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    #include "include/android/SkAndroidFrameworkUtils.h"
#endif

#include <jpeglib.h>
#include <cstddef>
#include <utility>

class SkStream;

/*
 * Print information, warning, and error messages
 */
static void print_message(const j_common_ptr info, const char caller[]) {
    [[maybe_unused]] char buffer[JMSG_LENGTH_MAX];
    SkCodecPrintf("libjpeg error %d <%s> from %s\n",
                  info->err->msg_code,
                  (info->err->format_message(info, buffer), buffer),
                  caller);
}

/*
 * Reporting function for error and warning messages.
 */
static void output_message(j_common_ptr info) {
    print_message(info, "output_message");
}

static void progress_monitor(j_common_ptr info) {
  int scan = ((j_decompress_ptr)info)->input_scan_number;
  // Progressive images with a very large number of scans can cause the
  // decoder to hang.  Here we use the progress monitor to abort on
  // a very large number of scans.  100 is arbitrary, but much larger
  // than the number of scans we might expect in a normal image.
  if (scan >= 100) {
      skjpeg_err_exit(info);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JpegDecoderMgr

bool JpegDecoderMgr::returnFalse(const char caller[]) {
    print_message((j_common_ptr) &fDInfo, caller);
    return false;
}

SkCodec::Result JpegDecoderMgr::returnFailure(const char caller[], SkCodec::Result result) {
    print_message((j_common_ptr) &fDInfo, caller);
    return result;
}

bool JpegDecoderMgr::getEncodedColor(SkEncodedInfo::Color* outColor) {
    switch (fDInfo.jpeg_color_space) {
        case JCS_GRAYSCALE:
            *outColor = SkEncodedInfo::kGray_Color;
            return true;
        case JCS_YCbCr:
            *outColor = SkEncodedInfo::kYUV_Color;
            return true;
        case JCS_RGB:
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
            SkAndroidFrameworkUtils::SafetyNetLog("118372692");
#endif
            *outColor = SkEncodedInfo::kRGB_Color;
            return true;
        case JCS_YCCK:
            *outColor = SkEncodedInfo::kYCCK_Color;
            return true;
        case JCS_CMYK:
            *outColor = SkEncodedInfo::kInvertedCMYK_Color;
            return true;
        default:
            return false;
    }
}

SkJpegSourceMgr* JpegDecoderMgr::getSourceMgr() {
    return fSrcMgr.fSourceMgr.get();
}

JpegDecoderMgr::JpegDecoderMgr(SkStream* stream)
        : fSrcMgr(SkJpegSourceMgr::Make(stream)), fInit(false) {
    // An error manager must be set before any calls to libjpeg, in order to handle failures.
    fDInfo.err = jpeg_std_error(&fErrorMgr);
    fErrorMgr.error_exit = skjpeg_err_exit;
}

void JpegDecoderMgr::init() {
    jpeg_create_decompress(&fDInfo);
    fInit = true;
    fDInfo.src = &fSrcMgr;
    fDInfo.err->output_message = &output_message;
    fDInfo.progress = &fProgressMgr;
    fProgressMgr.progress_monitor = &progress_monitor;
}

JpegDecoderMgr::~JpegDecoderMgr() {
    if (fInit) {
        jpeg_destroy_decompress(&fDInfo);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JpegDecoderMgr::SourceMgr

// static
void JpegDecoderMgr::SourceMgr::InitSource(j_decompress_ptr dinfo) {
    JpegDecoderMgr::SourceMgr* src = (JpegDecoderMgr::SourceMgr*)dinfo->src;
    src->fSourceMgr->initSource(src->next_input_byte, src->bytes_in_buffer);
}

// static
void JpegDecoderMgr::SourceMgr::SkipInputData(j_decompress_ptr dinfo, long num_bytes_long) {
    JpegDecoderMgr::SourceMgr* src = (JpegDecoderMgr::SourceMgr*)dinfo->src;
    size_t num_bytes = static_cast<size_t>(num_bytes_long);
    if (!src->fSourceMgr->skipInputBytes(num_bytes, src->next_input_byte, src->bytes_in_buffer)) {
        SkCodecPrintf("Failure to skip.\n");
        src->next_input_byte = nullptr;
        src->bytes_in_buffer = 0;
        dinfo->err->error_exit((j_common_ptr)dinfo);
    }
}

// static
boolean JpegDecoderMgr::SourceMgr::FillInputBuffer(j_decompress_ptr dinfo) {
    JpegDecoderMgr::SourceMgr* src = (JpegDecoderMgr::SourceMgr*)dinfo->src;
    if (!src->fSourceMgr->fillInputBuffer(src->next_input_byte, src->bytes_in_buffer)) {
        SkCodecPrintf("Failure to fill input buffer.\n");
        src->next_input_byte = nullptr;
        src->bytes_in_buffer = 0;
        return false;
    }
    return true;
}

// static
void JpegDecoderMgr::SourceMgr::TermSource(j_decompress_ptr dinfo) {}

JpegDecoderMgr::SourceMgr::SourceMgr(std::unique_ptr<SkJpegSourceMgr> sourceMgr)
        : fSourceMgr(std::move(sourceMgr)) {
    init_source = JpegDecoderMgr::SourceMgr::InitSource;
    fill_input_buffer = JpegDecoderMgr::SourceMgr::FillInputBuffer;
    skip_input_data = JpegDecoderMgr::SourceMgr::SkipInputData;
    resync_to_restart = jpeg_resync_to_restart;
    term_source = JpegDecoderMgr::SourceMgr::TermSource;
}
