/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include "gm.h"
#include "Resources.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkImage.h"
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
            if (!pic) {
                auto skp = SkData::MakeFromFileName((dir+"/"+path).c_str());
                pic = SkPicture::MakeFromData(skp.get());
            }
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

struct DecodeStream : Stream {
    std::string dir;
    SkImageInfo overrides;
    std::vector<std::string> images;

    static std::unique_ptr<Stream> Create(Options options) {
        DecodeStream stream;
        stream.dir = options("dir", "resources");

        const char* exts[] = { ".bmp", ".gif", ".ico", ".jpg", ".png", ".webp" };
        for (auto ext : exts) {
            SkOSFile::Iter it{stream.dir.c_str(), ext};
            for (SkString path; it.next(&path); ) {
                stream.images.push_back(path.c_str());
            }
        }

        std::string cs = options("cs");
        if (cs == "srgb") {
            stream.overrides.makeColorSpace(SkColorSpace::MakeSRGB());
        } else if (!cs.empty()) {
            auto resource = SkStringPrintf("icc_profiles/%s.icc", cs.c_str());
            auto profile = SkData::MakeFromFileName(GetResourcePath(resource.c_str()).c_str());
            stream.overrides.makeColorSpace(SkColorSpace::MakeICC(profile->data(),
                                                                  profile->size()));
        }

        return move_unique(stream);
    }

    struct DecodeSrc : Src {
        std::string dir, path;
        SkImageInfo overrides;
        std::unique_ptr<SkCodec> codec;

        void init() {
            if (!codec) {
                auto encoded = SkData::MakeFromFileName((dir+"/"+path).c_str());
                codec.reset(SkCodec::NewFromData(encoded));
            }
        }

        std::string name() override {
            return path;
        }

        SkISize size() override {
            this->init();
            return codec->getInfo().bounds().size();
        }

        Status draw(SkCanvas* canvas) override {
            this->init();

            SkCodec::Options options;

            SkImageInfo info = codec->getInfo();
            if (auto cs = overrides.refColorSpace()) {
                info = info.makeColorSpace(std::move(cs));
            }
            if (info.colorType() == kIndex_8_SkColorType) {
                // Index8 is on its way out.  Easier to decode to 8888 than set up color tables...
                info = info.makeColorType(kN32_SkColorType);
            }

            // We play a little trick here to help visualize decoding to different color spaces:
            // decode correctly, but tell the rest of the program the decoded image is sRGB.
            // Decodes into wide gamut will look duller, and narrow-gamut brighter.

            SkBitmap srgb;
            if (!srgb.tryAllocPixels(info.makeColorSpace(SkColorSpace::MakeSRGB())) ||
                SkCodec::kSuccess != codec->getPixels(info, srgb.getPixels(), srgb.rowBytes())) {
                return Status::Failed;
            }
            canvas->drawImage(SkImage::MakeFromBitmap(srgb), 0,0);
            return Status::OK;
        }
    };

    std::unique_ptr<Src> next() override {
        if (images.empty()) {
            return nullptr;
        }
        DecodeSrc src;
        src.dir  = dir;
        src.path = images.back();
        src.overrides = overrides;
        images.pop_back();
        return move_unique(src);
    }
};
static Register decode{"decode",
                       "decode images from dir=resources",
                       DecodeStream::Create};
