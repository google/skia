/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSON_DEFINED
#define SkJSON_DEFINED

#include "SkArenaAlloc.h"
#include "SkTypes.h"

namespace skjson {

class Value {
public:
    template <typename T>
    bool to(T*) const;

    template <typename T>
    T toDefault(const T& defaultValue) const {
        T v;
        if (!this->to<T>(&v)) {
            v = defaultValue;
        }
        return v;
    }

    struct Rec;

protected:
    const Rec* fRec;
};

class Dom final : public SkNoncopyable {
public:
    Dom();
    Dom(const char*, size_t);

private:
    SkArenaAlloc fAlloc;
};

bool Parse(const char*, size_t);

} // namespace skjson

#endif // SkJSON_DEFINED

