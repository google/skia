/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdbool.h>

bool skcms_GetCHAD(const skcms_ICCProfile* profile, skcms_Matrix3x3* m);
