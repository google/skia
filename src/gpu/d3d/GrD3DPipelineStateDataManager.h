/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrD3DPipelineStateDataManager_DEFINED
#define GrD3DPipelineStateDataManager_DEFINED

#include "src/gpu/GrUniformDataManager.h"

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrSPIRVUniformHandler.h"

class GrD3DPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrSPIRVUniformHandler::UniformInfoArray UniformInfoArray;

    GrD3DPipelineStateDataManager(const UniformInfoArray&,
                                  uint32_t uniformSize);

    // TODO: upload to uniform buffer

private:
    typedef GrUniformDataManager INHERITED;
};

#endif
