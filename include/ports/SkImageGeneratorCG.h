/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "../codec/SkEncodedOrigin.h"
#include "../core/SkData.h"
#include "../core/SkImageGenerator.h"
#include "../private/SkTemplates.h"
#include "../utils/mac/SkCGUtils.h"

class SK_API SkImageGeneratorCG : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> MakeFromEncodedCG(sk_sp<SkData>);

protected:
    sk_sp<SkData> onRefEncodedData() override;

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options&)
    override;

private:
    /*
     * Takes ownership of the imageSrc
     */
    SkImageGeneratorCG(const SkImageInfo& info, const void* imageSrc, sk_sp<SkData> data,
                       SkEncodedOrigin origin);

    SkAutoTCallVProc<const void, CFRelease> fImageSrc;
    sk_sp<SkData>                           fData;
    const SkEncodedOrigin                   fOrigin;

    typedef SkImageGenerator INHERITED;
};

#endif //defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
