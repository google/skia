//
// Copyright (c) 2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_GLSLANG_H_
#define COMPILER_TRANSLATOR_GLSLANG_H_

class TParseContext;
extern int glslang_initialize(TParseContext* context);
extern int glslang_finalize(TParseContext* context);

extern int glslang_scan(size_t count,
                        const char* const string[],
                        const int length[],
                        TParseContext* context);
extern int glslang_parse(TParseContext* context);

#endif // COMPILER_TRANSLATOR_GLSLANG_H_
