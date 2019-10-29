// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkGlyphCacheBench_DEFINED
#define SkGlyphCacheBench_DEFINED

#include "bench/Benchmark.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"

#include <functional>
#include <memory>

Benchmark* CreateTextBlobTraceBench(SkString name,
                                    std::function<std::unique_ptr<SkStreamAsset>()> dataSrc,
                                    Benchmark::Backend backend);

#endif  // SkGlyphCacheBench_DEFINED
