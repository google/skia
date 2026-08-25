/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/sparse_strips/SkpValidator.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "tests/Test.h"
#include "tests/graphite/sparse_strips/OracleValidator.h"
#include "tools/ToolUtils.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

std::vector<SkpValidator::ExtractedPath> SkpValidator::ExtractPaths(const char* filepath) {
    std::vector<ExtractedPath> paths;
    auto callback = [&](const SkMatrix& ctm, const SkPath& path, const SkPaint& paint) {
        SkPath fillPath = skpathutils::FillPathWithPaint(path, paint);
        SkPath devPath = fillPath.makeTransform(ctm);
        paths.push_back({path, paint, ctm, devPath});
    };
    ToolUtils::ExtractPathsFromSKP(filepath, callback);
    return paths;
}

std::vector<SkpValidator::ExtractedPath> SkpValidator::ExtractPaths(SkStream* stream) {
    if (!stream) {
        return {};
    }
    sk_sp<SkPicture> picture = SkPicture::MakeFromStream(stream, nullptr);
    if (!picture) {
        return {};
    }
    return ExtractPaths(picture.get());
}

std::vector<SkpValidator::ExtractedPath> SkpValidator::ExtractPaths(const SkPicture* picture) {
    if (!picture) {
        return {};
    }
    class PathSniffer : public SkCanvas {
    public:
        explicit PathSniffer(std::function<ToolUtils::PathSniffCallback> callback)
                : SkCanvas(4096, 4096, nullptr), fCallback(callback) {}

    private:
        void onDrawPath(const SkPath& path, const SkPaint& paint) override {
            fCallback(this->getTotalMatrix(), path, paint);
        }
        std::function<ToolUtils::PathSniffCallback> fCallback;
    };

    std::vector<ExtractedPath> paths;
    auto callback = [&](const SkMatrix& ctm, const SkPath& path, const SkPaint& paint) {
        SkPath fillPath = skpathutils::FillPathWithPaint(path, paint);
        SkPath devPath = fillPath.makeTransform(ctm);
        paths.push_back({path, paint, ctm, devPath});
    };
    PathSniffer sniffer(callback);
    picture->playback(&sniffer);
    return paths;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool SkpValidator::ValidatePath(skiatest::Reporter* reporter,
                                const SkPath& path,
                                const char* testName,
                                const SkTDArray<uint8_t>& maskLut,
                                std::array<uint32_t, 3>* minorErrorCount) {
    if (path.isEmpty() || !path.isFinite()) {
        return true;
    }

    SkRect bounds = path.getBounds();
    if (bounds.width() <= 0.0f || bounds.height() <= 0.0f) {
        return true;
    }

    float margin = 4.0f;
    SkMatrix trans;
    trans.setTranslate(margin - bounds.fLeft, margin - bounds.fTop);
    SkPath localPath = path.makeTransform(trans);
    SkRect localBounds = localPath.getBounds();

    uint16_t rawWidth = static_cast<uint16_t>(std::ceil(localBounds.fRight + margin));
    uint16_t rawHeight = static_cast<uint16_t>(std::ceil(localBounds.fBottom + margin));

    uint16_t vpWidth = (rawWidth + kTileWidth - 1) & ~(kTileWidth - 1);
    uint16_t vpHeight = (rawHeight + kTileHeight - 1) & ~(kTileHeight - 1);

    Flatten flattener;
    Polyline polyline;
    flattener.processPaths<FlattenMode::kSimd>(localPath,
                                               SkMatrix(),
                                               static_cast<float>(vpWidth),
                                               static_cast<float>(vpHeight),
                                               &polyline);

    if (polyline.count() == 0) {
        return true;
    }

    Tiles<kTileWidth, kTileHeight> tiler;
    tiler.makeTilesMSAA(polyline, vpWidth, vpHeight);
    tiler.sortTiles();

    if (tiler.getTiles().empty()) {
        return true;
    }

    SkTDArray<Strip> stripBuf;
    SkTDArray<uint8_t> alphaBuf;
    SkTDArray<skvx::int8> exactWindings;

    auto observer = [&](uint8_t exactMask, skvx::int8 winding) {
        exactWindings.push_back(winding);
    };

    MakeStrips::MsaaSimd(
            tiler, &stripBuf, &alphaBuf, localPath.getFillType(), polyline, maskLut, observer);

    OracleValidator<kTileWidth, kTileHeight> validator(
            localPath,
            stripBuf,
            alphaBuf,
            exactWindings,
            &polyline,
            &tiler,
            testName,
            OracleValidator<kTileWidth, kTileHeight>::Strictness::kNonStrict);

    uint32_t maxSampleDiff = 0;
    bool pass = validator.validate(reporter, vpWidth, vpHeight, &maxSampleDiff);
    if (minorErrorCount && maxSampleDiff > 0 && maxSampleDiff <= 3) {
        (*minorErrorCount)[maxSampleDiff - 1]++;
    }
    return pass;
}

template <uint16_t kTileWidth, uint16_t kTileHeight>
bool SkpValidator::ValidateSkp(skiatest::Reporter* reporter,
                               const char* filepath,
                               const SkTDArray<uint8_t>& maskLut) {
    auto paths = ExtractPaths(filepath);
    if (paths.empty()) {
        INFOF(reporter, "[SkpValidator] SKP file not found or empty: %s", filepath);
        return true;
    }

    int count = static_cast<int>(paths.size());
    std::array<uint32_t, 3> minorErrors = {0, 0, 0};
    int passed = 0;

    for (int i = 0; i < count; ++i) {
        SkString name;
        name.printf("%s_path_%d", filepath, i);
        if (ValidatePath<kTileWidth, kTileHeight>(
                    reporter, paths[i].devicePath, name.c_str(), maskLut, &minorErrors)) {
            passed++;
        }
    }

    INFOF(reporter,
          "[SIMD (%dx%d)] Tested %d / %d paths from '%s'. Passed: %d. Minor Errors: [1s:%u, "
          "2s:%u, 3s:%u]\n",
          kTileWidth,
          kTileHeight,
          count,
          static_cast<int>(paths.size()),
          filepath,
          passed,
          minorErrors[0],
          minorErrors[1],
          minorErrors[2]);

    return passed == count;
}

// Explicit template instantiations for SkpValidator
template bool SkpValidator::ValidatePath<4, 4>(skiatest::Reporter*,
                                               const SkPath&,
                                               const char*,
                                               const SkTDArray<uint8_t>&,
                                               std::array<uint32_t, 3>*);
template bool SkpValidator::ValidatePath<8, 8>(skiatest::Reporter*,
                                               const SkPath&,
                                               const char*,
                                               const SkTDArray<uint8_t>&,
                                               std::array<uint32_t, 3>*);
template bool SkpValidator::ValidateSkp<4, 4>(skiatest::Reporter*,
                                              const char*,
                                              const SkTDArray<uint8_t>&);
template bool SkpValidator::ValidateSkp<8, 8>(skiatest::Reporter*,
                                              const char*,
                                              const SkTDArray<uint8_t>&);

}  // namespace skgpu::graphite
