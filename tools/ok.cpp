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

static long fib(int n) {
    return (n < 2) ? n : fib(n-1) + fib(n-2);
}

static int kid(int i) {
    long f = fib(i);
    if (f % 3) {
        return 1;
    }
    if (f % 2) {
        __builtin_trap();
    }
    printf("fib(%d) = %ld\n", i, f);
    return 0;
}

int main(int argc, char** argv) {
    int jobs = argc > 1 ? atoi(argv[1])
                        : std::thread::hardware_concurrency();

    int good = 0, bad = 0, ugly = 0;

    auto update_stats = [&](int status) {
        if (WIFEXITED(status)) {
            (WEXITSTATUS(status) == 0 ? good : bad)++;
        } else {
            ugly++;
        }
    };

    auto safe_wait = [&] {
        do {
            int status;
            pid_t pid = wait(&status);
            if (pid > 0) {
                update_stats(status);
                return true;
            }
        } while (errno == EINTR);
        return false;
    };

    for (int i = 0; i < 45; i++) {
        if (fork()) {
            if (--jobs < 0) {
                safe_wait();
            }
            continue;
        }

        exit(kid(i));
    }

    while (safe_wait());
    printf("%d good, %d bad, %d ugly\n", good, bad, ugly);

    return 0;
}
