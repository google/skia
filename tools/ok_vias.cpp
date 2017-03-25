/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include "SkPictureRecorder.h"

static std::unique_ptr<Src> proxy(Src* original, std::function<bool(SkCanvas*)> fn) {
    struct : Src {
        Src*                           original;
        std::function<bool(SkCanvas*)> fn;

        std::string name() override { return original->name(); }
        SkISize     size() override { return original->size(); }
        bool draw(SkCanvas* canvas) override { return fn(canvas); }
    } src;
    src.original = original;
    src.fn       = fn;
    return move_unique(src);
}

struct ViaPic : Dst {
    std::unique_ptr<Dst> target;
    bool                 rtree = false;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        ViaPic via;
        via.target = std::move(dst);
        if (options("bbh") == "rtree") { via.rtree = true; }
        return move_unique(via);
    }

    bool draw(Src* src) override {
        SkRTreeFactory factory;
        SkPictureRecorder rec;
        rec.beginRecording(SkRect::MakeSize(SkSize::Make(src->size())),
                           rtree ? &factory : nullptr);

        if (!src->draw(rec.getRecordingCanvas())) {
            return false;
        }
        auto pic = rec.finishRecordingAsPicture();

        return target->draw(proxy(src, [=](SkCanvas* canvas) {
            pic->playback(canvas);
            return true;
        }).get());
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register via_pic{"via_pic", ViaPic::Create};
