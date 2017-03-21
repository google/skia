/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ok is an experimental test harness, maybe to replace DM.  Key features:
//   * work is balanced across separate processes for stability and isolation;
//   * ok is entirely opt-in.  No more maintaining huge --blacklists.

#include <deque>
#include <functional>
#include <future>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include "SkSurface.h"
#include "gm.h"

struct Engine {
    virtual ~Engine() {}
    virtual bool spawn(std::function<int(void)>) = 0;
    virtual int wait_one() = 0;
    enum { kNothing = -1, kCrashed = -2 };
};

struct SerialEngine : Engine {
    int last = Engine::kNothing;

    bool spawn(std::function<int(void)> fn) override {
        last = fn();
        return true;
    }

    int wait_one() override {
        int rc = last;
        last = Engine::kNothing;
        return rc;
    }
};

struct ThreadEngine : Engine {
    std::deque<std::future<int>> live;

    bool spawn(std::function<int(void)> fn) override {
        live.push_back(std::async(std::launch::async, fn));
        return true;
    }

    int wait_one() override {
        if (live.empty()) {
            return Engine::kNothing;
        }
        int rc = live.front().get();
        live.pop_front();
        return rc;
    }
};

#if defined(_MSC_VER)
    using ForkEngine = ThreadEngine;
#else
    #include <sys/wait.h>
    #include <unistd.h>

    struct ForkEngine : Engine {
        bool spawn(std::function<int(void)> fn) override {
            pid_t pid = fork();
            if (pid == 0) {
                exit(fn());
            }
            return pid > 0;
        }

        int wait_one() override {
            do {
                int status;
                pid_t pid = wait(&status);
                if (pid > 0) {
                    return WIFEXITED(status) ? WEXITSTATUS(status)
                                             : Engine::kCrashed;
                }
            } while (errno == EINTR);
            return Engine::kNothing;
        }
    };
#endif

int main(int argc, char** argv) {
    int jobs = argc > 1 ? atoi(argv[1]) : 1;

    std::unique_ptr<Engine> engine;
    if (jobs == 0) { engine.reset(new SerialEngine);               }
    if (jobs  > 0) { engine.reset(new   ForkEngine);               }
    if (jobs  < 0) { engine.reset(new ThreadEngine); jobs = -jobs; }

    if (jobs == 1) { jobs = std::thread::hardware_concurrency(); }

    int good = 0, bad = 0, ugly = 0;

    auto update_stats = [&](int rc) {
        switch (rc) {
            case 0:                 good++; break;
            default:                 bad++; break;
            case Engine::kCrashed:  ugly++; break;
            case Engine::kNothing:          break;
        }
    };

    auto spawn = [&](std::function<int(void)> fn) {
        if (--jobs < 0) {
            update_stats(engine->wait_one());
        }
        while (!engine->spawn(fn)) {
            update_stats(engine->wait_one());
        }
    };

    for (auto r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        spawn([r] {
            std::unique_ptr<skiagm::GM> gm{r->factory()(nullptr)};
            auto size = gm->getISize();

            auto surface = SkSurface::MakeRasterN32Premul(size.width(), size.height());
            auto canvas = surface->getCanvas();

            canvas->clear(0xffffffff);
            canvas->concat(gm->getInitialTransform());
            gm->draw(canvas);

            return 0;
        });
    }

    for (int rc = 0; rc != Engine::kNothing; ) {
        rc = engine->wait_one();
        update_stats(rc);
    }

    printf("%d good, %d bad, %d ugly\n", good, bad, ugly);
    return (ugly || bad) ? 1 : 0;
}
