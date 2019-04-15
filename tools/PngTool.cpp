// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "PngTool.h"

#include "png.h"

#include <cstdio>
#include <cassert>

namespace {
template <void (*F)(png_struct**, png_info**)>
struct PNGCleaner {
    png_struct* ptr = nullptr;
    png_info* info = nullptr;
    ~PNGCleaner() { if (ptr || info) { F(&ptr, &info); }}
    PNGCleaner() = default;
    PNGCleaner(const PNGCleaner&) = delete;
    PNGCleaner& operator=(const PNGCleaner&) = delete;
};

struct FCloseWrapper { void operator()(FILE* fp) { fclose(fp); } };
}  // namespace

bool PngTool::Write(const char* path,
                    const Pixmap& pixmap,
                    const PngTool::WriteOptions& opts) {
    if (!pixmap.pixels) {
        return false;
    }
    std::unique_ptr<FILE, FCloseWrapper> f(fopen(path, "wb"));
    if (!f) {
        return false;
    }
    PNGCleaner<png_destroy_write_struct> png;
    png.ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png.ptr) {
        return false;
    }
    png.info = png_create_info_struct(png.ptr);
    if (!png.info) {
        return false;
    }
    constexpr unsigned kMaxTextCount = 2;
    png_text ptext[kMaxTextCount];
    unsigned textCount = 0;
    if (opts.author) {
        ptext[textCount].key  = (png_charp)"Author";
        ptext[textCount].text = (png_charp)opts.author;
        ptext[textCount].compression = PNG_TEXT_COMPRESSION_NONE;
        ++textCount;
    }
    if (opts.description) {
        ptext[textCount].key  = (png_charp)"Description";
        ptext[textCount].text = (png_charp)opts.description;
        ptext[textCount].compression = PNG_TEXT_COMPRESSION_NONE;
        ++textCount;
    }
    if (textCount) {
        assert(textCount <= kMaxTextCount);
        png_set_text(png.ptr, png.info, ptext, textCount);
    }

    png_init_io(png.ptr, f.get());

    static_assert(PNG_COLOR_TYPE_GRAY       == (int)PngTool::ColorType::kGray, "");
    static_assert(PNG_COLOR_TYPE_RGB        == (int)PngTool::ColorType::kRGB, "");
    static_assert(PNG_COLOR_TYPE_GRAY_ALPHA == (int)PngTool::ColorType::kGrayAlpha, "");
    static_assert(PNG_COLOR_TYPE_RGBA       == (int)PngTool::ColorType::kRGBA, "");
    png_set_IHDR(png.ptr, png.info, pixmap.width, pixmap.height, pixmap.bitDepth,
                 (int)pixmap.colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_set_filter(png.ptr, PNG_FILTER_TYPE_BASE, opts.fast ? PNG_FILTER_NONE : PNG_ALL_FILTERS);
    png_set_compression_level(png.ptr, opts.fast ? 1 : 9);

    if (opts.iccData && opts.iccDataLength && opts.iccName) {
        png_set_iCCP(png.ptr, png.info, opts.iccName, PNG_COMPRESSION_TYPE_BASE,
                     (const unsigned char*)opts.iccData, opts.iccDataLength);
    }

    png_write_info(png.ptr, png.info);
    unsigned rowBytes = png_get_rowbytes(png.ptr, png.info);
    const unsigned char* ptr = pixmap.pixels;
    for (unsigned y = 0; y < pixmap.height; y++) {
        png_write_row(png.ptr, ptr);
        ptr += rowBytes;
    }
    png_write_end(png.ptr, png.info);
    return true;
}

static void destroy_read(png_struct** p, png_info** i) { png_destroy_read_struct(p, i, nullptr); }

PngTool::Pixmap PngTool::Read(const char* path, void* (*mallocFn)(size_t)) {
    PngTool::Pixmap result;
    std::unique_ptr<FILE, FCloseWrapper> infile(fopen(path, "rb"));
    if (!infile) {
        return result;
    }
    unsigned char sig[8];
    fread(sig, 1, 8, infile.get());
    if (!png_check_sig(sig, 8)) {
        return result;
    }
    PNGCleaner<destroy_read> png;
    png.ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png.ptr) {
        return result;
    }
    png.info = png_create_info_struct(png.ptr);
    if (!png.info) {
        return result;
    }
    if (setjmp(png_jmpbuf(png.ptr))) {
        return result;
    }
    png_init_io(png.ptr, infile.get());
    png_set_sig_bytes(png.ptr, 8);
    png_read_info(png.ptr, png.info);
    int colorType = 0;
    png_get_IHDR(png.ptr, png.info, &result.width, &result.height, &result.bitDepth,
                 &colorType, nullptr, nullptr, nullptr);
    if (colorType != PNG_COLOR_TYPE_GRAY &&
        colorType != PNG_COLOR_TYPE_RGB &&
        colorType != PNG_COLOR_TYPE_GRAY_ALPHA &&
        colorType != PNG_COLOR_TYPE_RGBA) { return result; }
    result.colorType = (PngTool::ColorType)colorType;
    unsigned rowBytes = png_get_rowbytes(png.ptr, png.info);
    unsigned char* ptr = (unsigned char*)mallocFn(rowBytes * result.height);
    result.pixels = ptr;
    for (unsigned y = 0; y < result.height; ++y) {
        png_read_row(png.ptr, ptr, nullptr);
        ptr += rowBytes;
    }
    return result;
}
