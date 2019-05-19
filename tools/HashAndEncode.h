// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "tools/flags/CommandLineFlags.h"

class HashAndEncode {
public:
    explicit HashAndEncode(const SkBitmap&);

    void write(SkWStream*) const;

    bool writePngTo(const char* path,
                    const char* md5,
                    CommandLineFlags::StringArray key,
                    CommandLineFlags::StringArray properties) const;

private:
    const SkISize               fSize;
    std::unique_ptr<uint64_t[]> fPixels;
};

