/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ok_DEFINED
#define ok_DEFINED

#include "SkCanvas.h"
#include <functional>
#include <map>
#include <memory>
#include <string>

// Not really ok-specific, but just kind of generally handy.
template <typename T>
static std::unique_ptr<T> move_unique(T& v) {
    return std::unique_ptr<T>{new T{std::move(v)}};
}

struct Src {
    virtual ~Src() {}
    virtual std::string name()   = 0;  // ok always calls Src methods in order:
    virtual SkISize     size()   = 0;  // name() -> size() -> draw(), possibly
    virtual void draw(SkCanvas*) = 0;  // stopping after calling name().
};

struct Stream {
    virtual ~Stream() {}
    virtual std::unique_ptr<Src> next() = 0;
};

struct Dst {
    virtual ~Dst() {}
    virtual SkCanvas* canvas()                  = 0;
    virtual void write(std::string path_prefix) = 0;  // All but the file extension.
};

class Options {
    std::map<std::string, std::string> kv;
public:
    explicit Options(std::string str = "");
    std::string operator()(std::string k, std::string fallback = "") const;
};

// Create globals to register your new type of Stream or Dst.
struct Register {
    Register(const char* name,
             std::unique_ptr<Stream> (*factory)(Options));
    Register(const char* name,
             std::unique_ptr<Dst> (*factory)(Options, SkISize));
    Register(const char* name,
             std::unique_ptr<Dst> (*factory)(Options, SkISize, std::unique_ptr<Dst>));
};

#endif//ok_DEFINED
