/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkData.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "Timer.h"
#include "gm.h"
#include "ok.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include <stdlib.h>
#include <vector>

struct GMStream : Stream {
    const skiagm::GMRegistry* registry = skiagm::GMRegistry::Head();

    static std::unique_ptr<Stream> Create(Options) {
        GMStream stream;
        return move_unique(stream);
    }

    struct GMSrc : Src {
        skiagm::GM* (*factory)(void*);
        std::unique_ptr<skiagm::GM> gm;

        void init() {
            if (gm) { return; }
            gm.reset(factory(nullptr));
        }

        std::string name() override {
            this->init();
            return gm->getName();
        }

        SkISize size() override {
            this->init();
            return gm->getISize();
        }

        Status draw(SkCanvas* canvas) override {
            this->init();
            canvas->clear(0xffffffff);
            gm->draw(canvas);
            return Status::OK;
        }
    };

    std::unique_ptr<Src> next() override {
        if (!registry) {
            return nullptr;
        }
        GMSrc src;
        src.factory = registry->factory();
        registry = registry->next();
        return move_unique(src);
    }
};
static Register gm{"gm", "draw GMs linked into this binary", GMStream::Create};

struct SKPStream : Stream {
    std::string dir;
    std::vector<std::string> skps;

    static std::unique_ptr<Stream> Create(Options options) {
        SKPStream stream;
        stream.dir = options("dir", "skps");
        SkOSFile::Iter it{stream.dir.c_str(), ".skp"};
        for (SkString path; it.next(&path); ) {
            stream.skps.push_back(path.c_str());
        }
        return move_unique(stream);
    }

    struct SKPSrc : Src {
        std::string dir, path;
        sk_sp<SkPicture> pic;

        void init() {
            if (pic) { return; }
            auto skp = SkData::MakeFromFileName((dir+"/"+path).c_str());
            pic = SkPicture::MakeFromData(skp.get());
        }

        std::string name() override {
            return path;
        }

        SkISize size() override {
            this->init();
            return pic->cullRect().roundOut().size();
        }

        Status draw(SkCanvas* canvas) override {
            this->init();
            canvas->clear(0xffffffff);
            pic->playback(canvas);
            return Status::OK;
        }
    };

    std::unique_ptr<Src> next() override {
        if (skps.empty()) {
            return nullptr;
        }
        SKPSrc src;
        src.dir  = dir;
        src.path = skps.back();
        skps.pop_back();
        return move_unique(src);
    }
};
static Register skp{"skp", "draw SKPs from dir=skps", SKPStream::Create};

struct BenchStream : Stream {
    const BenchRegistry* registry = BenchRegistry::Head();
    int samples;

    static std::unique_ptr<Stream> Create(Options options) {
        BenchStream stream;
        stream.samples = std::max(1, atoi(options("samples", "20").c_str()));
        return move_unique(stream);
    }

    struct BenchSrc : Src {
        Benchmark* (*factory)(void*);
        std::unique_ptr<Benchmark> bench;
        int samples;

        void init() {
            if (bench) { return; }
            bench.reset(factory(nullptr));
        }

        std::string name() override {
            this->init();
            return bench->getName();
        }

        SkISize size() override {
            this->init();
            return { bench->getSize().x(), bench->getSize().y() };
        }

        Status draw(SkCanvas* canvas) override {
            this->init();

            using ms = std::chrono::duration<double, std::milli>;
            std::vector<ms> sample(samples);

            bench->delayedSetup();
            if (canvas) {
                bench->perCanvasPreDraw(canvas);
            }
            for (int i = 0; i < samples; i++) {
                using clock = std::chrono::high_resolution_clock;
                for (int loops = 1; loops < 1000000000; loops *= 2) {
                    bench->preDraw(canvas);
                    auto start = clock::now();
                        bench->draw(loops, canvas);
                    ms elapsed = clock::now() - start;
                    bench->postDraw(canvas);

                    if (elapsed.count() < 10) {
                        continue;
                    }

                    sample[i] = elapsed / loops;
                    break;
                }
            }
            if (canvas) {
                bench->perCanvasPostDraw(canvas);
            }

            std::sort(sample.begin(), sample.end());

            SkString msg = SkStringPrintf("%s\t@0", HumanizeMs(sample[0].count()).c_str());
            if (samples > 2) {
                msg.appendf("\t%s\t@%g", HumanizeMs(sample[samples-2].count()).c_str()
                                       , 100.0*(samples-1) / samples);
            }
            if (samples > 1) {
                msg.appendf("\t%s\t@100", HumanizeMs(sample[samples-1].count()).c_str());
            }
            ok_log(msg.c_str());

            return Status::OK;
        }
    };

    std::unique_ptr<Src> next() override {
        if (!registry) {
            return nullptr;
        }
        BenchSrc src;
        src.factory = registry->factory();
        src.samples = samples;
        registry = registry->next();
        return move_unique(src);
    }
};
static Register bench{
    "bench",
    "time benchmarks linked into this binary samples=20 times each",
    BenchStream::Create,
};
