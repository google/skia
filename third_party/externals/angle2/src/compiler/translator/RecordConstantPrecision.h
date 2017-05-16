//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// During parsing, all constant expressions are folded to constant union nodes. The expressions that have been
// folded may have had precision qualifiers, which should affect the precision of the consuming operation.
// If the folded constant union nodes are written to output as such they won't have any precision qualifiers,
// and their effect on the precision of the consuming operation is lost.
//
// RecordConstantPrecision is an AST traverser that inspects the precision qualifiers of constants and hoists
// the constants outside the containing expression as precision qualified named variables in case that is
// required for correct precision propagation.
//

#ifndef COMPILER_TRANSLATOR_RECORDCONSTANTPRECISION_H_
#define COMPILER_TRANSLATOR_RECORDCONSTANTPRECISION_H_

class TIntermNode;

void RecordConstantPrecision(TIntermNode *root, unsigned int *temporaryIndex);

#endif   // COMPILER_TRANSLATOR_RECORDCONSTANTPRECISION_H_
