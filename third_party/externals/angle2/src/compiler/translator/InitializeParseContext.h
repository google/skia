//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_INITIALIZEPARSECONTEXT_H_
#define COMPILER_TRANSLATOR_INITIALIZEPARSECONTEXT_H_

bool InitializeParseContextIndex();
void FreeParseContextIndex();

class TParseContext;
extern void SetGlobalParseContext(TParseContext* context);
extern TParseContext* GetGlobalParseContext();

#endif // COMPILER_TRANSLATOR_INITIALIZEPARSECONTEXT_H_
