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
#include "ok.h"
#include <chrono>
#include <future>
#include <list>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#if !defined(__has_include)
    #define  __has_include(x) 0
#endif

static thread_local const char* tls_name = "";

#if __has_include(<execinfo.h>) && __has_include(<fcntl.h>) && __has_include(<signal.h>)
    #include <execinfo.h>
    #include <fcntl.h>
    #include <signal.h>

    static int crash_stacktrace_fd = 2/*stderr*/;

    static void setup_crash_handler() {
        static void (*original_handlers[32])(int);

        for (int sig : std::vector<int>{ SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV }) {
            original_handlers[sig] = signal(sig, [](int sig) {
                auto ez_write = [](const char* str) {
                    write(crash_stacktrace_fd, str, strlen(str));
                };
                ez_write("\ncaught signal ");
                switch (sig) {
                #define CASE(s) case s: ez_write(#s); break
                    CASE(SIGABRT);
                    CASE(SIGBUS);
                    CASE(SIGFPE);
                    CASE(SIGILL);
                    CASE(SIGSEGV);
                #undef CASE
                }
                ez_write(" while running '");
                ez_write(tls_name);
                ez_write("'\n");

                void* stack[128];
                int frames = backtrace(stack, sizeof(stack)/sizeof(*stack));
                backtrace_symbols_fd(stack, frames, crash_stacktrace_fd);
                signal(sig, original_handlers[sig]);
                raise(sig);
            });
        }
    }

    static void defer_crash_stacktraces() {
        crash_stacktrace_fd = fileno(tmpfile());
        atexit([] {
            lseek(crash_stacktrace_fd, 0, SEEK_SET);
            char buf[1024];
            while (size_t bytes = read(crash_stacktrace_fd, buf, sizeof(buf))) {
                write(2, buf, bytes);
            }
        });
    }
#else
    static void setup_crash_handler() {}
    static void defer_crash_stacktraces() {}
#endif

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
    const char* name;
    std::unique_ptr<Stream> (*factory)(Options);
};
static std::vector<StreamType> stream_types;

struct DstType {
    const char* name;
    std::unique_ptr<Dst> (*factory)(Options, SkISize);
};
static std::vector<DstType> dst_types;

struct ViaType {
    const char* name;
    std::unique_ptr<Dst> (*factory)(Options, SkISize, std::unique_ptr<Dst>);
};
static std::vector<ViaType> via_types;

int main(int argc, char** argv) {
    SkGraphics::Init();
    setup_crash_handler();

    int         jobs        {1};
    std::regex  match       {".*"};
    std::regex  search      {".*"};
    std::string write_dir   {""};

    std::unique_ptr<Stream>                                             stream;
    std::function<std::unique_ptr<Dst> (SkISize)>                       dst_factory;
    std::function<std::unique_ptr<Dst> (SkISize, std::unique_ptr<Dst>)> via_factory;

    auto help = [&] {
        std::string stream_names, dst_names, via_names;
        for (auto s : stream_types) {
            if (!stream_names.empty()) {
                stream_names += ", ";
            }
            stream_names += s.name;
        }
        for (auto d : dst_types) {
            if (!dst_names.empty()) {
                dst_names += ", ";
            }
            dst_names += d.name;
        }
        for (auto v : via_types) {
            if (!via_names.empty()) {
                via_names += ", ";
            }
            via_names += v.name;
        }

        printf("%s [-j N] [-m regex] [-s regex] [-w dir] [-h]                        \n"
                "  src[:k=v,...] dst[:k=v,...] [via[:k=v,...]]                       \n"
                "  -j: Run at most N processes at any time.                          \n"
                "      If <0, use -N threads instead.                                \n"
                "      If 0, use one thread in one process.                          \n"
                "      If 1 (default) or -1, auto-detect N.                          \n"
                "  -m: Run only names matching regex exactly.                        \n"
                "  -s: Run only names matching regex anywhere.                       \n"
                "  -w: If set, write .pngs into dir.                                 \n"
                "  -h: Print this message and exit.                                  \n"
                " src: content to draw: %s                                           \n"
                " dst: how to draw that content: %s                                  \n"
                " via: front-patch the dst: %s                                       \n"
                " Some srcs, dsts and vias have options, e.g. skp:dir=skps sw:ct=565 \n",
                argv[0], stream_names.c_str(), dst_names.c_str(), via_names.c_str());
        return 1;
    };

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-j", argv[i])) { jobs      = atoi(argv[++i]); }
        if (0 == strcmp("-m", argv[i])) { match     =      argv[++i] ; }
        if (0 == strcmp("-s", argv[i])) { search    =      argv[++i] ; }
        if (0 == strcmp("-w", argv[i])) { write_dir =      argv[++i] ; }
        if (0 == strcmp("-h", argv[i])) { return help(); }

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
                    case '\0': dst_factory = [=](SkISize size){
                                   return d.factory(Options{argv[i]+len}, size);
                               };
                }
            }
        }
        for (auto v : via_types) {
            size_t len = strlen(v.name);
            if (0 == strncmp(v.name, argv[i], len)) {
                switch (argv[i][len]) {
                    case  ':': len++;
                    case '\0': via_factory = [=](SkISize size, std::unique_ptr<Dst> dst) {
                                   return v.factory(Options{argv[i]+len}, size, std::move(dst));
                               };
                }
            }
        }
    }
    if (!stream || !dst_factory) { return help(); }

    std::unique_ptr<Engine> engine;
    if (jobs == 0) { engine.reset(new SerialEngine);                            }
    if (jobs  > 0) { engine.reset(new   ForkEngine); defer_crash_stacktraces(); }
    if (jobs  < 0) { engine.reset(new ThreadEngine); jobs = -jobs;              }

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
            tls_name = name.c_str();
            if (!std::regex_match (name, match) ||
                !std::regex_search(name, search)) {
                return Status::Skipped;
            }

            auto size = src->size();
            auto dst = dst_factory(size);
            if (via_factory) {
                dst = via_factory(size, std::move(dst));
            }

            auto canvas = dst->canvas();
            src->draw(canvas);
            canvas->restoreToCount(0);

            if (!write_dir.empty()) {
                dst->write(write_dir + "/" + name);
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


Register::Register(const char* name,
                   std::unique_ptr<Stream> (*factory)(Options)) {
    stream_types.push_back(StreamType{name, factory});
}
Register::Register(const char* name,
                   std::unique_ptr<Dst> (*factory)(Options, SkISize)) {
    dst_types.push_back(DstType{name, factory});
}
Register::Register(const char* name,
                   std::unique_ptr<Dst> (*factory)(Options, SkISize, std::unique_ptr<Dst>)) {
    via_types.push_back(ViaType{name, factory});
}

Options::Options(std::string str) {
    std::string k,v, *curr = &k;
    for (auto c : str) {
        switch(c) {
            case ',': this->kv[k] = v;
                      curr = &(k = "");
                      break;
            case '=': curr = &(v = "");
                      break;
            default: *curr += c;
        }
    }
    this->kv[k] = v;
}

std::string Options::operator()(std::string k, std::string fallback) const {
    for (auto it = kv.find(k); it != kv.end(); ) {
        return it->second;
    }
    return fallback;
}
