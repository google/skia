/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/GpuToolUtils.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/core/SkTiledImageUtils.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkLRUCache.h"
#endif


namespace ToolUtils {

sk_sp<SkImage> MakeTextureImage(SkCanvas* canvas, sk_sp<SkImage> orig) {
    if (!orig) {
        return nullptr;
    }

#if defined(SK_GANESH)
    if (canvas->recordingContext() && canvas->recordingContext()->asDirectContext()) {
        GrDirectContext* dContext = canvas->recordingContext()->asDirectContext();
        const GrCaps* caps = dContext->priv().caps();

        if (orig->width() >= caps->maxTextureSize() || orig->height() >= caps->maxTextureSize()) {
            // Ganesh is able to tile large SkImage draws. Always forcing SkImages to be uploaded
            // prevents this feature from being tested by our tools. For now, leave excessively
            // large SkImages as bitmaps.
            return orig;
        }

        return SkImages::TextureFromImage(dContext, orig);
    }
#endif
#if defined(SK_GRAPHITE)
    if (canvas->recorder()) {
        return SkImages::TextureFromImage(canvas->recorder(), orig, {false});
    }
#endif
    return orig;
}

#if defined(SK_GRAPHITE)

// Currently, we give each new Recorder its own ImageProvider. This means we don't have to deal
// w/ any threading issues.
// TODO: We should probably have this class generate and report some cache stats
// TODO: Hook up to listener system?
// TODO: add testing of a single ImageProvider passed to multiple recorders
class TestingImageProvider : public skgpu::graphite::ImageProvider {
public:
    TestingImageProvider() : fCache(kDefaultNumCachedImages) {}
    ~TestingImageProvider() override {}

    sk_sp<SkImage> findOrCreate(skgpu::graphite::Recorder* recorder,
                                const SkImage* image,
                                SkImage::RequiredProperties requiredProps) override {
        if (!requiredProps.fMipmapped) {
            // If no mipmaps are required, check to see if we have a mipmapped version anyway -
            // since it can be used in that case.
            // TODO: we could get fancy and, if ever a mipmapped key eclipsed a non-mipmapped
            // key, we could remove the hidden non-mipmapped key/image from the cache.
            ImageKey mipMappedKey(image, /* mipmapped= */ true);
            auto result = fCache.find(mipMappedKey);
            if (result) {
                return *result;
            }
        }

        ImageKey key(image, requiredProps.fMipmapped);

        auto result = fCache.find(key);
        if (result) {
            return *result;
        }

        sk_sp<SkImage> newImage = SkImages::TextureFromImage(recorder, image, requiredProps);
        if (!newImage) {
            return nullptr;
        }

        result = fCache.insert(key, std::move(newImage));
        SkASSERT(result);

        return *result;
    }

private:
    static constexpr int kDefaultNumCachedImages = 256;

    class ImageKey {
    public:
        ImageKey(const SkImage* image, bool mipmapped) {
            uint32_t flags = mipmapped ? 0x1 : 0x0;
            SkTiledImageUtils::GetImageKeyValues(image, &fValues[1]);
            fValues[kNumValues-1] = flags;
            fValues[0] = SkChecksum::Hash32(&fValues[1], (kNumValues-1) * sizeof(uint32_t));
        }

        uint32_t hash() const { return fValues[0]; }

        bool operator==(const ImageKey& other) const {
            for (int i = 0; i < kNumValues; ++i) {
                if (fValues[i] != other.fValues[i]) {
                    return false;
                }
            }

            return true;
        }
        bool operator!=(const ImageKey& other) const { return !(*this == other); }

    private:
        static const int kNumValues = SkTiledImageUtils::kNumImageKeyValues + 2;

        uint32_t fValues[kNumValues];
    };

    struct ImageHash {
        size_t operator()(const ImageKey& key) const { return key.hash(); }
    };

    SkLRUCache<ImageKey, sk_sp<SkImage>, ImageHash> fCache;
};

skgpu::graphite::RecorderOptions CreateTestingRecorderOptions() {
    skgpu::graphite::RecorderOptions options;

    options.fImageProvider.reset(new TestingImageProvider);

    return options;
}

#endif // SK_GRAPHITE

}  // namespace ToolUtils
