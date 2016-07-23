/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if defined(SK_BUILD_FOR_WIN)

#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkTemplates.h"
#include "SkTScopedComPtr.h"

#include <wincodec.h>

/*
 * Any Windows program that uses COM must initialize the COM library by calling
 * the CoInitializeEx function.  In addition, each thread that uses a COM
 * interface must make a separate call to this function.
 *
 * For every successful call to CoInitializeEx, the thread must call
 * CoUninitialize before it exits.
 *
 * SkImageGeneratorWIC requires the COM library and leaves it to the client to
 * initialize COM for their application.
 *
 * For more information on initializing COM, please see:
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ff485844.aspx
 */
class SkImageGeneratorWIC : public SkImageGenerator {
public:
    /*
     * Refs the data if an image generator can be returned.  Otherwise does
     * not affect the data.
     */
    static SkImageGenerator* NewFromEncodedWIC(SkData* data);

protected:
    SkData* onRefEncodedData(SK_REFENCODEDDATA_CTXPARAM) override;

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
            int* ctableCount) override;

private:
    /*
     * Takes ownership of the imagingFactory
     * Takes ownership of the imageSource
     * Refs the data
     */
    SkImageGeneratorWIC(const SkImageInfo& info, IWICImagingFactory* imagingFactory,
            IWICBitmapSource* imageSource, SkData* data);

    SkTScopedComPtr<IWICImagingFactory> fImagingFactory;
    SkTScopedComPtr<IWICBitmapSource>   fImageSource;
    SkAutoTUnref<SkData>                fData;

    typedef SkImageGenerator INHERITED;
};

#endif // SK_BUILD_FOR_WIN
