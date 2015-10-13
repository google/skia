//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATEGLOBALINITIALIZER_H_
#define COMPILER_TRANSLATOR_VALIDATEGLOBALINITIALIZER_H_

class TIntermTyped;
class TParseContext;

// Returns true if the initializer is valid.
bool ValidateGlobalInitializer(TIntermTyped *initializer, const TParseContext *context, bool *warning);

#endif // COMPILER_TRANSLATOR_VALIDATEGLOBALINITIALIZER_H_
