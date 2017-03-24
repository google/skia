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
#include "SkPicture.h"
#include "SkSurface.h"
#include "gm.h"
#include <chrono>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <regex>
#include <stdint.h>
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

struct Src {
    virtual ~Src() {}
    virtual std::string name() = 0;
    virtual SkISize     size() = 0;
    virtual void draw(SkCanvas*) = 0;
};

struct Stream {
    virtual ~Stream() {}
    virtual std::unique_ptr<Src> next() = 0;
};

struct Options {
    std::map<std::string, std::string> kv;

    explicit Options(std::string str = "") {
        std::string k,v, *curr = &k;
        for (auto c : str) {
            switch(c) {
                case ',': kv[k] = v;
                          curr = &(k = "");
                          break;
                case '=': curr = &(v = "");
                          break;
                default: *curr += c;
            }
        }
        kv[k] = v;
    }

    std::string lookup(std::string k, std::string fallback = "") {
        for (auto it = kv.find(k); it != kv.end(); ) {
            return it->second;
        }
        return fallback;
    }
};

template <typename T>
static std::unique_ptr<T> move_unique(T& v) {
    return std::unique_ptr<T>{new T{std::move(v)}};
}

struct GMStream : Stream {
    const skiagm::GMRegistry* registry = skiagm::GMRegistry::Head();

    static std::unique_ptr<Stream> Create(Options) {
        GMStream stream;
        return move_unique(stream);
    }

    struct GMSrc : Src {
        skiagm::GM* (*factory)(void*);
        std::unique_ptr<skiagm::GM> gm;

        std::string name() override {
            gm.reset(factory(nullptr));
            return gm->getName();
        }

        SkISize size() override {
            return gm->getISize();
        }

        void draw(SkCanvas* canvas) override {
            canvas->clear(0xffffffff);
            canvas->concat(gm->getInitialTransform());
            gm->draw(canvas);
        }
    };

    std::unique_ptr<Src> next() override {
        if (!registry) {
            return nullptr;
        }
        GMSrc src;
        src.factory = registry->factory();
        registry = registry->next();
        return move_unique(src);
    }
};

struct SKPStream : Stream {
    std::string dir;
    std::vector<std::string> skps;

    static std::unique_ptr<Stream> Create(Options options) {
        SKPStream stream;
        stream.dir = options.lookup("dir", "skps");
        SkOSFile::Iter it{stream.dir.c_str(), ".skp"};
        for (SkString path; it.next(&path); ) {
            stream.skps.push_back(path.c_str());
        }
        return move_unique(stream);
    }

    struct SKPSrc : Src {
        std::string dir, path;
        sk_sp<SkPicture> pic;

        std::string name() override {
            return path;
        }

        SkISize size() override {
            auto skp = SkData::MakeFromFileName((dir+"/"+path).c_str());
            pic = SkPicture::MakeFromData(skp.get());
            return pic->cullRect().roundOut().size();
        }

        void draw(SkCanvas* canvas) override {
            canvas->clear(0xffffffff);
            pic->playback(canvas);
        }
    };

    std::unique_ptr<Src> next() override {
        if (skps.empty()) {
            return nullptr;
        }
        SKPSrc src;
        src.dir  = dir;
        src.path = skps.back();
        skps.pop_back();
        return move_unique(src);
    }
};

struct Dst {
    virtual ~Dst() {}
    virtual SkCanvas* canvas() = 0;
    virtual void write(std::string path_prefix) = 0;
};

struct SWDst : Dst {
    sk_sp<SkSurface> surface;

    static std::unique_ptr<Dst> Create(SkISize size, Options options) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(size.width(), size.height());
        if (options.lookup("ct") == "565") { info = info.makeColorType(kRGB_565_SkColorType); }
        if (options.lookup("ct") == "f16") { info = info.makeColorType(kRGBA_F16_SkColorType); }
        SWDst dst;
        dst.surface = SkSurface::MakeRaster(info);
        return move_unique(dst);
    }

    SkCanvas* canvas() override {
        return surface->getCanvas();
    }

    void write(std::string path_prefix) override {
        auto image = surface->makeImageSnapshot();
        sk_sp<SkData> png{image->encode()};
        SkFILEWStream{(path_prefix + ".png").c_str()}.write(png->data(), png->size());
    }
};

struct {
    const char* name;
    std::unique_ptr<Stream> (*factory)(Options);
} streams[] = {
    {"gm",  GMStream::Create },
    {"skp", SKPStream::Create },
};

struct {
    const char* name;
    std::unique_ptr<Dst> (*factory)(SkISize, Options);
} dsts[] = {
    {"sw",  SWDst::Create },
};


int main(int argc, char** argv) {
    SkGraphics::Init();
    setup_crash_handler();

    int         jobs        {1};
    std::regex  match       {".*"};
    std::regex  search      {".*"};
    std::string write_dir   {""};

    std::unique_ptr<Stream> stream;
    std::unique_ptr<Dst> (*dst_factory)(SkISize, Options) = nullptr;
    Options dst_options;

    auto help = [&] {
        std::string stream_types, dst_types;
        for (auto s : streams) {
            if (!stream_types.empty()) {
                stream_types += ", ";
            }
            stream_types += s.name;
        }
        for (auto d : dsts) {
            if (!dst_types.empty()) {
                dst_types += ", ";
            }
            dst_types += d.name;
        }

        printf("%s [-j N] [-m regex] [-s regex] [-w dir] [-h] src[:k=v,...] dst[:k=v,...] \n"
                "  -j: Run at most N processes at any time.                               \n"
                "      If <0, use -N threads instead.                                     \n"
                "      If 0, use one thread in one process.                               \n"
                "      If 1 (default) or -1, auto-detect N.                               \n"
                "  -m: Run only names matching regex exactly.                             \n"
                "  -s: Run only names matching regex anywhere.                            \n"
                "  -w: If set, write .pngs into dir.                                      \n"
                "  -h: Print this message and exit.                                       \n"
                " src: content to draw: %s                                                \n"
                " dst: how to draw that content: %s                                       \n"
                " Some srcs and dsts have options, e.g. skp:dir=skps sw:ct=565            \n",
                argv[0], stream_types.c_str(), dst_types.c_str());
        return 1;
    };

    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-j", argv[i])) { jobs      = atoi(argv[++i]); }
        if (0 == strcmp("-m", argv[i])) { match     =      argv[++i] ; }
        if (0 == strcmp("-s", argv[i])) { search    =      argv[++i] ; }
        if (0 == strcmp("-w", argv[i])) { write_dir =      argv[++i] ; }
        if (0 == strcmp("-h", argv[i])) { return help(); }

        for (auto s : streams) {
            size_t len = strlen(s.name);
            if (0 == strncmp(s.name, argv[i], len)) {
                switch (argv[i][len]) {
                    case  ':': len++;
                    case '\0': stream = s.factory(Options{argv[i]+len});
                }
            }
        }
        for (auto d : dsts) {
            size_t len = strlen(d.name);
            if (0 == strncmp(d.name, argv[i], len)) {
                switch (argv[i][len]) {
                    case  ':': len++;
                    case '\0': dst_factory = d.factory;
                               dst_options = Options{argv[i]+len};
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

            auto dst = dst_factory(src->size(), dst_options);

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
