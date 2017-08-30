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
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#if !defined(__has_include)
    #define  __has_include(x) 0
#endif

static thread_local const char* tls_currently_running = "";

#if __has_include(<execinfo.h>)
    #include <execinfo.h>

    #define CAN_BACKTRACE
    static void backtrace(int fd) {
        void* stack[128];
        int frames = backtrace(stack, sizeof(stack)/sizeof(*stack));
        backtrace_symbols_fd(stack, frames, fd);
    }

#elif __has_include(<dlfcn.h>) && __has_include(<unwind.h>)
    #include <cxxabi.h>
    #include <dlfcn.h>
    #include <unwind.h>

    #define CAN_BACKTRACE
    static void backtrace(int fd) {
        FILE* file = fdopen(fd, "a");
        _Unwind_Backtrace([](_Unwind_Context* ctx, void* arg) {
            auto file = (FILE*)arg;
            if (auto ip = (void*)_Unwind_GetIP(ctx)) {
                const char* name = "[unknown]";
                void*       addr = nullptr;
                Dl_info info;
                if (dladdr(ip, &info) && info.dli_sname && info.dli_saddr) {
                    name = info.dli_sname;
                    addr = info.dli_saddr;
                }

                int ok;
                char* demangled = abi::__cxa_demangle(name, nullptr,0, &ok);
                if (ok == 0 && demangled) {
                    name = demangled;
                }

                fprintf(file, "\t%p %s+%zu\n", ip, name, (size_t)ip - (size_t)addr);
                free(demangled);
            }
            return _URC_NO_REASON;
        }, file);
        fflush(file);
    }
#endif

#if defined(CAN_BACKTRACE) && __has_include(<fcntl.h>) && __has_include(<signal.h>)
    #include <fcntl.h>
    #include <signal.h>

    // We'd ordinarily just use lockf(), but fcntl() is more portable to older Android NDK APIs.
    static void lock_or_unlock_fd(int fd, short type) {
        struct flock fl{};
        fl.l_type   = type;
        fl.l_whence = SEEK_CUR;
        fl.l_start  = 0;
        fl.l_len    = 0;  // 0 == the entire file
        fcntl(fd, F_SETLKW, &fl);
    }
    static void   lock_fd(int fd) { lock_or_unlock_fd(fd, F_WRLCK); }
    static void unlock_fd(int fd) { lock_or_unlock_fd(fd, F_UNLCK); }

    static int log_fd = 2/*stderr*/;

    static void log(const char* msg) {
        write(log_fd, msg, strlen(msg));
    }

    static void setup_crash_handler() {
        static void (*original_handlers[32])(int);
        for (int sig : std::vector<int>{ SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV }) {
            original_handlers[sig] = signal(sig, [](int sig) {
                lock_fd(log_fd);
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
                    backtrace(log_fd);
                unlock_fd(log_fd);

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
        lock_fd(log_fd);
            log("[");
            log(tls_currently_running);
            log("]\t");
            log(msg);
            log("\n");
        unlock_fd(log_fd);
    }

#else
    static void setup_crash_handler() {}
    static void defer_logging() {}

    void ok_log(const char* msg) {
        fprintf(stderr, "[%s]\t%s\n", tls_currently_running, msg);
    }
#endif

struct EngineType {
    const char *name, *help;
    std::unique_ptr<Engine> (*factory)(Options);
};
static std::vector<EngineType> engine_types;

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

    std::unique_ptr<Engine>                   engine;
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
        std::string engine_help = help_for(engine_types),
                    stream_help = help_for(stream_types),
                       dst_help = help_for(   dst_types),
                       via_help = help_for(   via_types);

        printf("%s [engine] src[:k=v,...] dst[:k=v,...] [via[:k=v,...] ...]          \n"
                " engine: how to execute tasks%s                                     \n"
                " src: content to draw%s                                             \n"
                " dst: how to draw that content%s                                    \n"
                " via: wrappers around dst%s                                         \n"
                " Most srcs, dsts and vias have options, e.g. skp:dir=skps sw:ct=565 \n",
                argv[0],
                engine_help.c_str(), stream_help.c_str(), dst_help.c_str(), via_help.c_str());
        return 1;
    };

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-h",     argv[i])) { return help(); }
        if (0 == strcmp("--help", argv[i])) { return help(); }

        for (auto e : engine_types) {
            size_t len = strlen(e.name);
            if (0 == strncmp(e.name, argv[i], len)) {
                switch (argv[i][len]) {
                    case  ':': len++;
                    case '\0': engine = e.factory(Options{argv[i]+len});
                }
            }
        }

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

    if (!engine) { engine = engine_types.back().factory(Options{}); }

    // If we know engine->spawn() will never crash, we can defer logging until we exit.
    if (engine->crashproof()) {
        defer_logging();
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

    std::list<std::future<Status>> live;
    const auto the_past = std::chrono::steady_clock::now();

    auto wait_one = [&] {
        if (live.empty()) {
            return Status::None;
        }

        for (;;) {
            for (auto it = live.begin(); it != live.end(); it++) {
                if (it->wait_until(the_past) != std::future_status::timeout) {
                    Status s = it->get();
                    live.erase(it);
                    return s;
                }
            }
        }
    };

    auto spawn = [&](std::function<Status(void)> fn) {
        std::future<Status> status;
        for (;;) {
            status = engine->spawn(fn);
            if (status.valid()) {
                break;
            }
            update_stats(wait_one());
        }
        live.push_back(std::move(status));
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
        s = wait_one();
        update_stats(s);
    }
    printf("\n");
    return (failed || crashed) ? 1 : 0;
}


Register::Register(const char* name, const char* help,
                   std::unique_ptr<Engine> (*factory)(Options)) {
    engine_types.push_back(EngineType{name, help, factory});
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
