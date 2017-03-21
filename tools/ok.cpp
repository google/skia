/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// ok is an experimental test harness, maybe to replace DM.  Key features:
//   * work is balanced across separate processes for stability and isolation;
//   * ok is entirely opt-in.  No more maintaining huge --blacklists.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "SkSurface.h"
#include "gm.h"

static int good = 0, bad = 0, ugly = 0;

static void update_stats(int status) {
    if (WIFEXITED(status)) {
        (WEXITSTATUS(status) == 0 ? good : bad)++;
    } else {
        ugly++;
    }
}

// wait(), with retries when interrupted by signals.
static bool safe_wait() {
    do {
        int status;
        pid_t pid = wait(&status);
        if (pid > 0) {
            update_stats(status);
            return true;
        }
    } while (errno == EINTR);
    return false;
}

// fork() and call fn(), throttled by jobs.
template <typename Fn>
static void spawn_job(int* jobs, Fn&& fn) {
    if (--(*jobs) < 0) {
        safe_wait();
    }
    if (!fork()) {
        exit(fn());
    }
}

int main(int argc, char** argv) {
    int jobs = argc > 1 ? atoi(argv[1])
                        : std::thread::hardware_concurrency();

    for (auto r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        spawn_job(&jobs, [r] {
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

    while (safe_wait());

    if (ugly || bad) {
        printf("%d good, %d bad, %d ugly\n", good, bad, ugly);
        return 1;
    }
    return 0;
}
