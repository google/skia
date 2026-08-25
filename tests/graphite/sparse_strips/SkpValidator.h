/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_SkpValidator_DEFINED
#define skgpu_graphite_sparse_strips_SkpValidator_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/private/SkTDArray.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <vector>

class SkPicture;
class SkStream;

namespace skgpu::graphite {

class SkpValidator {
public:
    struct ExtractedPath {
        SkPath path;
        SkPaint paint;
        SkMatrix matrix;
        SkPath devicePath;
    };

    static std::vector<ExtractedPath> ExtractPaths(const char* filepath);
    static std::vector<ExtractedPath> ExtractPaths(SkStream* stream);
    static std::vector<ExtractedPath> ExtractPaths(const SkPicture* picture);

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static bool ValidatePath(
            skiatest::Reporter* reporter,
            const SkPath& path,
            const char* testName,
            const SkTDArray<uint8_t>& maskLut,
            std::array<uint32_t, 3>* minorErrorCount = nullptr);

    template <uint16_t kTileWidth, uint16_t kTileHeight>
    static bool ValidateSkp(
            skiatest::Reporter* reporter,
            const char* filepath,
            const SkTDArray<uint8_t>& maskLut);
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_SkpValidator_DEFINED
