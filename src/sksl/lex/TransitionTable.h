/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TRANSITIONTABLE
#define SKSL_TRANSITIONTABLE

#include <fstream>

struct DFA;

void WriteTransitionTable(std::ofstream& out, const DFA& dfa, size_t states);

#endif // SKSL_TRANSITIONTABLE
