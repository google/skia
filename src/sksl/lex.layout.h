/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

int layoutlex_init(yyscan_t* ptr_yy_globals);
int layoutlex_destroy(void* scanner);
YY_BUFFER_STATE layout_scan_string(const char* s, void* scanner);
int layoutlex(void* yyscanner);
void layout_delete_buffer(YY_BUFFER_STATE b, void* yyscanner);
