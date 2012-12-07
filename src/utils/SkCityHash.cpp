/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * Pass any calls through to the CityHash library.
 * This is the only source file that accesses the CityHash code directly.
 */

#include "SkCityHash.h"
#include "SkTypes.h"
#include "city.h"

uint32_t SkCityHash::Compute32(const char *data, size_t size) {
    return CityHash32(data, size);
}

uint64_t SkCityHash::Compute64(const char *data, size_t size) {
    return CityHash64(data, size);
}
