/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJpegDecoderMgr.h"

#include "SkJpegUtility.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    #include "SkAndroidFrameworkUtils.h"
#endif

/*
 * Print information, warning, and error messages
 */
static void print_message(const j_common_ptr info, const char caller[]) {
    char buffer[JMSG_LENGTH_MAX];
    info->err->format_message(info, buffer);
    SkCodecPrintf("libjpeg error %d <%s> from %s\n", info->err->msg_code, buffer, caller);
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

JpegDecoderMgr::JpegDecoderMgr(SkStream* stream)
    : fSrcMgr(stream)
    , fInit(false)
{
    // Error manager must be set before any calls to libjeg in order to handle failures
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
