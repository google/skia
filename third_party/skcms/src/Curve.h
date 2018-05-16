/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "../skcms.h"

// Evaluate an skcms_Curve at x.
float skcms_eval_curve(const skcms_Curve*, float x);
