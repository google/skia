/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include "SkPictureRecorder.h"

struct ViaPic : Dst {
    std::unique_ptr<Dst> target;
    SkPictureRecorder    rec;

    static std::unique_ptr<Dst> Create(Options options, SkISize size, std::unique_ptr<Dst> dst) {
        SkBBHFactory* bbh = nullptr;
        SkRTreeFactory rtree;

        if (options("bbh") == "rtree") { bbh = &rtree; }

        auto via = std::unique_ptr<ViaPic>(new ViaPic);
        via->target = std::move(dst);
        via->rec.beginRecording(SkRect::MakeSize(SkSize::Make(size)), bbh);
        return std::move(via);
    }

    SkCanvas* canvas() override {
        return rec.getRecordingCanvas();
    }

    void write(std::string path_prefix) override {
        auto pic = rec.finishRecordingAsPicture();
        pic->playback(target->canvas());
        target->write(path_prefix);
    }
};
static Register via_pic{"via_pic", ViaPic::Create};
