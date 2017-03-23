/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ok is an experimental test harness, maybe to replace DM.  Key features:
//   * work is balanced across separate processes for stability and isolation;
//   * ok is entirely opt-in.  No more maintaining huge --blacklists.

#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkSurface.h"
#include "gm.h"
#include <chrono>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <regex>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

enum class Status { OK, Failed, Crashed, Skipped, None };

struct Engine {
    virtual ~Engine() {}
    virtual bool spawn(std::function<Status(void)>) = 0;
    virtual Status wait_one() = 0;
};

struct SerialEngine : Engine {
    Status last = Status::None;

    bool spawn(std::function<Status(void)> fn) override {
        last = fn();
        return true;
    }

    Status wait_one() override {
        Status s = last;
        last = Status::None;
        return s;
    }
};

struct ThreadEngine : Engine {
    std::list<std::future<Status>> live;

    bool spawn(std::function<Status(void)> fn) override {
        live.push_back(std::async(std::launch::async, fn));
        return true;
    }

    Status wait_one() override {
        if (live.empty()) {
            return Status::None;
        }

        for (;;) {
            for (auto it = live.begin(); it != live.end(); it++) {
                if (it->wait_for(std::chrono::seconds::zero()) == std::future_status::ready) {
                    Status s = it->get();
                    live.erase(it);
                    return s;
                }
            }
        }
    }
};

#if defined(_MSC_VER)
    using ForkEngine = ThreadEngine;
#else
    #include <sys/wait.h>
    #include <unistd.h>

    struct ForkEngine : Engine {
        bool spawn(std::function<Status(void)> fn) override {
            switch (fork()) {
                case  0: exit((int)fn());
                case -1: return false;
                default: return true;
            }
        }

        Status wait_one() override {
            do {
                int status;
                if (wait(&status) > 0) {
                    return WIFEXITED(status) ? (Status)WEXITSTATUS(status)
                                             : Status::Crashed;
                }
            } while (errno == EINTR);
            return Status::None;
        }
    };
#endif

struct Src {
    virtual ~Src() {}
    virtual std::string name() = 0;
    virtual SkISize     size() = 0;
    virtual void draw(SkCanvas*) = 0;
};

struct Stream {
    virtual ~Stream() {}
    virtual std::unique_ptr<Src> next() = 0;
};

struct Options {
    std::map<std::string, std::string> kv;

    explicit Options(std::string str) {
        std::string k,v, *curr = &k;
        for (auto c : str) {
            switch(c) {
                case ',': kv[k] = v;
                          curr = &(k = "");
                          break;
                case '=': curr = &(v = "");
                          break;
                default: *curr += c;
            }
        }
        kv[k] = v;
    }

    std::string lookup(std::string k, std::string fallback) {
        for (auto it = kv.find(k); it != kv.end(); ) {
            return it->second;
        }
        return fallback;
    }
};

struct GMStream : Stream {
    const skiagm::GMRegistry* registry = skiagm::GMRegistry::Head();

    struct GMSrc : Src {
        skiagm::GM* (*factory)(void*);
        std::unique_ptr<skiagm::GM> gm;

        std::string name() override {
            gm.reset(factory(nullptr));
            return gm->getName();
        }

        SkISize size() override {
            return gm->getISize();
        }

        void draw(SkCanvas* canvas) override {
            canvas->clear(0xffffffff);
            canvas->concat(gm->getInitialTransform());
            gm->draw(canvas);
        }
    };

    std::unique_ptr<Src> next() override {
        if (!registry) {
            return nullptr;
        }
        GMSrc src;
        src.factory = registry->factory();
        registry = registry->next();
        return std::unique_ptr<Src>{new GMSrc{std::move(src)}};
    }
};

struct SKPStream : Stream {
    std::string dir;
    std::vector<std::string> skps;

    explicit SKPStream(Options options) : dir(options.lookup("dir", "skps")) {
        SkOSFile::Iter it{dir.c_str(), ".skp"};
        for (SkString path; it.next(&path); ) {
            skps.push_back(path.c_str());
        }
    }

    struct SKPSrc : Src {
        std::string dir, path;
        sk_sp<SkPicture> pic;

        std::string name() override {
            return path;
        }

        SkISize size() override {
            auto skp = SkData::MakeFromFileName((dir+"/"+path).c_str());
            pic = SkPicture::MakeFromData(skp.get());
            return pic->cullRect().roundOut().size();
        }

        void draw(SkCanvas* canvas) override {
            canvas->clear(0xffffffff);
            pic->playback(canvas);
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
        return std::unique_ptr<Src>{new SKPSrc{std::move(src)}};
    }
};

struct {
    const char* name;
    std::unique_ptr<Stream> (*factory)(Options);
} streams[] = {
    {"gm",  [](Options options) { return std::unique_ptr<Stream>{new GMStream}; }},
    {"skp", [](Options options) { return std::unique_ptr<Stream>{new SKPStream{options}}; }},
};

int main(int argc, char** argv) {
    SkGraphics::Init();

    int         jobs        {1};
    std::regex  match       {".*"};
    std::regex  search      {".*"};
    std::string write_dir   {""};

    std::unique_ptr<Stream> stream;

    auto help = [&] {
        std::string stream_types;
        for (auto st : streams) {
            if (!stream_types.empty()) {
                stream_types += ", ";
            }
            stream_types += st.name;
        }

        printf("%s [-j N] [-m regex] [-s regex] [-w dir] [-h] stream[:k=v,k=v,...]  \n"
                "     -j: Run at most N processes at any time.                      \n"
                "         If <0, use -N threads instead.                            \n"
                "         If 0, use one thread in one process.                      \n"
                "         If 1 (default) or -1, auto-detect N.                      \n"
                "     -m: Run only names matching regex exactly.                    \n"
                "     -s: Run only names matching regex anywhere.                   \n"
                "     -w: If set, write .pngs into dir.                             \n"
                "     -h: Print this message and exit.                              \n"
                " stream: content to draw:  %s                                      \n"
                "         Some streams have options, e.g. skp:dir=skps              \n",
                argv[0], stream_types.c_str());
        return 1;
    };

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-j", argv[i])) { jobs      = atoi(argv[++i]); }
        if (0 == strcmp("-m", argv[i])) { match     =      argv[++i] ; }
        if (0 == strcmp("-s", argv[i])) { search    =      argv[++i] ; }
        if (0 == strcmp("-w", argv[i])) { write_dir =      argv[++i] ; }
        if (0 == strcmp("-h", argv[i])) { return help(); }

        for (auto st : streams) {
            size_t len = strlen(st.name);
            if (0 == strncmp(st.name, argv[i], len)) {
                switch (argv[i][len]) {
                    case  ':': len++;
                    case '\0': stream = st.factory(Options{argv[i]+len});
                }
            }
        }
    }
    if (!stream) { return help(); }

    std::unique_ptr<Engine> engine;
    if (jobs == 0) { engine.reset(new SerialEngine);               }
    if (jobs  > 0) { engine.reset(new   ForkEngine);               }
    if (jobs  < 0) { engine.reset(new ThreadEngine); jobs = -jobs; }

    if (jobs == 1) { jobs = std::thread::hardware_concurrency(); }

    if (!write_dir.empty()) {
        sk_mkdir(write_dir.c_str());
    }

    int ok = 0, failed = 0, crashed = 0, skipped = 0;

    auto update_stats = [&](Status s) {
        switch (s) {
            case Status::OK:      ok++;      break;
            case Status::Failed:  failed++;  break;
            case Status::Crashed: crashed++; break;
            case Status::Skipped: skipped++; break;
            case Status::None:              return;
        }
        const char* leader = "\r";
        auto print = [&](int count, const char* label) {
            if (count) {
                printf("%s%d %s", leader, count, label);
                leader = ", ";
            }
        };
        print(ok,      "ok");
        print(failed,  "failed");
        print(crashed, "crashed");
        print(skipped, "skipped");
        fflush(stdout);
    };

    auto spawn = [&](std::function<Status(void)> fn) {
        if (--jobs < 0) {
            update_stats(engine->wait_one());
        }
        while (!engine->spawn(fn)) {
            update_stats(engine->wait_one());
        }
    };

    for (std::unique_ptr<Src> owned = stream->next(); owned; owned = stream->next()) {
        Src* raw = owned.release();  // Can't move std::unique_ptr into a lambda in C++11. :(
        spawn([=] {
            std::unique_ptr<Src> src{raw};

            auto name = src->name();
            if (!std::regex_match (name, match) ||
                !std::regex_search(name, search)) {
                return Status::Skipped;
            }

            auto size = src->size();
            auto surface = SkSurface::MakeRasterN32Premul(size.width(), size.height());
            src->draw(surface->getCanvas());

            if (!write_dir.empty()) {
                auto image = surface->makeImageSnapshot();
                sk_sp<SkData> png{image->encode()};

                std::string path = write_dir + "/" + name + ".png";
                SkFILEWStream{path.c_str()}.write(png->data(), png->size());
            }
            return Status::OK;
        });
    }

    for (Status s = Status::OK; s != Status::None; ) {
        s = engine->wait_one();
        update_stats(s);
    }
    printf("\n");
    return (failed || crashed) ? 1 : 0;
}
