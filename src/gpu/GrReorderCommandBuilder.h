/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrReorderCommandBuilder_DEFINED
#define GrReorderCommandBuilder_DEFINED

#include "GrCommandBuilder.h"

class GrReorderCommandBuilder : public GrCommandBuilder {
public:
    typedef GrCommandBuilder::Cmd Cmd;

    GrReorderCommandBuilder() : INHERITED() {}

    Cmd* recordDrawBatch(GrBatch*, const GrCaps&) override;

private:
    typedef GrCommandBuilder INHERITED;
};

#endif
