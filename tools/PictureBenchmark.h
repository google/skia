/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PictureBenchmark_DEFINED
#define PictureBenchmark_DEFINED
#include "SkTypes.h"
#include "SkRefCnt.h"
#include "PictureRenderer.h"

class BenchTimer;
class SkBenchLogger;
class SkPicture;
class SkString;

namespace sk_tools {

class PictureBenchmark : public SkRefCnt {
public:
    PictureBenchmark()
    : fRepeats(1)
    , fLogger(NULL) {}

    void run(SkPicture* pict);

    void setRepeats(int repeats) {
        fRepeats = repeats;
    }

    void setDeviceType(PictureRenderer::SkDeviceTypes deviceType) {
        sk_tools::PictureRenderer* renderer = getRenderer();

        if (renderer != NULL) {
            renderer->setDeviceType(deviceType);
        }
    }

    void setLogger(SkBenchLogger* logger) { fLogger = logger; }

private:
    int            fRepeats;
    SkBenchLogger* fLogger;

    void logProgress(const char msg[]);

    virtual sk_tools::PictureRenderer* getRenderer() = 0;

    BenchTimer* setupTimer();

    typedef SkRefCnt INHERITED;
};

// TODO: Use just one PictureBenchmark with different renderers.

class PipePictureBenchmark : public PictureBenchmark {
private:
    PipePictureRenderer fRenderer;

    virtual sk_tools::PictureRenderer* getRenderer() SK_OVERRIDE {
        return &fRenderer;
    }

    typedef PictureBenchmark INHERITED;
};

class RecordPictureBenchmark : public PictureBenchmark {
private:
    RecordPictureRenderer fRenderer;

    virtual sk_tools::PictureRenderer* getRenderer() SK_OVERRIDE {
        return &fRenderer;
    }

    typedef PictureBenchmark INHERITED;
};

class SimplePictureBenchmark : public PictureBenchmark {
private:
    SimplePictureRenderer fRenderer;

    virtual sk_tools::PictureRenderer* getRenderer() SK_OVERRIDE {
        return &fRenderer;
    }

    typedef PictureBenchmark INHERITED;
};

class TiledPictureBenchmark : public PictureBenchmark {
public:
    void setTileWidth(int width) {
        fRenderer.setTileWidth(width);
    }

    int getTileWidth() const {
        return fRenderer.getTileWidth();
    }

    void setTileHeight(int height) {
        fRenderer.setTileHeight(height);
    }

    int getTileHeight() const {
        return fRenderer.getTileHeight();
    }

    void setTileWidthPercentage(double percentage) {
        fRenderer.setTileWidthPercentage(percentage);
    }

    double getTileWidthPercentage() const {
        return fRenderer.getTileWidthPercentage();
    }

    void setTileHeightPercentage(double percentage) {
        fRenderer.setTileHeightPercentage(percentage);
    }

    double getTileHeightPercentage() const {
        return fRenderer.getTileHeightPercentage();
    }

    void setTileMinPowerOf2Width(int width) {
        fRenderer.setTileMinPowerOf2Width(width);
    }

    int getTileMinPowerOf2Width() {
        return fRenderer.getTileMinPowerOf2Width();
    }

    void setThreading(bool multi) {
        fRenderer.setMultiThreaded(multi);
    }

    void setUsePipe(bool usePipe) {
        fRenderer.setUsePipe(usePipe);
    }

private:
    TiledPictureRenderer fRenderer;

    virtual sk_tools::PictureRenderer* getRenderer() SK_OVERRIDE{
        return &fRenderer;
    }

    typedef PictureBenchmark INHERITED;
};

class PlaybackCreationBenchmark : public PictureBenchmark {
private:
    PlaybackCreationRenderer fRenderer;

    virtual sk_tools::PictureRenderer* getRenderer() SK_OVERRIDE{
        return &fRenderer;
    }

    typedef PictureBenchmark INHERITED;
};

}

#endif  // PictureBenchmark_DEFINED
