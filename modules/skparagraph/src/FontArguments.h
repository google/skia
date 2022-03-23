// Copyright 2019 Google LLC.
#ifndef FontArguments_DEFINED
#define FontArguments_DEFINED

#include <functional>
#include "include/core/SkFontArguments.h"

bool operator==(const SkFontArguments& a, const SkFontArguments& b);
bool operator!=(const SkFontArguments& a, const SkFontArguments& b);

namespace std {
    template<> struct hash<SkFontArguments> {
        size_t operator()(const SkFontArguments& args) const;
    };
}

#endif  // FontArguments_DEFINED
