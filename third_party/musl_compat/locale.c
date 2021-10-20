/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Fixes linking issues with musl due to missing locale-specific string functions.
 */

#include <stdlib.h>

long long strtoll_l(const char *nptr, char **endptr, int base,
                    int locale_t) {
  return strtoll(nptr, endptr, base);
}

long long strtoull_l(const char *nptr, char **endptr, int base,
                     int locale_t) {
  return strtoull(nptr, endptr, base);
}
