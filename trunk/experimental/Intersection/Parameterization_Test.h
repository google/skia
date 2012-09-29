/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DataTypes.h"

// utilities used only for unit testing
bool point_on_parameterized_curve(const Cubic& cubic, const _Point& point);
bool point_on_parameterized_line(const _Line& line, const _Point& point);
bool point_on_parameterized_curve(const Quadratic& quad, const _Point& point);
