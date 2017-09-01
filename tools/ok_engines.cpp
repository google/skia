/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include <stdlib.h>

struct SerialEngine : Engine {
    static std::unique_ptr<Engine> Factory(Options) {
        SerialEngine engine;
        return move_unique(engine);
    }

    bool crashproof() override { return false; }

    std::future<Status> spawn(std::function<Status(void)> fn) override {
        return std::async(std::launch::deferred, fn);
    }
};
static Register serial("serial",
                       "Run tasks serially on the main thread of a single process.",
                       SerialEngine::Factory);

struct ThreadEngine : Engine {
    static std::unique_ptr<Engine> Factory(Options) {
        ThreadEngine engine;
        return move_unique(engine);
    }

    bool crashproof() override { return false; }

    std::future<Status> spawn(std::function<Status(void)> fn) override {
        return std::async(std::launch::async, fn);
    }
};
static Register thread("thread",
                       "Run each task on its own thread of a single process.",
                       ThreadEngine::Factory);

#if !defined(_MSC_VER)
    #include <sys/wait.h>
    #include <unistd.h>

    struct ForkEngine : Engine {
        int limit;      // How many concurrent subprocesses do we allow to run at max?
        int alive = 0;  // How many concurrent subprocesses do we have running right now?

        static std::unique_ptr<Engine> Factory(Options options) {
            ForkEngine engine;
            engine.limit = atoi(options("limit", "0").c_str());
            if (engine.limit < 1) {
                engine.limit = std::thread::hardware_concurrency();
            }
            return move_unique(engine);
        }

        bool crashproof() override { return true; }

        std::future<Status> spawn(std::function<Status(void)> fn) override {
            if (alive == limit) {
                // The caller will wait for a child process to finish then try again.
                return std::future<Status>();
            }

            switch (fork()) {
                case  0:
                    // We are the spawned child process.
                    // Run fn() and exit() with its Status as our return code.
                    _exit((int)fn());

                case -1:
                    // The OS won't let us fork() another process right now.
                    // We'll need to wait for at least one live task to finish and try again.
                    return std::future<Status>();

                default:
                    // We succesfully spawned a child process!
                    // This will wait for any spawned process to finish and return its Status.
                    alive++;
                    return std::async(std::launch::deferred, [&] {
                        do {
                            int status;
                            if (wait(&status) > 0) {
                                alive--;
                                return WIFEXITED(status) ? (Status)WEXITSTATUS(status)
                                                         : Status::Crashed;
                            }
                        } while (errno == EINTR);
                        return Status::None;
                    });
            }
        }
    };
    static Register _fork("fork",
                          "Run each task in an independent process with fork(), limit=ncpus.",
                          ForkEngine::Factory);
#endif
