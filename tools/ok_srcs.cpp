/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include "gm.h"
#include "SkData.h"
#include "SkOSFile.h"
#include "SkPicture.h"
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
            canvas->concat(gm->getInitialTransform());
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
