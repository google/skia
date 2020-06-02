/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRive_Streamreader_DEFINED
#define SkRive_Streamreader_DEFINED

#include <memory>

#include "include/core/SkRefCnt.h"

class SkData;
class SkStream;

namespace skrive::internal {

class StreamReader {
public:
    virtual ~StreamReader() = default;

    static std::unique_ptr<StreamReader> Make(sk_sp<SkData>);
    static std::unique_ptr<StreamReader> Make(std::unique_ptr<SkStream>);
    static std::unique_ptr<StreamReader> Make(const char[], size_t);

    enum class BlockType : uint8_t {
        kUnknown       = 0x00,
        kComponents    = 0x01,
        kActorNode     = 0x02,
        kActorBone     = 0x03,
        kActorRootBone = 0x04,

        // End-of-block marker
        kEoB           = 0xff,
    };

    virtual BlockType openBlock() = 0;
    virtual void     closeBlock() = 0;
};

}

#endif // SkRiveStreamreader_DEFINED
