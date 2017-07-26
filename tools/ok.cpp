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
#include "SkImage.h"
#include "ok.h"
#include <chrono>
#include <future>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <vector>

#if !defined(__has_include)
    #define  __has_include(x) 0
#endif

static thread_local const char* tls_currently_running = "";

#if __has_include(<execinfo.h>) && __has_include(<fcntl.h>) && __has_include(<signal.h>)
    #include <execinfo.h>
    #include <fcntl.h>
    #include <signal.h>

    static int log_fd = 2/*stderr*/;

    static void log(const char* msg) {
        write(log_fd, msg, strlen(msg));
    }

    static void setup_crash_handler() {
        static void (*original_handlers[32])(int);
        for (int sig : std::vector<int>{ SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV }) {
            original_handlers[sig] = signal(sig, [](int sig) {
                lockf(log_fd, F_LOCK, 0);
                    log("\ncaught signal ");
                    switch (sig) {
                    #define CASE(s) case s: log(#s); break
                        CASE(SIGABRT);
                        CASE(SIGBUS);
                        CASE(SIGFPE);
                        CASE(SIGILL);
                        CASE(SIGSEGV);
                    #undef CASE
                    }
                    log(" while running '");
                    log(tls_currently_running);
                    log("'\n");

                    void* stack[128];
                    int frames = backtrace(stack, sizeof(stack)/sizeof(*stack));
                    backtrace_symbols_fd(stack, frames, log_fd);
                lockf(log_fd, F_ULOCK, 0);

                signal(sig, original_handlers[sig]);
                raise(sig);
            });
        }
    }

    static void defer_logging() {
        log_fd = fileno(tmpfile());
        atexit([] {
            lseek(log_fd, 0, SEEK_SET);
            char buf[1024];
            while (size_t bytes = read(log_fd, buf, sizeof(buf))) {
                write(2, buf, bytes);
            }
        });
    }

    void ok_log(const char* msg) {
        lockf(log_fd, F_LOCK, 0);
            log("[");
            log(tls_currently_running);
            log("]\t");
            log(msg);
            log("\n");
        lockf(log_fd, F_ULOCK, 0);
    }

#else
    static void setup_crash_handler() {}
    static void defer_logging() {}

    void ok_log(const char* msg) {
        fprintf(stderr, "%s\n", msg);
    }
#endif

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
    const std::chrono::steady_clock::time_point the_past = std::chrono::steady_clock::now();

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
                if (it->wait_until(the_past) == std::future_status::ready) {
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
                case  0: _exit((int)fn());
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

struct StreamType {
    const char *name, *help;
    std::unique_ptr<Stream> (*factory)(Options);
};
static std::vector<StreamType> stream_types;

struct DstType {
    const char *name, *help;
    std::unique_ptr<Dst> (*factory)(Options);
};
static std::vector<DstType> dst_types;

struct ViaType {
    const char *name, *help;
    std::unique_ptr<Dst> (*factory)(Options, std::unique_ptr<Dst>);
};
static std::vector<ViaType> via_types;

template <typename T>
static std::string help_for(std::vector<T> registered) {
    std::string help;
    for (auto r : registered) {
        help += "\n    ";
        help += r.name;
        help += ": ";
        help += r.help;
    }
    return help;
}

int main(int argc, char** argv) {
    SkGraphics::Init();
    setup_crash_handler();

    int                                       jobs{1};
    std::unique_ptr<Stream>                   stream;
    std::function<std::unique_ptr<Dst>(void)> dst_factory = []{
        // A default Dst that's enough for unit tests and not much else.
        struct : Dst {
            Status draw(Src* src)  override { return src->draw(nullptr); }
            sk_sp<SkImage> image() override { return nullptr; }
        } dst;
        return move_unique(dst);
    };

    auto help = [&] {
        std::string stream_help = help_for(stream_types),
                       dst_help = help_for(   dst_types),
                       via_help = help_for(   via_types);

        printf("%s [-j N] src[:k=v,...] dst[:k=v,...] [via[:k=v,...] ...]            \n"
                "  -j: Run at most N processes at any time.                          \n"
                "      If <0, use -N threads instead.                                \n"
                "      If 0, use one thread in one process.                          \n"
                "      If 1 (default) or -1, auto-detect N.                          \n"
                " src: content to draw%s                                             \n"
                " dst: how to draw that content%s                                    \n"
                " via: wrappers around dst%s                                         \n"
                " Most srcs, dsts and vias have options, e.g. skp:dir=skps sw:ct=565 \n",
                argv[0], stream_help.c_str(), dst_help.c_str(), via_help.c_str());
        return 1;
    };

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-j",     argv[i])) { jobs = atoi(argv[++i]); }
        if (0 == strcmp("-h",     argv[i])) { return help(); }
        if (0 == strcmp("--help", argv[i])) { return help(); }

        for (auto s : stream_types) {
            size_t len = strlen(s.name);
            if (0 == strncmp(s.name, argv[i], len)) {
                switch (argv[i][len]) {
                    case  ':': len++;
                    case '\0': stream = s.factory(Options{argv[i]+len});
                }
            }
        }
        for (auto d : dst_types) {
            size_t len = strlen(d.name);
            if (0 == strncmp(d.name, argv[i], len)) {
                switch (argv[i][len]) {
                    case  ':': len++;
                    case '\0': dst_factory = [=]{
                                   return d.factory(Options{argv[i]+len});
                               };
                }
            }
        }
        for (auto v : via_types) {
            size_t len = strlen(v.name);
            if (0 == strncmp(v.name, argv[i], len)) {
                if (!dst_factory) { return help(); }
                switch (argv[i][len]) {
                    case  ':': len++;
                    case '\0': dst_factory = [=]{
                                   return v.factory(Options{argv[i]+len}, dst_factory());
                               };
                }
            }
        }
    }
    if (!stream) { return help(); }

    std::unique_ptr<Engine> engine;
    if (jobs == 0) { engine.reset(new SerialEngine);                  }
    if (jobs  > 0) { engine.reset(new   ForkEngine); defer_logging(); }
    if (jobs  < 0) { engine.reset(new ThreadEngine); jobs = -jobs;    }

    if (jobs == 1) { jobs = std::thread::hardware_concurrency(); }

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

            std::string name = src->name();
            tls_currently_running = name.c_str();

            return dst_factory()->draw(src.get());
        });
    }

    for (Status s = Status::OK; s != Status::None; ) {
        s = engine->wait_one();
        update_stats(s);
    }
    printf("\n");
    return (failed || crashed) ? 1 : 0;
}


Register::Register(const char* name, const char* help,
                   std::unique_ptr<Stream> (*factory)(Options)) {
    stream_types.push_back(StreamType{name, help, factory});
}
Register::Register(const char* name, const char* help,
                   std::unique_ptr<Dst> (*factory)(Options)) {
    dst_types.push_back(DstType{name, help, factory});
}
Register::Register(const char* name, const char* help,
                   std::unique_ptr<Dst> (*factory)(Options, std::unique_ptr<Dst>)) {
    via_types.push_back(ViaType{name, help, factory});
}

Options::Options(std::string str) {
    std::string k,v, *curr = &k;
    for (auto c : str) {
        switch(c) {
            case ',': (*this)[k] = v;
                      curr = &(k = "");
                      break;
            case '=': curr = &(v = "");
                      break;
            default: *curr += c;
        }
    }
    (*this)[k] = v;
}

std::string& Options::operator[](std::string k) { return this->kv[k]; }

std::string Options::operator()(std::string k, std::string fallback) const {
    for (auto it = kv.find(k); it != kv.end(); ) {
        return it->second;
    }
    return fallback;
}
