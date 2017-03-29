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

void ok_log(const char*);

enum class Status { OK, Failed, Crashed, Skipped, None };

struct Src {
    virtual ~Src() {}
    virtual std::string name()     = 0;
    virtual SkISize     size()     = 0;
    virtual Status draw(SkCanvas*) = 0;
};

struct Stream {
    virtual ~Stream() {}
    virtual std::unique_ptr<Src> next() = 0;
};

struct Dst {
    virtual ~Dst() {}
    virtual Status draw(Src*)      = 0;
    virtual sk_sp<SkImage> image() = 0;
};

class Options {
    std::map<std::string, std::string> kv;
public:
    explicit Options(std::string = "");
    std::string& operator[](std::string k);
    std::string  operator()(std::string k, std::string fallback = "") const;
};

// Create globals to register your new type of Stream or Dst.
struct Register {
    Register(const char* name, const char* help, std::unique_ptr<Stream> (*factory)(Options));
    Register(const char* name, const char* help, std::unique_ptr<Dst>    (*factory)(Options));
    Register(const char* name, const char* help,
             std::unique_ptr<Dst>(*factory)(Options, std::unique_ptr<Dst>));
};

#endif//ok_DEFINED
