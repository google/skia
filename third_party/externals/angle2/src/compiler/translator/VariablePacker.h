//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VARIABLEPACKER_H_
#define COMPILER_TRANSLATOR_VARIABLEPACKER_H_

#include <vector>
#include "compiler/translator/VariableInfo.h"

class VariablePacker {
 public:
    // Returns true if the passed in variables pack in maxVectors following
    // the packing rules from the GLSL 1.017 spec, Appendix A, section 7.
    template <typename VarT>
    bool CheckVariablesWithinPackingLimits(unsigned int maxVectors,
                                           const std::vector<VarT> &in_variables);

    // Gets how many components in a row a data type takes.
    static int GetNumComponentsPerRow(sh::GLenum type);

    // Gets how many rows a data type takes.
    static int GetNumRows(sh::GLenum type);

  private:
    static const int kNumColumns = 4;
    static const unsigned kColumnMask = (1 << kNumColumns) - 1;

    unsigned makeColumnFlags(int column, int numComponentsPerRow);
    void fillColumns(int topRow, int numRows, int column, int numComponentsPerRow);
    bool searchColumn(int column, int numRows, int* destRow, int* destSize);

    int topNonFullRow_;
    int bottomNonFullRow_;
    int maxRows_;
    std::vector<unsigned> rows_;
};

#endif // COMPILER_TRANSLATOR_VARIABLEPACKER_H_
