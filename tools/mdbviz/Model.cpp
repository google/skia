/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>

#include "Model.h"

#include "DebugCanvas.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkStream.h"

Model::Model() : fCurOp(0) {
    SkImageInfo ii = SkImageInfo::MakeN32Premul(1024, 1024);
    fBM.allocPixels(ii, 0);
}

Model::~Model() {
    this->resetOpList();
}

Model::ErrorCode Model::load(const char* filename) {
    std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(filename);
    if (!stream) {
        return ErrorCode::kCouldntOpenFile;
    }
    sk_sp<SkPicture> pic(SkPicture::MakeFromStream(stream.get()));
    if (!pic) {
        return ErrorCode::kCouldntDecodeSKP;
    }

    {
        std::unique_ptr<DebugCanvas> temp(
                new DebugCanvas(SkScalarCeilToInt(pic->cullRect().width()),
                                SkScalarCeilToInt(pic->cullRect().height())));

        temp->setPicture(pic.get());
        pic->playback(temp.get());
        temp->setPicture(nullptr);
        this->resetOpList();
        temp->detachCommands(&fOps);
    }

    this->setCurOp(fOps.count()-1);

    return ErrorCode::kOK;
}

const char* Model::ErrorString(ErrorCode err) {
    static const char* kStrings[] = {
        "OK",
        "Couldn't read file",
        "Couldn't decode picture"
    };

    return kStrings[(int)err];
}

const char* Model::getOpName(int index) const {
    return DrawCommand::GetCommandString(fOps[index]->getType());
}

bool Model::isHierarchyPush(int index) const {
    DrawCommand::OpType type = fOps[index]->getType();

    return DrawCommand::kSave_OpType == type || DrawCommand::kSaveLayer_OpType == type ||
           DrawCommand::kBeginDrawPicture_OpType == type;
}

bool Model::isHierarchyPop(int index) const {
    DrawCommand::OpType type = fOps[index]->getType();

    return DrawCommand::kRestore_OpType == type || DrawCommand::kEndDrawPicture_OpType == type;
}

void Model::setCurOp(int curOp) {
    SkASSERT(curOp < fOps.count());

    if (curOp == fCurOp) {
        return; // the render state is already up to date
    }

    fCurOp = curOp;
    this->drawTo(fCurOp);
}

void Model::drawTo(int index) {
    SkASSERT(index < fOps.count());

    SkCanvas canvas(fBM);

    int saveCount = canvas.save();

    for (int i = 0; i <= index; ++i) {
        if (fOps[i]->isVisible()) {
            fOps[i]->execute(&canvas);
        }
    }

    canvas.restoreToCount(saveCount);
}

void Model::resetOpList() {
    for (int i = 0; i < fOps.count(); ++i) {
        delete fOps[i];
    }
    fOps.reset();
    fCurOp = 0;
}
