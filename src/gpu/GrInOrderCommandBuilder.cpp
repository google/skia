/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInOrderCommandBuilder.h"

#include "GrBufferedDrawTarget.h"

#include "GrColor.h"
#include "SkPoint.h"

GrTargetCommands::Cmd* GrInOrderCommandBuilder::recordDrawBatch(GrBatch* batch,
                                                                const GrCaps& caps) {
    GrBATCH_INFO("In-Recording (%s, %u)\n", batch->name(), batch->uniqueID());
    if (!this->cmdBuffer()->empty() &&
        Cmd::kDrawBatch_CmdType == this->cmdBuffer()->back().type()) {
        DrawBatch* previous = static_cast<DrawBatch*>(&this->cmdBuffer()->back());
        if (previous->batch()->combineIfPossible(batch, caps)) {
            GrBATCH_INFO("\tBatching with (%s, %u)\n",
                         previous->batch()->name(), previous->batch()->uniqueID());
            return nullptr;
        }
    }

    return GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), DrawBatch, (batch));
}
