/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageGenerator.h"

std::unique_ptr<SkImageGenerator> SkImageGenerator::MakeFromEncodedImpl(sk_sp<SkData>) {
    return nullptr;
}
