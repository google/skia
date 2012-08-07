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

class SkPicture;
class SkString;

namespace sk_tools {

class PictureBenchmark : public SkRefCnt {
public:
    virtual void run(SkPicture* pict) = 0;

    void setRepeats(int repeats) {
        fRepeats = repeats;
    }

    int getRepeats() const {
        return fRepeats;
    }

protected:
    int fRepeats;

private:
    typedef SkRefCnt INHERITED;
};

class PipePictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;
private:
    PipePictureRenderer renderer;
    typedef PictureBenchmark INHERITED;
};

class RecordPictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;
private:
    typedef PictureBenchmark INHERITED;
};

class SimplePictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;
private:
    SimplePictureRenderer renderer;
    typedef PictureBenchmark INHERITED;
};

class TiledPictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;

    void setTileWidth(int width) {
        renderer.setTileWidth(width);
    }

    int getTileWidth() const {
        return renderer.getTileWidth();
    }

    void setTileHeight(int height) {
        renderer.setTileHeight(height);
    }

    int getTileHeight() const {
        return renderer.getTileHeight();
    }

    void setTileWidthPercentage(double percentage) {
        renderer.setTileWidthPercentage(percentage);
    }

    double getTileWidthPercentage() const {
        return renderer.getTileWidthPercentage();
    }

    void setTileHeightPercentage(double percentage) {
        renderer.setTileHeightPercentage(percentage);
    }

    double getTileHeightPercentage() const {
        return renderer.getTileHeightPercentage();
    }

private:
    TiledPictureRenderer renderer;
    typedef PictureBenchmark INHERITED;
};

class UnflattenPictureBenchmark : public PictureBenchmark {
public:
    virtual void run(SkPicture* pict) SK_OVERRIDE;
private:
    typedef PictureBenchmark INHERITED;
};

}

#endif  // PictureBenchmark_DEFINED
