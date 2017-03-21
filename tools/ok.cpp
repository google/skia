/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ok is an experimental test harness, maybe to replace DM.  Key features:
//   * work is balanced across separate processes for stability and isolation;
//   * ok is entirely opt-in.  No more maintaining huge --blacklists.

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

enum class Status { Good, Bad, Crashed, Skipped, None };

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
            pid_t pid = fork();
            if (pid == 0) {
                exit((int)fn());
            }
            return pid > 0;
        }

        Status wait_one() override {
            do {
                int status;
                pid_t pid = wait(&status);
                if (pid > 0) {
                    return WIFEXITED(status) ? (Status)WEXITSTATUS(status)
                                             : Status::Crashed;
                }
            } while (errno == EINTR);
            return Status::None;
        }
    };
#endif

int main(int argc, char** argv) {
    int jobs = 1;
    std::regex match(".*"), search(".*");

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-j", argv[i])) { jobs   = atoi(argv[++i]); }
        if (0 == strcmp("-m", argv[i])) { match  =      argv[++i] ; }
        if (0 == strcmp("-s", argv[i])) { search =      argv[++i] ; }
        if (0 == strcmp("-h", argv[i])) {
            printf("%s [-j n] [-m regex] [-s regex] [-h]             \n"
                   "   -j: Run at most this many parallel processes. \n"
                   "       If <0, use threads instead.               \n"
                   "       If  0, use one thread in one process.     \n"
                   "       If 1 (default) or -1, auto-detect.        \n"
                   "   -m: Run only GMs matching exactly this regex. \n"
                   "   -s: Run only GMs matching this regex anywhere.\n"
                   "   -h: Print this message and exit.              \n", argv[0]);
            return 1;
        }
    }

    std::unique_ptr<Engine> engine;
    if (jobs == 0) { engine.reset(new SerialEngine);               }
    if (jobs  > 0) { engine.reset(new   ForkEngine);               }
    if (jobs  < 0) { engine.reset(new ThreadEngine); jobs = -jobs; }

    if (jobs == 1) { jobs = std::thread::hardware_concurrency(); }

    int good = 0, bad = 0, skipped = 0, crashed = 0;

    auto update_stats = [&](Status s) {
        switch (s) {
            case Status::Good:    good++;    break;
            case Status::Bad:     bad++;     break;
            case Status::Crashed: crashed++; break;
            case Status::Skipped: skipped++; break;
            case Status::None:               break;
        }
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
        spawn([r,&match,&search] {
            std::unique_ptr<skiagm::GM> gm{r->factory()(nullptr)};
            auto size = gm->getISize();

            if (!std::regex_match (gm->getName(), match) ||
                !std::regex_search(gm->getName(), search)) {
                return Status::Skipped;
            }

            auto surface = SkSurface::MakeRasterN32Premul(size.width(), size.height());
            auto canvas = surface->getCanvas();

            canvas->clear(0xffffffff);
            canvas->concat(gm->getInitialTransform());
            gm->draw(canvas);

            return Status::Good;
        });
    }

    for (Status s = Status::Good; s != Status::None; ) {
        s = engine->wait_one();
        update_stats(s);
    }

    printf("%d good, %d bad, %d skipped, %d crashed\n", good, bad, skipped, crashed);
    return (bad || crashed) ? 1 : 0;
}
