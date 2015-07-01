/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJpegDecoderMgr.h"
#include "SkJpegUtility_codec.h"

/*
 * Print information, warning, and error messages
 */
static void print_message(const j_common_ptr info, const char caller[]) {
    char buffer[JMSG_LENGTH_MAX];
    info->err->format_message(info, buffer);
    SkCodecPrintf("libjpeg error %d <%s> from %s\n", info->err->msg_code, buffer, caller);
}

/*
 * Reporting functions for libjpeg
 */
static void emit_message(j_common_ptr info, int) {
    print_message(info, "emit_message");
}
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

SkColorType JpegDecoderMgr::getColorType() {
    switch (fDInfo.jpeg_color_space) {
        case JCS_GRAYSCALE:
            return kGray_8_SkColorType;
        default:
            return kN32_SkColorType;
    }
}

JpegDecoderMgr::JpegDecoderMgr(SkStream* stream)
    : fSrcMgr(stream)
    , fInit(false)
{
    // Error manager must be set before any calls to libjeg in order to handle failures
    fDInfo.err = turbo_jpeg_std_error(&fErrorMgr);
    fErrorMgr.error_exit = skjpeg_err_exit;
}

void JpegDecoderMgr::init() {
    jpeg_create_decompress(&fDInfo);
    fInit = true;
    fDInfo.src = &fSrcMgr;
    fDInfo.err->emit_message = &emit_message;
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
