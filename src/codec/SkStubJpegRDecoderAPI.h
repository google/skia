/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStubJpegRDecoderAPI_DEFINED
#define SkStubJpegRDecoderAPI_DEFINED

typedef enum {
    JPEGR_COLORSPACE_UNSPECIFIED,
    JPEGR_COLORSPACE_BT709,
    JPEGR_COLORSPACE_P3,
    JPEGR_COLORSPACE_BT2100,
} jpegr_color_space;

struct jpegr_info_struct {
    size_t width;
    size_t height;
    std::vector<uint8_t>* iccData;
    std::vector<uint8_t>* exifData;
};

/*
 * Holds information for uncompressed image or recovery map.
 */
struct jpegr_uncompressed_struct {
    // Pointer to the data location.
    void* data;
    // Width of the recovery map or image in pixels.
    int width;
    // Height of the recovery map or image in pixels.
    int height;
    // Color space.
    jpegr_color_space colorSpace;
};

/*
 * Holds information for compressed image or recovery map.
 */
struct jpegr_compressed_struct {
    // Pointer to the data location.
    void* data;
    // Data length.
    int length;
    // Color space.
    jpegr_color_space colorSpace;
};

/*
 * Holds information for EXIF metadata.
 */
struct jpegr_exif_struct {
    // Pointer to the data location.
    void* data;
    // Data length;
    int length;
};

typedef struct jpegr_uncompressed_struct* jr_uncompressed_ptr;
typedef struct jpegr_compressed_struct* jr_compressed_ptr;
typedef struct jpegr_exif_struct* jr_exif_ptr;
typedef struct jpegr_info_struct* jr_info_ptr;
class RecoveryMap {
public:
    int32_t encodeJPEGR(jr_uncompressed_ptr uncompressed_p010_image,
                        jr_uncompressed_ptr uncompressed_yuv_420_image,
                        jr_compressed_ptr dest,
                        int quality,
                        jr_exif_ptr exif) {
        return -1;
    }

    int32_t encodeJPEGR(jr_uncompressed_ptr uncompressed_p010_image,
                        jr_uncompressed_ptr uncompressed_yuv_420_image,
                        jr_compressed_ptr compressed_jpeg_image,
                        jr_compressed_ptr dest) {
        return -1;
    }

    int32_t encodeJPEGR(jr_uncompressed_ptr uncompressed_p010_image,
                        jr_compressed_ptr compressed_jpeg_image,
                        jr_compressed_ptr dest) {
        return -1;
    }

    int32_t decodeJPEGR(jr_compressed_ptr compressed_jpegr_image,
                        jr_uncompressed_ptr dest,
                        jr_exif_ptr exif = nullptr,
                        bool request_sdr = false) {
        return -1;
    }

    int32_t getJPEGRInfo(jr_compressed_ptr compressed_jpegr_image, jr_info_ptr jpegr_info) {
        return -1;
    }
};

#endif
