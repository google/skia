/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ok is an experimental test harness, maybe to replace DM.  Key features:
//   * work is balanced across separate processes for stability and isolation;
//   * ok is entirely opt-in.  No more maintaining huge --blacklists.

#include "SkOSFile.h"
#include "SkSurface.h"
#include "gm.h"
#include <deque>
#include <functional>
#include <future>
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
    std::deque<std::future<Status>> live;

    bool spawn(std::function<Status(void)> fn) override {
        live.push_back(std::async(std::launch::async, fn));
        return true;
    }

    Status wait_one() override {
        if (live.empty()) {
            return Status::None;
        }
        Status s = live.front().get();
        live.pop_front();
        return s;
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

int main(int argc, char** argv) {
    int         jobs      {1};
    std::regex  match     {".*"};
    std::regex  search    {".*"};
    std::string write_dir {""};

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-j", argv[i])) { jobs      = atoi(argv[++i]); }
        if (0 == strcmp("-m", argv[i])) { match     =      argv[++i] ; }
        if (0 == strcmp("-s", argv[i])) { search    =      argv[++i] ; }
        if (0 == strcmp("-w", argv[i])) { write_dir =      argv[++i] ; }
        if (0 == strcmp("-h", argv[i])) {
            printf("%s [-j N] [-m regex] [-s regex] [-w dir] [-h] \n"
                   "   -j: Run at most N processes at any time.   \n"
                   "       If <0, use -N threads instead.         \n"
                   "       If 0, use one thread in one process.   \n"
                   "       If 1 (default) or -1, auto-detect N.   \n"
                   "   -m: Run only GMs matching regex exactly.   \n"
                   "   -s: Run only GMs matching regex anywhere.  \n"
                   "   -w: If set, write .pngs into dir.          \n"
                   "   -h: Print this message and exit.           \n", argv[0]);
            return 1;
        }
    }

    std::unique_ptr<Engine> engine;
    if (jobs == 0) { engine.reset(new SerialEngine);               }
    if (jobs  > 0) { engine.reset(new   ForkEngine);               }
    if (jobs  < 0) { engine.reset(new ThreadEngine); jobs = -jobs; }

    if (jobs == 1) { jobs = std::thread::hardware_concurrency(); }

    auto mkdir = [&](std::string dir) {
        if (!write_dir.empty()) {
            sk_mkdir((write_dir + dir).c_str());
        }
    };
    mkdir("/");
    mkdir("/gm-8888");

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

    for (auto r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        spawn([=] {
            std::unique_ptr<skiagm::GM> gm{r->factory()(nullptr)};

            std::string name = gm->getName();
            if (!std::regex_match (name, match) ||
                !std::regex_search(name, search)) {
                return Status::Skipped;
            }

            auto size = gm->getISize();
            auto surface = SkSurface::MakeRasterN32Premul(size.width(), size.height());
            auto canvas = surface->getCanvas();

            canvas->clear(0xffffffff);
            canvas->concat(gm->getInitialTransform());
            gm->draw(canvas);

            if (!write_dir.empty()) {
                auto image = surface->makeImageSnapshot();
                sk_sp<SkData> png{image->encode()};

                std::string path = write_dir + "/gm-8888" + "/" + name + ".png";
                SkFILEWStream(path.c_str()).write(png->data(), png->size());
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
