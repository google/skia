// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BRIDGE_H
#define BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

  bool   init_skunicode_impl(char* impl);
  void   cleanup_unicode_impl();
  void*  toUpper(char* str);
  void   print(void* str);
  double perf_compute_codeunit_flags(char* text);
  int    getFlags(int index);
  void*  getSentences(char* text, int* length);
  bool   trimSentence(char* text, int* sentence, int wordLimit);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // BRIDGE_H
