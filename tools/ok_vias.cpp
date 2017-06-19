/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage.h"
#include "SkOSFile.h"
#include "SkPictureRecorder.h"
#include "SkPngEncoder.h"
#include "ProcStats.h"
#include "Timer.h"
#include "ok.h"
#include <chrono>
#include <regex>

static std::unique_ptr<Src> proxy(Src* original, std::function<Status(SkCanvas*)> fn) {
    struct : Src {
        Src*                             original;
        std::function<Status(SkCanvas*)> fn;

        std::string name() override { return original->name(); }
        SkISize     size() override { return original->size(); }
        Status draw(SkCanvas* canvas) override { return fn(canvas); }
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

    Status draw(Src* src) override {
        SkRTreeFactory factory;
        SkPictureRecorder rec;
        rec.beginRecording(SkRect::MakeSize(SkSize::Make(src->size())),
                           rtree ? &factory : nullptr);

        for (auto status = src->draw(rec.getRecordingCanvas()); status != Status::OK; ) {
            return status;
        }
        auto pic = rec.finishRecordingAsPicture();

        return target->draw(proxy(src, [=](SkCanvas* canvas) {
            pic->playback(canvas);
            return Status::OK;
        }).get());
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register via_pic{"via_pic", "record then play back an SkPicture", ViaPic::Create};

struct Png : Dst {
    std::unique_ptr<Dst> target;
    std::string          dir;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Png via;
        via.target = std::move(dst);
        via.dir    = options("dir", "ok");
        return move_unique(via);
    }

    Status draw(Src* src) override {
        for (auto status = target->draw(src); status != Status::OK; ) {
            return status;
        }

        SkBitmap bm;
        SkPixmap pm;
        if (!target->image()->asLegacyBitmap(&bm, SkImage::kRO_LegacyBitmapMode) ||
            !bm.peekPixels(&pm)) {
            return Status::Failed;
        }

        sk_mkdir(dir.c_str());
        SkFILEWStream dst{(dir + "/" + src->name() + ".png").c_str()};

        SkPngEncoder::Options options;
        options.fFilterFlags      = SkPngEncoder::FilterFlag::kNone;
        options.fZLibLevel        = 1;
        options.fUnpremulBehavior = pm.colorSpace() ? SkTransferFunctionBehavior::kRespect
                                                    : SkTransferFunctionBehavior::kIgnore;
        return SkPngEncoder::Encode(&dst, pm, options) ? Status::OK
                                                       : Status::Failed;
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register png{"png", "dump PNGs to dir=ok" , Png::Create};

struct Filter : Dst {
    std::unique_ptr<Dst> target;
    std::regex match, search;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Filter via;
        via.target = std::move(dst);
        via.match  = options("match",  ".*");
        via.search = options("search", ".*");
        return move_unique(via);
    }

    Status draw(Src* src) override {
        auto name = src->name();
        if (!std::regex_match (name, match) ||
            !std::regex_search(name, search)) {
            return Status::Skipped;
        }
        return target->draw(src);
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register filter{"filter",
                       "run only srcs matching match=.* exactly and search=.* somewhere",
                       Filter::Create};

struct Time : Dst {
    std::unique_ptr<Dst> target;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Time via;
        via.target = std::move(dst);
        return move_unique(via);
    }

    Status draw(Src* src) override {
        auto start = std::chrono::steady_clock::now();
            Status status = target->draw(src);
        std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now()
                                                          - start;

        auto msg = HumanizeMs(elapsed.count());
        ok_log(msg.c_str());
        return status;
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register _time{"time", "print wall run time", Time::Create};

struct Memory : Dst {
    std::unique_ptr<Dst> target;

    static std::unique_ptr<Dst> Create(Options options, std::unique_ptr<Dst> dst) {
        Memory via;
        via.target = std::move(dst);
        return move_unique(via);
    }

    Status draw(Src* src) override {
        Status status = target->draw(src);

        auto msg = SkStringPrintf("%dMB", sk_tools::getMaxResidentSetSizeMB());
        ok_log(msg.c_str());

        return status;
    }

    sk_sp<SkImage> image() override {
        return target->image();
    }
};
static Register memory{"memory", "print process maximum memory usage", Memory::Create};
