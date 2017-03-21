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
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "SkSurface.h"
#include "gm.h"

struct Engine {
    virtual ~Engine() {}
    virtual void spawn(std::function<int(void)>) = 0;
    virtual int wait_one() = 0;
    enum { kNothing = -1, kCrashed = -2 };
};

struct SerialEngine : Engine {
    int last = Engine::kNothing;

    void spawn(std::function<int(void)> fn) override {
        last = fn();
    }

    int wait_one() override {
        int rc = last;
        last = Engine::kNothing;
        return rc;
    }
};

struct ThreadEngine : Engine {
    std::deque<std::future<int>> live;

    void spawn(std::function<int(void)> fn) override {
        live.push_back(std::async(std::launch::async, fn));
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

struct ForkEngine : Engine {
    void spawn(std::function<int(void)> fn) override {
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Can't fork another process.\n");
            abort();
        }
        if (pid == 0) {
            exit(fn());
        }
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

int main(int argc, char** argv) {
    int jobs = argc > 1 ? atoi(argv[1])
                        : std::thread::hardware_concurrency();

    std::unique_ptr<Engine> engine;
    if (jobs == 0) { engine.reset(new SerialEngine);               }
    if (jobs  > 0) { engine.reset(new   ForkEngine);               }
    if (jobs  < 0) { engine.reset(new ThreadEngine); jobs = -jobs; }

    int good = 0, bad = 0, ugly = 0;

    auto spawn = [&](std::function<int(void)> fn) {
        if (--jobs < 0) {
            switch(engine->wait_one()) {
                case 0:                 good++; break;
                default:                 bad++; break;
                case Engine::kCrashed:  ugly++; break;
                case Engine::kNothing:          break;
            }
        }
        engine->spawn(fn);
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

    for (bool done = false; !done; ) {
        switch (engine->wait_one()) {
            case 0:                     good++; break;
            default:                     bad++; break;
            case Engine::kCrashed:      ugly++; break;
            case Engine::kNothing: done = true; break;
        }
    }

    printf("%d good, %d bad, %d ugly\n", good, bad, ugly);
    return (ugly || bad) ? 1 : 0;
}
