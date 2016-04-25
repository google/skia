/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJpegDecoderMgr.h"

#include "SkJpegUtility.h"

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
}

JpegDecoderMgr::~JpegDecoderMgr() {
    if (fInit) {
        jpeg_destroy_decompress(&fDInfo);
    }
}

jmp_buf& JpegDecoderMgr::getJmpBuf() {
    return fErrorMgr.fJmpBuf;
}

jpeg_decompress_struct* JpegDecoderMgr::dinfo() {
    return &fDInfo;
}
