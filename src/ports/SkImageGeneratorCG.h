/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "SkCGUtils.h"
#include "SkData.h"
#include "SkEncodedOrigin.h"
#include "SkImageGenerator.h"
#include "SkTemplates.h"

class SkImageGeneratorCG : public SkImageGenerator {
public:
#ifdef SK_LEGACY_NEW_FROM_ENCODED_CG
    /*
     * Refs the data if an image generator can be returned.  Otherwise does
     * not affect the data.
     */
    static SkImageGenerator* NewFromEncodedCG(SkData* data);
#endif

    static std::unique_ptr<SkImageGenerator> MakeFromEncodedCG(sk_sp<SkData>);

protected:
    SkData* onRefEncodedData() override;

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
