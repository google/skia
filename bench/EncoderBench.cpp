/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Resources.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkImageEncoder.h"

#include "sk_tool_utils.h"

class EncodeBench : public Benchmark {
public:
    EncodeBench(const char* filename, SkEncodedImageFormat type, int quality)
        : fFilename(filename)
        , fType(type)
        , fQuality(quality)
    {
        // Set the name of the bench
        SkString name("Encode_");
        name.append(filename);
        name.append("_");
        switch (type) {
            case SkEncodedImageFormat::kJPEG:
                name.append("JPEG");
                break;
            case SkEncodedImageFormat::kPNG:
                name.append("PNG");
                break;
            case SkEncodedImageFormat::kWEBP:
                name.append("WEBP");
                break;
            default:
                name.append("Unknown");
                break;
        }
        
        fName = name;
    }

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    
    const char* onGetName() override { return fName.c_str(); }
    
    void onPreDraw(SkCanvas*) override {
#ifdef SK_DEBUG
        bool result =
#endif
        GetResourceAsBitmap(fFilename, &fBitmap);
        SkASSERT(result);
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            sk_sp<SkData> data(sk_tool_utils::EncodeImageToData(fBitmap, fType, fQuality));
            SkASSERT(data);
        }
    }

private:
    const char*                fFilename;
    const SkEncodedImageFormat fType;
    const int                  fQuality;
    SkString                   fName;
    SkBitmap                   fBitmap;
};


// The Android Photos app uses a quality of 90 on JPEG encodes
DEF_BENCH(return new EncodeBench("mandrill_512.png", SkEncodedImageFormat::kJPEG, 90));
DEF_BENCH(return new EncodeBench("color_wheel.jpg", SkEncodedImageFormat::kJPEG, 90));

// PNG encodes are lossless so quality should be ignored
DEF_BENCH(return new EncodeBench("mandrill_512.png", SkEncodedImageFormat::kPNG, 90));
DEF_BENCH(return new EncodeBench("color_wheel.jpg", SkEncodedImageFormat::kPNG, 90));

// TODO: What is the appropriate quality to use to benchmark WEBP encodes?
DEF_BENCH(return new EncodeBench("mandrill_512.png", SkEncodedImageFormat::kWEBP, 90));
DEF_BENCH(return new EncodeBench("color_wheel.jpg", SkEncodedImageFormat::kWEBP, 90));
